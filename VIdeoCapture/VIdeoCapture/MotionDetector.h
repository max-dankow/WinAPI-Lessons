#pragma once
#include "VideoCaptureService.h"
#include "Bitmap.h"

class CMotionDetector
{
public:
    CMotionDetector();
    ~CMotionDetector();

    static void Detect(HDC hDC, CBitmap& previous, CBitmap& current) {
        // TODO: very dirty
        auto hMemDC = CreateCompatibleDC(hDC);
        for (int i = 0; i < current.GetWidth(); i++) {
            for (int j = 0; j < current.GetHeight(); j++) {
                COLORREF pixelColor = GetPixel(hMemDC, i, j);
                if (GetRValue(pixelColor) + GetGValue(pixelColor) + GetBValue(pixelColor) < 0XFF) {
                    SetPixel(hMemDC, i, j, RGB(0, 0, 0));
                }
                else {
                    SetPixel(hMemDC, i, j, RGB(0XFF, 0xFF, 0xFF));
                }
            }
        }
    }
};
