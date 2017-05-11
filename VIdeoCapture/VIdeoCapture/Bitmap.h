#pragma once
#include <windows.h>
#include <assert.h>
#include "Utils.h"

class CPixel {
public:
    CPixel(BYTE r, BYTE g, BYTE b) : r(r), g(g), b(b) {}

    COLORREF GetColorRef() const {
        return RGB(r, g, b);
    }
    BYTE GetR() const {
        return r;
    }
    BYTE GetG() const {
        return g;
    }
    BYTE GetB() const {
        return b;
    }
    double GetGrey() const {
        return 0.299 * static_cast<double>(r) + 0.587 * static_cast<double>(g) + 0.114 * static_cast<double>(b);
    }

private:
    BYTE r, g, b;
};

class CBitmap {
public:
    CBitmap() : bitmap(NULL), memoryContext(NULL) {}

    CBitmap(const CBitmap&) = delete;
    CBitmap operator=(const CBitmap&) = delete;

    CBitmap(CBitmap&& other) {
        Release();
        this->bitmap = other.bitmap;
        this->memoryContext = other.memoryContext;
        this->height = other.height;
        this->width = other.width;
        other.invalidate();
    }

    CBitmap& operator=(CBitmap&& other) {
        Release();
        this->bitmap = other.bitmap;
        this->memoryContext = other.memoryContext;
        this->height = other.height;
        this->width = other.width;
        other.invalidate();
        return *this;
    }

    CBitmap(HDC actualContext, BITMAPINFOHEADER*&& bitmapDIB) {
        assert(bitmapDIB != NULL);
        height = bitmapDIB->biHeight;
        width = bitmapDIB->biWidth;
        bitmap = CreateDIBitmap(actualContext, bitmapDIB, CBM_INIT, reinterpret_cast<byte*>(bitmapDIB + 1), reinterpret_cast<BITMAPINFO*>(bitmapDIB), DIB_RGB_COLORS);
        CoTaskMemFree(bitmapDIB);
        bitmapDIB = NULL;
        memoryContext = CreateCompatibleDC(actualContext);
        SelectObject(memoryContext, bitmap);
    }

    ~CBitmap() {
        Release();
    }

    void Show(HDC actualContext, RECT imageRect) {
        BitBlt(actualContext, imageRect.left, imageRect.top, width, height, memoryContext, 0, 0, SRCCOPY);
    }

    bool IsNull() const {
        return bitmap == NULL;
    }

    void Release() {
        if (bitmap != NULL) {
            DeleteObject(bitmap);
            bitmap = NULL;
        }
        if (memoryContext != NULL) {
            DeleteDC(memoryContext);
            memoryContext = NULL;
        }
    }

    int GetHeight() const {
        return height;
    }

    int GetWidth() const {
        return width;
    }

    CPixel GetPixelAt(int x, int y) const {
        assert(x >= 0 && x < width && y >= 0 && y < height);
        auto color = GetPixel(memoryContext, x, y);
        return { GetRValue(color), GetGValue(color), GetBValue(color) };
    }

    void SetPixelAt(int x, int y, CPixel newColor) {
        assert(x >= 0 && x < width && y >= 0 && y < height);
        SetPixel(memoryContext, x, y, newColor.GetColorRef());
    }
private:
    void invalidate() {
        bitmap = NULL;
        memoryContext = NULL;
        height = 0;
        width = 0;
    }

    HDC memoryContext;
    HBITMAP bitmap;
    int height, width;
};