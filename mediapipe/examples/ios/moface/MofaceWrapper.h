#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

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

typedef NS_ENUM(NSUInteger, WarningType) {
    TooFar,
    TooClose,
    MoveToCenter,
    NoFace,
    Timeout,
};

typedef void(^EventCallback)(EventType event);
typedef void(^WarningCallback)(WarningType warning);

@interface MofaceWrapper : NSObject
    - (instancetype)init;
    - (void)setCallbacks:(EventCallback)eventCallback warningCallback:(WarningCallback)warningCallback;
    - (void)feed:(CMSampleBufferRef)sampleBuffer;
    - (NSString *)stop;
@end
