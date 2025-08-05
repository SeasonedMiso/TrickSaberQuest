#!/usr/bin/env pwsh

param(
    [Parameter(Mandatory=$false)]
    [Switch] $Clean,
    [Parameter(Mandatory=$false)]
    [String] $TestType = "all",
    [Parameter(Mandatory=$false)]
    [Switch] $Verbose
)

$ErrorActionPreference = "Stop"

Write-Host "TrickSaber Quest Integration Tests" -ForegroundColor Cyan
Write-Host "=================================" -ForegroundColor Cyan

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
    Write-Host "Configuring integration tests..." -ForegroundColor Yellow
    & /opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="Debug" -DBUILD_TESTS=ON ..
    
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed"
    }

    # Build tests
    Write-Host "Building integration tests..." -ForegroundColor Yellow
    & /opt/homebrew/bin/cmake --build . --target tricksaber_test
    
    if ($LASTEXITCODE -ne 0) {
        throw "Integration test build failed"
    }

    # Run specific test types
    $testFilters = @{
        "all" = "*"
        "system" = "*IntegrationTest*"
        "platform" = "*QuestPlatformIntegrationTest*"
        "workflow" = "*SystemIntegrationTest*"
        "performance" = "*PerformanceUnderLoad*"
    }
    
    $filter = $testFilters[$TestType]
    if (-not $filter) {
        $filter = "*$TestType*"
    }
    
    Write-Host "Running integration tests (filter: $filter)..." -ForegroundColor Yellow
    
    $testArgs = @("--gtest_filter=$filter")
    if ($Verbose.IsPresent) {
        $testArgs += "--gtest_output=xml:integration_test_results.xml"
        $testArgs += "--gtest_print_time=1"
    }
    
    & ./tricksaber_test @testArgs
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Some integration tests failed" -ForegroundColor Red
        exit 1
    } else {
        Write-Host "All integration tests passed!" -ForegroundColor Green
        
        if ($Verbose.IsPresent -and (Test-Path "integration_test_results.xml")) {
            Write-Host "Test results saved to integration_test_results.xml" -ForegroundColor Blue
        }
    }
    
} finally {
    Pop-Location
}

Write-Host ""
Write-Host "Integration Test Summary:" -ForegroundColor Cyan
Write-Host "- Test Type: $TestType" -ForegroundColor White
Write-Host "- Filter: $filter" -ForegroundColor White
Write-Host "- Status: PASSED" -ForegroundColor Green