//
//  MOImageConverter.m
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
#import "MOImageConverter.h"
//===== OPENCV Module Imported Above =====

#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>
#import <CoreFoundation/CoreFoundation.h>

@implementation MOImageConverter

+ (CVPixelBufferRef)convertBGRAToCvPixelBuffer:(cv::Mat)bgraImage {
    CVPixelBufferRef pixelBuffer;

    size_t width = bgraImage.cols;
    size_t height = bgraImage.rows;
    size_t bytesPerRow = width*bgraImage.elemSize();

    CVPixelBufferCreate(
                        NULL,
                        width,
                        height,
                        kCVPixelFormatType_32BGRA,
                        (__bridge CFDictionaryRef)@{
                            (id)kCVPixelBufferIOSurfacePropertiesKey: @{}
                        },
                        &pixelBuffer);
    CVPixelBufferLockBaseAddress(pixelBuffer, 0);
    //void *bytes = CVPixelBufferGetBaseAddress(pixelBuffer);
    char* bytes = (char*)CVPixelBufferGetBaseAddress(pixelBuffer);
    size_t bytesPerRowForIOSurf = CVPixelBufferGetBytesPerRow(pixelBuffer);
    memcpy(bytes,bgraImage.data,bytesPerRow*height);
    for(int row = 0; row < height ; row ++){
        char* dest = bytes + bytesPerRowForIOSurf*row;
        char* src = (char*)bgraImage.data + bytesPerRow*row;
        memcpy(dest,src,bytesPerRow);
    }
    CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

    return pixelBuffer;
}

+ (cv::Mat)toMat:(CMSampleBufferRef)sampleBuffer {
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    CVPixelBufferLockBaseAddress(imageBuffer, 0);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
    // Get the pixel buffer width and height
    size_t width = CVPixelBufferGetWidth(imageBuffer);
    size_t height = CVPixelBufferGetHeight(imageBuffer);
    void *baseaddress = CVPixelBufferGetBaseAddress(imageBuffer);
    cv::Mat image((int)height, (int)width, CV_8UC4, baseaddress, bytesPerRow);
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
    return image;
}

+ (cv::Mat)toBgrMat:(CMSampleBufferRef)sampleBuffer {
    cv::Mat image = [self toMat:sampleBuffer];
    cv::Mat result(image.cols,image.rows,CV_8UC3);
    cv::cvtColor(image, result, cv::COLOR_BGRA2BGR);
    return result;
}

+ (cv::Mat)toMatFromPixelBuf:(CVPixelBufferRef)pixelBuf {
    CVPixelBufferLockBaseAddress(pixelBuf, 0);
    size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuf);
    // Get the pixel buffer width and height
    size_t width = CVPixelBufferGetWidth(pixelBuf);
    size_t height = CVPixelBufferGetHeight(pixelBuf);
    void *baseaddress = CVPixelBufferGetBaseAddress(pixelBuf);
    cv::Mat image((int)height, (int)width, CV_8UC4, baseaddress, bytesPerRow);
    CVPixelBufferUnlockBaseAddress(pixelBuf, 0);
    return image;
}

+ (cv::Mat)toBgrMatFromPixelBuf:(CVPixelBufferRef)pixelBuf {
    cv::Mat image = [self toMatFromPixelBuf:pixelBuf];
    cv::Mat result(image.cols,image.rows,CV_8UC3);
    cv::Mat test(image.cols,image.rows,CV_8UC1);
    cv::cvtColor(image, test, cv::COLOR_BGRA2GRAY);
    if (cv::countNonZero(test) == 0) {
        NSLog(@"feed buffer is black");
        test.release();
        return cv::Mat();
    }
    test.release();
    cv::cvtColor(image, result, cv::COLOR_BGRA2BGR);
    return result;
}

+ (UIImage *)fromCvMat:(cv::Mat)cvMat {
    NSData *data = [NSData dataWithBytes:cvMat.data length:cvMat.elemSize()*cvMat.total()];
    CGColorSpaceRef colorSpace;

    if (cvMat.elemSize() == 1) {
        NSLog(@"fromCvMat Mat is gray");
        colorSpace = CGColorSpaceCreateDeviceGray();
    } else {
        colorSpace = CGColorSpaceCreateDeviceRGB();
    }

    CGDataProviderRef provider = CGDataProviderCreateWithCFData((__bridge CFDataRef)data);

    // Creating CGImage from cv::Mat
    CGImageRef imageRef = CGImageCreate(cvMat.cols,                                 //width
                                        cvMat.rows,                                 //height
                                        8,                                          //bits per component
                                        8 * cvMat.elemSize(),                       //bits per pixel
                                        cvMat.step[0],                            //bytesPerRow
                                        colorSpace,                                 //colorspace
                                        kCGImageAlphaNone|kCGBitmapByteOrderDefault,// bitmap info
                                        provider,                                   //CGDataProviderRef
                                        NULL,                                       //decode
                                        false,                                      //should interpolate
                                        kCGRenderingIntentDefault                   //intent
                                        );

    // Getting UIImage from CGImage
    UIImage *finalImage = [UIImage imageWithCGImage:imageRef];
    CGImageRelease(imageRef);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(colorSpace);

    return finalImage;
}

@end
