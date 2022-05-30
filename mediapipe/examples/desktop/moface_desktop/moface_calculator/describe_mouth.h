#ifndef UTIL_MOUTH_H
#define UTIL_MOUTH_H

#include <cstdlib>
#include <vector>
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "include/face_observation_snapshot.h"

double calculateMHAR(
  ::mediapipe::NormalizedLandmarkList const& landmark
);

void addAngryActionToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::tuple<int, int> slice,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
);

bool checkAngryActionAndAddToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
);

double calculateMWAR(
  ::mediapipe::NormalizedLandmarkList const& reference,
  ::mediapipe::NormalizedLandmarkList const& landmark
);

void addHappyActionToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::tuple<int, int> slice,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
);

bool checkHanppyActionAndAddToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
);

#endif
