#pragma once
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <memory>
#include "iimagefilter.h"

class CBitmapBitsLocker {
public:
    CBitmapBitsLocker(Gdiplus::Bitmap* bitmap) : bitmap(bitmap), bitmapData(new Gdiplus::BitmapData) {
        Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
        bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, bitmapData.get());
    }

    ~CBitmapBitsLocker() {
        if (bitmap != nullptr) {
            bitmap->UnlockBits(bitmapData.get());
        }
    }

    Gdiplus::BitmapData* GetData() {
        return bitmapData.get();
    }
private:
    Gdiplus::Bitmap* bitmap;
    std::unique_ptr<Gdiplus::BitmapData> bitmapData;
};

class CImageReader {
public:
    static CMatrix<CColor> read(std::wstring filename) {
        Gdiplus::Bitmap bitmap(filename.c_str());
        if (bitmap.GetLastStatus() != Gdiplus::Ok) {
            throw std::runtime_error("Fail to binarize file " + std::to_string(GetLastError()));
        }
        
        CBitmapBitsLocker bitmapLocker(&bitmap);
        Gdiplus::BitmapData* bitmapData = bitmapLocker.GetData();

        UINT* pixels = (UINT*)bitmapData->Scan0;
        CMatrix<CColor> imageMatrix(bitmap.GetHeight(), bitmap.GetWidth());

        for (UINT row = 0; row <  bitmap.GetHeight(); ++row)
        {
            for (UINT col = 0; col < bitmap.GetWidth(); ++col)
            {
                 Gdiplus::Color color(pixels[row * bitmapData->Stride / 4 + col]);
                 imageMatrix.SetAt({ color.GetR(), color.GetG(), color.GetB() }, col, row);
            }
            
        }

        return imageMatrix;
    }
};
