#!/bin/bash

CONFIG=${1:-Debug}
BUILD_DIR=${2:-xcbuild}

# rm -rf $BUILD_DIR
mkdir -p \
    $BUILD_DIR/ios-arm64\
    $BUILD_DIR/ios-x86_64\
    $BUILD_DIR/ios-simulator\
    $BUILD_DIR/macos-arm64\
    $BUILD_DIR/macos-x86_64

cmake -S . -B $BUILD_DIR/ios-arm64 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_INSTALL_PREFIX=install/ios-arm64 \
    -DGERIUM_BUILD_SAMPLE=OFF
cmake --build $BUILD_DIR/ios-arm64 --config $CONFIG
cmake --install $BUILD_DIR/ios-arm64 --config $CONFIG

cmake -S . -B $BUILD_DIR/ios-x86_64 \
    -DCMAKE_OSX_ARCHITECTURES=x86_64 \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_INSTALL_PREFIX=install/ios-x86_64 \
    -DGERIUM_BUILD_SAMPLE=OFF
cmake --build $BUILD_DIR/ios-x86_64 --config $CONFIG
cmake --install $BUILD_DIR/ios-x86_64 --config $CONFIG

cmake -S . -B $BUILD_DIR/ios-simulator \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_INSTALL_PREFIX=install/ios-simulator \
    -DGERIUM_BUILD_SAMPLE=OFF
cmake --build $BUILD_DIR/ios-simulator --config $CONFIG
cmake --install $BUILD_DIR/ios-simulator --config $CONFIG

cmake -S . -B $BUILD_DIR/macos-arm64 \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_SYSTEM_NAME=Darwin \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_INSTALL_PREFIX=install/macos-arm64 \
    -DGERIUM_BUILD_SAMPLE=OFF
cmake --build $BUILD_DIR/macos-arm64 --config $CONFIG
cmake --install $BUILD_DIR/macos-arm64 --config $CONFIG

cmake -S . -B $BUILD_DIR/macos-x86_64 \
    -DCMAKE_OSX_ARCHITECTURES=x86_64 \
    -DCMAKE_SYSTEM_NAME=Darwin \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=13.0 \
    -DCMAKE_INSTALL_PREFIX=install/macos-x86_64 \
    -DGERIUM_BUILD_SAMPLE=OFF
cmake --build $BUILD_DIR/macos-x86_64 --config $CONFIG
cmake --install $BUILD_DIR/macos-x86_64 --config $CONFIG

xcodebuild -create-xcframework \
    -library install/ios-arm64/lib/libyourlibrary.a \
    -headers install/ios-arm64/include \
    -library install/ios-x86_64/lib/libyourlibrary.a \
    -headers install/ios-x86_64/include \
    -library install/ios-simulator/lib/libyourlibrary.a \
    -headers install/ios-simulator/include \
    -library install/macos-arm64/lib/libyourlibrary.a \
    -headers install/macos-arm64/include \
    -library install/macos-x86_64/lib/libyourlibrary.a \
    -headers install/macos-x86_64/include \
    -output build/yourlibrary.xcframework

# rm -rf $BUILD_DIR
