#include <QCoreApplication>
#include <QImage>
#include <QDebug>

#include <iostream>
#include <stdexcept>
#include <string>

#include "iimagefilter.h"
#include "greyscale.h"
#include "thresholdbinarization.h"
#include "bradleybinarization.h"

class Original : public IImageFilter {
public:
    virtual QImage apply(const QImage &source) {
        return source;
    }
};

template<class F>
void processImage(const QString& filePath, const QString& outputFile) {
    F filter;
    processImage(filePath, outputFile, filter);
}

template<class F>
void processImage(const QString& filePath, const QString& outputFile, F &filter) {
    QImage inputImage;
    if (inputImage.load(filePath)) {
        QImage result = filter.apply(inputImage);
        result.save(outputFile);
    } else {
        throw std::runtime_error("Fail to binarize file " + filePath.toStdString());
    }
}

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        QString filePath(argv[i]);
        try {
            //processImage<Original>(filePath, QString("output/" + QString::number(i) + ".png"));
            //processImage<GreyScale>(filePath, QString("output/" + QString::number(i) + "grey.png"));
            //ThresholdBinarization thresholdBinarization(128);
            //processImage<ThresholdBinarization>(filePath, QString("output/" + QString::number(i) + "t.png"), thresholdBinarization);
            processImage<BradleyBinarization>(filePath, QString("output/" + QString::number(i) + "n.png"));
        } catch(std::runtime_error e) {
            std::cerr << e.what();
        }
        qDebug() << QString::number(i) + "/" + QString::number(argc - 1);
    }

}
