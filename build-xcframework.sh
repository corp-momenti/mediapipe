#!/bin/bash

set -e

frameworks=""

build_archive_for_simulator() {
    frameworks+=" -framework ios-simulator/MofaceFramework.framework"
}

build_archive_for_device() {
    frameworks+=" -framework ios-device/MofaceFramework.framework"
}

#rm -rf build
rm -rf MofaceFramework.xcframework

build_archive_for_simulator

build_archive_for_device

xcodebuild -create-xcframework $frameworks -output "MofaceFramework.xcframework"
