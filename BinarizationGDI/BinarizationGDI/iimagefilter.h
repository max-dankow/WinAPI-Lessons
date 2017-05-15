#ifndef IIMAGEFILTER_H
#define IIMAGEFILTER_H

#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <assert.h>

#include "vector"

struct CColor {
    byte r, g, b;
};

template <typename T>
class CMatrix {
public:
    CMatrix() { }

    CMatrix(int height, int width) : height(height), width(width) {
        data.assign(height * width, T());
    }

    CMatrix(CMatrix&&) = default;
    CMatrix& operator=(CMatrix&& other) = default;

    T GetAt(int col, int row) const {
        assert(col >= 0 && col < width && row >= 0 && row < height);
        return data[row * width + col];
    }

    void SetAt(const T& color, int col, int row) {
        assert(col >= 0 && col < width && row >= 0 && row < height);
        data[row * width + col] = color;
    }

    int GetHeight() const {
        return height;
    }

    int GetWidth() const {
        return width;
    }
private:
    std::vector<T> data;
    int height, width;
};

class IImageFilter {
public:
    virtual CMatrix<CColor> Apply(const CMatrix<CColor>&) = 0;
};

#endif // IIMAGEFILTER_H
