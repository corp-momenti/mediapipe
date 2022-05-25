#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include "moface_calculator.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"
#include "include/face_observation_snapshot.h"
#include "include/moface_state.h"

using namespace moface;
//Decision Configuration
//Reference Frame Range
constexpr double kReferencePitchMin = 357.0;
constexpr double kReferencePitchMax = 3.0;
constexpr double kReferenceYawMin = 357.0;
constexpr double kReferenceYawMax = 3.0;
constexpr double kReferenceRollMin = 357.0;
constexpr double kReferenceRollMax = 3.0;

//Reference Range Range
constexpr double kReferenceRangePitchMin = 355.0;
constexpr double kReferenceRangePitchMax = 5.0;
constexpr double kReferenceRangeYawMin = 355.0;
constexpr double kReferenceRangeYawMax = 5.0;
constexpr double kReferenceRangeRollMin = 355.0;
constexpr double kReferenceRangeRollMax = 5.0;

//Drag Action Limit
constexpr double kDragLimit = 20.0;

//Drag Backward Limit
constexpr double kDragBackwardLimit = 3.0;

//Drag Slow Limit
constexpr double kDragSlowLimit = 7.0; // degree per sec

std::string state_to_string(MoFaceState const& in_code) {
    std::string ret_string;
    switch (in_code) {
        case eInit:
            ret_string = "init";
            break;
        case eStart:
            ret_string = "start";
            break;
        case eReady:
            ret_string = "ready";
            break;
        case eDragTracking:
            ret_string = "drag-tracking";
            break;
        case eDragAnalyzing:
            ret_string = "drag-analyzing";
            break;
        case eNop:
            ret_string = "nop";
            break;
        default:
            ret_string = "not-available";
    }
    return ret_string;
}

bool isReferenceFrame(
  double pitch,
  double yaw,
  double roll
) {
  bool pitch_in_range = false, yaw_in_range = false, roll_in_range = false;
  if ( (pitch >= kReferencePitchMin && pitch <= 360.0) ||
       (pitch >= 0.0 && pitch <= kReferencePitchMax)
  ) {
    pitch_in_range = true;
    //LOG(INFO) << "Reference Frame Pitch Angle : " << pitch;
  }

  if ( (yaw >= kReferenceYawMin && yaw <= 360.0) ||
       (yaw >= 0.0 && yaw <= kReferenceYawMax)
  ) {
    yaw_in_range = true;
    //LOG(INFO) << "Reference Frame Yaw Angle : " << yaw;
  }

  if ( (roll >= kReferenceRollMin && roll <= 360.0) ||
       (roll >= 0.0 && roll <= kReferenceRollMax)
  ) {
    roll_in_range = true;
    //LOG(INFO) << "Reference Frame Roll Angle : " << roll;
  }

  return pitch_in_range && yaw_in_range && roll_in_range;
}

bool isWithinReferenceRange(
  double pitch,
  double yaw,
  double roll
) {
  bool pitch_in_range = false, yaw_in_range = false, roll_in_range = false;
  if ( (pitch >= kReferenceRangePitchMin && pitch <= 360.0) ||
       (pitch >= 0.0 && pitch <= kReferenceRangePitchMax)
  ) {
    pitch_in_range = true;
    //LOG(INFO) << "Reference Range Pitch Angle : " << pitch;
  }

  if ( (yaw >= kReferenceRangeYawMin && yaw <= 360.0) ||
       (yaw >= 0.0 && yaw <= kReferenceRangeYawMax)
  ) {
    yaw_in_range = true;
    //LOG(INFO) << "Reference Range Yaw Angle : " << yaw;
  }

  if ( (roll >= kReferenceRangeRollMin && roll <= 360.0) ||
       (roll >= 0.0 && roll <= kReferenceRangeRollMax)
  ) {
    roll_in_range = true;
    //LOG(INFO) << "Reference Range Roll Angle : " << roll;
  }

  return pitch_in_range && yaw_in_range && roll_in_range;
}

bool hitDragLimit(
  double pitch,
  double yaw,
  double roll
) {
    if ( (yaw >= kDragLimit && yaw <= 180.0) ||
        (yaw >= 180.0 && yaw <= 360.0 - kDragLimit)
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
  //check if it's left drag
  //check if it's right drag
  //check if it's up drag
  //check if it's down drag
  if (snapshot_array.size() < 5) {
    std::cout << "too small snapshots!!!";
    return false;
  }
  bool ret = true;
  for(std::vector<moface::FaceObservationSnapShot>::size_type i = 0; i != (snapshot_array.size() - 1); i++) {
    if (dragGoingBackward(snapshot_array, i, i + 1)) {
      ret = false;
    }
    if (dragTooSlow(snapshot_array, i, i + 1)) {
      ret = false;
    }
  }
  return ret;
}

bool isRightDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
    //todo
    return true;
}

bool isLeftDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
    //todo
    return true;
}

bool isUpDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
    //todo
    return true;
}

bool isDownDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
) {
    //todo
    return true;
}

void addDragToFaceObservation(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
) {
    if (!face_observation->objectCount()) {
    moface::ObservationObject *new_object = new moface::ObservationObject("face");
    face_observation->addObject(*new_object);
    }
    moface::ObservationAction *new_action = new moface::ObservationAction("drag", 0.0, 0.0, 1080.0, 1920.0);
    for (auto snapshot : snapshot_array) {
    moface::ObservationFeed *new_feed = new moface::ObservationFeed(snapshot.frame_id / 30.0, snapshot.pitch, snapshot.yaw, snapshot.roll);
    moface::ObservationTrackedPosition *new_track = new moface::ObservationTrackedPosition(0.0, 0.0, 0.0);
    new_feed->addTrackedPosition(*new_track);
    new_action->addFeed(*new_feed);
    }
    face_observation->lastObject().addAction(*new_action);
    return;
}

