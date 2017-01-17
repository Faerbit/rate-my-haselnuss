#ifndef RATEPROCESSING_H
#define RATEPROCESSING_H

#include <QObject>
#include "processing.h"
#include <memory>
#include <map>

class RateProcessing : public Processing {

    Q_OBJECT

public:
    RateProcessing(QObject *parent = 0);
    void init(Json::Value json_steps);
    void reset();
    std::map<std::string, double> getResults();
    const std::vector<std::vector<cv::Point>>& getContours();
    void setRateCurNut(bool val);

// Processing methods
public slots:
    cv::Mat _dilate(const cv::Mat& input , int kernel_size, int iterations);
    cv::Mat _threshold(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _thresh_otsu(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _thresh_color(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _erode(const cv::Mat& input , int kernel_size, int iterations);
    cv::Mat _invert(const cv::Mat& input, int iter, int ignored);
    cv::Mat _hough(const cv::Mat& input, int canny_1, int canny_2);
    cv::Mat _fit_ellipse(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _rate_spots(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _find_contours(const cv::Mat& input, int ignored, int ignored2);
    cv::Mat _find_contours_resized(const cv::Mat& input, int max_length, int ignored2);
    cv::Mat _mean_color(const cv::Mat& input, int ignored, int ignored2);

private:
    const cv::Mat* getInput();

// Members
private:
    bool rateCurNut = true;
    std::map<std::string, double> results;
    std::vector<std::vector<cv::Point>> contours;
};

#endif // RATEPROCESSING_H
