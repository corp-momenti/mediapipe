
#import "MofaceWrapper.h"

//#import "mediapipe/objc/MPPCameraInputSource.h"
#import "mediapipe/objc/MPPGraph.h"

#include <map>
#include <string>
#include <utility>

#include "mediapipe/framework/formats/matrix_data.pb.h"
#include "mediapipe/framework/calculator_framework.h"
#include "mediapipe/modules/face_geometry/protos/face_geometry.pb.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/examples/desktop/moface_desktop/moface_calculator/moface_calculator.h"

static NSString* const kGraphName = @"moface_mobile_gpu";

static const char* kInputStream = "input_video";
static const char* kOutputStream = "output_video";

static const char* kMultiFaceGeometryStream = "multi_face_geometry";
static const char* kMultiFaceLandmark = "multi_smoothed_face_landmarks";
static const char* kVideoQueueLabel = "com.google.mediapipe.example.videoQueue";

static const int kMatrixTranslationZIndex = 14;

static EventCallback eventCallback_;
static WarningCallback warningCallback_;
static SignalCallback signalCallback_;

@interface MofaceWrapper() <MPPGraphDelegate>

@property (nonatomic) MPPGraph* mediapipeGraph;
@property (nonatomic) moface::MofaceCalculator* moface_calculator;

@property (nonatomic) std::vector<::mediapipe::NormalizedLandmarkList> multi_face_landmarks;
@property (nonatomic) std::vector<::mediapipe::face_geometry::FaceGeometry> multi_face_geometry;

@end

@implementation MofaceWrapper {
    /// Process camera frames on this queue.
    dispatch_queue_t _videoQueue;
}

#pragma mark - Cleanup methods

- (void)dealloc {
    self.mediapipeGraph.delegate = nil;
    [self.mediapipeGraph cancel];
    // Ignore errors since we're cleaning up.
    [self.mediapipeGraph closeAllInputStreamsWithError:nil];
    [self.mediapipeGraph waitUntilDoneWithError:nil];
}

#pragma mark - MediaPipe graph methods

+ (MPPGraph*)loadGraphFromResource:(NSString*)resource {
    // Load the graph config resource.
    NSError* configLoadError = nil;
    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    if (!resource || resource.length == 0) {
        return nil;
    }

    NSURL* graphURL = [bundle URLForResource:resource withExtension:@"binarypb"];
    NSData* data = [NSData dataWithContentsOfURL:graphURL options:0 error:&configLoadError];
    if (!data) {
        NSLog(@"Failed to load MediaPipe graph config: %@", configLoadError);
        return nil;
    }

    // Parse the graph config resource into mediapipe::CalculatorGraphConfig proto object.
    mediapipe::CalculatorGraphConfig config;
    config.ParseFromArray(data.bytes, data.length);

    // Create MediaPipe graph with mediapipe::CalculatorGraphConfig proto object.
    MPPGraph* newGraph = [[MPPGraph alloc] initWithGraphConfig:config];
    [newGraph addFrameOutputStream:kOutputStream outputPacketType:MPPPacketTypePixelBuffer];
    [newGraph addFrameOutputStream:kMultiFaceGeometryStream outputPacketType:MPPPacketTypeRaw];
    [newGraph addFrameOutputStream:kMultiFaceLandmark outputPacketType:MPPPacketTypeRaw];
    return newGraph;
}

void eventNotifier(moface::MoFaceEventType event) {
  switch (event) {
    case moface::eRightActionDetected:
        //std::cout << "RightActionDetected" << std::endl;
        eventCallback_(RightActionDetected);
    break;
    case moface::eLeftActionDetected:
        //std::cout << "LeftActionDetected" << std::endl;
        eventCallback_(LeftActionDetected);
    break;
    case moface::eUpActionDetected:
        //std::cout << "UpActionDetected" << std::endl;
        eventCallback_(UpActionDetected);
    break;
    case moface::eDownActionDetected:
        //std::cout << "DownActionDetected" << std::endl;
        eventCallback_(DownActionDetected);
    break;
    case moface::eBlinkActionDetected:
        //std::cout << "EyesActionDetected" << std::endl;
        eventCallback_(EyesActionDetected);
    break;
    case moface::eMouthActionDetected:
        //std::cout << "MouthActionDetected" << std::endl;
        eventCallback_(MouthActionDetected);
    break;
    case moface::eAngryActionDetected:
        //std::cout << "AngryActionDetected" << std::endl;
        eventCallback_(AngryActionDetected);
    break;
    case moface::eHappyActionDetected:
        //std::cout << "HappyActionDetected" << std::endl;
        eventCallback_(HappyActionDetected);
    break;
    case moface::eReferenceDetected:
        //std::cout << "ReferenceDetected" << std::endl;
        eventCallback_(ReferenceDetected);
    //   std::string tmp_file_name = std::string(kDetectedReferenceFramePath) + "/" + moface::generate_uuid_v4() + ".png";
    //   cv::imwrite(tmp_file_name, output_frame_mat);
    break;
  }
}

