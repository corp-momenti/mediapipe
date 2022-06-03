#ifndef UTIL_REFERENCE_H
#define UTIL_REFERENCE_H

#include "mediapipe/framework/formats/landmark.pb.h"

bool isReferenceFrame(
  double pitch,
  double yaw,
  double roll
);

bool isWithinReferenceRange(
  double pitch,
  double yaw,
  double roll
);

bool isTooFar(
  double distance
);

bool isTooClose(
  double distance
);

bool isWithinFrame(
  ::mediapipe::NormalizedLandmarkList const& landmarks
);

#endif
