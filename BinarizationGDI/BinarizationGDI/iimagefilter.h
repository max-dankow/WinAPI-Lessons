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
    CMatrix(int height, int width) : height(height), width(width) {
        data.assign(height, std::vector<T>(width));
    }
    CMatrix(CMatrix&&) = default;

    void GetAt(int col, int row) const {
        assert(col >= 0 && col < width && row >= 0 && row < height);
        return data[row][col];
    }

    void SetAt(const CColor& color, int col, int row) {
        assert(col >= 0 && col < width && row >= 0 && row < height);
        data[row][col] = color;
    }
private:
    std::vector<std::vector<T> > data;
    int height, width;
};

class IImageFilter {
public:
    virtual CMatrix<CColor> apply(const Gdiplus::Bitmap&) = 0;
protected:
    /*QImage toImage(const std::vector< std::vector<float> >& data, const QImage& source) {
        QImage dest(source);
        for (int i = 0; i < source.height(); ++i) {
            QRgb* line = reinterpret_cast<QRgb*>(dest.scanLine(i));
            for (int j = 0; j < source.width(); ++j) {
                auto grey = data[i][j] * 255;
                if (grey > 255) {
                    grey = 255;
                }
                if (grey < 0) {
                    grey = 0;
                }
                line[j] = qRgb(grey, grey, grey);
            }
        }
        return dest;
    }*/
};

#endif // IIMAGEFILTER_H
