#!/usr/bin/env pwsh
# Ultra-minimal build script using the quietest possible commands

param([Switch]$clean)

# Clean build if requested
if ($clean) {
    Remove-Item "build" -Recurse -Force -ErrorAction SilentlyContinue
}

# Create build directory
New-Item -ItemType Directory -Path "build" -Force | Out-Null

# Configure (suppress output)
& /opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build | Out-Null

# Build using make with silent flag (most minimal output)
& make -C build -s -j8

# Simple success indicator
if (Test-Path "build/libtricksaber.so") {
    Write-Output "✅ Build complete"
} else {
    Write-Output "❌ Build failed"
    exit 1
}