param(
    [switch]$Clean,
    [string]$Filter = "*"
)

$ErrorActionPreference = "Stop"

$TestDir = "host-tests-isolated"

if ($Clean -and (Test-Path $TestDir)) {
    Remove-Item -Path $TestDir -Recurse -Force
    Write-Host "Cleaned isolated test directory" -ForegroundColor Green
}

if (!(Test-Path $TestDir)) {
    New-Item -ItemType Directory -Path $TestDir | Out-Null
}

Set-Location $TestDir

try {
    # Create minimal CMakeLists.txt
    @'
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
'@ | Out-File -FilePath "CMakeLists.txt" -Encoding UTF8

    # Configure with clean environment
    Write-Host "Configuring isolated host-native tests..." -ForegroundColor Yellow
    $env:CMAKE_TOOLCHAIN_FILE = $null
    $env:ANDROID_NDK = $null
    $env:ANDROID_NDK_ROOT = $null

    & /opt/homebrew/bin/cmake -G "Unix Makefiles" `
        -DCMAKE_BUILD_TYPE="Debug" `
        .
    
    if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed" }

    Write-Host "Building isolated host-native tests..." -ForegroundColor Yellow
    & /opt/homebrew/bin/cmake --build . --target tricksaber_host_test
    
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }

    Write-Host "Running tests..." -ForegroundColor Yellow
    & ./tricksaber_host_test --gtest_filter="$Filter"
    
    if ($LASTEXITCODE -ne 0) { throw "Tests failed" }

    Write-Host "All isolated host tests passed!" -ForegroundColor Green
}
finally {
    Set-Location ..
}