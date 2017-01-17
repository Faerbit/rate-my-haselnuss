#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QApplication>
#include <QDebug>

using namespace std;

MainWindow::MainWindow(QWidget *parent) : controller(Controller::inst()),
    detProc(controller.getDetectionProcessing()),
    rateProc(controller.getRateProcessing()),
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    //QDir::setCurrent(qApp->applicationDirPath());
    connect(ui->loadButton, SIGNAL( clicked() ), 
            this, SLOT( load() ));
    connect(ui->nextButton, SIGNAL( clicked() ),
            &detProc, SLOT( nextStep() ));
    connect(ui->previousButton, SIGNAL( clicked() ),
            &detProc, SLOT( previousStep() ));
    connect(ui->saveButton, SIGNAL( clicked() ),
            &detProc, SLOT( saveCur() ));
    connect(ui->saveConfigButton, SIGNAL( clicked() ),
            &controller, SLOT( saveDetectionConfig() ));
    connect(ui->captureButton, SIGNAL( clicked() ),
            this, SLOT( capture() ));
    connect(ui->resetButton, SIGNAL( clicked() ),
            &controller, SLOT( reset() ));
    captureShortcut = new QShortcut(QKeySequence("Return"),
            ui->liveTab);
    connect(captureShortcut, SIGNAL( activated() ),
            ui->captureButton, SLOT ( animateClick() ));
    captureShortcut2 = new QShortcut(QKeySequence("Space"),
            ui->liveTab);
    connect(captureShortcut2, SIGNAL( activated() ),
            ui->captureButton, SLOT ( animateClick() ));
    connect(&detProc, SIGNAL( newInputImage(const cv::Mat&) ),
            ui->inputView, SLOT( showImage(const cv::Mat&)));
    connect(&detProc, SIGNAL( newOutputImage(const cv::Mat&) ),
            ui->outputView, SLOT( showImage(const cv::Mat&)));
    connect(ui->param1SpinBox, SIGNAL ( valueChanged(int) ),
            &detProc, SLOT( setParam1(int) ));
    connect(ui->param2SpinBox, SIGNAL ( valueChanged(int) ),
            &detProc, SLOT( setParam2(int) ));
    connect(&detProc, SIGNAL( requestUpdate() ),
            this, SLOT( updateDetection() ));
    connect(&rateProc, SIGNAL( requestUpdate() ),
            this, SLOT( updateRating() ));
    connect(ui->rate_nextButton, SIGNAL( clicked() ),
            &rateProc, SLOT( nextStep() ));
    connect(ui->rate_previousButton, SIGNAL( clicked() ),
            &rateProc, SLOT( previousStep() ));
    connect(ui->rate_saveButton, SIGNAL( clicked() ),
            &rateProc, SLOT( saveCur() ));
    connect(ui->rate_saveConfigButton, SIGNAL( clicked() ),
            &controller, SLOT( saveRatingConfig() ));
    connect(&rateProc, SIGNAL( newInputImage(const cv::Mat&) ),
            ui->rate_inputView, SLOT( showImage(const cv::Mat&)));
    connect(&rateProc, SIGNAL( newOutputImage(const cv::Mat&) ),
            ui->rate_outputView, SLOT( showImage(const cv::Mat&)));
    connect(ui->rate_param1SpinBox, SIGNAL ( valueChanged(int) ),
            &rateProc, SLOT( setParam1(int) ));
    connect(ui->rate_param2SpinBox, SIGNAL ( valueChanged(int) ),
            &rateProc, SLOT( setParam2(int) ));
    connect(ui->configTabWidget, SIGNAL( currentChanged(int) ),
            this, SLOT( modeConfig(int) ));
    connect(ui->superTabWidget, SIGNAL( currentChanged(int) ),
            this, SLOT( mode(int) ));
    connect(ui->previousNut, SIGNAL( clicked() ),
            &controller, SLOT( previousNut() ));
    connect(ui->nextNut, SIGNAL( clicked() ),
            &controller, SLOT( nextNut() ));
    connect(&controller, SIGNAL( newNut(const cv::Mat&) ),
            ui->rate_inputView , SLOT( showImage(const cv::Mat&) ));
    connect(&controller, SIGNAL( newInputImage(const cv::Mat&) ),
            ui->liveView , SLOT( showImage(const cv::Mat&) ));
    connect(&controller, SIGNAL( newNutToRate(const cv::Mat&) ),
            ui->liveNut, SLOT( showImage(const cv::Mat&) ));
    connect(&controller, SIGNAL( message(std::string ) ),
            ui->liveNut, SLOT( showTextImage(std::string) ));
    connect(ui->rateButton1, SIGNAL(clicked()),
            this, SLOT( select1()));
    connect(ui->rateButton2, SIGNAL(clicked()),
            this, SLOT( select2()));
    connect(ui->rateButton3, SIGNAL(clicked()),
            this, SLOT( select3()));
    connect(ui->rateButton4, SIGNAL(clicked()),
            this, SLOT( select4()));
    connect(ui->rateButton5, SIGNAL(clicked()),
            this, SLOT( select5()));
    connect(&controller, SIGNAL( statusMessage(QString) ),
            ui->statusLabel, SLOT( setText(QString) ));
    connect(ui->neuralNetCheckBox, SIGNAL( stateChanged(int) ),
            &controller, SLOT( neuralNetChange(int) ));
    updateDetection();
    updateRating();
    updateN();
}

