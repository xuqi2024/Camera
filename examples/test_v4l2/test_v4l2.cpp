/**
 * @file test_v4l2.cpp
 * @brief V4L2摄像头捕获测试程序
 */
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/videodev2.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <chrono>
#include <thread>

class V4L2Test {
public:
    bool start(int device_id = 0) {
        char dev_name[64];
        snprintf(dev_name, sizeof(dev_name), "/dev/video%d", device_id);
        
        // 打开设备
        fd_ = open(dev_name, O_RDWR);
        if (fd_ < 0) {
            std::cerr << "无法打开设备: " << dev_name << std::endl;
            return false;
        }

        // 查询设备能力
        struct v4l2_capability cap;
        if (ioctl(fd_, VIDIOC_QUERYCAP, &cap) < 0) {
            std::cerr << "无法查询设备能力" << std::endl;
            return false;
        }

        std::cout << "设备信息:" << std::endl
                  << "  驱动: " << cap.driver << std::endl
                  << "  卡名: " << cap.card << std::endl
                  << "  总线信息: " << cap.bus_info << std::endl;

        // 设置视频格式
        struct v4l2_format fmt = {};
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width = 640;
        fmt.fmt.pix.height = 480;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        
        if (ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) {
            std::cerr << "设置视频格式失败" << std::endl;
            return false;
        }

        // 请求缓冲区
        struct v4l2_requestbuffers req = {};
        req.count = 1;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        
        if (ioctl(fd_, VIDIOC_REQBUFS, &req) < 0) {
            std::cerr << "请求缓冲区失败" << std::endl;
            return false;
        }

        // 查询并映射缓冲区
        struct v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = 0;
        
        if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) < 0) {
            std::cerr << "查询缓冲区失败" << std::endl;
            return false;
        }

        buffer_ = mmap(nullptr, buf.length, PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd_, buf.m.offset);
        buffer_size_ = buf.length;
        
        if (buffer_ == MAP_FAILED) {
            std::cerr << "内存映射失败" << std::endl;
            return false;
        }

        // 将缓冲区加入队列
        if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
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

    void stop() {
        if (buffer_) {
            munmap(buffer_, buffer_size_);
            buffer_ = nullptr;
        }

        if (fd_ >= 0) {
            close(fd_);
            fd_ = -1;
        }
    }

    bool captureFrame() {
        struct v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = 0;

        // 从队列中取出缓冲区
        if (ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
            std::cerr << "取出缓冲区失败" << std::endl;
            return false;
        }

        // 将YUYV格式转换为JPEG
        cv::Mat yuyv_mat(480, 640, CV_8UC2, buffer_);
        cv::Mat bgr_mat;
        cv::cvtColor(yuyv_mat, bgr_mat, cv::COLOR_YUV2BGR_YUYV);
        
        // 保存图像
        static int frame_count = 0;
        std::string filename = "frame_" + std::to_string(frame_count++) + ".jpg";
        cv::imwrite(filename, bgr_mat);
        std::cout << "保存图像: " << filename << std::endl;

        // 将缓冲区重新加入队列
        if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
            std::cerr << "缓冲区入队失败" << std::endl;
            return false;
        }

        return true;
    }

private:
    int fd_{-1};
    void* buffer_{nullptr};
    size_t buffer_size_{0};
};

int main() {
    V4L2Test camera;
    
    if (!camera.start()) {
        std::cerr << "启动摄像头失败" << std::endl;
        return 1;
    }

    std::cout << "开始捕获图像，按Enter键退出..." << std::endl;

    // 捕获10帧图像
    for (int i = 0; i < 10; ++i) {
        if (!camera.captureFrame()) {
            std::cerr << "捕获图像失败" << std::endl;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    camera.stop();
    return 0;
} 