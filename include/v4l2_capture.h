#pragma once
#include "capture_interface.h"
#include <linux/videodev2.h>
#include <thread>
#include <mutex>
#include <atomic>

class V4L2Capture : public CaptureInterface {
public:
    V4L2Capture();
    ~V4L2Capture() override;
    
    bool start(int device_id = 0) override;
    void stop() override;
    std::string getLatestFrame() override;

private:
    void captureLoop();
    bool initDevice(int device_id);
    
    int fd_{-1};
    struct v4l2_buffer buf_{};
    void* buffer_{nullptr};
    size_t buffer_size_{0};
    
    std::thread capture_thread_;
    std::mutex frame_mutex_;
    std::atomic<bool> running_{false};
    std::string latest_frame_;
}; 