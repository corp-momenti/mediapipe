# Copyright 2020 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load(
    "@build_bazel_rules_apple//apple:ios.bzl",
    "ios_framework"
)

licenses(["notice"])

MIN_IOS_VERSION = "11.0"

genrule(
    name = "PatchedMofaceFramework",
    srcs = [":MofaceFramework"],
    outs = [
        "MofaceFramework.framework",
        "MofaceFramework.framework.dSYM",
    ],
    output_to_bindir = 1,
    tools = ["patch_ios_framework.sh"],
    cmd = """
        bash $(location patch_ios_framework.sh) "$(SRCS)" "$(OUTS)"
    """
)

ios_framework(
    name = "MofaceFramework",
    hdrs = [
        "MofaceWrapper.h",
        "headers/MofaceFramework.h",
    ],
    infoplists = ["Info.plist"],
    bundle_id = "tv.momenti.MofaceEngine",
    families = ["iphone", "ipad"],
    resources = [
        "modulemap/module.modulemap"
    ],
    minimum_os_version = MIN_IOS_VERSION,
    visibility = ["//visibility:public"],
    deps = [
        ":MofaceWrapper",
        "@ios_opencv//:OpencvFramework",
    ],
)

objc_library(
    name = "MofaceWrapper",
    srcs = [
        "MofaceWrapper.mm",
        "MOImageConverter.mm"
    ],
    hdrs = [
        "MofaceWrapper.h",
        "MOImageConverter.h"
    ],
    copts = ["-std=c++17"],
    data = [
        "//mediapipe/graphs/face_effect:moface_mobile_gpu.binarypb",
        "//mediapipe/modules/face_detection:face_detection_short_range.tflite",
        "//mediapipe/modules/face_geometry/data:geometry_pipeline_metadata.binarypb",
        "//mediapipe/modules/face_geometry/data:geometry_pipeline_metadata_detection.binarypb",
        "//mediapipe/modules/face_geometry/data:geometry_pipeline_metadata_landmarks.binarypb",
        "//mediapipe/modules/face_landmark:face_landmark.tflite",
    ],
    features = ["-layering_check"],
    sdk_frameworks = [
        "AVFoundation",
        "CoreGraphics",
        "CoreMedia",
        "UIKit",
    ],
    deps = [
        "//mediapipe/objc:mediapipe_framework_ios",
        "//mediapipe/examples/desktop/moface_desktop:moface_calculator"
    ] + select({
        "//mediapipe:ios_i386": [],
        "//mediapipe:ios_x86_64": [],
        "//conditions:default": [
            "//mediapipe/framework/formats:matrix_data_cc_proto",
            "//mediapipe/graphs/face_effect:moface_mobile_gpu_deps",
            "//mediapipe/modules/face_geometry/protos:face_geometry_cc_proto",
        ],
    }),
)
