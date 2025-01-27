#include "v4l2_capture.h"
#include "gst_capture.h"
#include "ffmpeg_capture.h"
#include "web_server.h"
#include <memory>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <backend>\n"
                  << "backend: v4l2, gst, ffmpeg\n";
        return 1;
    }
    
    std::shared_ptr<CaptureInterface> capture;
    std::string backend = argv[1];
    
    if (backend == "v4l2") {
        capture = std::make_shared<V4L2Capture>();
    } else if (backend == "gst") {
        capture = std::make_shared<GstCapture>();
    } else if (backend == "ffmpeg") {
        capture = std::make_shared<FFmpegCapture>();
    } else {
        std::cerr << "Unknown backend: " << backend << std::endl;
        return 1;
    }
    
    if (!capture->start(0)) {
        std::cerr << "Failed to start capture" << std::endl;
        return 1;
    }
    
    WebServer server(capture);
    std::cout << "Server running at http://localhost:8080" << std::endl;
    server.start(8080);
    
    return 0;
} 