#include <cstdlib>
#include <vector>

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
    //LOG(INFO) << "Reference Frame Pitch Angle : " << pitch;
  }

  if ( (yaw >= kReferenceYawMin && yaw <= 360.0) ||
       (yaw >= 0.0 && yaw <= kReferenceYawMax)
  ) {
    yaw_in_range = true;
    //LOG(INFO) << "Reference Frame Yaw Angle : " << yaw;
  }

  if ( (roll >= kReferenceRollMin && roll <= 360.0) ||
       (roll >= 0.0 && roll <= kReferenceRollMax)
  ) {
    roll_in_range = true;
    //LOG(INFO) << "Reference Frame Roll Angle : " << roll;
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
