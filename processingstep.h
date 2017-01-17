#ifndef PROCESSINGSTEP_H
#define PROCESSINGSTEP_H

#include <opencv2/opencv.hpp>
#include <functional>
#include <QMetaMethod>
#include <QObject>

struct Parameter {
    int val;
    int lowerBound;
    int upperBound;
    std::string name;
    bool odd;
};

enum InputType {
    previous,
    color
};

class ProcessingStep {

public:
    ProcessingStep(QObject* processing,
            std::string stepName,
            QMetaMethod operation,
            std::vector<Parameter> params,
            InputType inputType
    ) {
        this->processing = processing;
        this->stepName = stepName;
        this->operation = operation;
        this->params = params;
        this->inputType = inputType;
    }

    std::vector<Parameter> getParams() { return params; }
    std::string getStepName() { return stepName; }
    void setParam1 (int val) {
        if (params[0].odd && val % 2 == 0) {
            val--;
        }
        params[0].val = val;
    }

    int getParam1() { return params[0].val; }

    int getParam2() { return params[1].val; }

    void setParam2 (int val) {
        if (params[1].odd && val % 2 == 0) {
            val--;
        }
        params[1].val = val;
    }
    void execute() {
        if (input->empty()) {
            std::cerr << "Input empty for processing step: " << stepName << std::endl;
            return;
        }
        operation.invoke(processing,
                Q_RETURN_ARG(cv::Mat, output),
                Q_ARG(const cv::Mat&, *input),
                Q_ARG(int, params[0].val),
                Q_ARG(int, params[1].val)
        );
    }
    void save() {
        imwrite(stepName + ".jpg", output);
    }

    const cv::Mat& getOutput() { return output; }
    const cv::Mat& getInput() { return *input; }

    void setInput(const cv::Mat* input) { this->input = input; }

    InputType getInputType() { return inputType; }

    bool hasInputData() { return input != nullptr && !input->empty(); }

    bool hasData() { return input != nullptr && !input->empty()
        && !output.empty(); }

    void reset() {
        input = nullptr;
        output.release();
    }

private:
    const cv::Mat* input = nullptr;
    cv::Mat output;
    std::string stepName;
    QMetaMethod operation;
    std::vector<Parameter> params;
    InputType inputType;
    QObject* processing;
};

#endif
