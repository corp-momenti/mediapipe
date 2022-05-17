#ifndef FACE_OBSERVATION_SNAPSHOT_H
#define FACE_OBSERVATION_SNAPSHOT_H

#include <cstdlib>
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"

namespace moface {

    struct FaceObservationSnapShot {
        double timestamp;
        float pitch;
        float roll;
        float yaw;
        std::vector<::mediapipe::NormalizedLandmarkList>;
    };

}

#endif
