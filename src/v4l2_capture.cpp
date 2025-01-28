/**
 * @file v4l2_capture.cpp
 * @brief V4L2视频捕获实现
 * @details 使用Video4Linux2 API实现摄像头视频捕获功能
 */

#include "v4l2_capture.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

V4L2Capture::V4L2Capture() = default;

V4L2Capture::~V4L2Capture() {
    stop();
}

bool V4L2Capture::start(int device_id) {
    // 初始化设备
    if (!initDevice(device_id)) {
        std::cerr << "设备初始化失败" << std::endl;
        return false;
    }

    // 启动视频流
    running_ = true;
    capture_thread_ = std::thread(&V4L2Capture::captureLoop, this);
    return true;
}

void V4L2Capture::stop() {
    // 停止捕获线程
    if (running_) {
        running_ = false;
        if (capture_thread_.joinable()) {
            capture_thread_.join();
        }
    }

    // 释放设备资源
    if (buffer_) {
        munmap(buffer_, buffer_size_);
        buffer_ = nullptr;
    }

    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

std::string V4L2Capture::getLatestFrame() {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    return latest_frame_;
}

bool V4L2Capture::initDevice(int device_id) {
    char dev_name[64];
    snprintf(dev_name, sizeof(dev_name), "/dev/video%d", device_id);
    
    // 打开设备
    fd_ = open(dev_name, O_RDWR);
    if (fd_ < 0) {
        std::cerr << "无法打开设备: " << dev_name << std::endl;
        return false;
    }

    // 设置视频格式
    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;         // 设置捕获宽度
    fmt.fmt.pix.height = 480;        // 设置捕获高度
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;  // 使用YUYV格式
    
    if (ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
        std::cerr << "设置视频格式失败" << std::endl;
        return false;
    }

    // 请求缓冲区
    struct v4l2_requestbuffers req = {};
    req.count = 1;  // 请求一个缓冲区
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(fd_, VIDIOC_REQBUFS, &req) < 0) {
        std::cerr << "请求缓冲区失败" << std::endl;
        return false;
    }

    // 查询并映射缓冲区
    buf_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf_.memory = V4L2_MEMORY_MMAP;
    buf_.index = 0;
    
    if (ioctl(fd_, VIDIOC_QUERYBUF, &buf_) < 0) {
        std::cerr << "查询缓冲区失败" << std::endl;
        return false;
    }

    // 内存映射
    buffer_ = mmap(nullptr, buf_.length, PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd_, buf_.m.offset);
    buffer_size_ = buf_.length;
    
    if (buffer_ == MAP_FAILED) {
        std::cerr << "内存映射失败" << std::endl;
        return false;
    }

    // 将缓冲区加入队列
    if (ioctl(fd_, VIDIOC_QBUF, &buf_) < 0) {
        std::cerr << "缓冲区入队失败" << std::endl;
        return false;
    }

    // 开启视频流
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd_, VIDIOC_STREAMON, &type) < 0) {
        std::cerr << "启动视频流失败" << std::endl;
        return false;
    }

    return true;
}

void V4L2Capture::captureLoop() {
    while (running_) {
        // 从队列中取出缓冲区
        if (ioctl(fd_, VIDIOC_DQBUF, &buf_) < 0) {
            std::cerr << "取出缓冲区失败" << std::endl;
            continue;
        }

        // 将YUYV格式转换为JPEG
        cv::Mat yuyv_mat(480, 640, CV_8UC2, buffer_);
        cv::Mat bgr_mat;
        cv::cvtColor(yuyv_mat, bgr_mat, cv::COLOR_YUV2BGR_YUYV);
        
        std::vector<uchar> jpeg_buffer;
        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90};
        cv::imencode(".jpg", bgr_mat, jpeg_buffer, params);

        // 更新最新帧
        {
            std::lock_guard<std::mutex> lock(frame_mutex_);
            latest_frame_ = std::string(
                reinterpret_cast<char*>(jpeg_buffer.data()),
                jpeg_buffer.size()
            );
        }

        // 将缓冲区重新加入队列
        if (ioctl(fd_, VIDIOC_QBUF, &buf_) < 0) {
            std::cerr << "缓冲区入队失败" << std::endl;
        }

        // 控制帧率
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

std::string convertYUYVtoJPEG(uint8_t* yuyv, int width, int height) {
    // 创建OpenCV Mat来存储YUYV数据
    cv::Mat yuyv_mat(height, width, CV_8UC2, yuyv);
    
    // 转换到BGR格式
    cv::Mat bgr_mat;
    cv::cvtColor(yuyv_mat, bgr_mat, cv::COLOR_YUV2BGR_YUYV);
    
    // 编码为JPEG
    std::vector<uchar> jpeg_buffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90};
    cv::imencode(".jpg", bgr_mat, jpeg_buffer, params);
    
    return std::string(reinterpret_cast<char*>(jpeg_buffer.data()), jpeg_buffer.size());
} 