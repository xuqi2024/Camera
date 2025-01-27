#include "video_capture.h"
#include <vector>

VideoCapture::VideoCapture() = default;

VideoCapture::~VideoCapture() {
    stop();
}

bool VideoCapture::start(int device_id) {
    if (!cap_.open(device_id)) {
        return false;
    }
    
    running_ = true;
    capture_thread_ = std::thread(&VideoCapture::captureLoop, this);
    return true;
}

void VideoCapture::stop() {
    running_ = false;
    if (capture_thread_.joinable()) {
        capture_thread_.join();
    }
    cap_.release();
}

void VideoCapture::captureLoop() {
    std::vector<uchar> buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80};
    
    while (running_) {
        cv::Mat frame;
        if (!cap_.read(frame)) {
            continue;
        }
        
        // 将帧编码为JPEG格式
        cv::imencode(".jpg", frame, buffer, params);
        
        {
            std::lock_guard<std::mutex> lock(frame_mutex_);
            latest_frame_ = std::string(buffer.begin(), buffer.end());
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(30)); // ~30 FPS
    }
}

std::string VideoCapture::getLatestFrame() {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    return latest_frame_;
} 