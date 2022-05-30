#include <cstdlib>
#include <map>
#include <tuple>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <math.h>
#include "describe_drag.h"

//Drag Action Limit
constexpr double kDragEndingLimit = 20.0;

//Drag Backward Limit
constexpr double kDragBackwardLimit = 3.0;

//Drag Slow Limit
constexpr double kDragSlowLimit = 7.0; // degree per sec

//Drag Right Limit
constexpr double kDragOutOfRangeLimit = 5.0;

//Reference Point
//  Drag Left : kRightChinIndex -> kLeftChinIndex
//  Drag Right : kLeftChinIndex -> kRightChinIndex
//  Drag Up   : kUnderMouthIndex -> kNoseTipIndex
//  Drag Down : kForeheadIndex -> kNoseTipIndex
constexpr int kLeftChinIndex = 352;
constexpr int kRightChinIndex = 123;
constexpr int kForeheadIndex = 151;
constexpr int kUnderMouthIndex = 199;
constexpr int kNoseTipIndex = 19;

//utils for drag
bool hitDragLimit(
  double pitch,
  double yaw,
  double roll
) {
    if ( (yaw >= kDragEndingLimit && yaw <= 180.0) ||
        (yaw >= 180.0 && yaw <= 360.0 - kDragEndingLimit)
    ) {
      std::cout << "angle hit : " << yaw;
      return true;
    }
  return false;
}

bool dragGoingBackward(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  size_t from,
  size_t to
) {
  bool ret = false;
  moface::FaceObservationSnapShot last_item = snapshot_array[from];
  moface::FaceObservationSnapShot cmp_item = snapshot_array[to];
  if (abs(last_item.pitch - 180.0) >= abs(cmp_item.pitch - 180.0) + kDragBackwardLimit) {
    return true;
  }

  if (abs(last_item.yaw - 180.0) >= abs(cmp_item.yaw - 180.0) + kDragBackwardLimit) {
    return true;
  }
  return ret;
}

bool dragGoingBackward(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
  size_t length = snapshot_array.size();
  return dragGoingBackward(snapshot_array, length - 1, length - 2);
}


bool dragTooSlow(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  int from,
  int to
) {
  moface::FaceObservationSnapShot last_item = snapshot_array[from];
  moface::FaceObservationSnapShot cmp_item = snapshot_array[to];
  double time_elapsed = abs(from - to) / 30.0;
  double angle_distance = sqrt(
    pow(last_item.pitch - cmp_item.pitch,2) +
    pow(last_item.yaw - cmp_item.yaw,2) /*+
    pow(last_item.roll - cmp_item.roll,2)*/
  );
  if (kDragSlowLimit >= (angle_distance / time_elapsed)) {
    std::cout << "Drag Too Slow : Angle Velocity : " <<  (angle_distance / time_elapsed);
    return true;
  }
  return false;
}

bool dragTooSlow(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
  size_t length = snapshot_array.size();
  return dragTooSlow(snapshot_array, length - 1, length - 2);
}

bool hasValidDrag(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
  if (snapshot_array.size() < 5) {
    std::cout << "too small snapshots!!!";
    return false;
  }
  return true;
}

bool isRightDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
    double last_yaw = snapshot_array.back().yaw;
    if (last_yaw >= kDragEndingLimit && last_yaw <= 180.0) {
      std::cout << "right drag angle hit : " << last_yaw;
      for (auto snap : snapshot_array) {
        if ( !((snap.pitch >= kDragOutOfRangeLimit && snap.pitch <= 180.0) ||
              (snap.pitch >= 180 && snap.pitch < 360.0 - kDragOutOfRangeLimit))
        ) {
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
}

bool isLeftDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
    double last_yaw = snapshot_array.back().yaw;
    if (last_yaw >= 180.0 && last_yaw <= 360.0 - kDragEndingLimit) {
      std::cout << "left drag angle hit : " << last_yaw;
      for (auto snap : snapshot_array) {
        if ( !((snap.pitch >= kDragOutOfRangeLimit && snap.pitch <= 180.0) ||
              (snap.pitch >= 180 && snap.pitch < 360.0 - kDragOutOfRangeLimit))
        ) {
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
}

bool isUpDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
    double last_pitch = snapshot_array.back().pitch;
    if (last_pitch >= kDragEndingLimit && last_pitch <= 180.0) {
      std::cout << "up drag angle hit : " << last_pitch;
      for (auto snap : snapshot_array) {
        if ( !((snap.yaw >= kDragOutOfRangeLimit && snap.yaw <= 180.0) ||
              (snap.yaw >= 180 && snap.yaw < 360.0 - kDragOutOfRangeLimit))
        ) {
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
}

bool isDownDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
    double last_pitch = snapshot_array.back().pitch;
    if (last_pitch >= 180.0 && last_pitch <= 360.0 - kDragEndingLimit) {
      std::cout << "down drag angle hit : " << last_pitch;
      for (auto snap : snapshot_array) {
        if ( !((snap.yaw >= kDragOutOfRangeLimit && snap.yaw <= 180.0) ||
              (snap.yaw >= 180 && snap.yaw < 360.0 - kDragOutOfRangeLimit))
        ) {
          return false;
        }
      }
      return true;
    } else {
      return false;
    }
}

void addDragToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation,
  DragFaceType drag_face_type
) {
    if (!face_observation->objectCount()) {
        moface::ObservationObject *new_object = new moface::ObservationObject("face");
        face_observation->addObject(*new_object);
    }
    moface::ObservationAction *new_action = new moface::ObservationAction("drag", 0.0, 0.0, 1080.0, 1920.0);
    int delta_index = 0;
    for (auto snapshot : snapshot_array) {
      moface::ObservationFeed *new_feed = new moface::ObservationFeed(snapshot.frame_id / 30.0, snapshot.pitch, snapshot.yaw, snapshot.roll);
      double x, y;
      delta_index ++;
      if (drag_face_type == eDragLeft) {
        double delta =
          (reference.landmark(kRightChinIndex).x() - reference.landmark(kLeftChinIndex).x()) / snapshot_array.size();
        x = reference.landmark(kRightChinIndex).x() + delta * delta_index;
        y = reference.landmark(kRightChinIndex).y();
      } else if (drag_face_type == eDragRight) {
        double delta =
          (reference.landmark(kLeftChinIndex).x() - reference.landmark(kRightChinIndex).x()) / snapshot_array.size();
        x = reference.landmark(kLeftChinIndex).x() - delta * delta_index;
        y = reference.landmark(kLeftChinIndex).y();
      } else if (drag_face_type == eDragUp) {
        double delta =
          (reference.landmark(kUnderMouthIndex).y() - reference.landmark(kNoseTipIndex).y()) / snapshot_array.size();
        x = reference.landmark(kLeftChinIndex).x();
        y = reference.landmark(kUnderMouthIndex).y() - delta * delta_index;
      } else if (drag_face_type == eDragDown) {
        double delta =
          (reference.landmark(kNoseTipIndex).y() - reference.landmark(kForeheadIndex).y()) / snapshot_array.size();
        x = reference.landmark(kForeheadIndex).x();
        y = reference.landmark(kForeheadIndex).y() + delta * delta_index;
      } else {
        //todo draw lines through the original path
      }
      moface::ObservationTrackedPosition *new_track = new moface::ObservationTrackedPosition(x, y, 0.0);
      new_feed->addTrackedPosition(*new_track);
      new_action->addFeed(*new_feed);
      delta_index ++;
    }
    face_observation->lastObject().addAction(*new_action);
    return;
}
