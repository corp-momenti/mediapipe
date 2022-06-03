
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <iterator>

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
#include "moface_desktop/moface_calculator/moface_calculator.h"
#include "moface_desktop/moface_calculator/include/custum_uuid.h"  //todo get it out of moface calculator

constexpr char kInputStream[] = "input_video";
constexpr char kOutputStream[] = "output_video";
constexpr char kOutputFaceLandmarkStream[] = "multi_face_landmarks";
constexpr char kOutputFaceGeometryStream[] = "multi_face_geometry";
constexpr char kWindowName[] = "MediaPipe";

constexpr char kDetectedReferenceFramePath[] = "./mediapipe/examples/desktop/moface_desktop/reference";
constexpr char kDetectedFaceObservationPath[] = "./mediapipe/examples/desktop/moface_desktop/face-observation";
constexpr char kOuputVideoPath[] = "./mediapipe/examples/desktop/moface_desktop/output-video";

ABSL_FLAG(std::string, calculator_graph_config_file, "",
          "Name of file containing text format CalculatorGraphConfig proto.");
ABSL_FLAG(std::string, input_video_path, "",
          "Full path of video to load. "
          "If not provided, attempt to use a webcam.");
ABSL_FLAG(std::string, output_video_path, "",
          "Full path of where to save result (.mp4 only). "
          "If not provided, show result in a window.");

void showDebugInfo(
  std::string pitch, std::string yaw, std::string roll, std::string dist,
  std::string state,
  cv::Point2f nose,
  cv::Mat output_frame_mat
) {
    int text_left_align_pos = output_frame_mat.cols / 2;
    cv::putText(
      output_frame_mat,
      pitch,
      cv::Point(text_left_align_pos, 20), //cv::Point(10, output_frame_mat.rows / 2), //top-left position
      cv::FONT_HERSHEY_DUPLEX,
      0.5,
      CV_RGB(255, 0, 0),
      2
    );
    cv::putText(
      output_frame_mat,
      yaw,
      cv::Point(text_left_align_pos, 40),
      cv::FONT_HERSHEY_DUPLEX,
      0.5,
      CV_RGB(0, 255, 0),
      2
    );
    cv::putText(
      output_frame_mat,
      roll,
      cv::Point(text_left_align_pos, 60),
      cv::FONT_HERSHEY_DUPLEX,
      0.5,
      CV_RGB(0, 0, 255),
      2
    );
    cv::putText(
      output_frame_mat,
      dist,
      cv::Point(text_left_align_pos, 80),
      cv::FONT_HERSHEY_DUPLEX,
      0.5,
      CV_RGB(255, 0, 0),
      2
    );
    cv::putText(
      output_frame_mat,
      state,
      cv::Point(text_left_align_pos, 100),
      cv::FONT_HERSHEY_DUPLEX,
      0.5,
      CV_RGB(255, 0, 0),
      2
    );
}

void showDebugInfo2(
  std::string distance, std::string pose, std::string in_frame, std::string state,
  std::string left_drag, std::string right_drag, std::string up_drag, std::string down_drag,
  std::string blink, std::string angry, std::string happy,
  cv::Mat output_frame_mat
) {
  //signals & state
  int text_left_align_pos = output_frame_mat.cols / 2;
  int actions_left_align_pos = 10;
  cv::putText(
    output_frame_mat,
    distance,
    cv::Point(text_left_align_pos, 20), //cv::Point(10, output_frame_mat.rows / 2), //top-left position
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(255, 0, 0),
    2
  );
  cv::putText(
    output_frame_mat,
    pose,
    cv::Point(text_left_align_pos, 40),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(0, 255, 0),
    2
  );
  cv::putText(
    output_frame_mat,
    in_frame,
    cv::Point(text_left_align_pos, 60),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(0, 0, 255),
    2
  );
  cv::putText(
    output_frame_mat,
    state,
    cv::Point(text_left_align_pos, 80),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(255, 0, 0),
    2
  );

  //actions
  cv::putText(
    output_frame_mat,
    left_drag,
    cv::Point(actions_left_align_pos, 10),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(255, 0, 0),
    2
  );
  cv::putText(
    output_frame_mat,
    right_drag,
    cv::Point(actions_left_align_pos, 30),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(255, 0, 0),
    2
  );
  cv::putText(
    output_frame_mat,
    up_drag,
    cv::Point(actions_left_align_pos, 50),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(255, 0, 0),
    2
  );
  cv::putText(
    output_frame_mat,
    down_drag,
    cv::Point(actions_left_align_pos, 70),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(255, 0, 0),
    2
  );
  cv::putText(
    output_frame_mat,
    blink,
    cv::Point(actions_left_align_pos, 90),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(255, 0, 0),
    2
  );
  cv::putText(
    output_frame_mat,
    angry,
    cv::Point(actions_left_align_pos, 110),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(255, 0, 0),
    2
  );
  cv::putText(
    output_frame_mat,
    happy,
    cv::Point(actions_left_align_pos, 130),
    cv::FONT_HERSHEY_DUPLEX,
    0.5,
    CV_RGB(255, 0, 0),
    2
  );
}

