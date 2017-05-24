#include "MotionDetector.h"
#include <array>

std::vector<std::vector<int>> CMotionDetector::Detect(const CBitmap & previous, const CBitmap & current) {
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
            }
            else {
                moveMask[j][i] = 0;
            }
        }
    }
    std::vector< std::vector< int > > moveMaskWithoutNoise(current.GetHeight(), std::vector< int >(current.GetWidth(), 0));
    SimpleNoiseFiter(moveMask, moveMaskWithoutNoise, current.GetHeight(), current.GetWidth());
    return moveMaskWithoutNoise;
}

void erosion(const std::vector<std::vector<int>>& source,
    const std::vector<std::vector<int>>& mask,
    std::vector<std::vector<int>>& dest,
    int height, int width, int maskH, int maskW) 
{
    dest.assign(height, std::vector<int>(width, 0));

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            bool logicalAnd = true;
            for (int di = -maskW; di <= maskW; di++) {
                for (int dj = -maskH; dj <= maskH; dj++) {
                    int x = i + di;
                    int y = j + dj;
                    if (x >= 0 && x < width && y >= 0 && y < height) {
                        if (mask[dj + maskH][di + maskW] == 1) {
                            if (source[y][x] < 0.5) {
                                logicalAnd = false;
                                break;
                            }
                        }
                    }
                }
                if (logicalAnd == false) {
                    break;
                }
            }
            if (logicalAnd) {
                dest[j][i] = 0;
            }
            else {
                dest[j][i] = 1;
            }
        }
    }
}

void dilatation(const std::vector<std::vector<int>>& source,
    const std::vector<std::vector<int>>& mask,
    std::vector<std::vector<int>>& dest,
    int height, int width, int maskH, int maskW)
{
    dest.assign(height, std::vector<int>(width, 0));

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            if (source[j][i] > 0.5) {
                for (int di = -maskW; di <= maskW; di++) {
                    for (int dj = -maskH; dj <= maskH; dj++) {
                        int x = i + di;
                        int y = j + dj;
                        if (x >= 0 && x < width && y >= 0 && y < height) {
                            if (mask[dj + maskH][di + maskW] == 1) {
                                dest[y][x] = 1;
                            }
                        }
                    }
                }
            }
        }
    }
}


void CMotionDetector::SimpleNoiseFiter(const std::vector<std::vector<int>>& moveMask, std::vector<std::vector<int>>& dest, int height, int width) {
    const int maskH = 1, maskW = 1;
    const std::vector<std::vector<int>> mask = { {0,1,0}, {1, 1, 1}, {0, 1, 0} };


    std::vector<std::vector<int>> erosed(height, std::vector<int>(width, 0));
    erosion(moveMask, mask, dest, height, width, maskH, maskW);
    //dilatation(erosed, mask, dest, height, width, maskH, maskW);
}

void CMotionDetector::SaveMoveMaskToBitmap(const std::vector<std::vector<int>>& moveMask, CBitmap & dest) {
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
