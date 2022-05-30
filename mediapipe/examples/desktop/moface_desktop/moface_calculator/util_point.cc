#include "util_point.h"
#include <math.h>

double euclidian_distance(
    ::mediapipe::NormalizedLandmark from,
    ::mediapipe::NormalizedLandmark to
) {
    return sqrt(pow(from.x() - to.x(), 2) + pow(from.y() - to.y(), 2));
}
