#pragma once
#include <cmath>

#include "VideoCaptureService.h"
#include "Bitmap.h"

class CMotionDetector
{
public:
    static const int Threshhold = 32;

    static std::vector< std::vector< int > > Detect(const CBitmap& previous, const CBitmap& current);

    static void SimpleNoiseFiter(const std::vector< std::vector< int > > &moveMask, 
        std::vector< std::vector< int > > &dest, int height, int width);

    static void SaveMoveMaskToBitmap(const std::vector< std::vector< int > > &moveMask, CBitmap& dest);
};
