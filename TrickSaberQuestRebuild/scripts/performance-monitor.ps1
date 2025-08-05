#!/usr/bin/env pwsh

param(
    [Parameter(Mandatory=$false)]
    [int]$Duration = 60,  # Monitor for 60 seconds by default
    
    [Parameter(Mandatory=$false)]
    [string]$OutputFile = "performance_report.txt",
    
    [Parameter(Mandatory=$false)]
    [switch]$RealTime = $false,
    
    [Parameter(Mandatory=$false)]
    [switch]$Detailed = $false
)

Write-Host "TrickSaber Performance Monitor" -ForegroundColor Green
Write-Host "Duration: $Duration seconds" -ForegroundColor Yellow
Write-Host "Output: $OutputFile" -ForegroundColor Yellow

# Check if device is connected
$devices = adb devices
if ($devices -notmatch "device$") {
    Write-Error "No Quest device connected. Please connect your Quest and enable USB debugging."
    exit 1
}

# Start monitoring
$startTime = Get-Date
$endTime = $startTime.AddSeconds($Duration)

Write-Host "Starting performance monitoring..." -ForegroundColor Green
Write-Host "Press Ctrl+C to stop early" -ForegroundColor Yellow

# Create output file with header
$header = @"
TrickSaber Quest Performance Report
Generated: $(Get-Date)
Duration: $Duration seconds
Device: $(adb shell getprop ro.product.model)
Beat Saber Version: $(adb shell dumpsys package com.beatgames.beatsaber | Select-String "versionName" | Select-Object -First 1)

=== PERFORMANCE METRICS ===

"@

$header | Out-File -FilePath $OutputFile -Encoding UTF8

# Performance monitoring function
function Monitor-Performance {
    $logFilter = "QuestHook[TrickSaber*]:* AndroidRuntime:E *:S"
    
    if ($RealTime) {
        # Real-time monitoring with colored output
        adb logcat $logFilter | ForEach-Object {
            $line = $_
            $timestamp = Get-Date -Format "HH:mm:ss.fff"
            
            # Color code different types of messages
            if ($line -match "Performance Report") {
                Write-Host "[$timestamp] $line" -ForegroundColor Cyan
            } elseif ($line -match "ERROR|CRASH|FATAL") {
                Write-Host "[$timestamp] $line" -ForegroundColor Red
            } elseif ($line -match "FPS|Frame|Memory") {
                Write-Host "[$timestamp] $line" -ForegroundColor Green
            } elseif ($line -match "Trick|Throw|Spin") {
                Write-Host "[$timestamp] $line" -ForegroundColor Yellow
            } else {
                Write-Host "[$timestamp] $line" -ForegroundColor White
            }
            
            # Also write to file
            "[$timestamp] $line" | Out-File -FilePath $OutputFile -Append -Encoding UTF8
            
            # Check if we should stop
            if ((Get-Date) -gt $endTime) {
                break
            }
        }
    } else {
        # Batch monitoring - collect all logs then process
        $logOutput = adb logcat -t $Duration $logFilter
        $logOutput | Out-File -FilePath $OutputFile -Append -Encoding UTF8
    }
}

# Additional system metrics collection
function Collect-SystemMetrics {
    Write-Host "Collecting system metrics..." -ForegroundColor Yellow
    
    $systemInfo = @"

=== SYSTEM METRICS ===
CPU Usage: $(adb shell top -n 1 | Select-String "com.beatgames.beatsaber" | Select-Object -First 1)
Memory Info: $(adb shell dumpsys meminfo com.beatgames.beatsaber | Select-String "TOTAL" | Select-Object -First 1)
Battery Level: $(adb shell dumpsys battery | Select-String "level" | Select-Object -First 1)
Thermal State: $(adb shell dumpsys thermalservice | Select-String "Temperature" | Select-Object -First 3)
GPU Info: $(adb shell dumpsys SurfaceFlinger | Select-String "GLES" | Select-Object -First 1)

"@
    
    $systemInfo | Out-File -FilePath $OutputFile -Append -Encoding UTF8
}

