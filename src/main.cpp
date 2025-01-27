#include "video_capture.h"
#include "web_server.h"
#include <iostream>

int main() {
    auto video_capture = std::make_shared<VideoCapture>();
    
    if (!video_capture->start(0)) {
        std::cerr << "无法打开摄像头" << std::endl;
        return 1;
    }
    
    WebServer server(video_capture);
    std::cout << "服务器启动在 http://localhost:8080" << std::endl;
    server.start(8080);
    
    return 0;
} 