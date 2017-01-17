#ifndef PROCESSING_H
#define PROCESSING_H
#include <QObject>
#include <opencv2/opencv.hpp>
#include <json/json.h>

#include "processingstep.h"

class Processing : public QObject {

    Q_OBJECT

public:
    Processing(QObject *parent = 0);
    std::vector<Parameter> getCurParams();
    std::string getStepName();
    int getProgress();
    void setInteractive(bool val);
    bool isInteractive();
    void start(bool force = false);
    void reset();
    int getCurrentStep();
    bool process();

signals:
    void newInputImage(const cv::Mat& newImage);
    void newOutputImage(const cv::Mat& newImage);
    void requestUpdate();

public slots:
    void setParam1(int val);
    void setParam2(int val);
    void nextStep();
    void previousStep();
    void saveCur();
    void processImage();

protected:
    virtual const cv::Mat* getInput() = 0;

// Members
protected:
    std::vector<ProcessingStep> steps {};
    int current_step = 0;
    Json::Value config;
    void init(const QMetaObject* metaObj, Json::Value json_steps);
    bool interactive = false;
};

#endif // PROCESSING_H
