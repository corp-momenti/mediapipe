bazel build --config=ios_simulator mediapipe/examples/ios/moface:PatchedMofaceFramework

rm -rf ./ios-simulator/*

cp -rf bazel-bin/mediapipe/examples/ios/moface/MofaceFramework.framework* ./ios-simulator/

bazel build --config=ios_device mediapipe/examples/ios/moface:PatchedMofaceFramework

rm -rf ./ios-device/*

cp -rf bazel-bin/mediapipe/examples/ios/moface/MofaceFramework.framework* ./ios-device/

rm -rf ./MofaceFramework.xcframework*

sh ./build-xcframework.sh