# Detailed performance analysis
function Analyze-Performance {
    if (-not $Detailed) { return }
    
    Write-Host "Performing detailed analysis..." -ForegroundColor Yellow
    
    # Parse the log file for specific metrics
    $logContent = Get-Content $OutputFile
    
    # Extract FPS data
    $fpsData = $logContent | Select-String "FPS: (\d+\.?\d*)" | ForEach-Object {
        [float]$_.Matches[0].Groups[1].Value
    }
    
    # Extract memory data
    $memoryData = $logContent | Select-String "Memory.*: (\d+\.?\d*)MB" | ForEach-Object {
        [float]$_.Matches[0].Groups[1].Value
    }
    
    # Extract trick counts
    $throwCount = ($logContent | Select-String "Throws: (\d+)" | ForEach-Object {
        [int]$_.Matches[0].Groups[1].Value
    } | Measure-Object -Sum).Sum
    
    $spinCount = ($logContent | Select-String "Spins: (\d+)" | ForEach-Object {
        [int]$_.Matches[0].Groups[1].Value
    } | Measure-Object -Sum).Sum
    
    # Generate analysis
    $analysis = @"

=== PERFORMANCE ANALYSIS ===
FPS Statistics:
  Average: $([math]::Round(($fpsData | Measure-Object -Average).Average, 2))
  Minimum: $([math]::Round(($fpsData | Measure-Object -Minimum).Minimum, 2))
  Maximum: $([math]::Round(($fpsData | Measure-Object -Maximum).Maximum, 2))
  Samples: $($fpsData.Count)

Memory Statistics:
  Average: $([math]::Round(($memoryData | Measure-Object -Average).Average, 2))MB
  Peak: $([math]::Round(($memoryData | Measure-Object -Maximum).Maximum, 2))MB
  Samples: $($memoryData.Count)

Trick Statistics:
  Total Throws: $throwCount
  Total Spins: $spinCount
  Tricks per Minute: $([math]::Round((($throwCount + $spinCount) / ($Duration / 60)), 2))

Performance Rating:
$(if (($fpsData | Measure-Object -Average).Average -gt 85) { "  ✓ Excellent (>85 FPS average)" } 
  elseif (($fpsData | Measure-Object -Average).Average -gt 75) { "  ⚠ Good (75-85 FPS average)" }
  else { "  ✗ Poor (<75 FPS average)" })

$(if (($memoryData | Measure-Object -Maximum).Maximum -lt 100) { "  ✓ Memory usage acceptable (<100MB peak)" }
  else { "  ⚠ High memory usage (>100MB peak)" })

"@
    
    $analysis | Out-File -FilePath $OutputFile -Append -Encoding UTF8
}

# Main monitoring loop
try {
    # Start background system metrics collection
    if ($Detailed) {
        Start-Job -ScriptBlock { 
            param($OutputFile)
            while ($true) {
                Collect-SystemMetrics
                Start-Sleep 10
            }
        } -ArgumentList $OutputFile | Out-Null
    }
    
    # Start performance monitoring
    Monitor-Performance
    
    Write-Host "Monitoring completed!" -ForegroundColor Green
    
    # Collect final system metrics
    Collect-SystemMetrics
    
    # Perform analysis if requested
    Analyze-Performance
    
    Write-Host "Report saved to: $OutputFile" -ForegroundColor Green
    
    # Display summary
    if ($Detailed) {
        Write-Host "`nPerformance Summary:" -ForegroundColor Cyan
        Get-Content $OutputFile | Select-String "Performance Rating:" -A 10 | Write-Host
    }
    
} catch {
    Write-Error "Monitoring failed: $_"
} finally {
    # Clean up background jobs
    Get-Job | Stop-Job | Remove-Job
}

Write-Host "Performance monitoring complete!" -ForegroundColor Green