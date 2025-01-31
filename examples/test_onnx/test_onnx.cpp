/**
 * @file test_onnx.cpp
 * @brief ONNX Runtime推理测试程序
 */
#include <onnxruntime/core/session/onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <chrono>

class ONNXTest {
public:
    bool init(const std::string& model_path) {
        try {
            // 初始化ONNX Runtime环境
            env_ = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "ONNXTest");
            
            // 配置会话选项
            Ort::SessionOptions session_options;
            session_options.SetIntraOpNumThreads(1);
            session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);

            // 加载模型
            session_ = Ort::Session(env_, model_path.c_str(), session_options);

            // 获取输入输出信息
            Ort::AllocatorWithDefaultOptions allocator;
            
            // 获取输入节点名称
            auto input_name = session_.GetInputNameAllocated(0, allocator);
            std::cout << "输入节点名称: " << input_name.get() << std::endl;

            // 获取输入形状
            auto input_shape = session_.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
            std::cout << "输入形状: ";
            for (auto dim : input_shape) std::cout << dim << " ";
            std::cout << std::endl;

            // 获取输出节点名称
            auto output_name = session_.GetOutputNameAllocated(0, allocator);
            std::cout << "输出节点名称: " << output_name.get() << std::endl;

            return true;
        } catch (const Ort::Exception& e) {
            std::cerr << "ONNX Runtime错误: " << e.what() << std::endl;
            return false;
        }
    }

    bool runInference(const cv::Mat& input_image) {
        try {
            // 预处理输入图像
            cv::Mat processed;
            cv::resize(input_image, processed, cv::Size(640, 640));
            processed.convertTo(processed, CV_32F, 1.0/255.0);

            // 准备输入数据
            std::vector<float> input_tensor_values(640*640*3);
            float* input_ptr = input_tensor_values.data();
            
            // BGR转CHW格式
            for (int c = 0; c < 3; c++) {
                for (int h = 0; h < 640; h++) {
                    for (int w = 0; w < 640; w++) {
                        input_ptr[c * 640 * 640 + h * 640 + w] =
                            processed.at<cv::Vec3f>(h, w)[c];
                    }
                }
            }

            // 创建输入tensor
            std::vector<int64_t> input_shape = {1, 3, 640, 640};
            auto memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
            
            Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
                memory_info,
                input_tensor_values.data(),
                input_tensor_values.size(),
                input_shape.data(),
                input_shape.size()
            );

            // 运行推理
            auto start = std::chrono::high_resolution_clock::now();
            
            auto output_tensors = session_.Run(
                Ort::RunOptions{nullptr},
                &input_name,
                &input_tensor,
                1,
                &output_name,
                1
            );
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            
            std::cout << "推理时间: " << duration.count() << "ms" << std::endl;

            // 处理输出
            float* output = output_tensors.front().GetTensorMutableData<float>();
            auto output_shape = output_tensors.front().GetTensorTypeAndShapeInfo().GetShape();
            
            std::cout << "输出形状: ";
            for (auto dim : output_shape) std::cout << dim << " ";
            std::cout << std::endl;

            return true;
        } catch (const Ort::Exception& e) {
            std::cerr << "推理错误: " << e.what() << std::endl;
            return false;
        }
    }

private:
    Ort::Env env_{nullptr};
    Ort::Session session_{nullptr};
    const char* input_name;
    const char* output_name;
};

int main() {
    ONNXTest test;
    
    // 初始化模型
    if (!test.init("models/yolov11n.onnx")) {
        std::cerr << "模型初始化失败" << std::endl;
        return 1;
    }

    // 读取测试图像
    cv::Mat image = cv::imread("test.jpg");
    if (image.empty()) {
        std::cerr << "无法读取测试图像" << std::endl;
        return 1;
    }

    // 运行推理
    if (!test.runInference(image)) {
        std::cerr << "推理失败" << std::endl;
        return 1;
    }

    return 0;
} 