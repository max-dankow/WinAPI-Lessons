#ifndef BRADLEYBINARIZATION_H
#define BRADLEYBINARIZATION_H

#include "iimagefilter.h"
#include "greyscale.h"
#include <QRect>
#include <vector>
#include <assert.h>
#include <math.h>

class BradleyBinarization : public IImageFilter {
public:
    QImage apply(const QImage &source) {
        auto height = source.height();
        auto width = source.width();
        QRect imageRect(QPoint(0, 0), source.size());
        int wh = height / 16, ww = width / 16;
        inputData = extractData(GreyScale().apply(source));
        std::vector< std::vector<float> > outputData(height, std::vector<float>(width, 0));

        computeIntegralImage(height, width, inputData);
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                QRect rect(j - ww, i - wh, 2*ww + 1, 2*wh + 1);
                float localMean = calculateLocalMean(rect, imageRect);
                float threshold = localMean * (1. - 0.15);
                outputData[i][j] = inputData[i][j] >= threshold ? 1. : 0.;
            }
        }
        return toImage(outputData, source);
    }
private:

    void computeIntegralImage(int height, int width, const std::vector< std::vector<float> > &input) {
        integral.assign(height, std::vector<float>(width, 0));
        integralSquares.assign(height, std::vector<float>(width, 0));
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                integral[i][j] = input[i][j];
                integralSquares[i][j] = input[i][j] * input[i][j];
                if (i == 0 && j == 0) {
                    continue;
                }
                if (i == 0) {
                    integral[i][j] += integral[i][j - 1];
                    integralSquares[i][j] += integralSquares[i][j - 1];
                    continue;
                }
                if (j == 0) {
                    integral[i][j] += integral[i - 1][j];
                    integralSquares[i][j] += integralSquares[i - 1][j];
                    continue;
                }
                integral[i][j] += integral[i][j - 1] + integral[i - 1][j] - integral[i - 1][j - 1];
                integralSquares[i][j] += integralSquares[i][j - 1] + integralSquares[i - 1][j] - integralSquares[i - 1][j - 1];
            }
        }
    }

    float computeSum(const QRect& area, const std::vector< std::vector<float> >& matrix) const {
        auto top = area.top(),
                right = area.right(),
                bottom = area.bottom(),
                left = area.left();

        assert(top >= 0 && right < matrix.front().size()
               && bottom < matrix.size() && left >= 0);
        auto sum = matrix[bottom][right];
        if (top != 0 && left != 0) {
            return sum - matrix[top - 1][right] - matrix[bottom][left - 1] + matrix[top - 1][left - 1];
        }
        if (top != 0) {
            return sum - matrix[top - 1][right];
        }
        if (left != 0) {
            return sum - matrix[bottom][left - 1];
        }
        return sum;
    }

    float calculateLocalMean(const QRect& rect, const QRect& imageRect) const {
        QRect validArea = imageRect.intersect(rect);
        auto area = validArea.height() * validArea.width();
        assert(area >= 0);
        return computeSum(validArea, integral) / area;
    }

    float calculateLocalDeviation(const QRect& rect, const QRect& imageRect, float mean) {
        QRect validArea = imageRect.intersect(rect);
        auto area = validArea.height() * validArea.width();
        assert(area >= 0);
        float sum = computeSum(validArea, integralSquares);
        return sqrt(sum / area - mean * mean);
    }

    std::vector< std::vector<float> > integral, integralSquares;
    std::vector< std::vector<float> > inputData;
};

#endif // BRADLEYBINARIZATION_H
