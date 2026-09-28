#Requires -Version 5.1
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (-not $env:SEMVER) {
    $tag = $null
    try { $tag = git describe --tags --exact-match 2>$null } catch {}
    if ($tag) { $env:SEMVER = $tag } else { Remove-Item Env:SEMVER -ErrorAction SilentlyContinue }
}
$env:GITHUB_SHA_SHORT = git rev-parse --short HEAD

### --- MSVC ------------------------------------------------------------------
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw "MSVC not found on runner!" }
$vcvars = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
cmd /c "`"$vcvars`" && set" | ForEach-Object {
    if ($_ -match "^(.*?)=(.*)$") { Set-Item -Path "env:$($matches[1])" -Value $matches[2] }
}

### --- SDKs ------------------------------------------------------------------
$SDK_DIR = "$env:TEMP\sdk"
$SOURCE2TOOLKITSDK_DIR = "$SDK_DIR\source2toolkit-sdk"
$HL2SDK_DIR = "$SDK_DIR\hl2sdk-cs2"
$CSGO_PROTO_DIR = "$SDK_DIR\Protobufs"
if (Test-Path $SDK_DIR) { Remove-Item -Recurse -Force $SDK_DIR }
New-Item -ItemType Directory -Force $SDK_DIR | Out-Null
git clone --recursive https://github.com/SlynxCZ/source2toolkit-sdk.git $SOURCE2TOOLKITSDK_DIR
git clone --recursive --branch cs2 --single-branch https://github.com/alliedmodders/hl2sdk.git $HL2SDK_DIR
git clone --recursive https://github.com/SteamDatabase/Protobufs $CSGO_PROTO_DIR
$env:SOURCE2TOOLKIT_SDK = $SOURCE2TOOLKITSDK_DIR
$env:HL2SDKCS2 = $HL2SDK_DIR
$env:CSGO_PROTO = "$CSGO_PROTO_DIR\csgo"

### --- Build -----------------------------------------------------------------
$REPO_ROOT = Split-Path -Parent $PSScriptRoot
$BUILD_DIR = "$REPO_ROOT\build"
if (Test-Path $BUILD_DIR) { Remove-Item -Recurse -Force $BUILD_DIR }
New-Item -ItemType Directory $BUILD_DIR | Out-Null
Set-Location $BUILD_DIR
cmake $REPO_ROOT -G Ninja -DCMAKE_C_COMPILER=cl -DCMAKE_CXX_COMPILER=cl -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build $BUILD_DIR -- -j $env:NUMBER_OF_PROCESSORS

### --- Package ---------------------------------------------------------------
New-Item -ItemType Directory -Force "$BUILD_DIR\package" | Out-Null
Copy-Item -Recurse -Force "$BUILD_DIR\addons" "$BUILD_DIR\package\"
Copy-Item -Recurse -Force "$REPO_ROOT\panorama" "$BUILD_DIR\package\panorama"
