#ifndef UTIL_EYES_H
#define UTIL_EYES_H

#include <cstdlib>
#include <vector>
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "include/face_observation_snapshot.h"

std::tuple<int, int> getLeftEyeCenter(
  ::mediapipe::NormalizedLandmarkList const& referenece
);

std::tuple<int, int, int, int> getLeftEyeArea(
  ::mediapipe::NormalizedLandmarkList const& referenece
);

double calculateLeftEAR(const ::mediapipe::NormalizedLandmarkList &landmarks);

std::tuple<int, int> getRightEyeCenter(
  ::mediapipe::NormalizedLandmarkList const& referenece
);

std::tuple<int, int, int, int> getRightEyeArea(
  ::mediapipe::NormalizedLandmarkList const& referenece
);

double calculateRightEAR(const ::mediapipe::NormalizedLandmarkList &landmarks);

void addBlinkToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::tuple<int, int> slice,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
);

bool checkBlinkActionAndAddToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
);

#endif
