#include <math.h>
#include "describe_eyes.h"
#include "util_point.h"

//Left Eye
//For calculating center and area
constexpr int kLeftEyeLeftEdge = 353;
constexpr int kLeftEyeRightEdge = 413;
constexpr int kLeftEyeUpEdge = 443;
constexpr int kLeftEyeDownEdge = 451;
//For calculating EAR
constexpr int kLeftEyeVertial_1_UpEdge = 385;
constexpr int kLeftEyeVertial_1_DownEdge = 380;
constexpr int kLeftEyeVertial_2_UpEdge = 386;
constexpr int kLeftEyeVertial_2_DownEdge = 374;
constexpr int kLeftEyeHorzRightEdge = 362;
constexpr int kLeftEyeHorzLeftEdge = 263;

//Right Eye
//For calculating center and area
constexpr int kRightEyeLeftEdge = 189;
constexpr int kRightEyeRightEdge = 124;
constexpr int kRightEyeUpEdge = 223;
constexpr int kRightEyeDownEdge = 231;
//For calculating EAR
constexpr int kRightEyeVertial_1_UpEdge = 159;
constexpr int kRightEyeVertial_1_DownEdge = 145;
constexpr int kRightEyeVertial_2_UpEdge = 158;
constexpr int kRightEyeVertial_2_DownEdge = 153;
constexpr int kRightEyeHorzRightEdge = 33;
constexpr int kRightEyeHorzLeftEdge = 133;

constexpr double kEarThreshold = 0.35;


std::tuple<int, int> getLeftEyeCenter(
  ::mediapipe::NormalizedLandmarkList const& reference
) {
  int x = (reference.landmark(kLeftEyeLeftEdge).x() + reference.landmark(kLeftEyeRightEdge).x()) * 0.5;
  int y = (reference.landmark(kLeftEyeDownEdge).y() + reference.landmark(kLeftEyeUpEdge).y()) * 0.5;
  return std::tuple<int, int>(x, y);
}

// in : landmarks from midiapipe
// out : (x, y, width, height)
std::tuple<int, int, int, int> getLeftEyeArea(
  ::mediapipe::NormalizedLandmarkList const& reference
) {
  int x = reference.landmark(kLeftEyeRightEdge).x();
  int y = reference.landmark(kLeftEyeUpEdge).y();
  int width = reference.landmark(kLeftEyeLeftEdge).x() - x;
  int height = reference.landmark(kLeftEyeDownEdge).y() - y;
  return std::tuple<int, int, int, int>(x, y, width, height);
}

double calculateLeftEAR(const ::mediapipe::NormalizedLandmarkList &landmarks) {
  double vertical_size_1 = euclidian_distance(landmarks.landmark(kLeftEyeVertial_1_UpEdge), landmarks.landmark(kLeftEyeVertial_1_DownEdge));
  double vertical_size_2 = euclidian_distance(landmarks.landmark(kLeftEyeVertial_2_UpEdge), landmarks.landmark(kLeftEyeVertial_2_DownEdge));
  double horiozontal_size = euclidian_distance(landmarks.landmark(kLeftEyeHorzRightEdge), landmarks.landmark(kLeftEyeHorzLeftEdge));
  return (vertical_size_1 + vertical_size_2) / (2 * horiozontal_size);
}

std::tuple<int, int> getRightEyeCenter(
  ::mediapipe::NormalizedLandmarkList const& reference
) {
  int x = (reference.landmark(kRightEyeLeftEdge).x() + reference.landmark(kRightEyeRightEdge).x()) * 0.5;
  int y = (reference.landmark(kRightEyeDownEdge).y() + reference.landmark(kRightEyeUpEdge).y()) * 0.5;
  return std::tuple<int, int>(x, y);
}

// in : landmarks from midiapipe
// out : (x, y, width, height)
std::tuple<int, int, int, int> getRightEyeArea(
  ::mediapipe::NormalizedLandmarkList const& reference
) {
  int x = reference.landmark(kRightEyeRightEdge).x();
  int y = reference.landmark(kRightEyeUpEdge).y();
  int width = reference.landmark(kRightEyeLeftEdge).x() - x;
  int height = reference.landmark(kRightEyeDownEdge).y() - y;
  return std::tuple<int, int, int, int>(x, y, width, height);
}

