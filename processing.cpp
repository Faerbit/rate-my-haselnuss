#include "processing.h"
#include "controller.h"
#include <QDebug>
#include <QFileDialog>
#include <QApplication>
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace std;
using namespace cv;


Processing::Processing(QObject *parent) : QObject(parent) {}

void Processing::init(const QMetaObject* metaObj, Json::Value json_steps) {
    for(int i = 0; i<json_steps.size(); i++) {
        string json_name = json_steps[i].get("name", "").asString();
        string json_op_name = json_steps[i].get("operation", "").asString();
        QMetaMethod operation;
        bool found = false;
        for (int i = 0; i<metaObj->methodCount(); i++) {
            QMetaMethod method = metaObj->method(i);
            if (method.name().toStdString() == json_op_name) {
                operation = method;
                found = true;
            }
        }
        if (!found) {
            cerr << "Processing method " << json_op_name <<
                " not found.\n";
            exit(1);
        }
        Json::Value json_param1 = json_steps[i]["param1"];
        Parameter param1;
        param1.val = json_param1.get("default", 0).asInt();
        param1.lowerBound = json_param1.get("lowerBound", 0).asInt();
        param1.upperBound = json_param1.get("upperBound", 0).asInt();
        param1.name = json_param1.get("name", "").asString();
        param1.odd = json_param1.get("odd", false).asBool();
        Json::Value json_param2 = json_steps[i]["param2"];
        Parameter param2;
        param2.val = json_param2.get("default", 0).asInt();
        param2.lowerBound = json_param2.get("lowerBound", 0).asInt();
        param2.upperBound = json_param2.get("upperBound", 0).asInt();
        param2.name = json_param2.get("name", "").asString();
        param2.odd = json_param2.get("odd", false).asBool();
        vector<Parameter> params = {param1, param2};
        InputType type;
        if (json_steps[i]["input"] == "previous") {
            type = previous;
        }
        else if (json_steps[i]["input"] == "color") {
            type = color;
        }
        ProcessingStep step(this, json_name, operation, params, type);
        steps.push_back(step);
    }
}

void Processing::start(bool force) {
    if (!steps[current_step].hasInputData()) {
        return;
    }
    if (interactive) {
        emit newInputImage(steps[current_step].getInput());
    }
    if (steps[current_step].getOutput().empty() || force) {
        processImage();
    }
    else {
        emit newOutputImage(steps[current_step].getOutput());
    }
}

void Processing::reset() {
    current_step = 0;
    for (auto&& step : steps) {
        step.reset();
    }
    InputType type = steps[0].getInputType();
    if (type == color) {
        steps[0].setInput(Controller::inst().getCurImg());
    }
}

void Processing::processImage() {
    steps[current_step].execute();
    if (interactive) {
        emit newOutputImage(steps[current_step].getOutput());
    }
}

void Processing::setParam1(int val) {
    steps[current_step].setParam1(val);
    processImage();
}

void Processing::setParam2(int val) {
    steps[current_step].setParam2(val);
    processImage();
}

void Processing::nextStep() {
    current_step = min((int)steps.size() - 1, current_step+1);
    InputType type = steps[current_step].getInputType();
    if (type == previous) {
        steps[current_step].setInput(&steps[current_step -
                1].getOutput());
        if (interactive) {
            emit newInputImage(steps[current_step].getInput());
        }
    }
    else if (type == color) {
        steps[current_step].setInput(getInput());
        if (interactive && steps[current_step].hasInputData()) {
            emit newInputImage(steps[current_step].getInput());
        }
    }
    if (interactive) {
        emit requestUpdate();
    }
    if (steps[current_step].hasInputData()) {
        processImage();
    }
    else {
        cerr << "No input data for step: "
            + steps[current_step].getStepName() << endl;
    }
}

void Processing::previousStep() {
    current_step = max(0, current_step-1);
    if (interactive) {
        emit requestUpdate();
        if (steps[current_step].hasData()) {
            emit newInputImage(steps[current_step].getInput());
            emit newOutputImage(steps[current_step].getOutput());
        }
    }
}

int Processing::getProgress() {
    double prog = (double)(current_step+1)/steps.size();
    return ceil(prog*100);
}

string Processing::getStepName() {
    return steps[current_step].getStepName();
}

vector<Parameter> Processing::getCurParams() {
    return steps[current_step].getParams();
}

void Processing::saveCur() {
    steps[current_step].save();
}

void Processing::setInteractive(bool val) {
    interactive = val;
}

bool Processing::isInteractive() {
    return interactive;
}

int Processing::getCurrentStep() {
    return current_step;
}

bool Processing::process() {
    if (! steps[0].hasInputData()) {
        return false;
    }

    // also do step 0
    current_step = -1;
    for (unsigned int i = 0; i<steps.size(); i++) {
        nextStep();
    }
    return true;
}
