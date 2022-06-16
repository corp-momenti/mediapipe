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
constexpr double kReferenceRangePitchMin = 352.0;
constexpr double kReferenceRangePitchMax = 8.0;
constexpr double kReferenceRangeYawMin = 352.0;
constexpr double kReferenceRangeYawMax = 8.0;
constexpr double kReferenceRangeRollMin = 355.0;
constexpr double kReferenceRangeRollMax = 5.0;

//Distance Range
constexpr double kMaxDistanceLimit = 40.0;
constexpr double kMinDistanceLimit = 35.0;

//Frame Range
constexpr double kMinFrameInX = 0.0;
constexpr double kMaxFrameInX = 1080.0;
constexpr double kMinFrameInY = 420.0;
constexpr double kMaxFrameInY = 1500.0;

constexpr int kHightLandmark = 0;
constexpr int kLowestLandmark = 0;
constexpr int kRightMostLandmark = 0;
constexpr int kLeftMostLandmark = 0;

constexpr double kCenterRange = 50.0;

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
  double width, double height,
  ::mediapipe::NormalizedLandmarkList const& landmarks
) {
  double frame_center_x, frame_center_y;
  if (width < height) {
    double left_x = 0.0;
    double left_y = (height - width) * 0.5;
    double frame_width = width;
    double frame_height = width;
    frame_center_x = width * 0.5;
    frame_center_y = (left_y + left_y + frame_height) * 0.5;
  } else if (width > height) {
    double left_x = (width - height) * 0.5;
    double left_y = 0.0;
    double frame_width = height;
    double frame_height = height;
    frame_center_x = (left_x + left_x + frame_width) * 0.5;
    frame_center_y = height * 0.5;
  } else { // width == height
    frame_center_x = width * 0.5;
    frame_center_y = height * 0.5;
  }

  double nose_tip_x = landmarks.landmark(4).x() * width;
  double nose_tip_y = landmarks.landmark(4).y() * height;

  if ((nose_tip_x >= frame_center_x - kCenterRange && nose_tip_x <= frame_center_x + kCenterRange) &&
     (nose_tip_y >= frame_center_y && nose_tip_y <= frame_center_y + 2 * kCenterRange))
  {
    return true;
  }

  return false;
}
