#ifndef GREYSCALE_H
#define GREYSCALE_H
#include "iimagefilter.h"

class GreyScale : public IImageFilter {
public:
    QImage apply(const QImage &source) {
        QImage dest(source);
        for (int i = 0; i < dest.height(); ++i) {
            QRgb* line = reinterpret_cast<QRgb*>(dest.scanLine(i));
            for (int j = 0; j < dest.width(); ++j) {
                auto grey =
                        0.21 * qRed(line[j]) +
                        0.72 * qGreen(line[j]) +
                        0.07 * qBlue(line[j]);

                line[j] = qRgb(grey, grey, grey);
            }
        }
        return dest;
    }
};

#endif // GREYSCALE_H
