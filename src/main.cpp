/**
 * @file main.cpp
 * @brief 程序入口文件
 * @details 初始化视频捕获和Web服务器，处理信号和优雅退出
 */

#include "v4l2_capture.h"
#include "web_server.h"
#include <iostream>
#include <memory>
#include <csignal>
#include <condition_variable>
#include <mutex>
#include "version.h"

// 全局变量用于信号处理和优雅退出
std::condition_variable exit_cv;        ///< 退出条件变量
std::mutex exit_mutex;                  ///< 退出互斥锁
bool should_exit = false;               ///< 退出标志

/**
 * @brief 信号处理函数
 * @param sig 信号值
 * @details 处理SIGINT和SIGTERM信号，实现优雅退出
 */
void signal_handler(int) {
    std::cout << "\n收到关闭信号，正在停止服务..." << std::endl;
    {
        std::lock_guard<std::mutex> lock(exit_mutex);
        should_exit = true;
    }
    exit_cv.notify_one();
}

/**
 * @brief 打印版本信息
 * @details 显示程序名称、版本号和描述信息
 */
void printVersion() {
    std::cout << PROJECT_NAME << " v" << PROJECT_VERSION << std::endl;
    std::cout << PROJECT_DESCRIPTION << std::endl;
}

/**
 * @brief 主函数
 * @param argc 参数数量
 * @param argv 参数数组
 * @return 程序退出码
 */
int main(int argc, char* argv[]) {
    printVersion();
    
    // 设置信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // 创建视频捕获对象
    auto video_capture = std::make_shared<V4L2Capture>();
    
    // 启动视频捕获
    if (!video_capture->start(0)) {
        std::cerr << "启动视频捕获失败" << std::endl;
        return 1;
    }
    
    // 创建并启动Web服务器
    WebServer server(video_capture);
    std::cout << "服务器运行在 http://localhost:8080" << std::endl;
    
    try {
        server.start(8080);
        
        // 等待退出信号
        std::unique_lock<std::mutex> lock(exit_mutex);
        exit_cv.wait(lock, [] { return should_exit; });
        
        // 清理资源
        std::cout << "正在关闭服务..." << std::endl;
        server.stop();
        video_capture->stop();
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 