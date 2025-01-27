#include "v4l2_capture.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <jpeglib.h>

V4L2Capture::V4L2Capture() = default;

V4L2Capture::~V4L2Capture() {
    stop();
}

bool V4L2Capture::initDevice(int device_id) {
    char dev_name[64];
    snprintf(dev_name, sizeof(dev_name), "/dev/video%d", device_id);
    
    fd_ = open(dev_name, O_RDWR);
    if (fd_ < 0) return false;

    struct v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    
    if (ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) return false;

    struct v4l2_requestbuffers req = {};
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    
    if (ioctl(fd_, VIDIOC_REQBUFS, &req) < 0) return false;

    buf_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf_.memory = V4L2_MEMORY_MMAP;
    buf_.index = 0;
    
    if (ioctl(fd_, VIDIOC_QUERYBUF, &buf_) < 0) return false;

    buffer_ = mmap(nullptr, buf_.length, PROT_READ | PROT_WRITE,
                  MAP_SHARED, fd_, buf_.m.offset);
    buffer_size_ = buf_.length;
    
    return buffer_ != MAP_FAILED;
}

bool V4L2Capture::start(int device_id) {
    if (!initDevice(device_id)) return false;
    
    running_ = true;
    capture_thread_ = std::thread(&V4L2Capture::captureLoop, this);
    return true;
}

void V4L2Capture::stop() {
    running_ = false;
    if (capture_thread_.joinable()) {
        capture_thread_.join();
    }
    
    if (buffer_) {
        munmap(buffer_, buffer_size_);
        buffer_ = nullptr;
    }
    
    if (fd_ >= 0) {
        close(fd_);
        fd_ = -1;
    }
}

void V4L2Capture::captureLoop() {
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd_, VIDIOC_STREAMON, &type);
    
    while (running_) {
        if (ioctl(fd_, VIDIOC_QBUF, &buf_) < 0) continue;
        if (ioctl(fd_, VIDIOC_DQBUF, &buf_) < 0) continue;
        
        // 转换YUYV到JPEG
        // 这里需要添加YUYV到JPEG的转换代码
        std::string jpeg_data = convertYUYVtoJPEG(static_cast<uint8_t*>(buffer_), 
                                                640, 480);
        
        {
            std::lock_guard<std::mutex> lock(frame_mutex_);
            latest_frame_ = std::move(jpeg_data);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
    
    ioctl(fd_, VIDIOC_STREAMOFF, &type);
}

std::string V4L2Capture::getLatestFrame() {
    std::lock_guard<std::mutex> lock(frame_mutex_);
    return latest_frame_;
} 