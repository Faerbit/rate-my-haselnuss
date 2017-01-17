#ifndef DETECTIONPROCESSING_H
#define DETECTIONPROCESSING_H

#include <QObject>
#include "processing.h"
#include <memory>

class DetectionProcessing : public Processing {

    Q_OBJECT

public:
    DetectionProcessing(QObject *parent = 0);
    void init(Json::Value json_steps);
    const cv::Mat& getResult();
    const cv::Mat& getMarkers();
    int getNutCount();

// Processing methods
public slots:
    cv::Mat _dilate(const cv::Mat& input , int kernel_size, int iterations);
    cv::Mat _threshold(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _thresh_color(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _erode(const cv::Mat& input , int kernel_size, int iterations);
    cv::Mat _erode_intermediate(const cv::Mat& input , int kernel_size, int iterations);
    cv::Mat _watershed(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _invert(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _invert_markers(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _resize(const cv::Mat& input, int new_length, int ignored);
    cv::Mat _resizeToOriginal(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _markBackground(const cv::Mat& input, int ignored, int ignored2);

private:
    const cv::Mat* getInput();

// Members
private:
    cv::Mat markers;
    int nut_count;
    cv::Mat intermediate;
};

#endif // DETECTIONPROCESSING_H