void signalNotifier(moface::MofaceDistanceStatus distance, moface::MofacePoseStatus pose, bool inFrame, cv::Point center) {
  //show signals
  DistanceStatus distanceStatus;
  PoseStatus poseStatus;
  switch (distance) {
    case moface::eGoodDistance:
        distanceStatus = GoodDistance;
      break;
    case moface::eTooFar:
        distanceStatus = TooFar;
      break;
    case moface::eTooClose:
        distanceStatus = TooClose;
      break;
  }
  switch (pose) {
    case moface::eHoldStill:
        poseStatus = HoldStill;
      break;
    case moface::eMoving:
        poseStatus = Moving;
      break;
  }
  bool withinFrame = inFrame;
  signalCallback_(distanceStatus, poseStatus, withinFrame, CGPointMake(center.x, center.y));
}

void warningNotifier(moface::MoFaceWarningType event) {
    switch (event) {
        case moface::eGoingBackward:
        break;
        case moface::eTooSlow:
        break;
        case moface::eInvalidDragAction:
        break;
        case moface::eNoFace:
            //std::cout << "NoFace" << std::endl;
            warningCallback_(NoFace);
        break;
        case moface::eTimeout:
            warningCallback_(Timeout);
        break;
    }
}

void geometryNotifier(double pitch, double yaw, double roll, double distance) {

}

- (instancetype)init
{
    self = [super init];
    if (self) {

        dispatch_queue_attr_t qosAttribute = dispatch_queue_attr_make_with_qos_class(
             DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INTERACTIVE, /*relative_priority=*/0);
        _videoQueue = dispatch_queue_create(kVideoQueueLabel, qosAttribute);

        self.mediapipeGraph = [[self class] loadGraphFromResource:kGraphName];
        self.mediapipeGraph.delegate = self;
        // Set maxFramesInFlight to a small value to avoid memory contention for real-time processing.
        self.mediapipeGraph.maxFramesInFlight = 10;
        [self startGraph];
        NSLog(@"Initilaized MofaceFramework!!!");
    }
    return self;
}

- (void)setCallbacks:(EventCallback)eventCallback signalCallback:(SignalCallback)signalCallback warningCallback:(WarningCallback)warningCallback {
    eventCallback_ = eventCallback;
    signalCallback_ = signalCallback;
    warningCallback_ = warningCallback;
    self.moface_calculator = new moface::MofaceCalculator(
        eventNotifier, signalNotifier, warningNotifier, geometryNotifier
    );
}

- (void)provideHint:(DetectionHintType) hint {
    moface::MofaceDetectionHintType type;
    switch (hint) {
        case DetectionHintType::HintBlink:
            type = moface::eDetectBlink;
            break;
        case DetectionHintType::HintHappy:
            type = moface::eDetectHappy;
            break;
        case DetectionHintType::HintAngry:
            type = moface::eDetectAngry;
            break;
        default:
            type = moface::eDetectDrag;
            break;
    }
    self.moface_calculator->setHint(type);
}

- (void)startGraph {
    // Start running self.mediapipeGraph.
    NSError* error;
    if (![self.mediapipeGraph startWithError:&error]) {
        NSLog(@"Failed to start graph: %@", error);
    }
}

#pragma mark - MPPGraphDelegate methods

- (void)mediapipeGraph:(MPPGraph*)graph
    didOutputPixelBuffer:(CVPixelBufferRef)pixelBuffer
              fromStream:(const std::string&)streamName {
  if (streamName == kOutputStream) {
    // Display the captured image on the screen.
    //CVPixelBufferRelease(pixelBuffer);
    CVPixelBufferRetain(pixelBuffer);
    CVPixelBufferRelease(pixelBuffer);
  }
}

