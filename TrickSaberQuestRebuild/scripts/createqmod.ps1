Param(
    [Parameter(Mandatory=$false)]
    [String] $qmodName="",

    [Parameter(Mandatory=$false)]
    [Switch] $help
)

if ($help -eq $true) {
    Write-Output "`"createqmod`" - Creates a .qmod file with your compiled libraries and mod.json."
    Write-Output "`n-- Arguments --`n"

    Write-Output "-QmodName `t The file name of your qmod"

    exit
}

$mod = "./mod.json"

& $PSScriptRoot/validate-modjson.ps1
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
$modJson = Get-Content $mod -Raw | ConvertFrom-Json

if ($qmodName -eq "") {
    $qmodName = $modJson.name
}

$filelist = @($mod)

$cover = "./" + $modJson.coverImage
if ((-not ($cover -eq "./")) -and (Test-Path $cover)) {
    $filelist += ,$cover
}

foreach ($mod in $modJson.modFiles) {
    $path = "./build/" + $mod
    if (-not (Test-Path $path)) {
        $path = "./extern/libs/" + $mod
    }
    if (-not (Test-Path $path)) {
        Write-Output "Error: could not find dependency: $path"
        exit 1
    }
    $filelist += $path
}

foreach ($mod in $modJson.lateModFiles) {
    $path = "./build/" + $mod
    if (-not (Test-Path $path)) {
        $path = "./extern/libs/" + $mod
    }
    if (-not (Test-Path $path)) {
        Write-Output "Error: could not find dependency: $path"
        exit 1
    }
    $filelist += $path
}


foreach ($lib in $modJson.libraryFiles) {
    $path = "./build/" + $lib
    if (-not (Test-Path $path)) {
        $path = "./extern/libs/" + $lib
    }
    if (-not (Test-Path $path)) {
        Write-Output "Error: could not find dependency: $path"
        exit 1
    }
    $filelist += $path
}

# Ensure binaries directory exists
if (-not (Test-Path "binaries")) {
    New-Item -ItemType Directory -Path "binaries" | Out-Null
}

# Get version from mod.json
$version = $modJson.version

# Create both debug and release versions with version numbers
$debugQmod = "binaries/" + $qmodName + "_debug_v" + $version + ".qmod"
$releaseQmod = "binaries/" + $qmodName + "_v" + $version + ".qmod"

# Remove existing qmods if they exist
if (Test-Path $debugQmod) { Remove-Item $debugQmod -Force }
if (Test-Path $releaseQmod) { Remove-Item $releaseQmod -Force }

# Create debug version (unstripped)
Compress-Archive -Path $filelist -DestinationPath ($debugQmod -replace "\.qmod$", ".zip") -Force
Move-Item ($debugQmod -replace "\.qmod$", ".zip") $debugQmod -Force

# Create stripped version for release
$strippedFiles = @()
foreach ($file in $filelist) {
    if ($file -like "*build/*.so") {
        $strippedFile = $file -replace "\.so$", "_stripped.so"
        if (Test-Path $strippedFile) {
            $strippedFiles += $strippedFile
        } else {
            $strippedFiles += $file
        }
    } else {
        $strippedFiles += $file
    }
}

Compress-Archive -Path $strippedFiles -DestinationPath ($releaseQmod -replace "\.qmod$", ".zip") -Force
Move-Item ($releaseQmod -replace "\.qmod$", ".zip") $releaseQmod -Force

Write-Output "Created debug version: $debugQmod"
Write-Output "Created release version: $releaseQmod"
Write-Output "Contents:"
foreach ($file in $filelist) {
    Write-Output "  - $(Split-Path $file -Leaf)"
}