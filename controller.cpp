#include "controller.h"
#include <fstream>
#include <QDebug>
#include <QString>
#include <set>
#include <QFileDialog>
#include <stdio.h>
#include <chrono>
#include "filenames.h"

using namespace std;
using namespace cv;

Controller::Controller(QObject* parent) : QObject(parent),
    detProc(), markers(detProc.getMarkers()) {
    string errors;
    ifstream file;
    file.open(FileNames::configName);
    bool ok = Json::parseFromStream(Json::CharReaderBuilder(),
            file, &config, &errors);
    file.close();
    if (!ok) {
        cerr << "Error while reading " << FileNames::configName
            << ":\n" << errors << "\n";
        exit(1);
    }

    detProc.init(config["detection"]);
    rateProc.init(config["rating"]["steps"]);
    net.init(config["rating"]["results"]);
}

Controller& Controller::inst() {
    static Controller ctrlr;
    return ctrlr;
}

const Mat* Controller::getCurImg() {
    return &cur_img;
}

const Mat* Controller::getCurNut() {
    return &nuts[current_nut].img;
}

void Controller::openImage() {
    filePath = QFileDialog::getOpenFileName(nullptr,
            tr("Öffnen"), "",
            tr("Bilder (*.png *.jpg)"));
    if (filePath.toStdString() == "") {
        return;
    }
    loadImage();
}

void Controller::loadImage() {
    if ( !cur_img.empty() ){
        cur_img.copyTo(prev_img);
    }
    cur_img = imread(filePath.toStdString(), IMREAD_COLOR);
    emit statusMessage("Bild geladen ...");
    process();
}

void Controller::process() {
    if (valid_nut) {
        if (nut_rating > 0) {
            emit statusMessage("Bewerte gegessene Nuss...");
            rateProc.reset();
            rateProc.setRateCurNut(false);
            rateProc.process();
            rateProc.setRateCurNut(true);
            debugResultPrint();
            net.saveNut();
        }
        else {
            cout << "No score selected. Not saving Nut." << endl;
        }
    }
    detProc.reset();
    emit newInputImage(cur_img);
    emit statusMessage("Erkenne Nüsse ...");
    if (!detect()) {
        return;
    }
    emit newInputImage(detProc.getResult());
    if (!prev_img.empty()) {
        vector<Nut> results = {};
        emit statusMessage("Extrahiere Nüsse ...");
        extractNuts(results);
        if (matchNuts(results)) {
            nuts = results;
            valid_nut = true;
        }
        else {
            valid_nut = false;
            nuts = results;
        }
    }
    else {
        emit statusMessage("Extrahiere Nüsse ...");
        extractNuts();
    }
    nut_rating = -1;
    //debugPaint();
    emit statusMessage("Bewerte Nüsse ...");
    Mat withScores = Mat::zeros(cur_img.size(), CV_8UC3);
    for (int i = 0; i<nuts.size(); i++) {
        current_nut = i;
        rateProc.reset();
        rateProc.process();
        float score = net.run(rateProc.getResults());
        nuts[current_nut].score = score;
        const vector<vector<Point>>& contours = rateProc.getContours();
        vector<Rect> rects(contours.size());
        for (unsigned int i = 0; i<contours.size(); i++) {
            Rect size = boundingRect(contours[i]);
            rects[i] = size;
        }
        int max_rect_index = -1;
        int max_rect_size = 0;
        for (unsigned int i = 0; i<rects.size(); i++) {
            if (rects[i].height * rects[i].width > max_rect_size) {
                max_rect_size = rects[i].height * rects[i].width;
                max_rect_index = i;
            }
        }
        drawContours(withScores, contours, max_rect_index, 
                Scalar(static_cast<uchar>(score * 100.0f *0.708333f), 255, 255),
                CV_FILLED, LINE_8, noArray(), INT_MAX, 
                Point(nuts[current_nut].min_y, nuts[current_nut].min_x));
    }
    emit statusMessage("Zeige Ergebnisse an ...");
    cvtColor(withScores, withScores, CV_HSV2BGR);
    withScores = 0.5 *withScores + 0.5 * cur_img;
    paintScores(withScores);
    emit statusMessage("Fertig.");
}

void Controller::debugResultPrint() {
    ostringstream sstr;
    sstr << "Results: ";
    for (const auto& result : rateProc.getResults()) {
        sstr << result.first << ": " << result.second << ", ";
    }
    cout << sstr.str() << endl;
}

void Controller::paintScores(Mat& img) {
    for (unsigned int i = 0; i<nuts.size(); i++){
        ostringstream sstr;
        sstr.precision(1);
        sstr << nuts[i].score;
        string text = sstr.str();
        int font = FONT_HERSHEY_SIMPLEX;
        double fontScale = 3.0;
        int thickness = 5;
        int baseline = 0;
        Size textSize = getTextSize(text, font, fontScale, thickness,
                &baseline);
        baseline += thickness;
        Point textOrg(nuts[i].center_y() - 0.5 * textSize.width,
                nuts[i].center_x() - 0.5 * textSize.height);
        putText(img, text, textOrg,
                font, fontScale, Scalar::all(255), thickness);
        circle(img, Point(nuts[i].center_y(), nuts[i].center_x()), 5,
                Scalar::all(255), 5);
    }
    emit newInputImage(img);
}

