#!/usr/bin/env pwsh

Param(
    [Parameter(Mandatory=$false)]
    [Switch] $clean,
    [Parameter(Mandatory=$false)]
    [Switch] $help
)

if ($help) {
    Write-Output "Build-All - Complete build with debug and release QMODs"
    Write-Output ""
    Write-Output "-- Arguments --"
    Write-Output "-clean    Clean build directory before building"
    exit 0
}

Write-Output "=== TrickSaber Quest Build System ==="
Write-Output ""

# Ensure we're in the right directory
if (-not (Test-Path "CMakeLists.txt")) {
    Write-Error "Must run from TrickSaberQuestRebuild directory"
    exit 1
}

# Clean if requested
if ($clean) {
    Write-Output "Cleaning build directory..."
    if (Test-Path "build") { Remove-Item "build" -Recurse -Force }
    if (Test-Path "binaries") { Remove-Item "binaries" -Recurse -Force }
    Write-Output "Clean completed."
    Write-Output ""
}

# Ensure directories exist
New-Item -ItemType Directory -Path "build", "binaries" -Force | Out-Null

# Build
Write-Output "Building mod..."
& $PSScriptRoot/build.ps1
if ($LASTEXITCODE -ne 0) {
    Write-Error "❌ Build failed!"
    exit $LASTEXITCODE
}

Write-Output ""
Write-Output "✅ Build completed successfully!"

if (Test-Path "binaries") {
    $qmods = Get-ChildItem "binaries/*.qmod" -ErrorAction SilentlyContinue
    if ($qmods) {
        Write-Output ""
        Write-Output "📦 Generated QMODs:"
        $qmods | ForEach-Object {
            $size = [math]::Round($_.Length / 1KB, 1)
            Write-Output "  - $($_.Name) (${size} KB)"
        }
        Write-Output ""
        Write-Output "🎯 Ready for installation:"
        Write-Output "  Debug version (with symbols): *_debug_*.qmod"
        Write-Output "  Release version (stripped): *_v*.qmod (without debug suffix)"
    } else {
        Write-Output ""
        Write-Output "⚠️  No QMODs found in binaries/ - CMake post-build may have failed"
    }
} else {
    Write-Output "⚠️  No binaries directory found"
}