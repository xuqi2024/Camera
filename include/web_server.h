#pragma once
#include <crow.h>
#include "video_capture.h"
#include <memory>

class WebServer {
public:
    WebServer(std::shared_ptr<VideoCapture> video_capture);
    void start(int port = 8080);

private:
    crow::SimpleApp app_;
    std::shared_ptr<VideoCapture> video_capture_;
}; 