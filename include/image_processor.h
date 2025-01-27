#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

struct DetectionResult {
    std::string label;
    float confidence;
    cv::Rect bbox;
};

class ImageProcessor {
public:
    ImageProcessor();
    
    std::vector<DetectionResult> processFrame(const cv::Mat& frame);
    void setDetectionEnabled(bool enabled) { detection_enabled_ = enabled; }
    void setConfidenceThreshold(float threshold) { confidence_threshold_ = threshold; }

private:
    cv::CascadeClassifier face_cascade_;
    cv::dnn::Net net_;
    bool detection_enabled_{true};
    float confidence_threshold_{0.5f};
}; 