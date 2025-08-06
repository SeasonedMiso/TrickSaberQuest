Param(
    [Parameter(Mandatory=$false)]
    [Switch] $clean,

    [Parameter(Mandatory=$false)]
    [Switch] $help
)

if ($help -eq $true) {
    Write-Output "`"Build`" - Copiles your mod into a `".so`" or a `".a`" library"
    Write-Output "`n-- Arguments --`n"

    Write-Output "-Clean `t`t Deletes the `"build`" folder, so that the entire library is rebuilt"

    exit
}

# if user specified clean, remove all build files
if ($clean.IsPresent) {
    if (Test-Path -Path "build") {
        remove-item build -R -Force 
    }
}


if (($clean.IsPresent) -or (-not (Test-Path -Path "build"))) {
    new-item -Path build -ItemType Directory
} 

& /opt/homebrew/bin/cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE="RelWithDebInfo" -B build
& /opt/homebrew/bin/cmake --build ./build --config RelWithDebInfo -- -j8 VERBOSE=0

if ($LASTEXITCODE -eq 0) {
    Write-Output "`nBuild completed successfully!"
    if (Test-Path "binaries") {
        Write-Output "`nGenerated QMODs:"
        Get-ChildItem "binaries/*.qmod" | ForEach-Object { Write-Output "  - $($_.Name)" }
    }
}

exit $LASTEXITCODE
