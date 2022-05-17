
#include <cstdlib>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "include/face_observation_snapshot.h"

constexpr char kInputStream[] = "input_video";
constexpr char kOutputStream[] = "output_video";
constexpr char kOutputFaceLandmarkStream[] = "multi_face_landmarks";
constexpr char kOutputFaceGeometryStream[] = "multi_face_geometry";
constexpr char kWindowName[] = "MediaPipe";

constexpr char kDetectedReferenceFramePath[] = "reference";
constexpr char kDetectedFaceObservationPath[] = "face-observation";

ABSL_FLAG(std::string, calculator_graph_config_file, "",
          "Name of file containing text format CalculatorGraphConfig proto.");
ABSL_FLAG(std::string, input_video_path, "",
          "Full path of video to load. "
          "If not provided, attempt to use a webcam.");
ABSL_FLAG(std::string, output_video_path, "",
          "Full path of where to save result (.mp4 only). "
          "If not provided, show result in a window.");

absl::Status RunMPPGraph() {
  std::string prev_state, cur_state;
  std::vector<moface::FaceObservationSnapShot> face_observation_snapshot_array;
  std::vector<::mediapipe::NormalizedLandmarkList> reference_landmark;
  rapidjson::Document face_observation_object;

  //Objects for displaying
  cv::Point2f nose_point;
  std::string pitch_text, roll_text, yaw_text, state_text, notification_text;

  std::string calculator_graph_config_contents;
  MP_RETURN_IF_ERROR(mediapipe::file::GetContents(
      absl::GetFlag(FLAGS_calculator_graph_config_file),
      &calculator_graph_config_contents));
  LOG(INFO) << "Get calculator graph config contents: "
            << calculator_graph_config_contents;
  mediapipe::CalculatorGraphConfig config =
      mediapipe::ParseTextProtoOrDie<mediapipe::CalculatorGraphConfig>(
          calculator_graph_config_contents);

  LOG(INFO) << "Initialize the calculator graph.";
  mediapipe::CalculatorGraph graph;
  MP_RETURN_IF_ERROR(graph.Initialize(config));

  LOG(INFO) << "Initialize the camera or load the video.";
  cv::VideoCapture capture;
  const bool load_video = !absl::GetFlag(FLAGS_input_video_path).empty();
  if (load_video) {
    capture.open(absl::GetFlag(FLAGS_input_video_path));
  } else {
    capture.open(0);
  }
  RET_CHECK(capture.isOpened());

  cv::VideoWriter writer;
  const bool save_video = !absl::GetFlag(FLAGS_output_video_path).empty();
  cv::namedWindow(kWindowName, /*flags=WINDOW_AUTOSIZE*/ 1);
#if (CV_MAJOR_VERSION >= 3) && (CV_MINOR_VERSION >= 2)
  capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
  capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
  capture.set(cv::CAP_PROP_FPS, 30);
#endif

  LOG(INFO) << "Start running the calculator graph.";
  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller poller,
                   graph.AddOutputStreamPoller(kOutputStream));
  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller landmark_poller,
                   graph.AddOutputStreamPoller(kOutputFaceLandmarkStream));
  ASSIGN_OR_RETURN(mediapipe::OutputStreamPoller geometry_poller,
                   graph.AddOutputStreamPoller(kOutputFaceGeometryStream));

  MP_RETURN_IF_ERROR(graph.StartRun({}));

  LOG(INFO) << "Start grabbing and processing frames.";
  bool grab_frames = true;
  while (grab_frames) {
    // Capture opencv camera or video frame.
    cv::Mat camera_frame_raw;
    capture >> camera_frame_raw;
    if (camera_frame_raw.empty()) {
      if (!load_video) {
        LOG(INFO) << "Ignore empty frames from camera.";
        continue;
      }
      LOG(INFO) << "Empty frame, end of video reached.";
      break;
    }
    cv::Mat camera_frame;
    cv::cvtColor(camera_frame_raw, camera_frame, cv::COLOR_BGR2RGB);
    if (!load_video) {
      cv::flip(camera_frame, camera_frame, /*flipcode=HORIZONTAL*/ 1);
    }

    // Wrap Mat into an ImageFrame.
    auto input_frame = absl::make_unique<mediapipe::ImageFrame>(
        mediapipe::ImageFormat::SRGB, camera_frame.cols, camera_frame.rows,
        mediapipe::ImageFrame::kDefaultAlignmentBoundary);
    cv::Mat input_frame_mat = mediapipe::formats::MatView(input_frame.get());
    camera_frame.copyTo(input_frame_mat);

    // Send image packet into the graph.
    size_t frame_timestamp_us =
        (double)cv::getTickCount() / (double)cv::getTickFrequency() * 1e6;
    MP_RETURN_IF_ERROR(graph.AddPacketToInputStream(
        kInputStream, mediapipe::Adopt(input_frame.release())
                          .At(mediapipe::Timestamp(frame_timestamp_us))));

    //Get the graph result packet for output stream, or stop if that fails.
    mediapipe::Packet packet;
    if (!poller.Next(&packet)) {
      LOG(INFO) << "No output video packet";
      break;
    }
    auto& output_frame = packet.Get<mediapipe::ImageFrame>();

    //Get the graph result packet for landmark stream
    mediapipe::Packet landmark_packet;
    if (!landmark_poller.Next(&landmark_packet)) {
      LOG(INFO) << "No landamrk packet";
      break;
    } else {
      //LOG(INFO) << "landamrk packet captured";
      // std::string output_data;
      // absl::StrAppend(&output_data, landmark_packet.Timestamp().Value(), ",");
    }
    const auto& multi_face_landmarks = landmark_packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
    LOG(INFO) << "[" << landmark_packet.Timestamp().Value() << "] Number of face instances with landmark : " << multi_face_landmarks.size();
    for (int face_index = 0; face_index < multi_face_landmarks.size(); ++face_index) {
      const auto& landmarks = multi_face_landmarks[face_index];
      LOG(INFO) << "Number of landmarks for face[" << face_index << "] size : " << landmarks.landmark_size();
      for (int i = 0; i < landmarks.landmark_size(); ++i) {
        LOG(INFO) << "\tLandmark[" << i << "] (" << landmarks.landmark(i).x() << ", " << landmarks.landmark(i).y() << ", " << landmarks.landmark(i).z() << ")";
      }
    }

    //Get the graph result packet for geometry sstream
    mediapipe::Packet geometry_packet;
    if (!geometry_poller.Next(&geometry_packet)) {
        LOG(INFO) << "No geometry packet";
    }
    const auto& multi_face_geometry = geometry_packet.Get<std::vector<::mediapipe::face_geometry::FaceGeometry>>();
    LOG(INFO) << "[" << geometry_packet.Timestamp().Value() << "] Number of face instances with face geometry : " << multi_face_geometry.size();
    for (int faceIndex = 0; faceIndex < multi_face_geometry.size(); ++faceIndex) {
      const auto& faceGeometry = multi_face_geometry[faceIndex];
      printf("\tApprox. distance away from camera for face[%d]: %.6f cm]\n", faceIndex,
            -faceGeometry.pose_transform_matrix().packed_data(14));


      float pitch = asin(faceGeometry.pose_transform_matrix().packed_data(6));
      float yaw, roll;
      if (cos(pitch) > 0.0001) {
          yaw = atan2(faceGeometry.pose_transform_matrix().packed_data(2), faceGeometry.pose_transform_matrix().packed_data(10));
          roll = atan2(faceGeometry.pose_transform_matrix().packed_data(4), faceGeometry.pose_transform_matrix().packed_data(5));
      } else {
          yaw = 0.0;
          roll = atan2(-faceGeometry.pose_transform_matrix().packed_data(1), faceGeometry.pose_transform_matrix().packed_data(0));
      }

      pitch = fmod(((pitch * 180.0 / M_PI) + 360.0), 360.0);
      yaw = fmod(((yaw * 180.0 / M_PI) + 360.0), 360);
      roll = fmod(((roll * 180.0 / M_PI) + 360.0), 360);

      LOG(INFO) << "pitch : " << pitch << ", yaw : " << yaw << ", roll : " << roll << ", distance : " << -faceGeometry.pose_transform_matrix().packed_data(14);
    }

    //Convert back to opencv for display or saving.
    cv::Mat output_frame_mat = mediapipe::formats::MatView(&output_frame);
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
    if (save_video) {
      if (!writer.isOpened()) {
        LOG(INFO) << "Prepare video writer.";
        writer.open(absl::GetFlag(FLAGS_output_video_path),
                    mediapipe::fourcc('a', 'v', 'c', '1'),  // .mp4
                    capture.get(cv::CAP_PROP_FPS), output_frame_mat.size());
        RET_CHECK(writer.isOpened());
      }
      writer.write(output_frame_mat);
    } else {
      cv::imshow(kWindowName, output_frame_mat);
      // Press any key to exit.
      const int pressed_key = cv::waitKey(5);
      if (pressed_key >= 0 && pressed_key != 255) grab_frames = false;
    }
  }

  LOG(INFO) << "Shutting down.";
  if (writer.isOpened()) writer.release();
  MP_RETURN_IF_ERROR(graph.CloseInputStream(kInputStream));
  return graph.WaitUntilDone();
}

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  absl::ParseCommandLine(argc, argv);
  absl::Status run_status = RunMPPGraph();
  if (!run_status.ok()) {
    LOG(ERROR) << "Failed to run the graph: " << run_status.message();
    return EXIT_FAILURE;
  } else {
    LOG(INFO) << "Success!";
  }
  return EXIT_SUCCESS;
}
