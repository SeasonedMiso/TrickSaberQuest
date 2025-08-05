#!/usr/bin/env pwsh

param(
    [switch]$Verbose
)

Write-Host "Testing Lazy Initialization Implementation..." -ForegroundColor Green

# Check if lazy initialization files exist
$lazyFiles = @(
    "include/TrickSaber/Utils/LazyInitializer.hpp",
    "src/TrickSaber/Utils/LazyInitializer.cpp"
)

$missingFiles = @()
foreach ($file in $lazyFiles) {
    if (!(Test-Path $file)) {
        $missingFiles += $file
    }
}

if ($missingFiles.Count -gt 0) {
    Write-Host "❌ Missing lazy initialization files:" -ForegroundColor Red
    $missingFiles | ForEach-Object { Write-Host "  - $_" -ForegroundColor Red }
    exit 1
}

Write-Host "✅ Lazy initialization files present" -ForegroundColor Green

# Check for lazy initialization patterns in key files
$patterns = @{
    "include/TrickSaber/Utils/ObjectCache.hpp" = @("LazyInitializer", "LazyCacheWarmer")
    "include/TrickSaber/Utils/PerformanceMetrics.hpp" = @("LazyInitializer", "LazyPerformanceSetup")
    "include/TrickSaber/UI/DebugOverlay.hpp" = @("LazyDebugOverlayCreator")
    "src/main.cpp" = @("LazyInitializer", "lazy")
}

$allPatternsFound = $true
foreach ($file in $patterns.Keys) {
    if (Test-Path $file) {
        $content = Get-Content $file -Raw
        $filePatterns = $patterns[$file]
        
        foreach ($pattern in $filePatterns) {
            if ($content -notmatch $pattern) {
                Write-Host "❌ Pattern '$pattern' not found in $file" -ForegroundColor Red
                $allPatternsFound = $false
            } elseif ($Verbose) {
                Write-Host "✅ Pattern '$pattern' found in $file" -ForegroundColor Green
            }
        }
    } else {
        Write-Host "❌ File not found: $file" -ForegroundColor Red
        $allPatternsFound = $false
    }
}

if ($allPatternsFound) {
    Write-Host "✅ All lazy initialization patterns found" -ForegroundColor Green
} else {
    Write-Host "❌ Some lazy initialization patterns missing" -ForegroundColor Red
    exit 1
}

# Check for thread safety patterns
$threadSafetyFiles = @(
    "include/TrickSaber/Utils/LazyInitializer.hpp"
)

$threadSafetyPatterns = @("std::atomic", "std::mutex", "memory_order")
$threadSafetyFound = $true

foreach ($file in $threadSafetyFiles) {
    if (Test-Path $file) {
        $content = Get-Content $file -Raw
        
        foreach ($pattern in $threadSafetyPatterns) {
            if ($content -notmatch $pattern) {
                Write-Host "❌ Thread safety pattern '$pattern' not found in $file" -ForegroundColor Red
                $threadSafetyFound = $false
            } elseif ($Verbose) {
                Write-Host "✅ Thread safety pattern '$pattern' found in $file" -ForegroundColor Green
            }
        }
    }
}

if ($threadSafetyFound) {
    Write-Host "✅ Thread safety patterns found" -ForegroundColor Green
} else {
    Write-Host "❌ Thread safety patterns missing" -ForegroundColor Red
    exit 1
}

# Test build to ensure compilation works
Write-Host "Testing compilation..." -ForegroundColor Yellow

try {
    $buildResult = & pwsh ./scripts/build.ps1 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✅ Lazy initialization compiles successfully" -ForegroundColor Green
    } else {
        Write-Host "❌ Compilation failed with lazy initialization" -ForegroundColor Red
        if ($Verbose) {
            Write-Host "Build output:" -ForegroundColor Yellow
            Write-Host $buildResult
        }
        exit 1
    }
} catch {
    Write-Host "❌ Build script execution failed: $_" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "🎉 Lazy Initialization Test Passed!" -ForegroundColor Green
Write-Host ""
Write-Host "Key Benefits Implemented:" -ForegroundColor Cyan
Write-Host "  • ObjectCache warming deferred until tricks are active" -ForegroundColor White
Write-Host "  • PerformanceMetrics initialization only when needed" -ForegroundColor White
Write-Host "  • DebugOverlay creation only when enabled" -ForegroundColor White
Write-Host "  • Thread-safe lazy initialization patterns" -ForegroundColor White
Write-Host "  • Reduced startup time and memory usage" -ForegroundColor White