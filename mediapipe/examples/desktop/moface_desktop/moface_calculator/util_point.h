#ifndef UTIL_POINT_H
#define UTIL_POINT_H

#include <cstdlib>
#include <vector>
#include "mediapipe/framework/formats/landmark.pb.h"

double euclidian_distance(
    ::mediapipe::NormalizedLandmark from,
    ::mediapipe::NormalizedLandmark to
);

#endif
