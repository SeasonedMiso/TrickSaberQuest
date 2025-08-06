#!/bin/bash

set -e

CLEAN=false
HELP=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --help)
            HELP=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--clean] [--help]"
            exit 1
            ;;
    esac
done

if [ "$HELP" = true ]; then
    echo "\"Build All\" - Compiles mod and creates both debug and release QMODs"
    echo ""
    echo "-- Arguments --"
    echo ""
    echo "--clean        Deletes the \"build\" folder for a clean rebuild"
    exit 0
fi

echo "=== TrickSaber Quest Build System ==="
echo ""

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    rm -rf build binaries
    echo "Clean completed."
    echo ""
fi

# Ensure directories exist
mkdir -p build binaries

echo "Building mod..."
/opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build
/opt/homebrew/bin/cmake --build ./build --config RelWithDebInfo -- -j$(nproc) VERBOSE=0

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Build completed successfully!"
    
    # Check if QMODs were created by CMake
    if [ -d "binaries" ] && [ "$(ls -A binaries/*.qmod 2>/dev/null)" ]; then
        echo ""
        echo "📦 Generated QMODs:"
        ls -1 binaries/*.qmod 2>/dev/null | sed 's/.*\///; s/^/  - /'
        
        echo ""
        echo "🎯 Ready for installation:"
        echo "  Debug version (with symbols): *_debug_*.qmod"
        echo "  Release version (stripped): *_v*.qmod (without debug suffix)"
    else
        echo ""
        echo "⚠️  No QMODs found in binaries/ - CMake post-build may have failed"
    fi
else
    echo ""
    echo "❌ Build failed!"
    exit 1
fi