void MofaceCalculator::sendObservations(
    const ::mediapipe::NormalizedLandmarkList &landmarks,
    const ::mediapipe::face_geometry::FaceGeometry &geometry
) {
    float pitch, yaw, roll, distance;
    pitch = asin(geometry.pose_transform_matrix().packed_data(6));
    if (cos(pitch) > 0.0001) {
        yaw = atan2(geometry.pose_transform_matrix().packed_data(2), geometry.pose_transform_matrix().packed_data(10));
        roll = atan2(geometry.pose_transform_matrix().packed_data(4), geometry.pose_transform_matrix().packed_data(5));
    } else {
        yaw = 0.0;
        roll = atan2(-geometry.pose_transform_matrix().packed_data(1), geometry.pose_transform_matrix().packed_data(0));
    }
    pitch = fmod(((pitch * 180.0 / M_PI) + 360.0), 360.0);
    yaw = fmod(((yaw * 180.0 / M_PI) + 360.0), 360);
    roll = fmod(((roll * 180.0 / M_PI) + 360.0), 360);
    distance = -geometry.pose_transform_matrix().packed_data(14);
    if (geometry_callback_) {
        geometry_callback_(pitch, yaw, roll, distance);
    }
    switch (cur_state_) {
      case moface::eInit:
        prev_state_ = cur_state_;
        cur_state_ = moface::eStart;
        break;
      case moface::eStart:
        if (isReferenceFrame(pitch, yaw, roll)) {
          event_callback_(moface::eReferenceDetected);
          reference_landmark_ = landmarks;
          prev_state_ = cur_state_;
          cur_state_ = moface::eReady;
        }
        break;
      case moface::eReady:
        if (!isWithinReferenceRange(pitch, yaw, roll)) {
          moface::FaceObservationSnapShot new_snapshot = {
            .timestamp = frame_id_ / 30.0, //geometry_packet.Timestamp().Seconds(),
            .frame_id = frame_id_,
            .pitch = pitch,
            .roll = roll,
            .yaw = yaw
          };
          new_snapshot.landmarks = landmarks;
          //copy(multi_face_landmarks.begin(), multi_face_landmarks.end(), back_inserter(new_snapshot.landmarks));
          face_observation_snapshot_array_.push_back(new_snapshot);
          prev_state_ = cur_state_;
          cur_state_ = moface::eDragTracking;
        } else {
          //todo
          //add face_observation_shapshot_array
          //if there's 5 seconds long snapshsots and check if it has an eye action
          //  add eye action
          //  notify we have an eye action
          //if there's 5 seconds long snapshots and check if it has a mouth action
          //  add mouth action
          //  notify we have an mouth action
          //if the lenth of snapshot array is longer than 5 sec, clear face_observation_snapshot_array
        }
        break;
      case moface::eDragTracking:
        if (isWithinReferenceRange(pitch, yaw, roll)) {
          prev_state_ = cur_state_;
          cur_state_ = moface::eReady;
          face_observation_snapshot_array_.clear();
        } else if (hitDragLimit(pitch, yaw, roll)) {
          prev_state_ = cur_state_;
          cur_state_ = moface::eDragAnalyzing;
        } else {
          moface::FaceObservationSnapShot new_snapshot = {
            .timestamp = frame_id_ / 30.0, //geometry_packet.Timestamp().Seconds(),
            .frame_id = frame_id_,
            .pitch = pitch,
            .roll = roll,
            .yaw = yaw
          };
          new_snapshot.landmarks = landmarks;
          //copy(multi_face_landmarks.begin(), multi_face_landmarks.end(), back_inserter(new_snapshot.landmarks));
          face_observation_snapshot_array_.push_back(new_snapshot);
          if (dragGoingBackward(face_observation_snapshot_array_)) {
            warning_callback_(moface::eGoingBackward);
          }
          if (dragTooSlow(face_observation_snapshot_array_)) {
            warning_callback_(moface::eTooSlow);
          }
        }
        break;
      case moface::eDragAnalyzing:
        if (hasValidDrag(face_observation_snapshot_array_)) {
          addDragToFaceObservation(face_observation_snapshot_array_, face_observation_object_);
          if (isLeftDrag(face_observation_snapshot_array_)) {
              event_callback_(moface::eLeftActionDetected);
          } else if (isRightDrag(face_observation_snapshot_array_)) {
              event_callback_(moface::eRightActionDetected);
          } else if (isUpDrag(face_observation_snapshot_array_)) {
              event_callback_(moface::eUpActionDetected);
          } else if (isDownDrag(face_observation_snapshot_array_)) {
              event_callback_(moface::eDownActionDetected);
          } else {
              std::cout << "Invald Drag Cases!!!!";
          }
        } else {
          warning_callback_(moface::eInvalidDragAction);
        }
        face_observation_snapshot_array_.clear();
        prev_state_ = cur_state_;
        cur_state_ = moface::eNop;
        break;
      case moface::eNop:
        if (isWithinReferenceRange(pitch, yaw, roll)) {
          prev_state_ = cur_state_;
          cur_state_ = moface::eReady;
          face_observation_snapshot_array_.clear();
        }
        break;
      default:
        std::cout << "Not a vaild state!!!";
        break;
    }
    frame_id_ ++;
}

std::string MofaceCalculator::getFaceObservation() {
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> json_writer(sb);
    face_observation_object_->serialize(json_writer);
    return sb.GetString();
}

std::string MofaceCalculator::curState() {
    return state_to_string(cur_state_);
}

void MofaceCalculator::updateMediaFilePath(std::string &file_path) {
    face_observation_object_->updateMediaFilePath(file_path);
}