void MainWindow::select1() {
    controller.currentRating(1);
}

void MainWindow::select2() {
    controller.currentRating(2);
}

void MainWindow::select3() {
    controller.currentRating(3);
}

void MainWindow::select4() {
    controller.currentRating(4);
}

void MainWindow::select5() {
    controller.currentRating(5);
}

void MainWindow::load() {
    resetRateButtons();
    controller.openImage();
    updateN();
}

void MainWindow::updateN() {
    ostringstream sstr;
    sstr << "N = " << to_string(controller.getN());
    ui->NLabel->setText(QString::fromStdString(sstr.str()));
}

void MainWindow::capture() {
    resetRateButtons();
    ui->captureButton->setEnabled(false);
    ui->captureButton->blockSignals(true);
    captureShortcut->blockSignals(true);
    captureShortcut2->blockSignals(true);
    qApp->processEvents();
    controller.captureImage();
    ui->captureButton->setEnabled(true);
    captureShortcut->blockSignals(false);
    captureShortcut2->blockSignals(false);
    ui->captureButton->blockSignals(false);
    updateN();
}

void MainWindow::resetRateButtons() {
    ui->rateButton1->setAutoExclusive(false);
    ui->rateButton2->setAutoExclusive(false);
    ui->rateButton3->setAutoExclusive(false);
    ui->rateButton4->setAutoExclusive(false);
    ui->rateButton5->setAutoExclusive(false);
    ui->rateButton1->setChecked(false);
    ui->rateButton2->setChecked(false);
    ui->rateButton3->setChecked(false);
    ui->rateButton4->setChecked(false);
    ui->rateButton5->setChecked(false);
    ui->rateButton1->setAutoExclusive(true);
    ui->rateButton2->setAutoExclusive(true);
    ui->rateButton3->setAutoExclusive(true);
    ui->rateButton4->setAutoExclusive(true);
    ui->rateButton5->setAutoExclusive(true);
}

