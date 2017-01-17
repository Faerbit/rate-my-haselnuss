#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QShortcut>
#include <QCloseEvent>
#include <QTimer>
#include "detectionprocessing.h"
#include "rateprocessing.h"
#include "controller.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
        Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void updateDetection();
    void updateRating();
    void modeConfig(int tabIndex);
    void mode(int tabIndex);
    void capture();
    void load();
    void select1();
    void select2();
    void select3();
    void select4();
    void select5();

private:
    void closeEvent(QCloseEvent* event);
    void resetRateButtons();
    void updateN();

// Members
private:
    Ui::MainWindow* ui;
    Controller& controller;
    DetectionProcessing& detProc;
    RateProcessing& rateProc;
    QTimer* autoSaveTimer;
    QShortcut* captureShortcut;
    QShortcut* captureShortcut2;
};

#endif // MAINWINDOW_H
