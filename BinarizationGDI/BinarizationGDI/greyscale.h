#ifndef GREYSCALE_H
#define GREYSCALE_H
#include "iimagefilter.h"

class GreyScale : public IImageFilter {
public:
    CMatrix<CColor> Apply(const CMatrix<CColor>& source) {
        CMatrix<CColor> dest(source.GetHeight(), source.GetWidth());
        for (int i = 0; i < dest.GetHeight(); ++i) {
            for (int j = 0; j < dest.GetWidth(); ++j) {
                auto color = source.GetAt(j, i);
                auto grey = static_cast<byte>(0.21 * color.r + 0.72 * color.g + 0.07 * color.b);
                if (grey > 255) {
                    grey = 255;
                }
                dest.SetAt({ grey, grey, grey }, j, i);
            }
        }
        return dest;
    }

    CMatrix<byte> GetGrey(const CMatrix<CColor>& source) {
        CMatrix<byte> dest(source.GetHeight(), source.GetWidth());
        for (int i = 0; i < dest.GetHeight(); ++i) {
            for (int j = 0; j < dest.GetWidth(); ++j) {
                auto color = source.GetAt(j, i);
                auto grey = static_cast<byte>(0.21 * color.r + 0.72 * color.g + 0.07 * color.b);
                if (grey > 255) {
                    grey = 255;
                }
                dest.SetAt(grey, j, i);
            }
        }
        return dest;
    }
};

#endif // GREYSCALE_H
