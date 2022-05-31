#include "describe_mouth.h"
#include "util_point.h"

constexpr int kUpperLipUpperEdge_1 = 38;
constexpr int kUpperLipUpperEdge_2 = 12;
constexpr int kUpperLipUpperEdge_3 = 268;

constexpr int kUpperLipLowerEdge_1 = 82;
constexpr int kUpperLipLowerEdge_2 = 13;
constexpr int kUpperLipLowerEdge_3 = 312;

constexpr int kLowerLipUpperEdge_1 = 87;
constexpr int kLowerLipUpperEdge_2 = 14;
constexpr int kLowerLipUpperEdge_3 = 317;

constexpr int kRigthEdgeForAngry = 167;
constexpr int kLeftEdgeForAngry = 393;
constexpr int kUpperEdgeForAngry = 164;
constexpr int kLowerEdgeForAngry = 18;

constexpr int kUpperEdgeStartForAngry = 0;
constexpr int kLowerEdgeStartForAngry = 17;
constexpr int kUpperEdgeEndForAngry = 164;
constexpr int kLowerEdgeEndForAngry = 18;

constexpr int kRightEdge = 61;
constexpr int kLeftEdge = 291;

constexpr int kRigthEdgeForHappy = 216;
constexpr int kLeftEdgeForHappy = 436;
constexpr int kUpperEdgeForHappy = 216;
constexpr int kLowerEdgeForHappy = 202;

constexpr int kRightEdgeStartForHappy = 61;
constexpr int kLeftEdgeStartForHappy = 291;
constexpr int kRightEdgeEndForHappy = 214;
constexpr int kLeftEdgeEndForHappy = 434;

constexpr double kMHARTheshold = 0.5;
constexpr int kNumOfObservationsToCheck = 10;
constexpr double kMWARTheshold = 1.3;
constexpr double kMWARReferece = 1.0;

//utils for mouth
double calculateMHAR(
  ::mediapipe::NormalizedLandmarkList const& landmarks
) {
  //upper lip upper points 38, 12, 268
  //upper lip lower points 82, 13, 312
  double lip_height_1 = euclidian_distance(landmarks.landmark(kUpperLipUpperEdge_1), landmarks.landmark(kUpperLipLowerEdge_1));
  double lip_height_2 = euclidian_distance(landmarks.landmark(kUpperLipUpperEdge_2), landmarks.landmark(kUpperLipLowerEdge_2));
  double lip_height_3 = euclidian_distance(landmarks.landmark(kUpperLipUpperEdge_3), landmarks.landmark(kUpperLipLowerEdge_3));
  double lip_height = (lip_height_1 + lip_height_2 + lip_height_3) / 3;

  //upper lip lower points 82, 13, 312
  //lower lip upper points 87, 14, 317
  double mouth_height_1 = euclidian_distance(landmarks.landmark(kUpperLipLowerEdge_1), landmarks.landmark(kLowerLipUpperEdge_1));
  double mouth_height_2 = euclidian_distance(landmarks.landmark(kUpperLipLowerEdge_2), landmarks.landmark(kLowerLipUpperEdge_2));
  double mouth_height_3 = euclidian_distance(landmarks.landmark(kUpperLipLowerEdge_3), landmarks.landmark(kLowerLipUpperEdge_3));
  double mouth_height = (mouth_height_1 + mouth_height_2 + mouth_height_3) / 3;

  return mouth_height / lip_height;
}

void addAngryActionToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::tuple<int, int> slice,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
) {
    if (!face_observation->objectCount()) {
        moface::ObservationObject *new_object = new moface::ObservationObject("face");
        face_observation->addObject(*new_object);
    }
    //get the bounding box for mouth
    //width : 167 -> 393, height : 164 -> 18
    moface::ObservationAction *new_action =
      new moface::ObservationAction("pinch",
        reference.landmark(kRigthEdgeForAngry).x(),
        reference.landmark(kUpperEdgeForAngry).y(),
        reference.landmark(kLeftEdgeForAngry).x() - reference.landmark(kRigthEdgeForAngry).x(),
        reference.landmark(kLowerEdgeForAngry).y() - reference.landmark(kUpperEdgeForAngry).y()
    );
    for (int i = std::get<0>(slice); i <= std::get<1>(slice); i ++) {
      auto snapshot = snapshot_array[i];
      moface::ObservationFeed *new_feed =
        new moface::ObservationFeed(snapshot.frame_id / 30.0, snapshot.pitch, snapshot.yaw, snapshot.roll);
      double up_x, up_y, down_x, down_y;
      if (i == std::get<0>(slice)) {
        //0, 17
        up_x = reference.landmark(kUpperEdgeStartForAngry).x();
        up_y = reference.landmark(kUpperEdgeStartForAngry).y();
        down_x = reference.landmark(kLowerEdgeStartForAngry).x();
        down_y = reference.landmark(kLowerEdgeStartForAngry).y();
      } else if (i == std::get<1>(slice)) {
        //164, 18
        up_x = reference.landmark(kUpperEdgeForAngry).x();
        up_y = reference.landmark(kUpperEdgeForAngry).y();
        down_x = reference.landmark(kLowerEdgeForAngry).x();
        down_y = reference.landmark(kLowerEdgeForAngry).y();
      } else {
        up_x = 0.0;
        up_y = 0.0;
        down_x = 0.0;
        down_y = 0.0;
      }
      moface::ObservationTrackedPosition *new_track_for_up = new moface::ObservationTrackedPosition(up_x, up_y, 0.0);
      new_feed->addTrackedPosition(*new_track_for_up);
      moface::ObservationTrackedPosition *new_track_for_down = new moface::ObservationTrackedPosition(down_x, down_y, 0.0);
      new_feed->addTrackedPosition(*new_track_for_down);
      new_action->addFeed(*new_feed);
    }
    face_observation->lastObject().addAction(*new_action);
    return;
}

