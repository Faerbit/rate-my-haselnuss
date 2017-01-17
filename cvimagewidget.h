#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QResizeEvent>
#include <QDebug>
#include <opencv2/opencv.hpp>

class CVImageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CVImageWidget(QWidget *parent = 0) : QWidget(parent) {}

    QSize sizeHint() const { return _qimage.size(); }
    QSize minimumSizeHint() const { return QSize(400, 400); }

public slots:
    
    void showImage(const cv::Mat& image) {
        if (image.empty()) {
            _qimage = QImage();
            repaint();
            return;
        }

        cv::Mat tmp;
        if (image.type() == CV_32S) {
            image.convertTo(tmp, CV_8U);
        }
        else {
            tmp = image.clone();
        }
        
        // Scale image to 512 pixels
        float fac = 1.0;
        if (size().width() > size().height()) {
            fac = (float) size().height()/image.rows;
        }
        else {
            fac = (float) size().width()/image.cols;
        }
        cv::Size new_size((int)image.cols*fac, (int) image.rows*fac) ;
        cv::resize(tmp, _tmp, new_size);
        // Convert the image to the RGB888 format
        switch (_tmp.type()) {
        case CV_8UC1:
            cvtColor(_tmp, _tmp, CV_GRAY2RGB);
            break;
        case CV_8UC3:
            cvtColor(_tmp, _tmp, CV_BGR2RGB);
            break;
        }

        // QImage needs the data to be stored continuously in memory
        assert(_tmp.isContinuous());
        // Assign OpenCV's image buffer to the QImage. Note that the
        // bytesPerLine parameter
        // (http://qt-project.org/doc/qt-4.8/qimage.html#QImage-6) is 3*width
        // because each pixel has three bytes.
        _qimage = QImage(_tmp.data, _tmp.cols, _tmp.rows, _tmp.cols*3,
                QImage::Format_RGB888);
        repaint();
    }

    void resizeEvent(QResizeEvent* event) {
        if (_tmp.empty()) {
            return;
        }
        float fac = 1.0;
        if (event->size().width() > event->size().height()) {
            fac = static_cast<float>(event->size().height())/
                static_cast<float>(_tmp.rows);
        }
        else {
            fac = static_cast<float>(event->size().width())/
                static_cast<float>(_tmp.cols);
        }
        cv::Size new_size((int)_tmp.cols*fac, (int) _tmp.rows*fac) ;
        cv::resize(_tmp, _tmp, new_size);
        // Assign OpenCV's image buffer to the QImage. Note that the
        // bytesPerLine parameter
        // (http://qt-project.org/doc/qt-4.8/qimage.html#QImage-6) is 3*width
        // because each pixel has three bytes.
        _qimage = QImage(_tmp.data, _tmp.cols, _tmp.rows, _tmp.cols*3,
                QImage::Format_RGB888);
        repaint();
    }

    void showTextImage(std::string text) {
        int fontFace = cv::FONT_HERSHEY_SIMPLEX;
        double fontScale = 2;
        int thickness = 3;
        cv::Mat img(600, 800, CV_8UC3, cv::Scalar::all(0));
        int baseline=0;
        cv::Size textSize = cv::getTextSize(text, fontFace,
                                    fontScale, thickness, &baseline);
        baseline += thickness;
        // center the text
        cv::Point textOrg((img.cols - textSize.width)/2,
                      (img.rows + textSize.height)/2);
        // then put the text itself
        cv::putText(img, text, textOrg, fontFace, fontScale,
                cv::Scalar::all(255), thickness, 8);
        showImage(img);
    }


protected:

    void paintEvent(QPaintEvent* /*event*/) {
        // Display the image
        QPainter painter(this);
        painter.drawImage(QPoint(0,0), _qimage);
        painter.end();
    }
    
    QImage _qimage;
    cv::Mat _tmp;
};

#endif //CVIMAGEWIDGET_H
