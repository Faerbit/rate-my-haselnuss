#include "detectionprocessing.h"
#include "controller.h"
#include <QDebug>
#include <QFileDialog>
#include <QApplication>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;
using namespace cv;

RateProcessing::RateProcessing(QObject* parent)
    : Processing(parent) {}

void RateProcessing::init(Json::Value json_steps) {
    Processing::init(this->metaObject(), json_steps);
}


Mat RateProcessing::_dilate(const Mat& input, int kernel_size,
        int iterations) {
    Mat out;
    dilate(input, out, getStructuringElement(MORPH_ELLIPSE,
                Size(kernel_size, kernel_size)), Point(-1, -1), iterations);
    return out;
}

Mat RateProcessing::_threshold(const Mat& input, int ignored,
        int ignored2) {
    Mat gray;
    cvtColor(input, gray, COLOR_BGR2GRAY);
    Mat out;
    threshold(gray, out, 1, 255, THRESH_BINARY);
    return out;
}

Mat RateProcessing::_thresh_otsu(const Mat& input, int ignored,
        int ignored2) {
    Mat gray;
    cvtColor(input, gray, COLOR_BGR2GRAY);
    Mat out;
    threshold(gray, out, 0, 255, THRESH_BINARY | THRESH_OTSU);
    return out;
}

Mat RateProcessing::_invert(const Mat& input, int ignored, int ignored2) {
    Mat out = Scalar::all(255) - input;
    return out;
}

Mat RateProcessing::_thresh_color(const Mat& input, int lowerThreshold, int
        upperThreshold) {
    Mat hsv;
    cvtColor(input, hsv, COLOR_BGR2HSV);
    Mat out(input.size(), CV_8U);
    inRange(hsv, Scalar(lowerThreshold, 0, 0), Scalar(upperThreshold,
                255, 255), out);
    return out;
}

Mat RateProcessing::_mean_color(const Mat& input, int ignored, int ignored2) {
    Mat hsv_nut;
    cvtColor(*getInput(), hsv_nut, CV_BGR2HSV);
    Scalar res = mean(hsv_nut, input);
    results["avg_h_val"] = res[0];
    if (isInteractive()) {
        emit requestUpdate();
    }
    return hsv_nut;
}

Mat RateProcessing::_erode(const Mat& input, int kernel_size, int
        iterations) {
    Mat out;
    erode(input, out, getStructuringElement(MORPH_ELLIPSE,
                Size(kernel_size, kernel_size)), Point(-1, -1), iterations);
    return out;
}

Mat RateProcessing::_hough(const Mat& input, int canny_1, int canny_2) {
    vector<Vec3f> circles;
    Mat gray;
    cvtColor(input, gray, COLOR_BGR2GRAY);
    HoughCircles(gray, circles, HOUGH_GRADIENT, 1, input.rows/4, canny_1,
            canny_2);
    Mat out;
    input.copyTo(out);
    for (int i = 0; i<circles.size(); i++) {
        Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
        int radius = cvRound(circles[i][2]);
        circle(out, center, 3, Scalar(0, 255, 0), -1, 8, 0);
        circle(out, center, radius, Scalar(0, 0, 255), 3, 8, 0);
    }
    return out;
}

Mat RateProcessing::_find_contours_resized(const Mat& input, int max_length,
        int ignored2) {
    vector<vector<Point>> contours;
    Mat in;
    float fac = 1.0;
    if (input.rows > input.cols) {
        fac = (float) max_length/input.rows;
    }
    else {
        fac = (float) max_length/input.cols;
    }
    Size new_size((int)input.cols*fac, (int) input.rows*fac) ;
    resize(input, in, new_size);
    findContours(in, contours, RETR_LIST, CHAIN_APPROX_TC89_KCOS);
    Mat out;
    if (isInteractive()) {
        out = Mat::zeros(in.size(), CV_8UC3);
        for(int i = 0; i < contours.size(); i++) {
            drawContours(out, contours, i, Scalar::all(255), 1, 8);
        }
    }
    return out;
}

Mat RateProcessing::_find_contours(const Mat& input, int ignored,
        int ignored2) {
    Mat in = input.clone();
    vector<vector<Point>> contours;
    Mat out;
    findContours(in, contours, RETR_LIST, CHAIN_APPROX_NONE);
    if (isInteractive()) {
        out = Mat::zeros(input.size(), CV_8UC3);
        for(int i = 0; i < contours.size(); i++) {
            drawContours(out, contours, i, Scalar::all(255), 1, 8);
        }
    }
    return out;
}


