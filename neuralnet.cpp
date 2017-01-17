#include "neuralnet.h"
#include "controller.h"
#include "filenames.h"
#include <fstream>
#include <limits>
#include <clocale>

using namespace std;
using namespace FANN;

void NeuralNet::saveNut() {
    if (Controller::inst().getCurRating() == -1) {
        cerr << "Error: current rating -1 during saving." << endl;
        exit(1);
    }
    ostringstream input_line;
    input_line << current_index << ";";
    input_line.precision(numeric_limits<double>::max_digits10);
    const auto& results = Controller::inst().getRateProcessing().getResults();
    for (const auto& result : result_names) {
        auto pos = results.find(result);
        if (pos == results.end()) {
            cerr << "Could not find " << result << " in results." << endl;
            exit(1);
        }
        inputTrainData.push_back(pos->second);
        input_line << scientific << pos->second << ";";
    }
    ofstream input_file;
    input_file.open(FileNames::inputName, ios_base::app);
    input_file << input_line.str() << "\n";

    outputTrainData.push_back(Controller::inst().getCurRating());
    ofstream output_file;
    output_file.open(FileNames::outputName, ios_base::app);
    output_file << current_index << ";";
    output_file << Controller::inst().getCurRating() << ";" << "\n";
    
    ostringstream img_file;
    img_file << FileNames::imgPath
        << "nut_" << to_string(current_index)
        << ".jpg";
    imwrite(img_file.str(), *Controller::inst().getNutToRate());
    current_index++;
    if (current_index % 5 == 0) {
        trainAndSave();
    }
}

void NeuralNet::trainAndSave() {
    setlocale(LC_ALL, "en_GB.UTF-8");
    getScalingFactors();
    scaleData();
    training_data data;
    data.set_train_data(scaledOutputTrainData.size(), result_names.size(),
            &scaledInputTrainData[0], 1, &scaledOutputTrainData[0]);
    net.train_on_data(data, 100000, 1000, 0.01);
    if (!net.save(FileNames::neuralNetName)) {
        cerr << "Error saving neural net:"
            << net.get_errstr() << endl;
        exit(1);
    }
}

void NeuralNet::scaleData() {
    scaledInputTrainData = vector<float>(inputTrainData.size());
    for(int i = 0; i<result_names.size(); i++) {
        float data_min = inputTrainData[i];
        float data_max = inputTrainData[i];
        for(int j = 0; j<outputTrainData.size(); j++) {

            scaledInputTrainData[(j*result_names.size())+i] = 
                (inputTrainData[(j*result_names.size())+i] -
                    get<0>(scaling_input_data[i])) *
                get <1>(scaling_input_data[i]);
        }
    }
    scaledOutputTrainData = vector<float>(outputTrainData.size());
    for(int i = 0; i<outputTrainData.size(); i++) {
        scaledOutputTrainData[i] = (outputTrainData[i] - 1) * 0.25;
    }
}

void NeuralNet::getScalingFactors() {
    scaling_input_data = vector<tuple<float, float>>(result_names.size());
    for(int i = 0; i<result_names.size(); i++) {
        float data_min = inputTrainData[i];
        float data_max = inputTrainData[i];
        for(int j = 0; j<outputTrainData.size(); j++) {

            data_min = min(data_min, inputTrainData[(j*result_names.size())+i]);
            data_max = max(data_max, inputTrainData[(j*result_names.size())+i]);
        }
        cout << "Input " << i << ": min, scaling_factor: " << data_min
            << ", " << 1.0f/(data_max - data_min)  
            << " min: " << data_min
            << " max: " << data_max << endl;
        scaling_input_data[i] = make_tuple(
                    data_min, 1.0f/(data_max-data_min));
    }
}

float NeuralNet::run(const map<string, double>& input) {
    float input_arr[input.size()];
    for (unsigned int i = 0; i<result_names.size(); i++) {
        auto pos = input.find(result_names[i]);
        if (pos == input.end()) {
            cerr << "Could not find " << result_names[i] << " in results."
                << endl;
            exit(1);
        }
        if (scaling_input_data.size() > 0) {
        input_arr[i] = 
            (pos->second - get<0>(scaling_input_data[i])) * 
            get<1>(scaling_input_data[i]);
        }
        else {
            input_arr[i] = pos->second;
        }
    }
    float ret = 0.0f;
    if (use_neural_net) {
        float* output;
        output = net.run(input_arr);
        ret = output[0];
    }
    else {
        ret =
        0.25 * input_arr[0] +
        0.125* input_arr[1] +
        0.125* (1.0f/input_arr[2]) +
        0.25 * input_arr[3] +
        0.25 * input_arr[4];
    }
    return min(1.0f, max(ret, 0.0f));
}