void Controller::debugPaint() {
    Mat withText = detProc.getResult().clone();
    for (unsigned int i = 0; i<nuts.size(); i++){
        string text = to_string(i);
        int font = FONT_HERSHEY_SIMPLEX;
        double fontScale = 3.0;
        int thickness = 5;
        int baseline = 0;
        Size textSize = getTextSize(text, font, fontScale, thickness,
                &baseline);
        baseline += thickness;
        Point textOrg(nuts[i].center_y() - 0.5 * textSize.width,
                nuts[i].center_x() - 0.5 * textSize.height);
        putText(withText, text, textOrg,
                font, fontScale, Scalar::all(255), thickness);
        circle(withText, Point(nuts[i].center_y(), nuts[i].center_x()), 5,
                Scalar::all(255), 5);
    }
    emit newInputImage(withText);
}

void Controller::captureImage() {
    system(
        "adb shell \"count=\\$(ls /mnt/sdcard/DCIM/CAMERA | wc -l) && "
        "am start -a android.media.action.STILL_IMAGE_CAMERA && "
        "sleep 1 && "
        "input keyevent 27 && "
        "while [ \\$count = \\$(ls /mnt/sdcard/DCIM/CAMERA | wc -l) ]; do "
        "sleep 0.5; "
        "echo 'not done'; done; "
        "echo done\"");
    string img = runCmd("adb shell \"ls /mnt/sdcard/DCIM/CAMERA | "
            "grep -v VID | tail -n 1\"");
    // remove white space
    img.erase(std::remove_if(img.begin(), img.end(), ::isspace), img.end());
    if (!transferFolder.isValid()) {
        cerr << "Failed to create temporary folder. Exiting.";
        exit(1);
    }
    string cmd = "adb pull \"/mnt/sdcard/DCIM/CAMERA/" + img + "\" \""
            + transferFolder.path().toStdString() + "/\"";
    system(cmd.c_str());
    system(("adb shell \"rm /mnt/sdcard/DCIM/CAMERA/" + img + "\"").c_str());
    filePath = transferFolder.path() + "/" + QString::fromStdString(img);
    loadImage();
    remove(filePath.toStdString().c_str());
}

bool Controller::detect() {
    auto start = chrono::steady_clock::now();
    bool success = detProc.process();
    if (!success) {
        cerr << "Error during detection." << endl;
        return success;
    }
    auto duration = chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now() - start);
    cout << "Detection took " << (float)duration.count()/1000.0 
        << " seconds." << endl;
    return success;
}

bool Controller::matchNuts(const vector<Nut>& cur_results) {
    auto start = chrono::steady_clock::now();
    vector<float> min_dists = vector<float>(nuts.size());
    int count = 0;
    for(const auto& nut : nuts) {
        vector<float> x_dist = vector<float>(cur_results.size());
        for (unsigned int i = 0; i<cur_results.size(); i++) {
            x_dist[i] = abs(cur_results[i].center_x() - nut.center_x());
        }
        vector<float> y_dist = vector<float>(cur_results.size());
        for (unsigned int i = 0; i<cur_results.size(); i++) {
            y_dist[i] = abs(cur_results[i].center_y() - nut.center_y());
        }
        vector<float> dist = vector<float>(cur_results.size());
        magnitude(x_dist, y_dist, dist);
        auto min = min_element(dist.begin(), dist.end());
        min_dists[count] = *min;
        cout << "Minimum distance for nut " << count <<
            ": " << *min 
            << " (" 
            << (*min/
                    (sqrt(pow(static_cast<float>(cur_img.cols), 2) +
                        pow(static_cast<float>(cur_img.rows), 2))))*100.0
            << "%)\n";
        count++;
    }
    bool success = false;
    if (cur_results.size() == nuts.size() - 1) {
        success = true;
        int nut_index;
        auto max = max_element(min_dists.begin(), min_dists.end());
        nut_index = distance(min_dists.begin(), max);
        cout << "Nut " << nut_index << " has been taken away.\n";
        nuts[nut_index].img.copyTo(nut_to_rate);
        emit newNutToRate(nut_to_rate);
    }
    else {
        emit message("Keine Nuss erkannt");
    }
    auto duration = chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now() - start);
    cout << "Matching took " << (float)duration.count()/1000.0 
        << " seconds." << endl;
    return success;
}

