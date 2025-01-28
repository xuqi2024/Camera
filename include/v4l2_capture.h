/**
 * @file v4l2_capture.h
 * @brief V4L2视频捕获类的定义
 * @details 使用Video4Linux2 API实现的视频捕获类
 */

#pragma once
#include "capture_interface.h"
#include <linux/videodev2.h>
#include <thread>
#include <mutex>
#include <atomic>

/**
 * @class V4L2Capture
 * @brief V4L2视频捕获实现类
 * @details 使用V4L2 API实现视频捕获，支持YUYV格式和JPEG编码
 */
class V4L2Capture : public CaptureInterface {
public:
    /**
     * @brief 构造函数
     * @details 初始化成员变量
     */
    V4L2Capture();
    
    /**
     * @brief 析构函数
     * @details 确保停止捕获并释放资源
     */
    ~V4L2Capture() override;
    
    /**
     * @brief 启动视频捕获
     * @param device_id 设备ID
     * @return 是否成功启动
     */
    bool start(int device_id = 0) override;
    
    /**
     * @brief 停止视频捕获
     */
    void stop() override;
    
    /**
     * @brief 获取最新帧
     * @return JPEG格式的图像数据
     */
    std::string getLatestFrame() override;

private:
    /**
     * @brief 视频捕获线程函数
     * @details 在独立线程中持续捕获视频帧
     */
    void captureLoop();
    
    /**
     * @brief 初始化视频设备
     * @param device_id 设备ID
     * @return 是否成功初始化
     */
    bool initDevice(int device_id);
    
    int fd_{-1};                     ///< 设备文件描述符
    struct v4l2_buffer buf_{};       ///< V4L2缓冲区结构
    void* buffer_{nullptr};          ///< 内存映射缓冲区指针
    size_t buffer_size_{0};          ///< 缓冲区大小
    
    std::thread capture_thread_;     ///< 捕获线程
    std::mutex frame_mutex_;         ///< 帧数据互斥锁
    std::atomic<bool> running_{false}; ///< 运行状态标志
    std::string latest_frame_;       ///< 最新帧数据缓存
}; 