//Objects for displaying
cv::Mat output_frame_mat;
cv::Point2f nose_point;
std::string pitch_text, roll_text, yaw_text, distance_text;
std::string distance_status_text, pose_status_text, within_frame_text;
std::string state_text = "init";
std::string left_drag_count_text = "LD : ";
int left_drag_count = 0;
std::string right_drag_count_text = "RD : ";
int right_drag_count = 0;
std::string up_drag_count_text = "UD : ";
int up_drag_count = 0;
std::string down_drag_count_text = "DD : ";
int down_drag_count = 0;
std::string blink_count_text = "BLK : ";
int blink_count = 0;
std::string happy_count_text = "HPY : ";
int happy_count = 0;
std::string angry_count_text = "ANG : ";
int angry_count = 0;

bool left_drag_captured = false;
bool right_drag_captured = false;
bool up_drag_captured = false;
bool down_drag_captured = false;
bool blink_captured = false;
bool angry_captured = false;
bool happy_captured = false;

void eventNotifier(moface::MoFaceEventType event) {
  switch (event) {
    case moface::eRightActionDetected:
      right_drag_count ++;
      right_drag_count_text = "RD : " + std::to_string(right_drag_count);
      right_drag_captured = true;
    break;
    case moface::eLeftActionDetected:
      left_drag_count ++;
      left_drag_count_text = "LD : " + std::to_string(left_drag_count);
      left_drag_captured = true;
    break;
    case moface::eUpActionDetected:
      up_drag_count ++;
      up_drag_count_text = "UD : " + std::to_string(up_drag_count);
      up_drag_captured = true;
    break;
    case moface::eDownActionDetected:
      down_drag_count ++;
      down_drag_count_text = "DD : " + std::to_string(down_drag_count);
      down_drag_captured = true;
    break;
    case moface::eBlinkActionDetected:
      blink_count ++;
      blink_count_text = "BLK : " + std::to_string(blink_count);
      blink_captured = true;
    break;
    case moface::eMouthActionDetected:
    break;
    case moface::eAngryActionDetected:
      angry_count ++;
      angry_count_text = "ANG : " + std::to_string(angry_count);
      angry_captured = true;
    break;
    case moface::eHappyActionDetected:
      happy_count ++;
      happy_count_text = "HPY : " + std::to_string(happy_count);
      happy_captured = true;
    break;
    case moface::eReferenceDetected:
      std::string tmp_file_name = std::string(kDetectedReferenceFramePath) + "/" + moface::generate_uuid_v4() + ".png";
      cv::imwrite(tmp_file_name, output_frame_mat);
    break;
  }
}

void warningNotifier(moface::MoFaceWarningType event) {
  //showNotification
  std::string event_text;
  switch (event) {
    case moface::eGoingBackward:
      event_text = "Face Going Backward";
    break;
    case moface::eTooSlow:
      event_text = "Face Too Slow";
    break;
    case moface::eInvalidDragAction:
      event_text = "Invalidd Drag Action";
    break;
    case moface::eNoFace:
      event_text = "No Face";
    break;
    case moface::eTimeout:
      event_text = "Timeout";
    break;
  }
}

