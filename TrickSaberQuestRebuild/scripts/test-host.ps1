param(
    [switch]$Clean,
    [string]$Filter = "*"
)

# Redirect to the isolated test script
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$IsolatedScript = Join-Path $ScriptDir "test-host-isolated.ps1"

if ($Clean) {
    & $IsolatedScript -Clean -Filter $Filter
} else {
    & $IsolatedScript -Filter $Filter
}