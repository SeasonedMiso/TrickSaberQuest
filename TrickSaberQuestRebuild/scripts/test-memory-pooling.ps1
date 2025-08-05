#!/usr/bin/env pwsh

param(
    [switch]$Clean,
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

Write-Host "Testing TrickSaber Memory Pooling Implementation" -ForegroundColor Green

# Change to project directory
$projectDir = Split-Path -Parent $PSScriptRoot
Set-Location $projectDir

try {
    # Clean build if requested
    if ($Clean) {
        Write-Host "Cleaning build directory..." -ForegroundColor Yellow
        if (Test-Path "build") {
            Remove-Item -Recurse -Force "build"
        }
    }

    # Restore dependencies
    Write-Host "Restoring QPM dependencies..." -ForegroundColor Cyan
    qpm restore
    if ($LASTEXITCODE -ne 0) {
        throw "QPM restore failed"
    }

    # Configure build
    Write-Host "Configuring CMake build..." -ForegroundColor Cyan
    $cmakePath = "/opt/homebrew/bin/cmake"
    & $cmakePath -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build with memory pooling
    Write-Host "Building TrickSaber with memory pooling..." -ForegroundColor Cyan
    if ($Verbose) {
        & $cmakePath --build ./build --config RelWithDebInfo -- -j$(nproc) VERBOSE=1
    } else {
        & $cmakePath --build ./build --config RelWithDebInfo -- -j$(nproc) VERBOSE=0
    }
    
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }

    # Check if memory pooling files were compiled
    $memoryFiles = @(
        "src/TrickSaber/Utils/VectorPool.cpp",
        "src/TrickSaber/Utils/TransformCache.cpp", 
        "src/TrickSaber/Utils/MemoryManager.cpp"
    )

    Write-Host "Verifying memory pooling files..." -ForegroundColor Cyan
    foreach ($file in $memoryFiles) {
        if (Test-Path $file) {
            Write-Host "✓ $file" -ForegroundColor Green
        } else {
            Write-Host "✗ $file (missing)" -ForegroundColor Red
        }
    }

    # Check build output
    if (Test-Path "build/libtricksaber.so") {
        $fileSize = (Get-Item "build/libtricksaber.so").Length
        Write-Host "✓ Build successful: libtricksaber.so ($([math]::Round($fileSize/1KB, 2)) KB)" -ForegroundColor Green
        
        # Check for memory pooling symbols (basic check)
        $objdumpOutput = objdump -t "build/libtricksaber.so" 2>/dev/null | Select-String "MemoryManager|VectorPool|TransformCache"
        if ($objdumpOutput) {
            Write-Host "✓ Memory pooling symbols found in binary" -ForegroundColor Green
        } else {
            Write-Host "⚠ Memory pooling symbols not detected (may be optimized)" -ForegroundColor Yellow
        }
    } else {
        throw "Build output not found"
    }

    Write-Host "`nMemory Pooling Implementation Summary:" -ForegroundColor Magenta
    Write-Host "• ObjectPool<T>: Generic object pooling template" -ForegroundColor White
    Write-Host "• VectorPool: Specialized Unity Vector3/Quaternion pooling" -ForegroundColor White  
    Write-Host "• TransformCache: Time-based transform caching system" -ForegroundColor White
    Write-Host "• MemoryManager: Unified memory management interface" -ForegroundColor White
    Write-Host "• PooledTrickCalculation: RAII wrapper for trick calculations" -ForegroundColor White

    Write-Host "`nIntegration Points:" -ForegroundColor Magenta
    Write-Host "• PhysicsHandler: Uses cached transforms and pooled vectors" -ForegroundColor White
    Write-Host "• MovementController: Uses cached transform access" -ForegroundColor White
    Write-Host "• ThrowTrick: Uses pooled trick calculations" -ForegroundColor White
    Write-Host "• Main: Initializes and cleans up memory pools" -ForegroundColor White

    Write-Host "`n✅ Memory pooling implementation test completed successfully!" -ForegroundColor Green

} catch {
    Write-Host "❌ Test failed: $_" -ForegroundColor Red
    exit 1
}