#pragma once
#include <windows.h>
#include <assert.h>
#include "Utils.h"

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