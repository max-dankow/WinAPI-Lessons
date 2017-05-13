#ifndef IIMAGEFILTER_H
#define IIMAGEFILTER_H
#include <QImage>

class IImageFilter {
public:
    virtual QImage apply(const QImage&) = 0;
};

#endif // IIMAGEFILTER_H