bool checkAngryActionAndAddToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
) {
  double reference_mar = calculateMHAR(reference);
  std::vector<double> mar_history;
  std::vector<std::tuple<int, int>> slices;
    for (int i = 0; i < snapshot_array.size(); i ++) {
    double mar = calculateMHAR(snapshot_array[i].landmarks);
    mar_history.push_back(mar);
    if (mar > kMHARTheshold && i > kNumOfObservationsToCheck) {
      for (int j = i - 1; j >= 0; j --) {
        if (mar_history[j] < reference_mar) {
          slices.push_back(std::make_tuple(i, j));
          break;
        }
      }
    }
  }
  if (slices.empty()) {
    return false;
  }
  addAngryActionToFaceObservation(reference, slices[0], snapshot_array, face_observation);
  return false;
}

double calculateMWAR(
  ::mediapipe::NormalizedLandmarkList const& reference,
  ::mediapipe::NormalizedLandmarkList const& landmarks
) {
  //upper lip upper points 38, 12, 268
  //upper lip lower points 82, 13, 312
  double lip_height_1 = euclidian_distance(landmarks.landmark(kUpperLipUpperEdge_1), landmarks.landmark(kUpperLipLowerEdge_1));
  double lip_height_2 = euclidian_distance(landmarks.landmark(kUpperLipUpperEdge_2), landmarks.landmark(kUpperLipLowerEdge_2));
  double lip_height_3 = euclidian_distance(landmarks.landmark(kUpperLipUpperEdge_3), landmarks.landmark(kUpperLipLowerEdge_3));
  double lip_height = (lip_height_1 + lip_height_2 + lip_height_3) / 3;

  //right mark : 61, left mark : 291
  double mouth_width = euclidian_distance(landmarks.landmark(kRightEdge), landmarks.landmark(kLeftEdge));
  return mouth_width / lip_height;
}

void addHappyActionToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::tuple<int, int> slice,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
) {
    if (!face_observation->objectCount()) {
      moface::ObservationObject *new_object = new moface::ObservationObject("face");
      face_observation->addObject(*new_object);
    }
    //get the bounding box for mouth
    //width : 216 -> 436, height : 216 -> 202
    moface::ObservationAction *new_action =
      new moface::ObservationAction("spread",
        reference.landmark(kRigthEdgeForHappy).x(),
        reference.landmark(kRigthEdgeForHappy).y(),
        reference.landmark(kRigthEdgeForHappy).x() - reference.landmark(kLeftEdgeForHappy).x(),
        reference.landmark(kRigthEdgeForHappy).y() - reference.landmark(kLowerEdgeForHappy).y()
    );
    for (int i = std::get<0>(slice); i <= std::get<1>(slice); i ++) {
      auto snapshot = snapshot_array[i];
      moface::ObservationFeed *new_feed =
        new moface::ObservationFeed(snapshot.frame_id / 30.0, snapshot.pitch, snapshot.yaw, snapshot.roll);
      double right_x, right_y, left_x, left_y;
      if (i == std::get<0>(slice)) {
        //61, 291
        right_x = reference.landmark(kRightEdgeStartForHappy).x();
        right_y = reference.landmark(kRightEdgeStartForHappy).y();
        left_x = reference.landmark(kLeftEdgeStartForHappy).x();
        left_y = reference.landmark(kLeftEdgeStartForHappy).y();
      } else if (i == std::get<1>(slice)) {
        //214, 434
        right_x = reference.landmark(kRightEdgeEndForHappy).x();
        right_y = reference.landmark(kRightEdgeEndForHappy).y();
        left_x = reference.landmark(kLeftEdgeEndForHappy).x();
        left_y = reference.landmark(kLeftEdgeEndForHappy).y();
      } else {
        right_x = 0.0;
        right_y = 0.0;
        left_x = 0.0;
        left_y = 0.0;
      }
      moface::ObservationTrackedPosition *new_track_for_right = new moface::ObservationTrackedPosition(right_x, right_y, 0.0);
      new_feed->addTrackedPosition(*new_track_for_right);
      moface::ObservationTrackedPosition *new_track_for_left = new moface::ObservationTrackedPosition(left_x, left_y, 0.0);
      new_feed->addTrackedPosition(*new_track_for_left);
      new_action->addFeed(*new_feed);
    }
    face_observation->lastObject().addAction(*new_action);
    return;

}

bool checkHanppyActionAndAddToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
) {
  std::vector<double> mar_history;
  std::vector<std::tuple<int, int>> slices;
    for (int i = 0; i < snapshot_array.size(); i ++) {
    double mar2ref = calculateMWAR(reference, snapshot_array[i].landmarks);
    mar_history.push_back(mar2ref);
    if (mar2ref > kMWARTheshold && i > kNumOfObservationsToCheck) {
      for (int j = i - 1; j >= 0; j --) {
        if (mar_history[j] <= kMWARReferece) {
          slices.push_back(std::make_tuple(i, j));
          break;
        }
      }
    }
  }
  if (slices.empty()) {
    return false;
  }
  addHappyActionToFaceObservation(reference, slices[0], snapshot_array, face_observation);
  return false;
}