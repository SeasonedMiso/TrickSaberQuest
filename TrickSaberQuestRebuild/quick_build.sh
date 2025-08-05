#!/bin/bash
# Quick build verification for TrickSaber Quest

set -e

echo "🔨 TrickSaber Quest - Quick Build"
echo "================================"

# Check build tools
if ! command -v /opt/homebrew/bin/cmake &> /dev/null; then
    echo "❌ CMake not found at expected path"
    exit 1
fi

if ! command -v qpm &> /dev/null; then
    echo "❌ QPM not found in PATH"
    exit 1
fi

# Restore dependencies if needed
if [ ! -d "extern/includes" ]; then
    echo "📦 Restoring QPM dependencies..."
    qpm restore || { echo "❌ QPM restore failed"; exit 1; }
fi

# Configure and build
echo "⚙️ Configuring and building..."
/opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build
/opt/homebrew/bin/cmake --build ./build --config RelWithDebInfo -- -j"$(nproc 2>/dev/null || echo '1')" VERBOSE=0

# Verify output
if [ -f "build/libtricksaber.so" ]; then
    SIZE="$(stat -f%z build/libtricksaber.so 2>/dev/null || echo 'unknown')"
    echo "✅ Build successful! libtricksaber.so (${SIZE} bytes)"
else
    echo "❌ Build failed - libtricksaber.so not found"
    exit 1
fi