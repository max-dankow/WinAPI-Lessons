#ifndef THRESHOLDBINARIZATION_H
#define THRESHOLDBINARIZATION_H

#include "iimagefilter.h"

class ThresholdBinarization : public IImageFilter
{
public:
    ThresholdBinarization(unsigned threshhold = 128) : IImageFilter() {
        this->threshold = threshhold;
    }

    QImage apply(const QImage &source) {
        QImage dest(source);
        for (int i = 0; i < dest.height(); ++i) {
            QRgb* line = reinterpret_cast<QRgb*>(dest.scanLine(i));
            for (int j = 0; j < dest.width(); ++j) {
                if (((qRed(line[j]) + qGreen(line[j]) + qBlue(line[j])) / 3) > threshold) {
                    line[j] = qRgb(0xFF, 0xFF, 0xFF);
                } else {
                    line[j] = qRgb(0, 0, 0);
                }
            }
        }
        return dest;
    }
private:
    unsigned threshold;
};

#endif // THRESHOLDBINARIZATION_H
