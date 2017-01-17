#include "detectionprocessing.h"
#include "controller.h"
#include "colors.h"
#include <QDebug>
#include <QFileDialog>
#include <QApplication>
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace std;
using namespace cv;

DetectionProcessing::DetectionProcessing(QObject* parent)
    : Processing(parent) {}

void DetectionProcessing::init(Json::Value json_steps) {
    Processing::init(this->metaObject(), json_steps);
}

Mat DetectionProcessing::_dilate(const Mat& input, int kernel_size,
        int iterations) {
    Mat out;
    dilate(input, out, getStructuringElement(MORPH_ELLIPSE,
                Size(kernel_size, kernel_size)), Point(-1, -1), iterations);
    return out;
}

Mat DetectionProcessing::_threshold(const Mat& input, int ignored,
        int ignored2) {
    Mat out;
    threshold(input, out, 127, 255, THRESH_BINARY);
    return out;
}

Mat DetectionProcessing::_invert(const Mat& input, int ignored, int ignored2) {
    Mat out = Scalar::all(255) - input;
    return out;
}

Mat DetectionProcessing::_invert_markers(const Mat& input, int ignored,
        int ignored2) {
    markers = Scalar::all(1) - markers;
    return markers;
}

Mat DetectionProcessing::_thresh_color(const Mat& input, int lowerThreshold, int
        upperThreshold) {
    Mat hsv;
    cvtColor(input, hsv, COLOR_BGR2HSV);
    intermediate = Mat (input.size(), CV_8U);
    inRange(hsv, Scalar(lowerThreshold, 0, 0), Scalar(upperThreshold,
                255, 255), intermediate);
    return intermediate;
}

Mat DetectionProcessing::_erode(const Mat& input, int kernel_size, int
        iterations) {
    Mat out;
    erode(input, out, getStructuringElement(MORPH_ELLIPSE,
                Size(kernel_size, kernel_size)), Point(-1, -1), iterations);
    return out;
}

Mat DetectionProcessing::_erode_intermediate(const Mat& input, int kernel_size,
        int iterations) {
    Mat out;
    erode(intermediate, out, getStructuringElement(MORPH_ELLIPSE,
                Size(kernel_size, kernel_size)), Point(-1, -1), iterations);
    return out;
}

Mat DetectionProcessing::_markBackground(const Mat& input, int ignored,
        int ignored2) {
    markers = Mat::zeros(input.size(), CV_32S);
    markers = Scalar::all(0);
    Mat in = input.clone();
    vector<vector<Point>> contours;
    findContours(in, contours, RETR_LIST, CHAIN_APPROX_NONE);
    for(int i = 0; i<contours.size(); i++) {
        drawContours(markers, contours, i, Scalar::all(1),
                CV_FILLED, 8);
    }
    return markers;
}

Mat DetectionProcessing::_watershed(const Mat& input, int ignored,
        int ignored2) {
    Mat in = input.clone();
    vector<vector<Point>> contours;
    findContours(in, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);
    nut_count = 0;
    for(int i = 0; i<contours.size(); i++) {
        drawContours(markers, contours, i, Scalar::all(i+2),
                CV_FILLED, 8);
        nut_count++;
    }
    const Mat& src = *Controller::inst().getCurImg();
    watershed(src, markers);
    Mat wshed(markers.size(), CV_8UC3);
    // paint the watershed image
    for(int i = 0; i < markers.rows; i++ ) {
        for(int j = 0; j < markers.cols; j++ )
        {
            int index = markers.at<int>(i,j);
            if( index == -1 )
                wshed.at<Vec3b>(i,j) = Vec3b(255,255,255);
            else if( index <= 0 || index > nut_count )
                wshed.at<Vec3b>(i,j) = Vec3b(0,0,0);
            else
                wshed.at<Vec3b>(i,j) = colors[(index - 1) % colors.size()];
        }
    }
    wshed = wshed*0.5 + src*0.5;
    return wshed;
}

Mat DetectionProcessing::_resize(const Mat& input, int new_length,
        int ignored) {
    Mat out;
    float fac = 1.0;
    if (input.rows > input.cols) {
        fac = (float) new_length/input.rows;
    }
    else {
        fac = (float) new_length/input.cols;
    }
    Size new_size((int)input.cols*fac, (int) input.rows*fac) ;
    resize(input, out, new_size, INTER_NEAREST);
    return out;
}

Mat DetectionProcessing::_resizeToOriginal(const Mat& input, int ignored,
        int ignored2) {
    Mat out;
    float fac = 1.0;
    if (input.rows > input.cols) {
        fac = (float) Controller::inst().getCurImg()->rows/input.rows;
    }
    else {
        fac = (float) Controller::inst().getCurImg()->cols/input.cols;
    }
    Size new_size((int)input.cols*fac, (int) input.rows*fac) ;
    resize(input, out, new_size, INTER_NEAREST);
    return out;
}

const cv::Mat& DetectionProcessing::getResult() {
    return steps.back().getOutput();
}

const cv::Mat& DetectionProcessing::getMarkers() {
    return markers;
}

const cv::Mat* DetectionProcessing::getInput() {
    return Controller::inst().getCurImg();
}

int DetectionProcessing::getNutCount() {
    return nut_count;
}
