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
  moface::FaceObservationSnapShot const& reference,
  std::tuple<int, int> slice,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
);

bool checkAngryActionAndAddToFaceObservation(
  moface::FaceObservationSnapShot const& reference,
  std::vector<moface::FaceObservationSnapShot> &snapshot_array,
  moface::FaceObservation *face_observation
);

double calculateMWAR(
  ::mediapipe::NormalizedLandmarkList const& reference,
  ::mediapipe::NormalizedLandmarkList const& landmark
);

double calculateMWAR2(
  ::mediapipe::NormalizedLandmarkList const& reference,
  ::mediapipe::NormalizedLandmarkList const& landmark
);

void addHappyActionToFaceObservation(
  moface::FaceObservationSnapShot const& reference,
  std::tuple<int, int> slice,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation
);

bool checkHanppyActionAndAddToFaceObservation(
  moface::FaceObservationSnapShot const& reference,
  std::vector<moface::FaceObservationSnapShot> &snapshot_array,
  moface::FaceObservation *face_observation
);

bool checkAngryMouth(
  ::mediapipe::NormalizedLandmarkList const& landmarks
);

bool checkHappyMouth(
  ::mediapipe::NormalizedLandmarkList const& reference,
  ::mediapipe::NormalizedLandmarkList const& landmarks
);

#endif
