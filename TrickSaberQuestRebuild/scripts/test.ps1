#!/usr/bin/env pwsh

param(
    [Parameter(Mandatory=$false)]
    [Switch] $Clean,
    [Parameter(Mandatory=$false)]
    [String] $Filter = "*"
)

$ErrorActionPreference = "Stop"

if ($Clean.IsPresent) {
    if (Test-Path -Path "build") {
        Remove-Item -Path "build" -Force -Recurse
        Write-Host "Cleaned build directory" -ForegroundColor Green
    }
}

# Create build directory
if (!(Test-Path -Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

Push-Location build

try {
    # Configure with tests enabled
    Write-Host "Configuring build with tests..." -ForegroundColor Yellow
    & /opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Debug" -DBUILD_TESTS=ON ..
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build tests
    Write-Host "Building tests..." -ForegroundColor Yellow
    & /opt/homebrew/bin/cmake --build . --target tricksaber_test
    
    if ($LASTEXITCODE -ne 0) {
        throw "Test build failed"
    }

    # Run tests
    Write-Host "Running tests..." -ForegroundColor Yellow
    & ./tricksaber_test --gtest_filter="$Filter"
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Some tests failed" -ForegroundColor Red
        exit 1
    } else {
        Write-Host "All tests passed!" -ForegroundColor Green
    }
    
} finally {
    Pop-Location
}