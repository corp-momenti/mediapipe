//
//  MOImageConverter.h
//  MOEngine
//
//  Created by Hoyoun Kim on 2020/01/09.
//  Copyright © 2020 momenti. All rights reserved.
//
//#import <opencv2/opencv.hpp>
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/port/opencv_highgui_inc.h"
#include "mediapipe/framework/port/opencv_imgproc_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
//===== OPENCV Module Imported Above =====
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface MOImageConverter : NSObject

+ (CVPixelBufferRef)convertBGRAToCvPixelBuffer:(cv::Mat)bgraImage;
+ (cv::Mat)toBgrMat:(CMSampleBufferRef)sampleBuffer;
+ (cv::Mat)toMatFromPixelBuf:(CVPixelBufferRef)pixelBuf;
+ (cv::Mat)toBgrMatFromPixelBuf:(CVPixelBufferRef)pixelBuf;
+ (UIImage *)fromCvMat:(cv::Mat)cvMat;

@end

NS_ASSUME_NONNULL_END
