#ifndef NEURALNET_H
#define NEURALNET_H

#include <vector>
#include <memory>
#include <json/json.h>
#include <fann.h>
#include <fann_cpp.h>

class NeuralNet {
public:
    void saveNut();
    void init(Json::Value jsonResults);
    int getN();
    float run(const std::map<std::string, double>& input);
    void neuralNetEnabled(bool val);

private:
    void createHeaders();
    bool filesExist();
    void trainAndSave();
    void getScalingFactors();
    void scaleData();

private:
    int current_index = 0;
    std::vector<float> inputTrainData = {};
    std::vector<float> scaledInputTrainData = {};
    std::vector<float> outputTrainData = {};
    std::vector<float> scaledOutputTrainData = {};
    std::vector<std::string> result_names = {};
    FANN::neural_net net;
    // tuple of min and scaling factor for each input
    std::vector<std::tuple<float, float>> scaling_input_data = {};
    bool use_neural_net = true;
};

#endif // NEURALNET_H
