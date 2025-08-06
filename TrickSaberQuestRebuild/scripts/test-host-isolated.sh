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

# Create isolated test directory
TEST_DIR="host-tests-isolated"

if [ "$CLEAN" = true ]; then
    if [ -d "$TEST_DIR" ]; then
        rm -rf "$TEST_DIR"
        echo -e "\033[32mCleaned isolated test directory\033[0m"
    fi
fi

if [ ! -d "$TEST_DIR" ]; then
    mkdir "$TEST_DIR"
fi

cd "$TEST_DIR"

# Create minimal CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.22)

# Ensure we're building for host platform
unset(CMAKE_TOOLCHAIN_FILE CACHE)
unset(ANDROID CACHE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

project(tricksaber_host_tests)

# Host-native compile options
add_compile_options(-std=c++20 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g)
else()
    add_compile_options(-O2 -g)
endif()

# GTest setup
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Get test source files
file(GLOB_RECURSE cpp_test_files "../test/*.cpp")

# Create test executable
add_executable(
    tricksaber_host_test
    ${cpp_test_files}
)

# Link with GTest
target_link_libraries(
    tricksaber_host_test
    PRIVATE GTest::gtest_main
)

# Include directories
target_include_directories(tricksaber_host_test PRIVATE 
    ../test
    ../include
    ../src
)

# Test-specific definitions
target_compile_definitions(tricksaber_host_test PRIVATE
    HOST_TESTS=1
    VERSION="1.12.1"
    MOD_ID="tricksaber"
)

include(GoogleTest)
gtest_discover_tests(tricksaber_host_test)
EOF

# Configure with clean environment
echo -e "\033[33mConfiguring isolated host-native tests...\033[0m"
unset CMAKE_TOOLCHAIN_FILE
unset ANDROID_NDK
unset ANDROID_NDK_ROOT

/opt/homebrew/bin/cmake -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE="Debug" \
    .

# Build tests
echo -e "\033[33mBuilding isolated host-native tests...\033[0m"
/opt/homebrew/bin/cmake --build . --target tricksaber_host_test

# Run tests
echo -e "\033[33mRunning tests...\033[0m"
./tricksaber_host_test --gtest_filter="$FILTER"

echo -e "\033[32mAll isolated host tests passed!\033[0m"