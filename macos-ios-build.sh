#!/bin/bash

set -e

CONFIG=Debug
BUILD_ROOT_DIR=xcbuild

while [[ $# -gt 0 ]]; do
  case $1 in
    --config)
      CONFIG="$2"
      shift
      shift
      ;;
    --build-dir)
      BUILD_ROOT_DIR="$2"
      shift
      shift
      ;;
    *)
      echo "Unknown arg $1"
      exit 1
      ;;
  esac
done

TARGETS=(
  "ios-arm64 arm64 iOS 13.0"
  "ios-simulator-x86_64 x86_64 iOS 13.0"
  "ios-simulator-arm64 arm64 iOS 13.0"
  "macos-arm64 arm64 Darwin 13.0"
  "macos-x86_64 x86_64 Darwin 13.0"
)

build_target() {
  local BUILD_DIR="$BUILD_ROOT_DIR/$1"
  local ARCH="$2"
  local SYSTEM_NAME="$3"
  local DEPLOYMENT_TARGET="$4"
  local INSTALL_DIR="${BUILD_DIR}/install"

  rm -rf $BUILD_DIR
  mkdir -p $BUILD_DIR

  cmake -S . -B $BUILD_DIR \
      -DCMAKE_OSX_ARCHITECTURES=$ARCH \
      -DCMAKE_SYSTEM_NAME=$SYSTEM_NAME \
      -DCMAKE_OSX_DEPLOYMENT_TARGET=$DEPLOYMENT_TARGET \
      -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR

  cmake --build $BUILD_DIR --config $CONFIG
  cmake --install $BUILD_DIR --config $CONFIG --verbose
}

for target in "${TARGETS[@]}"; do
  build_target $target
done

xcodebuild -create-xcframework \
    -library build/ios-arm64/install/lib/gerium.a \
    -headers build/ios-arm64/install/include \
    -library build/ios-simulator-x86_64/install/lib/gerium.a \
    -headers build/ios-simulator-x86_64/install/include \
    -library build/ios-simulator-arm64/install/lib/gerium.a \
    -headers build/ios-simulator-arm64/install/include \
    -library build/macos-arm64/install/lib/gerium.a \
    -headers build/macos-arm64/install/include \
    -library build/macos-x86_64/install/lib/gerium.a \
    -headers build/macos-x86_64/install/include \
    -output universal/gerium.xcframework

rm -rf $BUILD_ROOT_DIR
