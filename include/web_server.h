/**
 * @file web_server.h
 * @brief Web服务器类的定义
 * @details 使用Poco库实现HTTP和WebSocket服务
 */

#pragma once
#include "capture_interface.h"
#include "image_processor.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/WebSocket.h>
#include <memory>
#include <thread>
#include <atomic>

/**
 * @class WebServer
 * @brief Web服务器类
 * @details 提供HTTP和WebSocket服务，支持实时视频流和目标检测结果推送
 */
class WebServer {
public:
    /**
     * @brief 构造函数
     * @param video_capture 视频捕获对象
     */
    explicit WebServer(std::shared_ptr<CaptureInterface> video_capture);
    
    /**
     * @brief 析构函数
     */
    ~WebServer();
    
    /**
     * @brief 启动服务器
     * @param port 监听端口
     */
    void start(int port = 8080);
    
    /**
     * @brief 停止服务器
     */
    void stop();

private:
    /**
     * @class WebSocketHandler
     * @brief WebSocket请求处理器
     */
    class WebSocketHandler : public Poco::Net::HTTPRequestHandler {
    public:
        /**
         * @brief 构造函数
         * @param capture 视频捕获对象
         * @param processor 图像处理器
         */
        WebSocketHandler(std::shared_ptr<CaptureInterface> capture,
                        std::shared_ptr<ImageProcessor> processor);
        
        /**
         * @brief 处理HTTP/WebSocket请求
         */
        void handleRequest(Poco::Net::HTTPServerRequest& request,
                         Poco::Net::HTTPServerResponse& response) override;
    private:
        /**
         * @brief 处理WebSocket连接
         */
        void handleWebSocket(Poco::Net::WebSocket& ws);
        
        std::shared_ptr<CaptureInterface> video_capture_;  ///< 视频捕获对象
        std::shared_ptr<ImageProcessor> processor_;        ///< 图像处理器
    };

    /**
     * @class HandlerFactory
     * @brief 请求处理器工厂类
     */
    class HandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
    public:
        /**
         * @brief 构造函数
         */
        HandlerFactory(std::shared_ptr<CaptureInterface> capture,
                      std::shared_ptr<ImageProcessor> processor);
        
        /**
         * @brief 创建请求处理器
         */
        Poco::Net::HTTPRequestHandler* createRequestHandler(
            const Poco::Net::HTTPServerRequest& request) override;
    private:
        std::shared_ptr<CaptureInterface> video_capture_;  ///< 视频捕获对象
        std::shared_ptr<ImageProcessor> processor_;        ///< 图像处理器
    };

    std::shared_ptr<CaptureInterface> video_capture_;      ///< 视频捕获对象
    std::shared_ptr<ImageProcessor> processor_;            ///< 图像处理器
    std::unique_ptr<Poco::Net::HTTPServer> server_;       ///< HTTP服务器
    std::atomic<bool> running_{false};                    ///< 运行状态标志
}; 