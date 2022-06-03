#ifndef MOFACE_CALCULATOR_H
#define MOFACE_CALCULATOR_H

#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "include/face_observation_snapshot.h"
#include "include/moface_state.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"

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
                frame_id_(0)
                { }
            virtual ~MofaceCalculator() {}
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
            ::mediapipe::NormalizedLandmarkList reference_landmark_;
            moface::FaceObservation *face_observation_object_;
            int frame_id_;
    };
}

#endif
