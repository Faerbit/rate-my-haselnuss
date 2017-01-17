#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "detectionprocessing.h"
#include "rateprocessing.h"
#include "neuralnet.h"
#include <json/json.h>
#include <opencv2/opencv.hpp>
#include <QObject>
#include <memory>
#include <QTemporaryDir>
#include <QCloseEvent>

struct Nut {
    int min_x;
    int min_y;
    int max_x;
    int max_y;
    cv::Mat img;
    float score;

    int center_x() const {
        return min_x + ((max_x - min_x)/2);
    }

    int center_y() const {
        return min_y + ((max_y - min_y)/2);
    }
};

class Controller : public QObject {

    Q_OBJECT

public:
    DetectionProcessing& getDetectionProcessing() { return detProc; }
    RateProcessing& getRateProcessing() { return rateProc; }
    void extractNuts(std::vector<Nut>& result_nuts = inst().nuts);
    const cv::Mat* getCurImg();
    const cv::Mat* getCurNut();
    const cv::Mat* getNutToRate();
    void process();
    void resetNuts();
    int getCurRating();
    int getN();
    //Singleton
    static Controller& inst();
    Controller(Controller const&)       = delete;
    void operator=(Controller const&)   = delete;

signals:
    void newNut(const cv::Mat& nut);
    void newInputImage(const cv::Mat& img);
    void newNutToRate(const cv::Mat& nut);
    void message(std::string text);
    void statusMessage(QString);

public slots:
    void nextNut();
    void previousNut();
    void openImage();
    void captureImage();
    void saveDetectionConfig();
    void saveRatingConfig();
    void reset();
    void saveData();
    void currentRating(int score);
    void neuralNetChange(int val);

// private methods
private:
    std::string runCmd(std::string cmd);
    void loadImage();
    bool detect();
    void newNut();
    bool matchNuts(const std::vector<Nut>& cur_results);
    void debugPaint();
    void debugResultPrint();
    void paintScores(cv::Mat& img);

// members
private:
    // Singleton
    Controller(QObject* parent = nullptr);

    DetectionProcessing detProc;
    RateProcessing rateProc;
    NeuralNet net;
    Json::Value config;
    std::vector<Nut> nuts{};
    int current_nut = 0;
    const cv::Mat& markers;
    cv::Mat prev_img;
    cv::Mat cur_img;
    static Controller *singleton;
    QTemporaryDir transferFolder;
    QString filePath;
    cv::Mat nut_to_rate;
    int nut_rating = -1;
    bool valid_nut = false;
};

#endif // CONTROLLER_H
