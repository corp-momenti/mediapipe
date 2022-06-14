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
#include <cassert>
// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))


using namespace moface;

constexpr int kNumberOfObservationsForCheckingActions = 60;
constexpr int kReadyMovingCountThreshold = 5;

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
        case eNop:
            ret_string = "nop";
            break;
        default:
            ret_string = "not-available";
    }
    return ret_string;
}

void MofaceCalculator::setResolution(
  double width, double height
) {
  width_ = width;
  height_ = height;
}

void MofaceCalculator::sendObservations(
    const ::mediapipe::NormalizedLandmarkList &landmarks,
    const ::mediapipe::face_geometry::FaceGeometry &geometry
) {
    if (landmarks.landmark_size() == 0) {
      std::cout << "no landmarks data !!!" << std::endl;
      frame_id_ ++;
      return;
    }
    if (!geometry.has_pose_transform_matrix()) {
      std::cout << "no pose transform matrix data !!!" << std::endl;
      frame_id_ ++;
      return;
    }
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

    //check distance
    MofaceDistanceStatus distance_status;
    if (isTooFar(distance)) {
      distance_status = eTooFar;
    } else if (isTooClose(distance)) {
      distance_status = eTooClose;
    } else {
      distance_status = eGoodDistance;
    }
    //check pose
    MofacePoseStatus pose_status;
    if (isWithinReferenceRange(pitch, yaw, roll)) {
      pose_status = eHoldStill;
    } else {
      pose_status = eMoving;
    }
    //check if it's in the frame
    bool withinFrame = false;
    if (isWithinFrame(width_, height_, landmarks)) {
      withinFrame = true;
    }

    signal_callback_(
      distance_status,
      pose_status,
      withinFrame,
      cv::Point(landmarks.landmark(0).x(), landmarks.landmark(0).y())
    );

    switch (cur_state_) {
      case moface::eInit:
        prev_state_ = cur_state_;
        cur_state_ = moface::eStart;
        face_observation_snapshot_array_.clear();
        //face_observation_object_ = NULL;
        break;
      case moface::eStart:
        if (isReferenceFrame(pitch, yaw, roll)) {
          event_callback_(moface::eReferenceDetected);
          reference_snapshot_ = {
            .timestamp = 1000.0 * frame_id_ / 30.0, //geometry_packet.Timestamp().Seconds(),
            .frame_id = frame_id_,
            .pitch = pitch,
            .roll = roll,
            .yaw = yaw
          };
          reference_snapshot_.landmarks = landmarks;
          prev_state_ = cur_state_;
          cur_state_ = moface::eReady;
        }
        break;
      case moface::eReady:
        if (distance_status != eGoodDistance) {
          ready_moving_count_ = 0;
          prev_state_ = cur_state_;
          cur_state_ = moface::eNop;
        } else if (!withinFrame) {
          ready_moving_count_ = 0;
          prev_state_ = cur_state_;
          cur_state_ = moface::eNop;
        } else if (pose_status == eHoldStill) {
          ready_moving_count_ = 0;
          if (face_observation_snapshot_array_.size() > kNumberOfObservationsForCheckingActions) {
            //todo push reference at the front of face_observation_snapshot_array_
            if (checkBlinkActionAndAddToFaceObservation(reference_snapshot_, face_observation_snapshot_array_, face_observation_object_)) {
              event_callback_(eBlinkActionDetected);
            }
            if (checkAngryActionAndAddToFaceObservation(reference_snapshot_, face_observation_snapshot_array_, face_observation_object_)) {
              event_callback_(eAngryActionDetected);
            } else if (checkHanppyActionAndAddToFaceObservation(reference_snapshot_, face_observation_snapshot_array_, face_observation_object_)) {
              event_callback_(eHappyActionDetected);
            }
            face_observation_snapshot_array_.clear();
          } else {
            moface::FaceObservationSnapShot new_snapshot = {
              .timestamp = 1000.0 * frame_id_ / 30.0, //geometry_packet.Timestamp().Seconds(),
              .frame_id = frame_id_,
              .pitch = pitch,
              .roll = roll,
              .yaw = yaw
            };
            new_snapshot.landmarks = landmarks;
            //copy(multi_face_landmarks.begin(), multi_face_landmarks.end(), back_inserter(new_snapshot.landmarks));
            face_observation_snapshot_array_.push_back(new_snapshot);
          }
        } else if (pose_status == eMoving) {
          // std::cout << "pitch : " << pitch << std::endl;
          // std::cout << "yaw : " << yaw << std::endl;
          // std::cout << "roll : " << roll << std::endl;
          if (ready_moving_count_ == 0) {
            face_observation_snapshot_array_.clear();
          }
          ready_moving_count_ ++;
          if (ready_moving_count_ > kReadyMovingCountThreshold) {
            prev_state_ = cur_state_;
            cur_state_ = moface::eDragTracking;
          }
          moface::FaceObservationSnapShot new_snapshot = {
            .timestamp = 1000.0 * frame_id_ / 30.0, //geometry_packet.Timestamp().Seconds(),
            .frame_id = frame_id_,
            .pitch = pitch,
            .roll = roll,
            .yaw = yaw
          };
          new_snapshot.landmarks = landmarks;
          //copy(multi_face_landmarks.begin(), multi_face_landmarks.end(), back_inserter(new_snapshot.landmarks));
          face_observation_snapshot_array_.push_back(new_snapshot);
        } else {
          assertm(false, "invalid case in ready state");
        }
        break;
      case moface::eDragTracking:
        if (distance_status != eGoodDistance) {
          prev_state_ = cur_state_;
          cur_state_ = moface::eNop;
        } else if (!withinFrame) {
          prev_state_ = cur_state_;
          cur_state_ = moface::eNop;
        } else if (pose_status == eHoldStill) {
          prev_state_ = cur_state_;
          cur_state_ = moface::eReady;
          face_observation_snapshot_array_.clear();
        } else if (pose_status == eMoving) {
          if (hitDragLimit(pitch, yaw, roll)) {
            moface::FaceObservationSnapShot new_snapshot = {
              .timestamp = 1000.0 * frame_id_ / 30.0, //geometry_packet.Timestamp().Seconds(),
              .frame_id = frame_id_,
              .pitch = pitch,
              .roll = roll,
              .yaw = yaw
            };
            new_snapshot.landmarks = landmarks;
            face_observation_snapshot_array_.push_back(new_snapshot);
            if (hasValidDrag(face_observation_snapshot_array_)) {
              face_observation_snapshot_array_.insert(face_observation_snapshot_array_.begin(), reference_snapshot_);
              if (isLeftDrag(face_observation_snapshot_array_)) {
                  addDragToFaceObservation(reference_snapshot_, face_observation_snapshot_array_, face_observation_object_, eDragLeft);
                  event_callback_(moface::eLeftActionDetected);
              } else if (isRightDrag(face_observation_snapshot_array_)) {
                  addDragToFaceObservation(reference_snapshot_, face_observation_snapshot_array_, face_observation_object_, eDragRight);
                  event_callback_(moface::eRightActionDetected);
              } else if (isUpDrag(face_observation_snapshot_array_)) {
                  std::cout << "up pitch : " << pitch << std::endl;
                  addDragToFaceObservation(reference_snapshot_, face_observation_snapshot_array_, face_observation_object_, eDragUp);
                  event_callback_(moface::eUpActionDetected);
              } else if (isDownDrag(face_observation_snapshot_array_)) {
                  std::cout << "down pitch : " << pitch << std::endl;
                  addDragToFaceObservation(reference_snapshot_, face_observation_snapshot_array_, face_observation_object_, eDragDown);
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
          } else {
            moface::FaceObservationSnapShot new_snapshot = {
              .timestamp = 1000.0 * frame_id_ / 30.0, //geometry_packet.Timestamp().Seconds(),
              .frame_id = frame_id_,
              .pitch = pitch,
              .roll = roll,
              .yaw = yaw
            };
            new_snapshot.landmarks = landmarks;
            //copy(multi_face_landmarks.begin(), multi_face_landmarks.end(), back_inserter(new_snapshot.landmarks));
            face_observation_snapshot_array_.push_back(new_snapshot);
            //todo
            // if (dragGoingBackward(face_observation_snapshot_array_)) {
            //   face_observation_snapshot_array_.pop_back();
            //   std::cout << "going backward pop back!!!" << std::endl;
            //   //warning_callback_(moface::eGoingBackward);
            // } else if (dragTooSlow(face_observation_snapshot_array_)) {
            //   face_observation_snapshot_array_.pop_back();
            //   std::cout << "too slow pop back!!!" << std::endl;
            //   //warning_callback_(moface::eTooSlow);
            // }
          }
        } else {
          assertm(false, "invalid case in tracking state");
        }
        break;
      case moface::eNop:
        if (
          distance_status == eGoodDistance &&
          pose_status == eHoldStill &&
          withinFrame
        ) {
          prev_state_ = cur_state_;
          cur_state_ = moface::eReady;
          face_observation_snapshot_array_.clear();
        }
        break;
      default:
        assertm(false, "not a valid state");
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

void MofaceCalculator::reset() {
    prev_state_ = moface::eInit;
    cur_state_ = moface::eInit;
    frame_id_ = 0;
}

void MofaceCalculator::updateMediaFilePath(std::string &file_path) {
    face_observation_object_->updateMediaFilePath(file_path);
}
