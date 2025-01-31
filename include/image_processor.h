/**
 * @file image_processor.h
 * @brief 图像处理和目标检测类的定义
 * @details 使用YOLOv8和ONNX Runtime实现目标检测
 */

#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <onnxruntime/onnxruntime_cxx_api.h>

/**
 * @struct DetectionResult
 * @brief 目标检测结果结构
 */
struct DetectionResult {
    std::string label;      ///< 目标类别标签
    float confidence;       ///< 检测置信度
    cv::Rect bbox;         ///< 边界框坐标
};

/**
 * @class ImageProcessor
 * @brief 图像处理和目标检测类
 * @details 使用YOLOv8模型进行目标检测，支持实时处理视频帧
 */
class ImageProcessor {
public:
    /**
     * @brief 构造函数
     * @details 初始化ONNX Runtime环境和加载模型
     */
    ImageProcessor();
    
    /**
     * @brief 处理单帧图像
     * @param frame OpenCV格式的输入图像
     * @return 检测结果数组
     */
    std::vector<DetectionResult> processFrame(const cv::Mat& frame);
    
    /**
     * @brief 设置是否启用检测
     * @param enabled 启用状态
     */
    void setDetectionEnabled(bool enabled) { detection_enabled_ = enabled; }
    
    /**
     * @brief 设置检测置信度阈值
     * @param threshold 阈值值(0.0-1.0)
     */
    void setConfidenceThreshold(float threshold) { confidence_threshold_ = threshold; }

private:
    /**
     * @brief 加载模型和配置
     */
    void loadModel();
    
    /**
     * @brief 图像预处理
     * @param frame 输入图像
     * @return 预处理后的张量数据
     */
    std::vector<float> preprocess(const cv::Mat& frame);
    
    std::unique_ptr<Ort::Session> session_;    ///< ONNX会话对象
    std::unique_ptr<Ort::Env> env_;           ///< ONNX运行环境
    std::vector<std::string> class_names_;    ///< 类别名称列表
    std::vector<const char*> input_names_;    ///< 模型输入节点名称
    std::vector<const char*> output_names_;   ///< 模型输出节点名称
    bool detection_enabled_{true};            ///< 检测启用状态
    float confidence_threshold_{0.5f};        ///< 置信度阈值
    const int input_width_{640};              ///< 模型输入宽度
    const int input_height_{640};             ///< 模型输入高度
}; 