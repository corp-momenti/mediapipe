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
constexpr double kDragDownEndingLimitForPitch = 15.0;
constexpr double kDragUpEndingLimitForPitch = 15.0;
constexpr double kDragEndingLimitForYaw = 30.0;

//Drag Backward Limit
constexpr double kDragBackwardLimit = 3.0;

//Drag Slow Limit
constexpr double kDragSlowLimit = 7.0; // degree per sec

//Drag Right Limit
constexpr double kDragOutOfRangeLimit = 10.0;

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
    if ((yaw >= kDragEndingLimitForYaw && yaw <= 180.0) ||
        (yaw >= 180.0 && yaw <= 360.0 - kDragEndingLimitForYaw) ||
        (pitch >= kDragDownEndingLimitForPitch && pitch <= 180.0) || // check down
        (pitch >= 180.0 && pitch <= 360.0 - kDragUpEndingLimitForPitch) //check up
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
  return dragGoingBackward(snapshot_array, length - 2, length - 1);
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
    pow(last_item.pitch - cmp_item.pitch, 2) +
    pow(last_item.yaw - cmp_item.yaw, 2) /*+
    pow(last_item.roll - cmp_item.roll,2)*/
  );
  //std::cout << "angle distance : " << angle_distance << ", velocity : " << (angle_distance / time_elapsed) << std::endl;
  if (kDragSlowLimit >= (angle_distance / time_elapsed)) {
    //std::cout << "Drag Too Slow : Angle Velocity : " <<  (angle_distance / time_elapsed);
    return true;
  }
  return false;
}

bool dragTooSlow(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
  size_t length = snapshot_array.size();
  return dragTooSlow(snapshot_array, length - 2, length - 1);
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
    if (last_yaw >= kDragEndingLimitForYaw && last_yaw <= 180.0) {
      //std::cout << "right drag angle hit : " << last_yaw << std::endl;
      for (auto snap : snapshot_array) {
        if ( (snap.pitch >= kDragOutOfRangeLimit && snap.pitch <= 180.0) ||
             (snap.pitch >= 180 && snap.pitch < 360.0 - kDragOutOfRangeLimit)
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
    if (last_yaw >= 180.0 && last_yaw <= 360.0 - kDragEndingLimitForYaw) {
//      std::cout << "left drag angle hit : " << last_yaw << std::endl;
      for (auto snap : snapshot_array) {
        if ((snap.pitch >= kDragOutOfRangeLimit && snap.pitch <= 180.0) ||
            (snap.pitch >= 180 && snap.pitch < 360.0 - kDragOutOfRangeLimit)
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
    if (last_pitch >= kDragDownEndingLimitForPitch && last_pitch <= 180.0) {
 //     std::cout << "up drag angle hit : " << last_pitch << std::endl;
      for (auto snap : snapshot_array) {
        if ( (snap.yaw >= kDragOutOfRangeLimit && snap.yaw <= 180.0) ||
              (snap.yaw >= 180 && snap.yaw < 360.0 - kDragOutOfRangeLimit)
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
    if (last_pitch >= 180.0 && last_pitch <= 360.0 - kDragUpEndingLimitForPitch) {
//      std::cout << "down drag angle hit : " << last_pitch << std::endl;
      for (auto snap : snapshot_array) {
        if ((snap.yaw >= kDragOutOfRangeLimit && snap.yaw <= 180.0) ||
            (snap.yaw >= 180 && snap.yaw < 360.0 - kDragOutOfRangeLimit)
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
  moface::FaceObservationSnapShot const& reference,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation,
  DragFaceType drag_face_type
) {
    if (!face_observation->objectCount()) {
        moface::ObservationObject *new_object = new moface::ObservationObject("face");
        face_observation->addObject(*new_object);
    }
    std::string desc;
    switch (drag_face_type)
    {
      case eDragLeft:
        desc = "drag-left";
        break;
      case eDragRight:
        desc = "drag-right";
        break;
      case eDragUp:
        desc = "drag-up";
        break;
      case eDragDown:
        desc = "drag-down";
        break;
      default:
        break;
    }
    moface::ObservationAction *new_action = new moface::ObservationAction("drag", desc, 0.0, 0.0, 1080.0, 1920.0);
    int delta_index = 0;
    for (auto snapshot : snapshot_array) {
      moface::ObservationFeed *new_feed = new moface::ObservationFeed(1000.0 * snapshot.frame_id / 60.0/*snapshot.timestamp*/, snapshot.pitch, snapshot.yaw, snapshot.roll);
      double x, y;
      if (drag_face_type == eDragLeft) {
        double delta =
          (reference.landmarks.landmark(kLeftChinIndex).x() - reference.landmarks.landmark(kRightChinIndex).x()) / snapshot_array.size();
        x = reference.landmarks.landmark(kRightChinIndex).x() + delta * delta_index;
        y = reference.landmarks.landmark(kRightChinIndex).y();
      } else if (drag_face_type == eDragRight) {
        double delta =
          (reference.landmarks.landmark(kLeftChinIndex).x() - reference.landmarks.landmark(kRightChinIndex).x()) / snapshot_array.size();
        x = reference.landmarks.landmark(kLeftChinIndex).x() - delta * delta_index;
        y = reference.landmarks.landmark(kLeftChinIndex).y();
      } else if (drag_face_type == eDragUp) {
        double delta =
          (reference.landmarks.landmark(kUnderMouthIndex).y() - reference.landmarks.landmark(kNoseTipIndex).y()) / snapshot_array.size();
        x = reference.landmarks.landmark(kNoseTipIndex).x();
        y = reference.landmarks.landmark(kUnderMouthIndex).y() - delta * delta_index;
      } else if (drag_face_type == eDragDown) {
        double delta =
          (reference.landmarks.landmark(kNoseTipIndex).y() - reference.landmarks.landmark(kForeheadIndex).y()) / snapshot_array.size();
        x = reference.landmarks.landmark(kForeheadIndex).x();
        y = reference.landmarks.landmark(kForeheadIndex).y() + delta * delta_index;
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
