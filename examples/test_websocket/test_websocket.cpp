/**
 * @file test_websocket.cpp
 * @brief WebSocket服务器测试程序
 */
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Net/ServerSocket.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace Poco::Net;
using namespace std::chrono_literals;

class WebSocketHandler : public HTTPRequestHandler {
public:
    void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) override {
        try {
            if (request.find("Upgrade") != request.end() 
                && Poco::icompare(request["Upgrade"], "websocket") == 0) {
                WebSocket ws(request, response);
                std::cout << "WebSocket connection established" << std::endl;
                
                // 发送测试数据
                const std::string testMessage = "Hello from WebSocket server!";
                ws.sendFrame(testMessage.data(), testMessage.size(), WebSocket::FRAME_TEXT);
                
                // 接收和回显数据
                char buffer[1024];
                int flags;
                int n;
                do {
                    n = ws.receiveFrame(buffer, sizeof(buffer), flags);
                    if (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE) {
                        ws.sendFrame(buffer, n, flags);
                    }
                } while (n > 0 && (flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE);
            } else {
                // 返回HTML页面
                response.setContentType("text/html");
                std::ostream& out = response.send();
                out << "<html><head><title>WebSocket Test</title></head>"
                    << "<body><h1>WebSocket Test Page</h1>"
                    << "<script>"
                    << "var ws = new WebSocket('ws://' + location.host + '/ws');"
                    << "ws.onmessage = function(evt) { console.log('Received: ' + evt.data); };"
                    << "ws.onopen = function() { ws.send('Hello from client!'); };"
                    << "</script></body></html>";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
};

class WebSocketFactory : public HTTPRequestHandlerFactory {
public:
    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest&) override {
        return new WebSocketHandler;
    }
};

int main() {
    try {
        // 创建服务器
        ServerSocket socket(8080);
        HTTPServer server(new WebSocketFactory, socket, new HTTPServerParams);
        
        // 启动服务器
        server.start();
        std::cout << "WebSocket server running on port 8080" << std::endl;
        
        // 等待用户输入以退出
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        
        // 停止服务器
        server.stop();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 