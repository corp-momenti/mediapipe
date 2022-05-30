#ifndef UTIL_REFERENCE_H
#define UTIL_REFERENCE_H

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

#endif
