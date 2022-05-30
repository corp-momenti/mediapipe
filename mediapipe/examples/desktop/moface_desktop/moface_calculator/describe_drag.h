#ifndef DRAG_UTIL_H
#define DRAG_UTIL_H

#include <cstdlib>
#include <vector>
#include "include/face_observation_snapshot.h"

enum DragFaceType {
  eDragLeft,
  eDragRight,
  eDragUp,
  eDragDown,
  eDragEtc
};

bool hitDragLimit(
  double pitch,
  double yaw,
  double roll
);

bool dragGoingBackward(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  size_t from,
  size_t to
);

bool dragGoingBackward(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array
);

bool dragTooSlow(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  int from,
  int to
);

bool dragTooSlow(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array
);

bool hasValidDrag(
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array
);

bool isRightDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
);

bool isLeftDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
);

bool isUpDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
);

bool isDownDrag(
    std::vector<moface::FaceObservationSnapShot> const& snapshot_array
);

void addDragToFaceObservation(
  ::mediapipe::NormalizedLandmarkList const& reference,
  std::vector<moface::FaceObservationSnapShot> const& snapshot_array,
  moface::FaceObservation *face_observation,
  DragFaceType drag_face_type
);

#endif
