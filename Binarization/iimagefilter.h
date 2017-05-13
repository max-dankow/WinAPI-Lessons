#ifndef IIMAGEFILTER_H
#define IIMAGEFILTER_H
#include <QImage>

class IImageFilter {
public:
    virtual QImage apply(const QImage&) = 0;
protected:
    std::vector< std::vector<float> > extractData(const QImage& source) {
        std::vector< std::vector<float> > data(source.height(), std::vector<float>(source.width(), 0));
        for (int i = 0; i < source.height(); ++i) {
            const QRgb* line = reinterpret_cast<const QRgb*>(source.scanLine(i));
            for (int j = 0; j < source.width(); ++j) {
                data[i][j] = (qRed(line[j]) + qGreen(line[j]) + qBlue(line[j])) / 3;
            }
        }
        return data;
    }

    QImage toImage(const std::vector< std::vector<float> >& data, const QImage& source) {
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
    }
};

#endif // IIMAGEFILTER_H
