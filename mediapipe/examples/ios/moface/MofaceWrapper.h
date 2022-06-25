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

typedef NS_ENUM(NSUInteger, DetectionHintType) {
    HintDrag,
    HintBlink,
    HintHappy,
    HintAngry
};

typedef void(^EventCallback)(EventType event);
typedef void(^WarningCallback)(WarningType warning);
typedef void(^SignalCallback)(
    DistanceStatus distanceStatus,
    PoseStatus poseStatus,
    bool withinFrame,
    CGPoint point
);

@interface DetectionResult : NSObject
    @property(nonatomic) NSString *face_observation;
    @property(nonatomic) NSString *source_file_path;
@end

@interface MofaceWrapper : NSObject
    - (instancetype)init NS_UNAVAILABLE;
    // assetPath must include the trailing backslash
    - (nonnull instancetype)initWithAssetsPath:(NSString *) assetPath;
    - (void)setCallbacks:(EventCallback)eventCallback signalCallback:(SignalCallback)signalCallback warningCallback:(WarningCallback)warningCallback;
    - (void)provideHint:(DetectionHintType) hint;
    - (void)feed:(CMSampleBufferRef)sampleBuffer;
    - (nonnull DetectionResult *)stop;
@end
