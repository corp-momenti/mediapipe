#ifndef MOFACE_CALCULATOR_H
#define MOFACE_CALCULATOR_H

#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "include/face_observation_snapshot.h"
#include "include/moface_state.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include <mutex>

namespace moface {

    enum MoFaceEventType {
        eRightActionDetected,
        eLeftActionDetected,
        eUpActionDetected,
        eDownActionDetected,
        eBlinkActionDetected,
        eMouthActionDetected,
        eAngryActionDetected,
        eHappyActionDetected,
        eReferenceDetected
    };

    enum MofaceDistanceStatus {
        eGoodDistance,
        eTooFar,
        eTooClose
    };

    enum MofacePoseStatus {
        eHoldStill,
        eMoving
    };

    enum MoFaceWarningType {
        eGoingBackward,
        eTooSlow,
        eInvalidDragAction,
        eNoFace,
        eTimeout
    };

    enum MofaceDetectionHintType {
        eDetectBlink,
        eDetectHappy,
        eDetectAngry,
        eDetectDrag
    };

    typedef void (*eventCallback)(MoFaceEventType event);
    typedef void (*warningCallback)(MoFaceWarningType warning);
    typedef void (*signalCallback)(MofaceDistanceStatus distance, MofacePoseStatus pose, bool inFrame, cv::Point center);
    typedef void (*geometryCallback)(double pitch, double yaw, double roll, double distance);

    class MofaceCalculator {
        public:
            MofaceCalculator(
                eventCallback event_callback,
                signalCallback signal_callback,
                warningCallback warning_callback,
                geometryCallback geometry_callback
            ) : event_callback_(event_callback),
                signal_callback_(signal_callback),
                warning_callback_(warning_callback),
                geometry_callback_(geometry_callback),
                prev_state_(moface::eInit),
                cur_state_(moface::eInit),
                face_observation_object_(new moface::FaceObservation("")),
                frame_id_(0),
                width_(1080.0),
                height_(1920.0)
                { }
            virtual ~MofaceCalculator() {}
            void setResolution(double width, double height);
            void setHint(MofaceDetectionHintType hint) {
                state_mutex_.lock();
                //std::cout << "!! cur state : " << cur_state_ << ", prev stats: " << prev_state_ << std::endl;
                switch (hint) {
                    case eDetectBlink:
                        prev_state_ = cur_state_;
                        cur_state_ = moface::eCheckingEyes;
                        break;
                    case eDetectAngry:
                        prev_state_ = cur_state_;
                        cur_state_ = moface::eCheckingAngry;
                        break;
                    case eDetectHappy:
                        prev_state_ = cur_state_;
                        cur_state_ = moface::eCheckingHappy;
                        break;
                    case eDetectDrag:
                        // prev_state_ = cur_state_;
                        // cur_state_ = moface::eReady;
                        break;
                    default:
                        break;
                }
                //std::cout << "!! cur state : " << cur_state_ << ", prev stats: " << prev_state_ << std::endl;
                state_mutex_.unlock();
            }
            void feed_time(double timestamp) {
                state_mutex_.lock();
                //std::cout << "timestamp : " << timestamp << std::endl;
                feed_time_list_.push_back(timestamp);
                state_mutex_.unlock();
            }
            void sendObservations(const ::mediapipe::NormalizedLandmarkList &landmarks, const ::mediapipe::face_geometry::FaceGeometry &geometry);
            std::string getFaceObservation();
            std::string curState();
            void reset();
            void updateMediaFilePath(std::string &file_path);
        private:
            eventCallback event_callback_;
            warningCallback warning_callback_;
            signalCallback signal_callback_;
            geometryCallback geometry_callback_;
            moface::MoFaceState prev_state_, cur_state_;
            std::vector<moface::FaceObservationSnapShot> face_observation_snapshot_array_;
            moface::FaceObservationSnapShot reference_snapshot_;
            moface::FaceObservation *face_observation_object_;
            int frame_id_;
            double width_;
            double height_;
            std::vector<double> feed_time_list_;
            std::mutex state_mutex_;
    };
}

#endif
