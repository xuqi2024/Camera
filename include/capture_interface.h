#pragma once
#include <string>

class CaptureInterface {
public:
    virtual ~CaptureInterface() = default;
    virtual bool start(int device_id = 0) = 0;
    virtual void stop() = 0;
    virtual std::string getLatestFrame() = 0;
}; 