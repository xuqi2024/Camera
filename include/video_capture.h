#pragma once
#include <string>
#include <thread>
#include <mutex>
#include <atomic>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
}

class VideoCapture {
public:
    VideoCapture();
    ~VideoCapture();
    
    bool start(int device_id = 0);
    void stop();
    std::string getLatestFrame(); // 返回编码后的帧数据

private:
    void captureLoop();
    
    AVFormatContext* format_ctx_{nullptr};
    AVCodecContext* codec_ctx_{nullptr};
    SwsContext* sws_ctx_{nullptr};
    int video_stream_index_{-1};
    
    std::thread capture_thread_;
    std::mutex frame_mutex_;
    std::atomic<bool> running_{false};
    std::string latest_frame_;
}; 