void Controller::extractNuts(vector<Nut>& result_nuts) {
    auto start = chrono::steady_clock::now();
    if (result_nuts.size() > 0) {
        return;
    }
    if (markers.empty()) {
        if (!detProc.process()) {
            return;
        }
    }
    int nut_count = detProc.getNutCount();
    result_nuts = vector<Nut>(nut_count);
    for (unsigned int i = 0; i<result_nuts.size(); i++) {
        result_nuts[i].min_x = markers.rows;
        result_nuts[i].min_y = markers.cols;
        result_nuts[i].max_x = 0;
        result_nuts[i].max_y = 0;
    }
    for(int i = 0; i<markers.rows; i++) {
        for(int j = 0; j<markers.cols; j++) {
            int index = markers.at<int>(i, j) -2;
            if (index >= 0 && index < nut_count) {
                result_nuts[index].min_x = min(result_nuts[index].min_x, i);
                result_nuts[index].min_y = min(result_nuts[index].min_y, j);
                result_nuts[index].max_x = max(result_nuts[index].max_x, i);
                result_nuts[index].max_y = max(result_nuts[index].max_y, j);
            }
        }
    }
    for (unsigned int i = 0; i<result_nuts.size(); i++) {
        result_nuts[i].img = Mat::zeros(result_nuts[i].max_x - result_nuts[i].min_x,
                result_nuts[i].max_y - result_nuts[i].min_y, CV_8UC3);
    }
    for(int i = 0; i<markers.rows; i++) {
        for(int j = 0; j<markers.cols; j++) {
            int index = markers.at<int>(i, j)-2;
            if (index >= 0 && index < nut_count) {
                result_nuts[index].img.at<Vec3b>(i - result_nuts[index].min_x, 
                        j - result_nuts[index].min_y) = cur_img.at<Vec3b>(i, j);
            }
        }
    }
    auto duration = chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now() - start);
    cout << "Nut extraction took " << (float)duration.count()/1000.0 
        << " seconds.\n";
    cout << "Detected " << result_nuts.size() << " nuts." << endl;
    rateProc.reset();
    if (rateProc.isInteractive()) {
        emit newNut(result_nuts[current_nut].img);
    }
}

void Controller::nextNut() {
    if (nuts.size() == 0 || nuts[current_nut].img.empty()) {
        return;
    }
    int size = nuts.size();
    current_nut = min(current_nut + 1, size-1);
    newNut();
    if (rateProc.isInteractive()) {
        emit newNut(nuts[current_nut].img);
        emit rateProc.requestUpdate();
    }
}

void Controller::previousNut()  {
    if (nuts.size() == 0 || nuts[current_nut].img.empty()) {
        return;
    }
    current_nut = max(current_nut - 1, 0);
    newNut();
    if (rateProc.isInteractive()) {
        emit newNut(nuts[current_nut].img);
    }
}

void Controller::newNut() {
    rateProc.reset();
    if (rateProc.isInteractive()) {
        rateProc.start(true);
    }
}

string Controller::runCmd(string cmd) {
    char buffer[128];
    string result = "";
    shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) throw runtime_error("popen() failed!");
    while (!feof(pipe.get())) {
        if (fgets(buffer, 128, pipe.get()) != NULL)
            result += buffer;
    }
    return result;
}

void Controller::saveDetectionConfig() {
    int current_step = detProc.getCurrentStep();
    config["detection"][current_step]["param1"]["default"] =
        detProc.getCurParams()[0].val;
    config["detection"][current_step]["param2"]["default"] =
        detProc.getCurParams()[1].val;
    ofstream file ("config.json");
    file << Json::writeString(Json::StreamWriterBuilder(), config);
    file.close();
}

void Controller::saveRatingConfig() {
    int current_step = rateProc.getCurrentStep();
    config["rating"]["steps"][current_step]["param1"]["default"] =
        rateProc.getCurParams()[0].val;
    config["rating"]["steps"][current_step]["param2"]["default"] =
        rateProc.getCurParams()[1].val;
    ofstream file ("config.json");
    file << Json::writeString(Json::StreamWriterBuilder(), config);
    file.close();
}

void Controller::reset() {
    resetNuts();
    detProc.reset();
    rateProc.reset();
    cur_img.release();
    prev_img.release();
    Mat empty;
    emit newInputImage(empty);
    emit newNutToRate(empty);
}

void Controller::resetNuts() {
    current_nut = 0;
    nut_rating = -1;
    nuts = {};
}

void Controller::saveData() {
    cout << "... saving data ... " << endl;
    if (valid_nut) {
        if (nut_rating > 0) {
            net.saveNut();
        }
        else {
            cout << "No score selected. Not saving Nut." << endl;
        }
    }
}

void Controller::currentRating(int score) {
    nut_rating = score;
}

int Controller::getCurRating() {
    return nut_rating;
}

int Controller::getN() {
    return net.getN();
}

void Controller::neuralNetChange(int val) {
    if (val == Qt::Unchecked) {
        net.neuralNetEnabled(false);
        process();
    }
    else if (val == Qt::Checked) {
        net.neuralNetEnabled(true);
        process();
    }
}

const cv::Mat* Controller::getNutToRate() {
    return &nut_to_rate;
}