// Receives a raw packet from the MediaPipe graph. Invoked on a MediaPipe worker thread.
//
// This callback demonstrates how the output face geometry packet can be obtained and used in an
// iOS app. As an example, the Z-translation component of the face pose transform matrix is logged
// for each face being equal to the approximate distance away from the camera in centimeters.
- (void)mediapipeGraph:(MPPGraph*)graph
     didOutputPacket:(const ::mediapipe::Packet&)packet
          fromStream:(const std::string&)streamName {
    if (streamName == kMultiFaceLandmark) {
        if (packet.IsEmpty()) {
          NSLog(@"[TS:%lld] No face landmarks", packet.Timestamp().Value());
          return;
        }
        _multi_face_landmarks.clear();
        _multi_face_landmarks = packet.Get<std::vector<::mediapipe::NormalizedLandmarkList>>();
        // NSLog(@"[TS:%lld] Number of face instances with landmarks: %lu", packet.Timestamp().Value(),
        //       multi_face_landmarks.size());
        // for (int face_index = 0; face_index < multi_face_landmarks.size(); ++face_index) {
        //   const auto& landmarks = multi_face_landmarks[face_index];
        //   NSLog(@"\tNumber of landmarks for face[%d]: %d", face_index, landmarks.landmark_size());
        //   for (int i = 0; i < landmarks.landmark_size(); ++i) {
        //     NSLog(@"\t\tLandmark[%d]: (%f, %f, %f)", i, landmarks.landmark(i).x(),
        //           landmarks.landmark(i).y(), landmarks.landmark(i).z());
        //   }
        // }
    }
    if (streamName == kMultiFaceGeometryStream) {
        if (packet.IsEmpty()) {
            NSLog(@"[TS:%lld] No face geometry", packet.Timestamp().Value());
            return;
        }
        _multi_face_geometry.clear();
        _multi_face_geometry =
            packet.Get<std::vector<::mediapipe::face_geometry::FaceGeometry>>();

        /*for (int faceIndex = 0; faceIndex < multiFaceGeometry.size(); ++faceIndex) {
        const auto& faceGeometry = multiFaceGeometry[faceIndex];
        NSLog(@"\tApprox. distance away from camera for face[%d]: %.6f cm", faceIndex,
                -faceGeometry.pose_transform_matrix().packed_data(kMatrixTranslationZIndex));

            float pitch = asin(faceGeometry.pose_transform_matrix().packed_data(6));
            float yaw, roll;
            if (cos(pitch) > 0.0001) {
                yaw = atan2(faceGeometry.pose_transform_matrix().packed_data(2), faceGeometry.pose_transform_matrix().packed_data(10));
                roll = atan2(faceGeometry.pose_transform_matrix().packed_data(4), faceGeometry.pose_transform_matrix().packed_data(5));
            } else {
                yaw = 0.0;
                roll = atan2(-faceGeometry.pose_transform_matrix().packed_data(1), faceGeometry.pose_transform_matrix().packed_data(0));
            }

            pitch = fmod(((pitch * 180.0 / M_PI) + 360.0), 360.0);
            yaw = fmod(((yaw * 180.0 / M_PI) + 360.0), 360);
            roll = fmod(((roll * 180.0 / M_PI) + 360.0), 360);

            //NSLog(@"pitch [%f], yaw [%f], roll [%f]", pitch, yaw, roll);
            NSString* info = [NSString stringWithFormat:@"pitch [%f], \nyaw [%f], \nroll [%f]",
                            pitch,
                            yaw,
                            roll];

    //        euler.x = asinf(-mat._32);                  // Pitch
    //        if (cosf(euler.x) > 0.0001)                 // Not at poles
    //        {
    //            euler.y = atan2f(mat._31, mat._33);     // Yaw
    //            euler.z = atan2f(mat._12, mat._22);     // Roll
    //        }
    //        else
    //        {
    //            euler.y = 0.0f;                         // Yaw
    //            euler.z = atan2f(-mat._21, mat._11);    // Roll
    //        }
        }*/
    }
    if (_multi_face_geometry.size() != 0 && _multi_face_landmarks.size() != 0) {
        self.moface_calculator->sendObservations(_multi_face_landmarks[0], _multi_face_geometry[0]);
        _multi_face_landmarks.clear();
        _multi_face_geometry.clear();
    }
}

#pragma mark - MPPInputSourceDelegate methods

- (void)feed:(CMSampleBufferRef)sampleBuffer {
    CVPixelBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CMTime timestamp = CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
    mediapipe::Timestamp graphTimestamp(static_cast<mediapipe::TimestampBaseType>(
        mediapipe::Timestamp::kTimestampUnitsPerSecond * CMTimeGetSeconds(timestamp)));
    if (![self.mediapipeGraph sendPixelBuffer:imageBuffer
                              intoStream:kInputStream
                              packetType:MPPPacketTypePixelBuffer
                              timestamp:graphTimestamp]) {
                                  std::cout << "send buffer error !!!" << std::endl;
                              }
}

- (NSString *)stop {
    NSString *ret_string = [NSString stringWithCString:self.moface_calculator->getFaceObservation().c_str()];
    self.moface_calculator->reset();
    return ret_string;
}

@end
