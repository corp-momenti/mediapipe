#include <cstdlib>
#include <iostream>
#include <vector>

#include "mediapipe/framework/formats/landmark.pb.h"

//Reference Frame Range
constexpr double kReferencePitchMin = 357.0;
constexpr double kReferencePitchMax = 3.0;
constexpr double kReferenceYawMin = 357.0;
constexpr double kReferenceYawMax = 3.0;
constexpr double kReferenceRollMin = 357.0;
constexpr double kReferenceRollMax = 3.0;

//Reference Range Range
constexpr double kReferenceRangePitchMin = 355.0;
constexpr double kReferenceRangePitchMax = 5.0;
constexpr double kReferenceRangeYawMin = 355.0;
constexpr double kReferenceRangeYawMax = 5.0;
constexpr double kReferenceRangeRollMin = 355.0;
constexpr double kReferenceRangeRollMax = 5.0;

//Distance Range
constexpr double kMaxDistanceLimit = 40.0;
constexpr double kMinDistanceLimit = 30.0;

//Frame Range
constexpr double kMinFrameInX = 0.0;
constexpr double kMaxFrameInX = 1080.0;
constexpr double kMinFrameInY = 420.0;
constexpr double kMaxFrameInY = 1500.0;

constexpr int kHightLandmark = 0;
constexpr int kLowestLandmark = 0;
constexpr int kRightMostLandmark = 0;
constexpr int kLeftMostLandmark = 0;

bool isReferenceFrame(
  double pitch,
  double yaw,
  double roll
) {
  bool pitch_in_range = false, yaw_in_range = false, roll_in_range = false;
  if ( (pitch >= kReferencePitchMin && pitch <= 360.0) ||
       (pitch >= 0.0 && pitch <= kReferencePitchMax)
  ) {
    pitch_in_range = true;
    //std::cout << "Reference Frame Pitch Angle : " << pitch << std::endl;
  }

  if ( (yaw >= kReferenceYawMin && yaw <= 360.0) ||
       (yaw >= 0.0 && yaw <= kReferenceYawMax)
  ) {
    yaw_in_range = true;
    //std::cout << "Reference Frame Yaw Angle : " << yaw << std::endl;
  }

  if ( (roll >= kReferenceRollMin && roll <= 360.0) ||
       (roll >= 0.0 && roll <= kReferenceRollMax)
  ) {
    roll_in_range = true;
    //std::cout << "Reference Frame Roll Angle : " << roll << std::endl;
  }

  return pitch_in_range && yaw_in_range && roll_in_range;
}

bool isWithinReferenceRange(
  double pitch,
  double yaw,
  double roll
) {
  bool pitch_in_range = false, yaw_in_range = false, roll_in_range = false;
  if ( (pitch >= kReferenceRangePitchMin && pitch <= 360.0) ||
       (pitch >= 0.0 && pitch <= kReferenceRangePitchMax)
  ) {
    pitch_in_range = true;
    //LOG(INFO) << "Reference Range Pitch Angle : " << pitch;
  }

  if ( (yaw >= kReferenceRangeYawMin && yaw <= 360.0) ||
       (yaw >= 0.0 && yaw <= kReferenceRangeYawMax)
  ) {
    yaw_in_range = true;
    //LOG(INFO) << "Reference Range Yaw Angle : " << yaw;
  }

  if ( (roll >= kReferenceRangeRollMin && roll <= 360.0) ||
       (roll >= 0.0 && roll <= kReferenceRangeRollMax)
  ) {
    roll_in_range = true;
    //LOG(INFO) << "Reference Range Roll Angle : " << roll;
  }

  return pitch_in_range && yaw_in_range && roll_in_range;
}

bool isTooFar(
  double distance
) {
  if (distance > kMaxDistanceLimit) {
    return true;
  }
  return false;
}

bool isTooClose(
  double distance
) {
  if (distance < kMinDistanceLimit) {
    return true;
  }
  return false;
}

bool isWithinFrame(
  ::mediapipe::NormalizedLandmarkList const& landmarks
) {
  //todo
  return true;
  /*if (kMinFrameInY < landmarks.landmark(kHightLandmark).y() &&
      kMaxFrameInY > landmarks.landmark(kLowestLandmark).y() &&
      kMinFrameInX < landmarks.landmark(kRightMostLandmark).x() &&
      kMaxFrameInX > landmarks.landmark(kLeftMostLandmark).x()
  ) {
    return true;
  }
  return false;*/
}
