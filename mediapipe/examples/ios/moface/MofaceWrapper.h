#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>
#include "CoreGraphics/CoreGraphics.h"

@class MofaceFramework;

typedef NS_ENUM(NSUInteger, EventType) {
    RightActionDetected,
    LeftActionDetected,
    UpActionDetected,
    DownActionDetected,
    EyesActionDetected,
    MouthActionDetected,
    AngryActionDetected,
    HappyActionDetected,
    ReferenceDetected
};

typedef NS_ENUM(NSUInteger, DistanceStatus) {
    GoodDistance,
    TooFar,
    TooClose
};

typedef NS_ENUM(NSUInteger, PoseStatus) {
    HoldStill,
    Moving
};

typedef NS_ENUM(NSUInteger, WarningType) {
    MoveToCenter,
    NoFace,
    Timeout,
};

typedef void(^EventCallback)(EventType event);
typedef void(^WarningCallback)(WarningType warning);
typedef void(^SignalCallback)(
    DistanceStatus distanceStatus,
    PoseStatus poseStatus,
    bool withinFrame,
    CGPoint point
);

@interface MofaceWrapper : NSObject
    - (instancetype)init;
    - (void)setCallbacks:(EventCallback)eventCallback signalCallback:(SignalCallback)signalCallback warningCallback:(WarningCallback)warningCallback;
    - (void)feed:(CMSampleBufferRef)sampleBuffer;
    - (nonnull NSString *)stop;
@end
