#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

class VideoCapture {
public:
    VideoCapture();
    ~VideoCapture();
    
    bool start(int device_id = 0);
    void stop();
    std::string getLatestFrame(); // 返回编码后的帧数据

private:
    void captureLoop();
    
    cv::VideoCapture cap_;
    std::thread capture_thread_;
    std::mutex frame_mutex_;
    std::atomic<bool> running_{false};
    std::string latest_frame_;
}; 