void signalNotifier(moface::MofaceDistanceStatus distance, moface::MofacePoseStatus pose, bool inFrame, cv::Point center) {
  //show signals
  switch (distance) {
    case moface::eGoodDistance:
      distance_status_text = "Good Distance";
      break;
    case moface::eTooFar:
      distance_status_text = "TooFar";
      break;
    case moface::eTooClose:
      distance_status_text = "TooClose";
      break;
  }
  switch (pose) {
    case moface::eHoldStill:
      pose_status_text = "Hold Still";
      break;
    case moface::eMoving:
      pose_status_text = "Moving";
      break;
  }
  within_frame_text = inFrame ? "Within Frame" : "Not Within Frame";
}

void geometryNotifier(double pitch, double yaw, double roll, double distance) {
  pitch_text = std::to_string(pitch);
  yaw_text = std::to_string(yaw);
  roll_text = std::to_string(roll);
  distance_text = std::to_string(distance);
}

absl::Status RunMPPGraph() {

  moface::MofaceCalculator *moface_calculator = new moface::MofaceCalculator(
    eventNotifier, signalNotifier, warningNotifier, geometryNotifier
  );

  std::string calculator_graph_config_contents;

  std::filesystem::remove_all(kDetectedReferenceFramePath);
  std::filesystem::remove_all(kDetectedFaceObservationPath);
  std::filesystem::remove_all(kOuputVideoPath);

  std::filesystem::create_directories(kDetectedReferenceFramePath);
  std::filesystem::create_directories(kDetectedFaceObservationPath);
  std::filesystem::create_directories(kOuputVideoPath);

  std::string vido_out_file = std::string(kOuputVideoPath) + "/" + moface::generate_uuid_v4() + ".mp4";

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
    if (
      left_drag_captured &&
      right_drag_captured &&
      up_drag_captured &&
      down_drag_captured &&
      blink_captured &&
      angry_captured &&
      happy_captured
    ) {
      std::cout << "All actions are captured!!!!";
      break;
    }
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
    }
    const auto& multi_face_landmarks = landmark_packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();

    //Get the graph result packet for geometry sstream
    mediapipe::Packet geometry_packet;
    if (!geometry_poller.Next(&geometry_packet)) {
        LOG(INFO) << "No geometry packet";
    }
    const auto& multi_face_geometry = geometry_packet.Get<std::vector<::mediapipe::face_geometry::FaceGeometry>>();

    moface_calculator->sendObservations(multi_face_landmarks[0], multi_face_geometry[0]);

    //Convert back to opencv for display or saving.
    output_frame_mat = camera_frame;//mediapipe::formats::MatView(&output_frame);
    cv::cvtColor(output_frame_mat, output_frame_mat, cv::COLOR_RGB2BGR);
    if (!writer.isOpened()) {
        LOG(INFO) << "Prepare video writer.";
        writer.open(vido_out_file,
                    mediapipe::fourcc('a', 'v', 'c', '1'),  // .mp4
                    capture.get(cv::CAP_PROP_FPS), output_frame_mat.size());
        RET_CHECK(writer.isOpened());
    }
    writer.write(output_frame_mat);
    state_text = moface_calculator->curState();
    showDebugInfo2(
      distance_status_text, pose_status_text, within_frame_text, state_text,
      left_drag_count_text, right_drag_count_text, up_drag_count_text, down_drag_count_text,
      blink_count_text, angry_count_text, happy_count_text,
      output_frame_mat
    );
    // showDebugInfo(
    //   pitch_text, yaw_text, roll_text, distance_text,
    //   state_text, notification_text, nose_point,
    //   output_frame_mat
    // );
    cv::imshow(kWindowName, output_frame_mat);
    // Press any key to exit.
    const int pressed_key = cv::waitKey(5);
    if (pressed_key >= 0 && pressed_key != 255) grab_frames = false;

  }

  moface_calculator->updateMediaFilePath(vido_out_file);
  std::filesystem::path path {std::string(kDetectedFaceObservationPath) + "/" + moface::generate_uuid_v4() + ".json"};
  std::ofstream ofs(path);
  ofs << moface_calculator->getFaceObservation();
  ofs.close();

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
