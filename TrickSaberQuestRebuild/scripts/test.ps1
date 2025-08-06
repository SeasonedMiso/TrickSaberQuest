#!/usr/bin/env pwsh

param(
    [Parameter(Mandatory=$false)]
    [Switch] $Clean,
    [Parameter(Mandatory=$false)]
    [String] $Filter = "*"
)

# Legacy test script - redirects to host-native tests
Write-Host "Note: Using host-native tests (Android cross-compilation tests not supported)" -ForegroundColor Yellow

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$HostScript = Join-Path $ScriptDir "test-host.ps1"

if ($Clean.IsPresent) {
    & $HostScript -Clean -Filter $Filter
} else {
    & $HostScript -Filter $Filter
}