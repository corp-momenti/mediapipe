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

constexpr int kNumberOfObservationsInFlight = 10;
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
        case eCheckingEyes:
            ret_string = "checking-eyes";
            break;
        case eCheckingAngry:
            ret_string = "checking-angry";
            break;
        case eCheckingHappy:
            ret_string = "checking-happy";
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

    //check eyes
    bool eyes_closed = checkEyesClosed(landmarks);

    //check mouth angry
    bool angry_mouth = checkAngryMouth(landmarks);

    //check mouth angry
    bool happy_mouth = checkHappyMouth(reference_snapshot_.landmarks, landmarks);

    signal_callback_(
      distance_status,
      pose_status,
      withinFrame,
      cv::Point(landmarks.landmark(1).x(), landmarks.landmark(1).y())
    );
    state_mutex_.lock();
    switch (cur_state_) {
      case moface::eInit:
        prev_state_ = cur_state_;
        cur_state_ = moface::eStart;
        face_observation_snapshot_array_.clear();
        break;
      case moface::eStart:
        if (isReferenceFrame(pitch, yaw, roll) &&
            withinFrame &&
            distance_status == eGoodDistance
        ) {
          event_callback_(moface::eReferenceDetected);
          reference_snapshot_ = {
            .timestamp = feed_time_list_[0], //geometry_packet.Timestamp().Seconds(),
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
          prev_state_ = cur_state_;
          cur_state_ = moface::eNop;
        } else if (pose_status == eMoving) {
          face_observation_snapshot_array_.clear();
          moface::FaceObservationSnapShot new_snapshot = {
            .timestamp = feed_time_list_[0], //geometry_packet.Timestamp().Seconds(),
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
        }
        break;
      case moface::eDragTracking:
        if (pose_status == eHoldStill) {
          prev_state_ = cur_state_;
          cur_state_ = moface::eReady;
          face_observation_snapshot_array_.clear();
        } else if (pose_status == eMoving) {
          if (hitDragLimit(pitch, yaw, roll)) {
            moface::FaceObservationSnapShot new_snapshot = {
              .timestamp = feed_time_list_[0], //geometry_packet.Timestamp().Seconds(),
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
              .timestamp = feed_time_list_[0], //geometry_packet.Timestamp().Seconds(),
              .frame_id = frame_id_,
              .pitch = pitch,
              .roll = roll,
              .yaw = yaw
            };
            new_snapshot.landmarks = landmarks;
            face_observation_snapshot_array_.push_back(new_snapshot);
          }
        } else {
          assertm(false, "invalid case in tracking state");
        }
        break;
      case moface::eCheckingEyes:
        //std::cout << "checking eyes " << std::endl;
        if (pose_status == eHoldStill) {
            moface::FaceObservationSnapShot new_snapshot = {
                .timestamp = feed_time_list_[0], //geometry_packet.Timestamp().Seconds(),
                .frame_id = frame_id_,
                .pitch = pitch,
                .roll = roll,
                .yaw = yaw
            };
            new_snapshot.landmarks = landmarks;
            face_observation_snapshot_array_.push_back(new_snapshot);
            if (eyes_closed && face_observation_snapshot_array_.size() >= kNumberOfObservationsInFlight) {
                event_callback_(eBlinkActionDetected);
                int last_index = face_observation_snapshot_array_.size() - 1;
                face_observation_snapshot_array_.insert(face_observation_snapshot_array_.begin() + (last_index - 2), reference_snapshot_);
                addBlinkToFaceObservation(
                    reference_snapshot_,
                    std::make_tuple(last_index - 2, last_index),
                    face_observation_snapshot_array_,
                    face_observation_object_
                );
                face_observation_snapshot_array_.clear();
            } else {
                if (face_observation_snapshot_array_.size() > kNumberOfObservationsInFlight) {
                    face_observation_snapshot_array_.erase(face_observation_snapshot_array_.begin());
                }
            }
        } else {
            std::cout << "ignore eyes due to poses " << std::endl;
        }
        break;
      case moface::eCheckingAngry:
        {
            moface::FaceObservationSnapShot new_snapshot = {
                .timestamp = feed_time_list_[0], //geometry_packet.Timestamp().Seconds(),
                .frame_id = frame_id_,
                .pitch = pitch,
                .roll = roll,
                .yaw = yaw
            };
            new_snapshot.landmarks = landmarks;
            face_observation_snapshot_array_.push_back(new_snapshot);
            if (angry_mouth && face_observation_snapshot_array_.size() >= kNumberOfObservationsInFlight) {
                int last_index = face_observation_snapshot_array_.size() - 1;
                face_observation_snapshot_array_.insert(face_observation_snapshot_array_.begin() + (last_index - 10), reference_snapshot_);
                addAngryActionToFaceObservation(
                    reference_snapshot_,
                    std::make_tuple(last_index - 10, last_index),
                    face_observation_snapshot_array_,
                    face_observation_object_
                );
                face_observation_snapshot_array_.clear();
                event_callback_(eAngryActionDetected);
            } else {
                if (face_observation_snapshot_array_.size() > kNumberOfObservationsInFlight) {
                    face_observation_snapshot_array_.erase(face_observation_snapshot_array_.begin());
                }
            }
        }
        break;
      case moface::eCheckingHappy:
        {
            //std::cout << "checking happy : " << calculateMWAR2(reference_snapshot_.landmarks, landmarks) << std::endl;
            moface::FaceObservationSnapShot new_snapshot = {
                .timestamp = feed_time_list_[0], //geometry_packet.Timestamp().Seconds(),
                .frame_id = frame_id_,
                .pitch = pitch,
                .roll = roll,
                .yaw = yaw
            };
            new_snapshot.landmarks = landmarks;
            face_observation_snapshot_array_.push_back(new_snapshot);
            if (happy_mouth && face_observation_snapshot_array_.size() >= kNumberOfObservationsInFlight) {
                event_callback_(eHappyActionDetected);
                int last_index = face_observation_snapshot_array_.size() - 1;
                face_observation_snapshot_array_.insert(face_observation_snapshot_array_.begin() + (last_index - 10), reference_snapshot_);
                addHappyActionToFaceObservation(
                    reference_snapshot_,
                    std::make_tuple(last_index - 10, last_index),
                    face_observation_snapshot_array_,
                    face_observation_object_
                );
                face_observation_snapshot_array_.clear();
            } else {
                if (face_observation_snapshot_array_.size() > kNumberOfObservationsInFlight) {
                    face_observation_snapshot_array_.erase(face_observation_snapshot_array_.begin());
                }
            }
        }
        break;
      case moface::eNop:
        if (
          distance_status == eGoodDistance &&
          pose_status == eHoldStill /*&&
          withinFrame*/
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
    feed_time_list_.erase(feed_time_list_.begin());
    state_mutex_.unlock();
}

std::string MofaceCalculator::getFaceObservation() {
    state_mutex_.lock();
    rapidjson::StringBuffer sb;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> json_writer(sb);
    face_observation_object_->serialize(json_writer);
    state_mutex_.unlock();
    return sb.GetString();
}

std::string MofaceCalculator::curState() {
    return state_to_string(cur_state_);
}

void MofaceCalculator::reset() {
    state_mutex_.lock();
    prev_state_ = moface::eInit;
    cur_state_ = moface::eInit;
    frame_id_ = 0;
    face_observation_object_ = new moface::FaceObservation("");
    face_observation_snapshot_array_.clear();
    feed_time_list_.clear();
    state_mutex_.unlock();
}

void MofaceCalculator::updateMediaFilePath(std::string &file_path) {
    face_observation_object_->updateMediaFilePath(file_path);
}
