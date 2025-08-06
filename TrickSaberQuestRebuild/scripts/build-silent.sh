#!/bin/bash
# Ultra-minimal build script using the quietest possible commands

set -e

# Clean build if requested
if [[ "$1" == "--clean" ]]; then
    rm -rf build 2>/dev/null || true
fi

# Create build directory
mkdir -p build

# Configure (suppress most output)
/opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build >/dev/null

# Build using make with silent flag (most minimal output)
make -C build -s -j$(nproc 2>/dev/null || echo 4)

# Simple success indicator
if [ -f "build/libtricksaber.so" ]; then
    echo "✅ Build complete"
else
    echo "❌ Build failed"
    exit 1
fi