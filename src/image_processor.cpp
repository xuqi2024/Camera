/**
 * @file image_processor.cpp
 * @brief 图像处理和目标检测类的实现
 */

#include "image_processor.h"
#include <fstream>
#include <iostream>
#include <numeric>

ImageProcessor::ImageProcessor() {
    try {
        loadModel();
    } catch (const std::exception& e) {
        std::cerr << "模型加载失败: " << e.what() << std::endl;
        detection_enabled_ = false;
    }
}

void ImageProcessor::loadModel() {
    // 检查模型文件
    const std::string model_path = "models/yolov11n.onnx";
    const std::string names_path = "models/coco.names";

    std::ifstream model_file(model_path, std::ios::binary);
    std::ifstream names_file(names_path);
    if (!model_file.good() || !names_file.good()) {
        throw std::runtime_error("找不到模型文件，请先运行download_models.sh下载所需文件");
    }

    try {
        // 初始化ONNX Runtime环境
        env_ = std::make_unique<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "YOLOv11");
        
        // 配置会话选项
        Ort::SessionOptions session_options;
        session_options.SetIntraOpNumThreads(1);
        session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

        // 加载模型
        session_ = std::make_unique<Ort::Session>(*env_, model_path.c_str(), session_options);

        // 获取输入输出节点名称
        Ort::AllocatorWithDefaultOptions allocator;
        input_names_.push_back(session_->GetInputName(0, allocator));
        output_names_.push_back(session_->GetOutputName(0, allocator));

        // 加载类别名称
        std::string line;
        while (std::getline(names_file, line)) {
            if (!line.empty() && line[0] != '#') {
                size_t comment_pos = line.find('#');
                if (comment_pos != std::string::npos) {
                    line = line.substr(0, comment_pos);
                }
                line.erase(0, line.find_first_not_of(" \t"));
                line.erase(line.find_last_not_of(" \t") + 1);
                if (!line.empty()) {
                    class_names_.push_back(line);
                }
            }
        }

        if (class_names_.empty()) {
            throw std::runtime_error("未能加载任何类别名称");
        }
    } catch (const Ort::Exception& e) {
        throw std::runtime_error("ONNX Runtime错误: " + std::string(e.what()));
    }
}

std::vector<float> ImageProcessor::preprocess(const cv::Mat& frame) {
    // 调整图像大小
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(input_width_, input_height_));
    
    // 转换为浮点型并归一化
    cv::Mat float_img;
    resized.convertTo(float_img, CV_32F, 1.0/255.0);
    
    // 准备输入张量数据
    std::vector<float> input_tensor(input_width_ * input_height_ * 3);
    float* input_ptr = input_tensor.data();
    
    // BGR转CHW格式
    for (int c = 0; c < 3; c++) {
        for (int h = 0; h < input_height_; h++) {
            for (int w = 0; w < input_width_; w++) {
                input_ptr[c * input_width_ * input_height_ + h * input_width_ + w] =
                    float_img.at<cv::Vec3f>(h, w)[c];
            }
        }
    }
    
    return input_tensor;
}

std::vector<DetectionResult> ImageProcessor::processFrame(const cv::Mat& frame) {
    std::vector<DetectionResult> results;
    if (!detection_enabled_ || frame.empty()) return results;

    try {
        // 1. 图像预处理
        auto input_tensor_values = preprocess(frame);
        std::vector<int64_t> input_shape = {1, 3, input_height_, input_width_};
        
        // 2. 准备输入tensor
        auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info, input_tensor_values.data(), input_tensor_values.size(),
            input_shape.data(), input_shape.size());

        // 3. 执行推理
        auto output_tensors = session_->Run(
            Ort::RunOptions{nullptr},
            input_names_.data(),
            &input_tensor,
            1,
            output_names_.data(),
            1);

        // 4. 处理输出 - 更新为YOLOv11n的输出格式
        float* output = output_tensors.front().GetTensorMutableData<float>();
        const auto& output_shape = output_tensors.front().GetTensorTypeAndShapeInfo().GetShape();
        const int num_boxes = output_shape[1];
        const int num_classes = output_shape[2] - 4;  // 减去4个边界框坐标

        // 5. 解析检测结果
        for (int i = 0; i < num_boxes; ++i) {
            float* box_data = output + i * (num_classes + 4);
            float confidence = box_data[4];
            
            if (confidence >= confidence_threshold_) {
                int class_id = std::max_element(box_data + 5, box_data + 5 + num_classes) - (box_data + 5);
                float class_score = box_data[5 + class_id];
                
                if (class_score * confidence >= confidence_threshold_) {
                    DetectionResult det;
                    det.label = class_names_[class_id];
                    det.confidence = class_score * confidence;
                    det.bbox = cv::Rect(
                        static_cast<int>((box_data[0] - box_data[2] / 2) * frame.cols),
                        static_cast<int>((box_data[1] - box_data[3] / 2) * frame.rows),
                        static_cast<int>(box_data[2] * frame.cols),
                        static_cast<int>(box_data[3] * frame.rows)
                    );
                    results.push_back(det);
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "处理帧时发生错误: " << e.what() << std::endl;
    }

    return results;
} 