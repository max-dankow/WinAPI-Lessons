#pragma once
#include <cmath>

#include "VideoCaptureService.h"
#include "Bitmap.h"

class CMotionDetector
{
public:
    CMotionDetector();
    ~CMotionDetector();

    static void Detect(CBitmap& previous, CBitmap& current) {
        for (int i = 0; i < current.GetWidth(); i++) {
            for (int j = 0; j < current.GetHeight(); j++) {
                auto newPixel = current.GetPixelAt(i, j);
                auto oldPixel = previous.GetPixelAt(i, j);
                if (std::abs(newPixel.GetGrey() - oldPixel.GetGrey()) > 32) {
                    previous.SetPixelAt(i, j, { 255, 255, 255 });
                } else {
                    previous.SetPixelAt(i, j, { 0, 0, 0 });
                }
            }
        }
    }
};
