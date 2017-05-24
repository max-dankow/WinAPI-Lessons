#pragma once
#include <cmath>

#include "VideoCaptureService.h"
#include "Bitmap.h"

class CMotionDetector
{
public:
    static const int Threshhold = 32;
    CMotionDetector();
    ~CMotionDetector();

    static std::vector< std::vector< int > > Detect(CBitmap& previous, CBitmap& current) {
        std::vector< std::vector< int > > moveMask(current.GetHeight(), std::vector< int >(current.GetWidth(), 0));
        for (int i = 0; i < current.GetWidth(); i++) {
            for (int j = 0; j < current.GetHeight(); j++) {
                auto newPixel = current.GetPixelAt(i, j);
                auto oldPixel = previous.GetPixelAt(i, j);
                auto dr = std::abs(newPixel.GetR() - oldPixel.GetR());
                auto dg = std::abs(newPixel.GetG() - oldPixel.GetG());
                auto db = std::abs(newPixel.GetB() - oldPixel.GetB());

                if (((dr + dg + db) / 3) > Threshhold) {
                    moveMask[j][i] = 1;
                } else {
                    moveMask[j][i] = 0;
                }
            }
        }
        std::vector< std::vector< int > > moveMaskWithoutNoise(current.GetHeight(), std::vector< int >(current.GetWidth(), 0));
        simpleNoiseFiter(moveMask, moveMaskWithoutNoise, current.GetHeight(), current.GetWidth());
        //saveMoveMaskToBitmap(moveMaskWithoutNoise, previous);
        return moveMaskWithoutNoise;
    }

    static void simpleNoiseFiter(const std::vector< std::vector< int > > &moveMask, 
        std::vector< std::vector< int > > &dest, int height, int width) {
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                float sum = 0.f;
                int count = 0;
                for (int di = -2; di <= 2; di++) {
                    for (int dj = -2; dj <= 2; dj++) {
                        int x = i + di;
                        int y = j + dj;
                        if (x >= 0 && x < width && y >= 0 && y < height) {
                            sum += moveMask[j][i];
                            count += 1;
                        }
                    }
                }
                if (2 * sum < count) {
                    dest[j][i] = 1;
                }
                else {
                    dest[j][i] = 0;
                }
            }
        }
    }

    static void saveMoveMaskToBitmap(const std::vector< std::vector< int > > &moveMask, CBitmap& dest) {
        for (int h = 0; h < dest.GetWidth(); h++) {
            for (int w = 0; w < dest.GetHeight(); w++) {
                if (moveMask[w][h] == 1) {
                    dest.SetPixelAt(h, w, { 255, 255, 255 });
                }
                else {
                    dest.SetPixelAt(h, w, { 0, 0, 0 });
                }
            }
        }
    }
};
