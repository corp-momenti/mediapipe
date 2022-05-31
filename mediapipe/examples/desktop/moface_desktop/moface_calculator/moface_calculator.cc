#include <cstdlib>
#include <map>
#include <tuple>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <math.h>
#include "moface_calculator.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "include/rapidjson/document.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/stringbuffer.h"
#include "include/face_observation_snapshot.h"
#include "include/moface_state.h"
#include "describe_drag.h"
#include "describe_eyes.h"
#include "describe_mouth.h"
#include "describe_reference.h"

using namespace moface;

constexpr int kNumberOfObservationsForCheckingActions = 90;

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
    if (isTooFar(distance)) {
      warning_callback_(eTooFar);
      frame_id_ ++;
      return;
    }
    if (isTooClose(distance)) {
      warning_callback_(eTooClose);
      frame_id_ ++;
      return;
    }
    warning_callback_(eGoodDistance);
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
          if (face_observation_snapshot_array_.size() > kNumberOfObservationsForCheckingActions) {
            if (checkBlinkActionAndAddToFaceObservation(reference_landmark_, face_observation_snapshot_array_, face_observation_object_)) {
              event_callback_(eBlinkActionDetected);
            }
            if (checkAngryActionAndAddToFaceObservation(reference_landmark_, face_observation_snapshot_array_, face_observation_object_)) {
              event_callback_(eAngryActionDetected);
            }
            if (checkHanppyActionAndAddToFaceObservation(reference_landmark_, face_observation_snapshot_array_, face_observation_object_)) {
              event_callback_(eHappyActionDetected);
            }
            face_observation_snapshot_array_.clear();
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
          }
        }
        break;
      case moface::eDragTracking:
        if (isWithinReferenceRange(pitch, yaw, roll)) {
          prev_state_ = cur_state_;
          cur_state_ = moface::eReady;
          face_observation_snapshot_array_.clear();
        } else if (hitDragLimit(pitch, yaw, roll)) {
          moface::FaceObservationSnapShot new_snapshot = {
            .timestamp = frame_id_ / 30.0, //geometry_packet.Timestamp().Seconds(),
            .frame_id = frame_id_,
            .pitch = pitch,
            .roll = roll,
            .yaw = yaw
          };
          new_snapshot.landmarks = landmarks;
          face_observation_snapshot_array_.push_back(new_snapshot);
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
          if (isLeftDrag(face_observation_snapshot_array_)) {
              addDragToFaceObservation(reference_landmark_, face_observation_snapshot_array_, face_observation_object_, eDragLeft);
              event_callback_(moface::eLeftActionDetected);
          } else if (isRightDrag(face_observation_snapshot_array_)) {
              addDragToFaceObservation(reference_landmark_, face_observation_snapshot_array_, face_observation_object_, eDragRight);
              event_callback_(moface::eRightActionDetected);
          } else if (isUpDrag(face_observation_snapshot_array_)) {
              addDragToFaceObservation(reference_landmark_, face_observation_snapshot_array_, face_observation_object_, eDragUp);
              event_callback_(moface::eUpActionDetected);
          } else if (isDownDrag(face_observation_snapshot_array_)) {
              addDragToFaceObservation(reference_landmark_, face_observation_snapshot_array_, face_observation_object_, eDragDown);
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