Mat RateProcessing::_fit_ellipse(const Mat& input, int ignored, int ignored2) {
    Mat in = input.clone();
    //vector<vector<Point>> contours;
    contours = {};
    Mat out;
    findContours(in, contours, RETR_LIST, CHAIN_APPROX_NONE);
    if (isInteractive()) {
        out = Mat::zeros(input.size(), CV_8UC3);
    }
    vector<RotatedRect> boxes = {};
    for(int i = 0; i < contours.size(); i++) {
        int count = contours[i].size();
        if( count < 6 )
            continue;
        Mat pointsf;
        Mat(contours[i]).convertTo(pointsf, CV_32F);
        RotatedRect box = fitEllipse(pointsf);
        if( MAX(box.size.width, box.size.height) >
                MIN(box.size.width, box.size.height)*30 )
            continue;
        boxes.push_back(box);
        if (isInteractive()) {
            drawContours(out, contours, i, Scalar::all(255), 1, 8);
            ellipse(out, box, Scalar(0,0,255));
            Point2f vtx[4];
            box.points(vtx);
            for( int j = 0; j < 4; j++ ) {
                line(out, vtx[j], vtx[(j+1)%4], Scalar(0,255,0), 1, LINE_AA);
            }
        }
    }
    RotatedRect& max_box = boxes[0];
    int max_area = 0;
    for (auto&& box : boxes) {
        if ((box.size.width * box.size.height) > max_area) {
            max_box = box;
            max_area = box.size.width * box.size.height;
        }
    }
    results["size_ratio"] = max_box.size.width/max_box.size.height;
    results["size_diagonal"] =
            sqrt(max_box.size.height * max_box.size.height +
                max_box.size.height * max_box.size.height)
            / static_cast<float>(steps[current_step].getParam1());
    if (isInteractive()) {
        emit requestUpdate();
    }
    return out;
}

Mat RateProcessing::_rate_spots(const Mat& input, int ignored,
        int ignored2) {
    Mat in = input.clone();
    vector<vector<Point>> contours;
    Mat out;
    findContours(in, contours, RETR_LIST, CHAIN_APPROX_NONE);
    if (isInteractive()) {
        out = Mat::zeros(input.size(), CV_8UC3);
    }
    vector<RotatedRect> boxes = {};
    // image and mask of spots
    vector<tuple<Mat, Mat>> spots = {};
    for(int i = 0; i < contours.size(); i++) {
        int count = contours[i].size();
        if( count < 6 )
            continue;
        Mat pointsf;
        Mat(contours[i]).convertTo(pointsf, CV_32F);
        RotatedRect box = fitEllipse(pointsf);
        if( MAX(box.size.width, box.size.height) >
                MIN(box.size.width, box.size.height)*30 )
            continue;
        boxes.push_back(box);
        if (isInteractive()) {
            drawContours(out, contours, i, Scalar::all(255), 1, 8);
            ellipse(out, box, Scalar(0,0,255));
            Point2f vtx[4];
            box.points(vtx);
            for( int j = 0; j < 4; j++ ) {
                line(out, vtx[j], vtx[(j+1)%4], Scalar(0,255,0), 1, LINE_AA);
            }
        }
        // extract spot from image
        Rect roi = boundingRect(contours[i]);
        Mat mask = Mat::zeros(input.size(), CV_8UC1);
        drawContours(mask, contours, i, Scalar(255), CV_FILLED);
        Mat contourRegion;
        Mat imageROI;
        getInput()->copyTo(imageROI, mask);
        contourRegion = imageROI(roi);
        Mat maskRegion = mask(roi);
        spots.push_back(make_tuple(contourRegion, maskRegion));
    }

    // find largest ellipse to ignore it, because its no spot but the nut itself
    int max_box_index = -1;
    int max_area = 0;
    for (unsigned int i = 0; i<boxes.size(); i++) {
        if ((boxes[i].size.width * boxes[i].size.height) > max_area) {
            max_box_index = i;
            max_area = boxes[i].size.width * boxes[i].size.height;
        }
    }
    double box_avg_size = 0.0f;
    for (unsigned int i = 0; i<boxes.size(); i++) {
        if (i == max_box_index) {
            continue;
        }
        box_avg_size +=
            sqrt(boxes[i].size.width * boxes[i].size.width +
                    boxes[i].size.height * boxes[i].size.height);
    }
    if (boxes.size() > 1) {
        box_avg_size /= static_cast<double>(boxes.size() - 1);
    }
    else {
        box_avg_size = 0.0f;
    }
    results["spot_avg_size"] = box_avg_size/steps[current_step].getParam1();
    double spot_avg_h_val = 0.0f;
    for (unsigned int i = 0; i<spots.size(); i++) {
        if (i == max_box_index) {
            continue;
        }
        Mat hsv_spot;
        cvtColor(get<0>(spots[i]), hsv_spot, CV_BGR2HSV);
        Scalar res = mean(hsv_spot, get<1>(spots[i]));
        spot_avg_h_val += res[0];
    }
    if (spots.size() > 1) {
        spot_avg_h_val /= static_cast<double>(spots.size() - 1);
    }
    else {
        spot_avg_h_val = 0.0f;
    }
    results["spot_avg_h_val"] = spot_avg_h_val;
    if (isInteractive()) {
        emit requestUpdate();
    }
    return out;
}

void RateProcessing::reset() {
    current_step = 0;
    InputType type = steps[0].getInputType();
    if (type == color) {
        steps[0].setInput(Controller::inst().getCurNut());
    }
    results.clear();
    rateCurNut = true;
}

map<string, double> RateProcessing::getResults() {
    return results;
}

const cv::Mat* RateProcessing::getInput() {
    if (rateCurNut) {
        return Controller::inst().getCurNut();
    }
    else {
        return Controller::inst().getNutToRate();
    }
}

const vector<vector<Point>>& RateProcessing::getContours() {
    return contours;
}

void RateProcessing::setRateCurNut(bool val) {
    rateCurNut = val;
}
