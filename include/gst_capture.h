#pragma once
#include "capture_interface.h"
#include <gst/gst.h>
#include <thread>
#include <mutex>
#include <atomic>

class GstCapture : public CaptureInterface {
public:
    GstCapture();
    ~GstCapture() override;
    
    bool start(int device_id = 0) override;
    void stop() override;
    std::string getLatestFrame() override;

private:
    static void newSampleCallback(GstElement* sink, GstCapture* self);
    
    GstElement* pipeline_{nullptr};
    GstElement* appsink_{nullptr};
    
    std::mutex frame_mutex_;
    std::atomic<bool> running_{false};
    std::string latest_frame_;
}; 