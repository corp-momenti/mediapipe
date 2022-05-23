#include <Foundation/Foundation.h>
#include <AVFoundation/AVFoundation.h>

@class MofaceFramework;

typedef NS_ENUM(NSUInteger, EventType) {
    RightActionDetected,
    LeftActionDetected,
    UpActionDetected,
    DownActionDetected,
    eyesActionDetected,
    mouthActionDetected,
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

@interface MofaceFramework : NSObject
    - (instancetype)init NS_UNAVAILABLE;
    - (instancetype)initWithCallbacks:(EventCallback)eventCallback warningCallback:(WarningCallback)warningCallback;
    - (void)feed:(CVPixelBufferRef)imageBuffer;
    - (NSString *)stop;
//@property (nonatomic, copy) PickerCallback callback;
@end
