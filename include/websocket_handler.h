#pragma once
#include <Poco/Net/WebSocket.h>
#include <Poco/JSON/Object.h>
#include <memory>
#include <queue>
#include "capture_interface.h"
#include "image_processor.h"

class WebSocketHandler {
public:
    WebSocketHandler(std::shared_ptr<CaptureInterface> capture,
                    std::shared_ptr<ImageProcessor> processor);
    
    void handleConnection(Poco::Net::WebSocket& ws);
    void sendFrame(const std::string& frame_data,
                  const std::vector<DetectionResult>& detections);
    void handleCommand(const std::string& command);

private:
    std::shared_ptr<CaptureInterface> capture_;
    std::shared_ptr<ImageProcessor> processor_;
    std::queue<std::string> frame_queue_;
    static constexpr size_t MAX_QUEUE_SIZE = 2;
}; 