double calculateRightEAR(const ::mediapipe::NormalizedLandmarkList &landmarks) {
  double vertical_size_1 = euclidian_distance(landmarks.landmark(kRightEyeVertial_1_UpEdge), landmarks.landmark(kRightEyeVertial_1_DownEdge));
  double vertical_size_2 = euclidian_distance(landmarks.landmark(kRightEyeVertial_2_UpEdge), landmarks.landmark(kRightEyeVertial_2_DownEdge));
  double horiozontal_size = euclidian_distance(landmarks.landmark(kRightEyeHorzRightEdge), landmarks.landmark(kRightEyeHorzLeftEdge));
  return (vertical_size_1 + vertical_size_2) / (2 * horiozontal_size);
}

void addBlinkToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::tuple<int, int> slice,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
) {
    if (!face_observation->objectCount()) {
    moface::ObservationObject *new_object = new moface::ObservationObject("face");
    face_observation->addObject(*new_object);
    }
    std::tuple<int, int, int, int> left_eye_area = getLeftEyeArea(reference);
    moface::ObservationAction *left_eye_action =
      new moface::ObservationAction(
        "push",
        (double)std::get<0>(left_eye_area),
        (double)std::get<1>(left_eye_area),
        (double)std::get<2>(left_eye_area),
        (double)std::get<3>(left_eye_area)
      );
    std::tuple<int, int> left_eye_center = getLeftEyeCenter(reference);
    for (auto snapshot : snapshot_array) {
      moface::ObservationFeed *new_feed =
        new moface::ObservationFeed(
          snapshot.frame_id / 30.0, snapshot.pitch, snapshot.yaw, snapshot.roll
        );
      moface::ObservationTrackedPosition *new_track =
        new moface::ObservationTrackedPosition(
          (double)std::get<0>(left_eye_center), (double)std::get<1>(left_eye_center), 0.0
        );
      new_feed->addTrackedPosition(*new_track);
      left_eye_action->addFeed(*new_feed);
    }
    face_observation->lastObject().addAction(*left_eye_action);

    std::tuple<int, int, int, int> right_eye_area = getRightEyeArea(reference);
    moface::ObservationAction *right_eye_action =
      new moface::ObservationAction(
        "push",
        (double)std::get<0>(right_eye_area),
        (double)std::get<1>(right_eye_area),
        (double)std::get<2>(right_eye_area),
        (double)std::get<3>(right_eye_area)
      );
    std::tuple<int, int> right_eye_center = getRightEyeCenter(reference);
    for (auto snapshot : snapshot_array) {
      moface::ObservationFeed *new_feed =
        new moface::ObservationFeed(
          snapshot.frame_id / 30.0, snapshot.pitch, snapshot.yaw, snapshot.roll
        );
      moface::ObservationTrackedPosition *new_track =
        new moface::ObservationTrackedPosition(
          (double)std::get<0>(right_eye_center), (double)std::get<1>(right_eye_center), 0.0
        );
      new_feed->addTrackedPosition(*new_track);
      right_eye_action->addFeed(*new_feed);
    }
    face_observation->lastObject().addAction(*right_eye_action);
}

bool checkBlinkActionAndAddToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
) {
  double reference_ear = (calculateLeftEAR(reference) + calculateRightEAR(reference)) * 0.5;
  std::vector<double> ear_history;
  std::vector<std::tuple<int, int>> slices;
  for (int i = 0; i < snapshot_array.size(); i ++) {
    double ear = (calculateLeftEAR(snapshot_array[i].landmarks) + calculateRightEAR(snapshot_array[i].landmarks)) * 0.5;
    ear_history.push_back(ear);
    if (ear < kEarThreshold && i > 10) {
      slices.push_back(std::make_tuple(i, i - 10));
      for (int j = i - 1; j >= 0; j --) {
        if (ear_history[j] > reference_ear) {
          slices.push_back(std::make_tuple(i, j));
          break;
        }
      }
    }
  }
  if (slices.empty()) {
    return false;
  }
  addBlinkToFaceObservation(reference, slices[0], snapshot_array, face_observation);
  return true;
}