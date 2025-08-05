#!/bin/bash

set -e

CLEAN=false
FILTER="*"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --filter)
            FILTER="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--clean] [--filter PATTERN]"
            exit 1
            ;;
    esac
done

if [ "$CLEAN" = true ]; then
    if [ -d "build" ]; then
        rm -rf build
        echo -e "\033[32mCleaned build directory\033[0m"
    fi
fi

# Create build directory
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# Configure with tests enabled
echo -e "\033[33mConfiguring build with tests...\033[0m"
/opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Debug" -DBUILD_TESTS=ON ..

# Build tests
echo -e "\033[33mBuilding tests...\033[0m"
/opt/homebrew/bin/cmake --build . --target tricksaber_test

# Run tests
echo -e "\033[33mRunning tests...\033[0m"
./tricksaber_test --gtest_filter="$FILTER"

echo -e "\033[32mAll tests passed!\033[0m"