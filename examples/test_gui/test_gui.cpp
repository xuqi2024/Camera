/**
 * @file test_gui.cpp
 * @brief Qt GUI测试程序
 */
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QCheckBox>
#include <QSlider>
#include <QGroupBox>
#include <QDateTime>
#include <opencv2/opencv.hpp>
#include <iostream>

class CameraViewer : public QMainWindow {
    Q_OBJECT
public:
    CameraViewer(QWidget *parent = nullptr) : QMainWindow(parent) {
        setupUi();
        setupCamera();
    }

    ~CameraViewer() {
        if (capture_.isOpened()) {
            capture_.release();
        }
    }

private slots:
    void updateFrame() {
        cv::Mat frame;
        if (capture_.read(frame)) {
            if (detection_enabled_) {
                // 这里可以添加目标检测代码
                cv::rectangle(frame, cv::Point(100, 100), cv::Point(200, 200), cv::Scalar(0, 255, 0), 2);
            }

            // 转换为Qt图像
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            QImage image(frame.data, frame.cols, frame.rows, 
                        frame.step, QImage::Format_RGB888);
            
            // 更新显示
            image_label_->setPixmap(QPixmap::fromImage(image).scaled(
                image_label_->size(), Qt::KeepAspectRatio));

            // 更新FPS
            frame_count_++;
            auto current_time = std::chrono::steady_clock::now();
            auto time_diff = std::chrono::duration_cast<std::chrono::seconds>
                           (current_time - last_time_).count();
            
            if (time_diff >= 1) {
                fps_label_->setText(QString("FPS: %1").arg(frame_count_ / time_diff));
                frame_count_ = 0;
                last_time_ = current_time;
            }
        }
    }

    void toggleDetection(bool enabled) {
        detection_enabled_ = enabled;
    }

    void setConfidence(int value) {
        float threshold = value / 100.0f;
        confidence_label_->setText(QString("置信度: %1").arg(threshold));
        // 这里可以更新检测阈值
    }

    void takeSnapshot() {
        QString filename = QString("snapshot_%1.jpg").arg(
            QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        
        const QPixmap *pixmap = image_label_->pixmap();
        if (pixmap) {
            pixmap->save(filename);
            std::cout << "保存截图: " << filename.toStdString() << std::endl;
        }
    }

private:
    void setupUi() {
        // 创建中央部件
        QWidget *central_widget = new QWidget(this);
        setCentralWidget(central_widget);

        // 创建布局
        QVBoxLayout *main_layout = new QVBoxLayout(central_widget);
        QHBoxLayout *control_layout = new QHBoxLayout();

        // 创建视频显示标签
        image_label_ = new QLabel(this);
        image_label_->setMinimumSize(640, 480);
        image_label_->setAlignment(Qt::AlignCenter);
        main_layout->addWidget(image_label_);

        // 创建FPS标签
        fps_label_ = new QLabel("FPS: 0", this);
        main_layout->addWidget(fps_label_);

        // 创建控制组
        QGroupBox *control_group = new QGroupBox("控制", this);
        QVBoxLayout *control_group_layout = new QVBoxLayout(control_group);

        // 创建检测开关
        QCheckBox *detection_checkbox = new QCheckBox("启用目标检测", this);
        detection_checkbox->setChecked(true);
        connect(detection_checkbox, &QCheckBox::toggled, this, &CameraViewer::toggleDetection);
        control_group_layout->addWidget(detection_checkbox);

        // 创建置信度滑块
        QSlider *confidence_slider = new QSlider(Qt::Horizontal, this);
        confidence_slider->setRange(0, 100);
        confidence_slider->setValue(50);
        connect(confidence_slider, &QSlider::valueChanged, this, &CameraViewer::setConfidence);
        
        confidence_label_ = new QLabel("置信度: 0.50", this);
        control_group_layout->addWidget(confidence_label_);
        control_group_layout->addWidget(confidence_slider);

        // 创建截图按钮
        QPushButton *snapshot_button = new QPushButton("截图", this);
        connect(snapshot_button, &QPushButton::clicked, this, &CameraViewer::takeSnapshot);
        control_group_layout->addWidget(snapshot_button);

        // 添加控制组到主布局
        main_layout->addWidget(control_group);

        // 设置窗口标题和大小
        setWindowTitle("摄像头查看器");
        resize(800, 600);
    }

    void setupCamera() {
        // 打开摄像头
        capture_.open(0);
        if (!capture_.isOpened()) {
            std::cerr << "无法打开摄像头" << std::endl;
            return;
        }

        // 设置定时器更新画面
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &CameraViewer::updateFrame);
        timer->start(30); // 约30FPS

        // 初始化时间戳
        last_time_ = std::chrono::steady_clock::now();
    }

private:
    cv::VideoCapture capture_;
    QLabel *image_label_;
    QLabel *fps_label_;
    QLabel *confidence_label_;
    bool detection_enabled_ = true;
    int frame_count_ = 0;
    std::chrono::steady_clock::time_point last_time_;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    CameraViewer viewer;
    viewer.show();
    return app.exec();
}

#include "test_gui.moc" 