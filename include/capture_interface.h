/**
 * @file capture_interface.h
 * @brief 视频捕获接口类的定义
 * @details 定义了视频捕获设备的抽象接口，作为所有具体视频捕获实现的基类
 */

#pragma once
#include <string>

/**
 * @class CaptureInterface
 * @brief 视频捕获接口抽象类
 * @details 定义了与视频设备交互的基本操作接口，所有具体的视频捕获实现都需要继承此类
 */
class CaptureInterface {
public:
    /**
     * @brief 虚析构函数
     * @details 确保正确释放派生类资源
     */
    virtual ~CaptureInterface() = default;
    
    /**
     * @brief 启动视频捕获
     * @param device_id 视频设备ID，Linux系统中通常对应/dev/videoX中的X
     * @return 启动是否成功
     * @details 打开并初始化视频设备，开始捕获视频流
     */
    virtual bool start(int device_id = 0) = 0;
    
    /**
     * @brief 停止视频捕获
     * @details 停止视频流捕获，关闭设备，释放相关资源
     */
    virtual void stop() = 0;
    
    /**
     * @brief 获取最新的视频帧
     * @return JPEG格式的图像数据
     * @details 以字符串形式返回最新捕获的视频帧，使用JPEG编码
     */
    virtual std::string getLatestFrame() = 0;
}; 