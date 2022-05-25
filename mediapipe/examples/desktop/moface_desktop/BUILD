
licenses(["notice"])

package(default_visibility = ["//mediapipe/examples:__subpackages__"])

cc_binary(
    name = "moface_cpu",
    deps = [
        "//mediapipe/examples/desktop:moface_run_graph_main",
        "//mediapipe/graphs/face_effect:desktop_live_calculators",
        ":moface_calculator",
    ]
)

cc_library(
    name = "moface_calculator",
    srcs = ["moface_calculator/moface_calculator.cc",
            "moface_run_graph_main.cc"
    ],
    hdrs = [
        "moface_calculator/moface_calculator.h",
        "moface_calculator/include/rapidjson/document.h",
        "moface_calculator/include/rapidjson/reader.h",
        "moface_calculator/include/rapidjson/allocators.h",
        "moface_calculator/include/rapidjson/rapidjson.h",
        "moface_calculator/include/rapidjson/stream.h",
        "moface_calculator/include/rapidjson/encodings.h",
        "moface_calculator/include/rapidjson/encodedstream.h",
        "moface_calculator/include/rapidjson/memorystream.h",
        "moface_calculator/include/rapidjson/writer.h",
        "moface_calculator/include/rapidjson/prettywriter.h",
        "moface_calculator/include/rapidjson/stringbuffer.h",
        "moface_calculator/include/rapidjson/internal/swap.h",
        "moface_calculator/include/rapidjson/internal/meta.h",
        "moface_calculator/include/rapidjson/internal/clzll.h",
        "moface_calculator/include/rapidjson/internal/stack.h",
        "moface_calculator/include/rapidjson/internal/strtod.h",
        "moface_calculator/include/rapidjson/internal/ieee754.h",
        "moface_calculator/include/rapidjson/internal/biginteger.h",
        "moface_calculator/include/rapidjson/internal/diyfp.h",
        "moface_calculator/include/rapidjson/internal/pow10.h",
        "moface_calculator/include/rapidjson/internal/strfunc.h",
        "moface_calculator/include/rapidjson/internal/dtoa.h",
        "moface_calculator/include/rapidjson/internal/itoa.h",
        "moface_calculator/include/rapidjson/error/error.h",
        "moface_calculator/include/face_observation_snapshot.h",
        "moface_calculator/include/moface_state.h",
        "moface_calculator/include/custum_uuid.h"
    ],
    includes = ["moface_calculator/include"],
    deps = [
        "//mediapipe/framework/formats:landmark_cc_proto",
        "//mediapipe/modules/face_geometry/protos:face_geometry_cc_proto",
        "//mediapipe/framework:calculator_framework",
        "//mediapipe/framework/formats:image_frame",
        "//mediapipe/framework/formats:image_frame_opencv",
        "//mediapipe/framework/port:file_helpers",
        "//mediapipe/framework/port:opencv_highgui",
        "//mediapipe/framework/port:opencv_imgproc",
        "//mediapipe/framework/port:opencv_video",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
    ]
)
