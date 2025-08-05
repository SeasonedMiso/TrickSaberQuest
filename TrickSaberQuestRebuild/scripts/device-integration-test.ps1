#!/usr/bin/env pwsh

param(
    [Parameter(Mandatory=$false)]
    [Switch] $RealTime,
    [Parameter(Mandatory=$false)]
    [Int] $Duration = 30
)

$ErrorActionPreference = "Stop"

Write-Host "TrickSaber Quest Device Integration Tests" -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# Check if Quest is connected
Write-Host "Checking Quest connection..." -ForegroundColor Yellow
$devices = & adb devices
if ($devices -notmatch "device$") {
    Write-Host "ERROR: No Quest device connected" -ForegroundColor Red
    Write-Host "Please connect your Quest and enable USB debugging" -ForegroundColor Yellow
    exit 1
}
Write-Host "Quest device connected ✓" -ForegroundColor Green

# Check if Beat Saber is installed
Write-Host "Checking Beat Saber installation..." -ForegroundColor Yellow
$packages = & adb shell pm list packages | Select-String "com.beatgames.beatsaber"
if (-not $packages) {
    Write-Host "ERROR: Beat Saber not found on device" -ForegroundColor Red
    exit 1
}
Write-Host "Beat Saber installed ✓" -ForegroundColor Green

# Build and copy mod
Write-Host "Building and deploying mod..." -ForegroundColor Yellow
& ./copy.ps1
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Failed to build and deploy mod" -ForegroundColor Red
    exit 1
}
Write-Host "Mod deployed ✓" -ForegroundColor Green

# Start Beat Saber
Write-Host "Starting Beat Saber..." -ForegroundColor Yellow
& adb shell am start -n com.beatgames.beatsaber/com.unity3d.player.UnityPlayerActivity
Start-Sleep -Seconds 5

# Monitor mod loading
Write-Host "Monitoring mod initialization..." -ForegroundColor Yellow
$logJob = Start-Job -ScriptBlock {
    param($duration)
    $timeout = (Get-Date).AddSeconds($duration)
    $modLoaded = $false
    $tricksDetected = $false
    
    while ((Get-Date) -lt $timeout) {
        $logs = & adb logcat -d | Select-String "TrickSaber"
        
        if ($logs -match "TrickSaber.*loaded" -and -not $modLoaded) {
            Write-Host "✓ Mod loaded successfully" -ForegroundColor Green
            $modLoaded = $true
        }
        
        if ($logs -match "Trick.*activated" -and -not $tricksDetected) {
            Write-Host "✓ Trick system active" -ForegroundColor Green
            $tricksDetected = $true
        }
        
        if ($modLoaded -and $tricksDetected) {
            Write-Host "✓ All systems operational" -ForegroundColor Green
            break
        }
        
        Start-Sleep -Seconds 1
    }
    
    return @{
        ModLoaded = $modLoaded
        TricksDetected = $tricksDetected
    }
} -ArgumentList $Duration

# Real-time monitoring if requested
if ($RealTime.IsPresent) {
    Write-Host "Starting real-time monitoring (Ctrl+C to stop)..." -ForegroundColor Yellow
    Write-Host "Perform tricks on your Quest to see live feedback" -ForegroundColor Cyan
    
    try {
        & adb logcat | Select-String "TrickSaber|Performance|FPS" | ForEach-Object {
            $timestamp = Get-Date -Format "HH:mm:ss"
            if ($_ -match "ERROR") {
                Write-Host "[$timestamp] $_" -ForegroundColor Red
            } elseif ($_ -match "Performance|FPS") {
                Write-Host "[$timestamp] $_" -ForegroundColor Blue
            } else {
                Write-Host "[$timestamp] $_" -ForegroundColor White
            }
        }
    } catch {
        Write-Host "Monitoring stopped" -ForegroundColor Yellow
    }
} else {
    # Wait for monitoring job
    $result = Receive-Job -Job $logJob -Wait
    Remove-Job -Job $logJob
    
    # Test results
    Write-Host ""
    Write-Host "Integration Test Results:" -ForegroundColor Cyan
    Write-Host "========================" -ForegroundColor Cyan
    
    if ($result.ModLoaded) {
        Write-Host "✓ Mod Loading: PASSED" -ForegroundColor Green
    } else {
        Write-Host "✗ Mod Loading: FAILED" -ForegroundColor Red
    }
    
    if ($result.TricksDetected) {
        Write-Host "✓ Trick System: PASSED" -ForegroundColor Green
    } else {
        Write-Host "✗ Trick System: FAILED" -ForegroundColor Red
    }
    
    # Performance check
    Write-Host "Checking performance metrics..." -ForegroundColor Yellow
    $perfLogs = & adb logcat -d | Select-String "FPS|Performance"
    if ($perfLogs) {
        Write-Host "✓ Performance Monitoring: ACTIVE" -ForegroundColor Green
        $perfLogs | Select-Object -Last 3 | ForEach-Object {
            Write-Host "  $_" -ForegroundColor Blue
        }
    } else {
        Write-Host "⚠ Performance Monitoring: NO DATA" -ForegroundColor Yellow
    }
    
    # Overall result
    $overallSuccess = $result.ModLoaded -and $result.TricksDetected
    if ($overallSuccess) {
        Write-Host ""
        Write-Host "OVERALL RESULT: PASSED ✓" -ForegroundColor Green
        Write-Host "TrickSaber is working correctly on your Quest!" -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "OVERALL RESULT: FAILED ✗" -ForegroundColor Red
        Write-Host "Check the logs above for issues" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host ""
Write-Host "Device integration test completed" -ForegroundColor Cyan