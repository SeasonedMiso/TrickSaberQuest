#!/usr/bin/env pwsh

param(
    [Parameter(Mandatory=$false)]
    [string]$TestType = "full",  # full, quick, stress
    
    [Parameter(Mandatory=$false)]
    [string]$OutputDir = "benchmark_results",
    
    [Parameter(Mandatory=$false)]
    [switch]$AutoRestart = $false
)

Write-Host "TrickSaber Quest Benchmark Suite" -ForegroundColor Green
Write-Host "Test Type: $TestType" -ForegroundColor Yellow

# Create output directory
if (-not (Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$resultFile = "$OutputDir/benchmark_$TestType_$timestamp.json"

# Check device connection
$devices = adb devices
if ($devices -notmatch "device$") {
    Write-Error "No Quest device connected."
    exit 1
}

# Benchmark configuration
$benchmarkConfig = @{
    full = @{
        duration = 300  # 5 minutes
        tests = @("idle", "gameplay", "tricks", "stress")
        description = "Comprehensive performance test"
    }
    quick = @{
        duration = 60   # 1 minute
        tests = @("idle", "gameplay")
        description = "Quick performance check"
    }
    stress = @{
        duration = 180  # 3 minutes
        tests = @("stress")
        description = "Stress test with maximum trick usage"
    }
}

$config = $benchmarkConfig[$TestType]
if (-not $config) {
    Write-Error "Invalid test type: $TestType. Use: full, quick, or stress"
    exit 1
}

Write-Host "Running $($config.description)" -ForegroundColor Yellow
Write-Host "Duration: $($config.duration) seconds" -ForegroundColor Yellow

# Initialize results structure
$results = @{
    timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    testType = $TestType
    duration = $config.duration
    device = (adb shell getprop ro.product.model).Trim()
    beatSaberVersion = ""
    tests = @{}
    summary = @{}
}

# Helper functions
function Start-BeatSaber {
    Write-Host "Starting Beat Saber..." -ForegroundColor Yellow
    adb shell am start -n com.beatgames.beatsaber/com.unity3d.player.UnityPlayerActivity
    Start-Sleep 10  # Wait for app to fully load
}

function Stop-BeatSaber {
    Write-Host "Stopping Beat Saber..." -ForegroundColor Yellow
    adb shell am force-stop com.beatgames.beatsaber
    Start-Sleep 3
}

function Measure-Performance {
    param(
        [string]$TestName,
        [int]$Duration,
        [scriptblock]$TestAction = {}
    )
    
    Write-Host "Running test: $TestName ($Duration seconds)" -ForegroundColor Cyan
    
    $testStart = Get-Date
    $performanceData = @{
        fps = @()
        memory = @()
        cpu = @()
        battery = @()
        temperature = @()
        tricks = @{
            throws = 0
            spins = 0
            failed = 0
        }
    }
    
    # Start performance monitoring
    $monitorJob = Start-Job -ScriptBlock {
        param($Duration, $TestName)
        
        $endTime = (Get-Date).AddSeconds($Duration)
        $samples = @()
        
        while ((Get-Date) -lt $endTime) {
            $sample = @{
                timestamp = Get-Date -Format "HH:mm:ss.fff"
                fps = 0
                memory = 0
                cpu = 0
            }
            
            # Get FPS from logcat
            $fpsLog = adb logcat -t 1 | Select-String "FPS.*: (\d+\.?\d*)"
            if ($fpsLog) {
                $sample.fps = [float]$fpsLog.Matches[0].Groups[1].Value
            }
            
            # Get memory usage
            $memInfo = adb shell dumpsys meminfo com.beatgames.beatsaber | Select-String "TOTAL.*?(\d+)"
            if ($memInfo) {
                $sample.memory = [int]$memInfo.Matches[0].Groups[1].Value / 1024  # Convert to MB
            }
            
            # Get CPU usage
            $cpuInfo = adb shell top -n 1 | Select-String "com.beatgames.beatsaber.*?(\d+\.?\d*)%"
            if ($cpuInfo) {
                $sample.cpu = [float]$cpuInfo.Matches[0].Groups[1].Value
            }
            
            $samples += $sample
            Start-Sleep 1
        }
        
        return $samples
    } -ArgumentList $Duration, $TestName
    
    # Execute test-specific actions
    if ($TestAction) {
        & $TestAction
    }
    
    # Wait for monitoring to complete
    $samples = Receive-Job -Job $monitorJob -Wait
    Remove-Job $monitorJob
    
    # Process samples
    foreach ($sample in $samples) {
        if ($sample.fps -gt 0) { $performanceData.fps += $sample.fps }
        if ($sample.memory -gt 0) { $performanceData.memory += $sample.memory }
        if ($sample.cpu -gt 0) { $performanceData.cpu += $sample.cpu }
    }
    
    # Get trick statistics from logs
    $trickLogs = adb logcat -d | Select-String "TrickSaber.*Trick"
    $performanceData.tricks.throws = ($trickLogs | Select-String "Throw").Count
    $performanceData.tricks.spins = ($trickLogs | Select-String "Spin").Count
    $performanceData.tricks.failed = ($trickLogs | Select-String "Failed").Count
    
    # Calculate statistics
    $testResults = @{
        duration = $Duration
        fps = @{
            average = if ($performanceData.fps.Count -gt 0) { [math]::Round(($performanceData.fps | Measure-Object -Average).Average, 2) } else { 0 }
            min = if ($performanceData.fps.Count -gt 0) { [math]::Round(($performanceData.fps | Measure-Object -Minimum).Minimum, 2) } else { 0 }
            max = if ($performanceData.fps.Count -gt 0) { [math]::Round(($performanceData.fps | Measure-Object -Maximum).Maximum, 2) } else { 0 }
            samples = $performanceData.fps.Count
        }
        memory = @{
            average = if ($performanceData.memory.Count -gt 0) { [math]::Round(($performanceData.memory | Measure-Object -Average).Average, 2) } else { 0 }
            peak = if ($performanceData.memory.Count -gt 0) { [math]::Round(($performanceData.memory | Measure-Object -Maximum).Maximum, 2) } else { 0 }
            samples = $performanceData.memory.Count
        }
        cpu = @{
            average = if ($performanceData.cpu.Count -gt 0) { [math]::Round(($performanceData.cpu | Measure-Object -Average).Average, 2) } else { 0 }
            peak = if ($performanceData.cpu.Count -gt 0) { [math]::Round(($performanceData.cpu | Measure-Object -Maximum).Maximum, 2) } else { 0 }
        }
        tricks = $performanceData.tricks
    }
    
    $results.tests[$TestName] = $testResults
    
    Write-Host "Test completed: $TestName" -ForegroundColor Green
    Write-Host "  Average FPS: $($testResults.fps.average)" -ForegroundColor White
    Write-Host "  Peak Memory: $($testResults.memory.peak)MB" -ForegroundColor White
    Write-Host "  Tricks: $($testResults.tricks.throws) throws, $($testResults.tricks.spins) spins" -ForegroundColor White
}

# Test implementations
function Test-Idle {
    Measure-Performance -TestName "idle" -Duration 30 -TestAction {
        Write-Host "  Measuring idle performance (main menu)" -ForegroundColor Gray
        Start-Sleep 30
    }
}

function Test-Gameplay {
    Measure-Performance -TestName "gameplay" -Duration 60 -TestAction {
        Write-Host "  Starting gameplay test..." -ForegroundColor Gray
        # Navigate to solo play (this would need to be automated based on UI)
        # For now, assume user manually starts a song
        Write-Host "  Please start a song manually for gameplay testing" -ForegroundColor Yellow
        Start-Sleep 60
    }
}

function Test-Tricks {
    Measure-Performance -TestName "tricks" -Duration 90 -TestAction {
        Write-Host "  Testing trick performance..." -ForegroundColor Gray
        Write-Host "  Please perform various tricks during this test" -ForegroundColor Yellow
        Start-Sleep 90
    }
}

function Test-Stress {
    Measure-Performance -TestName "stress" -Duration 120 -TestAction {
        Write-Host "  Running stress test..." -ForegroundColor Gray
        Write-Host "  Please perform continuous tricks for maximum load" -ForegroundColor Yellow
        Start-Sleep 120
    }
}

# Main benchmark execution
try {
    Write-Host "Starting benchmark..." -ForegroundColor Green
    
    # Clear logcat buffer
    adb logcat -c
    
    # Start Beat Saber if needed
    if ($AutoRestart) {
        Stop-BeatSaber
        Start-BeatSaber
    }
    
    # Run tests based on configuration
    foreach ($test in $config.tests) {
        switch ($test) {
            "idle" { Test-Idle }
            "gameplay" { Test-Gameplay }
            "tricks" { Test-Tricks }
            "stress" { Test-Stress }
        }
        
        # Brief pause between tests
        Start-Sleep 5
    }
    
    # Calculate overall summary
    $allFps = @()
    $allMemory = @()
    $totalTricks = 0
    
    foreach ($test in $results.tests.Values) {
        if ($test.fps.samples -gt 0) {
            $allFps += $test.fps.average
        }
        if ($test.memory.samples -gt 0) {
            $allMemory += $test.memory.peak
        }
        $totalTricks += $test.tricks.throws + $test.tricks.spins
    }
    
    $results.summary = @{
        overallFps = if ($allFps.Count -gt 0) { [math]::Round(($allFps | Measure-Object -Average).Average, 2) } else { 0 }
        peakMemory = if ($allMemory.Count -gt 0) { [math]::Round(($allMemory | Measure-Object -Maximum).Maximum, 2) } else { 0 }
        totalTricks = $totalTricks
        rating = ""
    }
    
    # Performance rating
    $fps = $results.summary.overallFps
    $memory = $results.summary.peakMemory
    
    if ($fps -gt 85 -and $memory -lt 150) {
        $results.summary.rating = "Excellent"
    } elseif ($fps -gt 75 -and $memory -lt 200) {
        $results.summary.rating = "Good"
    } elseif ($fps -gt 60 -and $memory -lt 250) {
        $results.summary.rating = "Fair"
    } else {
        $results.summary.rating = "Poor"
    }
    
    # Save results
    $results | ConvertTo-Json -Depth 10 | Out-File -FilePath $resultFile -Encoding UTF8
    
    # Display summary
    Write-Host "`n=== BENCHMARK RESULTS ===" -ForegroundColor Green
    Write-Host "Overall FPS: $($results.summary.overallFps)" -ForegroundColor White
    Write-Host "Peak Memory: $($results.summary.peakMemory)MB" -ForegroundColor White
    Write-Host "Total Tricks: $($results.summary.totalTricks)" -ForegroundColor White
    Write-Host "Rating: $($results.summary.rating)" -ForegroundColor $(
        switch ($results.summary.rating) {
            "Excellent" { "Green" }
            "Good" { "Yellow" }
            "Fair" { "DarkYellow" }
            "Poor" { "Red" }
        }
    )
    
    Write-Host "`nDetailed results saved to: $resultFile" -ForegroundColor Cyan
    
} catch {
    Write-Error "Benchmark failed: $_"
    exit 1
}

Write-Host "Benchmark completed successfully!" -ForegroundColor Green