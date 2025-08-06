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
    echo "\"Build\" - Compiles your mod into a \".so\" or a \".a\" library"
    echo ""
    echo "-- Arguments --"
    echo ""
    echo "--clean        Deletes the \"build\" folder, so that the entire library is rebuilt"
    exit 0
fi

# if user specified clean, remove all build files
if [ "$CLEAN" = true ]; then
    if [ -d "build" ]; then
        rm -rf build
    fi
fi

if [ "$CLEAN" = true ] || [ ! -d "build" ]; then
    mkdir -p build
fi

/opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build
/opt/homebrew/bin/cmake --build ./build --config RelWithDebInfo -- -j8 VERBOSE=0

if [ $? -eq 0 ]; then
    echo ""
    echo "Build completed successfully!"
    if [ -d "binaries" ]; then
        echo ""
        echo "Generated QMODs:"
        ls -1 binaries/*.qmod 2>/dev/null | sed 's/.*\///; s/^/  - /' || echo "  No QMODs found"
    fi
fi