void MainWindow::updateDetection() {
    ui->progressBar->setValue(detProc.getProgress());
    ui->operationLabel->setText(QString::fromStdString("Aktuelle Operation: \""
                + detProc.getStepName() + "\""));
    vector<Parameter> params = detProc.getCurParams();
    ui->param1Label->setText(QString::fromStdString("Parameter 1: \"" +
                params[0].name + "\""));
    ui->param2Label->setText(QString::fromStdString("Parameter 2: \"" +
                params[1].name + "\""));
    ui->param1SpinBox->blockSignals(true);
    ui->param2SpinBox->blockSignals(true);
    ui->param1Slider->setMinimum(params[0].lowerBound);
    ui->param1Slider->setMaximum(params[0].upperBound);
    ui->param1Slider->setValue(params[0].val);
    ui->param2Slider->setMinimum(params[1].lowerBound);
    ui->param2Slider->setMaximum(params[1].upperBound);
    ui->param2Slider->setValue(params[1].val);
    ui->param1SpinBox->setMinimum(params[0].lowerBound);
    ui->param1SpinBox->setMaximum(params[0].upperBound);
    ui->param1SpinBox->setValue(params[0].val);
    ui->param2SpinBox->setMinimum(params[1].lowerBound);
    ui->param2SpinBox->setMaximum(params[1].upperBound);
    ui->param2SpinBox->setValue(params[1].val);
    ui->param1SpinBox->blockSignals(false);
    ui->param2SpinBox->blockSignals(false);
}

void MainWindow::updateRating() {
    ui->rate_progressBar->setValue(rateProc.getProgress());
    ui->rate_operationLabel->setText(QString::fromStdString(
                "Aktuelle Operation: \""
                + rateProc.getStepName() + "\""));
    vector<Parameter> params = rateProc.getCurParams();
    ui->rate_param1Label->setText(QString::fromStdString("Parameter 1: \"" +
                params[0].name + "\""));
    ui->rate_param2Label->setText(QString::fromStdString("Parameter 2: \"" +
                params[1].name + "\""));
    ui->rate_param1SpinBox->blockSignals(true);
    ui->rate_param2SpinBox->blockSignals(true);
    ui->rate_param1Slider->setMinimum(params[0].lowerBound);
    ui->rate_param1Slider->setMaximum(params[0].upperBound);
    ui->rate_param1Slider->setValue(params[0].val);
    ui->rate_param2Slider->setMinimum(params[1].lowerBound);
    ui->rate_param2Slider->setMaximum(params[1].upperBound);
    ui->rate_param2Slider->setValue(params[1].val);
    ui->rate_param1SpinBox->setMinimum(params[0].lowerBound);
    ui->rate_param1SpinBox->setMaximum(params[0].upperBound);
    ui->rate_param1SpinBox->setValue(params[0].val);
    ui->rate_param2SpinBox->setMinimum(params[1].lowerBound);
    ui->rate_param2SpinBox->setMaximum(params[1].upperBound);
    ui->rate_param2SpinBox->setValue(params[1].val);
    ui->rate_param1SpinBox->blockSignals(false);
    ui->rate_param2SpinBox->blockSignals(false);
    ostringstream sstr;
    sstr << "Ergebnisse: ";
    for (auto&& result : rateProc.getResults()) {
        sstr << result.first << ": " << result.second << ", ";
    }
    ui->resultLabel->setText(QString::fromStdString(sstr.str()));
}

void MainWindow::mode(int tabIndex) {
    if (tabIndex == 0) {
        detProc.setInteractive(false);
        rateProc.setInteractive(false);
        controller.process();
    }
    else if (tabIndex == 1) {
        modeConfig(ui->configTabWidget->currentIndex());
    }
}

void MainWindow::modeConfig(int tabIndex) {
    if (tabIndex == 0) {
        detProc.setInteractive(true);
        rateProc.setInteractive(false);
        updateDetection();
        detProc.start();
    }
    else if (tabIndex == 1) {
        detProc.setInteractive(false);
        rateProc.setInteractive(true);
        updateRating();
        controller.resetNuts();
        controller.extractNuts();
        rateProc.start(true);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    controller.saveData();
}

MainWindow::~MainWindow()
{
        delete ui;
        delete captureShortcut;
        delete captureShortcut2;
}