void NeuralNet::init(Json::Value jsonResults) {
    for(int i = 0; i<jsonResults.size(); i++) {
        result_names.push_back(jsonResults[i].asString());
    }
    if (filesExist()) {
        setlocale(LC_ALL, "en_GB.UTF-8");
        string inputLine;
        string outputLine;
        ifstream input_file(FileNames::inputName);
        ifstream output_file(FileNames::outputName);
        // check header
        getline(input_file, inputLine);
        ostringstream inputHeader;
        inputHeader << "nut_id" << ";";
        for (const auto& result : result_names) {
            inputHeader << result << ";";
        }
        if (inputLine != inputHeader.str()) {
            cerr << "Header for output CSV file invalid: " <<
                inputHeader.str() << endl;
            exit(1);
        }
        getline(output_file, outputLine);
        if (outputLine != "nut_id;score;") {
            cerr << "Header for output CSV file invalid: " <<
                outputLine << endl;
            exit(1);
        }
        while ( getline(input_file, inputLine) 
                && getline(output_file, outputLine)) {
            stringstream inputLineStream(inputLine);
            string cell;
            // read in index
            getline(inputLineStream, cell, ';');
            try {
                current_index = stoi(cell);
            }
            catch (invalid_argument e) {
                cerr << "Could not convert " << cell << " to int." << endl;
                exit(1);
            }
            while(getline(inputLineStream, cell, ';')) {
                double val;
                try {
                    val= stof(cell);
                }
                catch (invalid_argument e) {
                    cerr << "Could not convert " << cell << " to double." 
                        << endl;
                    exit(1);
                }
                if (val < 0) {
                    cerr << "Error reading input data. Index " << current_index 
                        << "has negative data." << endl;
                    exit(1);
                }
                inputTrainData.push_back(val);
            }
            stringstream outputLineStream(outputLine);
            getline(outputLineStream, cell, ';');
            int output_index;
            try {
                output_index = stoi(cell);
            }
            catch (invalid_argument e) {
                cerr << "Could not convert " << cell << " to int." << endl;
                exit(1);
            }
            if (current_index != output_index) {
                cerr << "Error reading data: Expected index " << current_index
                    << " but got index " << output_index << endl;
                exit(1);
            }
            getline(outputLineStream, cell, ';');
            double val;
            try {
                val= stof(cell);
            }
            catch (invalid_argument e) {
                cerr << "Could not convert " << cell << " to double." 
                    << endl;
                exit(1);
            }
            if (val < 0) {
                cerr << "Error reading output data. Index " << current_index 
                    << "has negative data." << endl;
                exit(1);
            }
            outputTrainData.push_back(val);
        }
        current_index++;
        net.create_from_file(FileNames::neuralNetName);
        net.set_training_algorithm(TRAIN_BATCH);
        net.set_activation_function_hidden(SIGMOID);
        net.set_activation_function_output(LINEAR);
        getScalingFactors();
        trainAndSave();
    }
    else {
        createHeaders();
        int input_size = result_names.size();
        net.create_standard(3, input_size, input_size + 1, 1);
        net.set_training_algorithm(TRAIN_BATCH);
        net.set_activation_function_hidden(SIGMOID);
        net.set_activation_function_output(LINEAR);
    }
    cout << "current_index: " << current_index << endl;
}

void NeuralNet::createHeaders() {
    ofstream input_file(FileNames::inputName);
    input_file << "nut_id" << ";";
    for (const auto& result : result_names) {
        input_file << result << ";";
    }
    input_file << "\n";
    ofstream output_file(FileNames::outputName);
    output_file << "nut_id" << ";";
    output_file << "score" << ";";
    output_file << "\n";
}

bool NeuralNet::filesExist() {
    ifstream input_file(FileNames::inputName);
    ifstream output_file(FileNames::outputName);
    ifstream net_file(FileNames::neuralNetName);
    return input_file.good() && output_file.good() && net_file.good();
}

int NeuralNet::getN() {
    return current_index;
}

void NeuralNet::neuralNetEnabled(bool val) {
    use_neural_net = val;
}
