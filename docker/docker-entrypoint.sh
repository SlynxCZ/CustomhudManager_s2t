#!/bin/bash
set -e

### --- Version -------------------------------------------------------------
if [ -n "${SEMVER:-}" ]; then
  export SEMVER
  echo "=== Version: $SEMVER (from the environment) ==="
elif SEMVER="$(git describe --tags --exact-match 2>/dev/null)"; then
  export SEMVER
  echo "=== Version: $SEMVER (tag on HEAD) ==="
else
  unset SEMVER
  echo '=== Version: no tag on HEAD, building as "Local" ==='
fi
export GITHUB_SHA_SHORT="$(git rev-parse --short HEAD)"

### --- SDKs ----------------------------------------------------------------
# A toolkit plugin builds against the Source2Toolkit SDK, which carries the
# s2sdk it needs (its vendor/s2sdk submodule, brought in by --recursive); the
# game protobufs come from SteamTracking.
SDK_DIR="/tmp/sdk"
SOURCE2TOOLKITSDK_DIR="$SDK_DIR/source2toolkit-sdk"
CSGO_PROTO_DIR="$SDK_DIR/Protobufs"

rm -rf "$SDK_DIR"
mkdir -p "$SDK_DIR"
echo "=== Downloading Source2Toolkit-SDK ==="
git clone --recursive https://github.com/Source2Toolkit/source2toolkit-sdk.git "$SOURCE2TOOLKITSDK_DIR"
echo "=== Downloading Protobufs ==="
git clone --recursive https://github.com/SteamTracking/Protobufs "$CSGO_PROTO_DIR"

export SOURCE2TOOLKIT_SDK="$SOURCE2TOOLKITSDK_DIR"
export CSGO_PROTO="$CSGO_PROTO_DIR/csgo"

### --- Build (AMBuild) ----------------------------------------------------
# s2sdk and the manifests both come from the SDK's submodules.
rm -rf build
mkdir build
cd build
CC=clang-18 CXX=clang++-18 python3 ../configure.py --enable-optimize --sdks cs2 --targets x86_64
ambuild

### --- Package -------------------------------------------------------------
# AMBuild wrote package/cs2/addons/source2toolkit/plugins/customhud_manager.stx;
# the panorama sources for the client addon go next to it.
cp -r ../panorama package/cs2/panorama
