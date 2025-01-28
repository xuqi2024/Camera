# 智能视频监控系统

基于V4L2和YOLOv8的实时视频监控和目标检测系统。

## 功能特点

### 视频捕获
- 使用V4L2直接与摄像头设备交互
- 支持YUYV格式视频流
- 实时JPEG编码和压缩
- 高效的内存映射(MMAP)实现

### 目标检测
- 集成YOLOv8目标检测模型
- ONNX Runtime高性能推理
- 支持80种常见物体识别
- 可调节的检测置信度阈值

### Web界面
- 基于WebSocket的实时视频流
- 响应式设计的控制面板
- 实时检测结果显示
- FPS和性能监控
- 支持截图和视频录制

## 系统要求

### 硬件要求
- 支持V4L2的USB摄像头
- x86_64或ARM处理器
- 至少2GB内存

### 软件要求
- Ubuntu 20.04或更高版本
- C++17兼容编译器
- CMake 3.10+
- OpenCV 4.x
- ONNX Runtime
- Poco库

## 快速开始

1. 安装系统依赖：

```
sudo apt-get update
sudo apt-get install -y build-essential cmake
sudo apt-get install -y libopencv-dev
```

2. 克隆仓库：

```bash
git clone https://github.com/yourusername/video-surveillance.git
cd video-surveillance
```

3. 下载模型：

```bash
chmod +x download_models.sh
./download_models.sh
```

4. 编译项目：

```bash
chmod +x build.sh
./build.sh
```

5. 运行程序：

```bash
./build/bin/video_streaming_app
```

6. 打开浏览器访问：

```
http://localhost:8080
```

## 实现说明

### 视频捕获模块
- 轻量级V4L2实现
- 直接设备访问
- 高效内存管理
- 实时格式转换

### 目标检测模块
- YOLOv8神经网络
- ONNX Runtime优化
- 异步处理设计
- 可配置检测参数

### Web服务器模块
- Poco库实现
- WebSocket实时传输
- 二进制帧推送
- JSON结果通信

## 详细文档

- [架构设计](docs/architecture/README.md)
- [API文档](docs/api/README.md)
- [安装指南](docs/setup/README.md)
- [使用示例](docs/examples/README.md)
- [开发指南](docs/development/README.md)
- [性能优化](docs/optimization/README.md)

## 贡献指南

1. Fork 项目
2. 创建特性分支
3. 提交更改
4. 推送到分支
5. 创建Pull Request

## 许可证

MIT License

## 作者

[你的名字]

## 致谢

- OpenCV团队
- Ultralytics YOLOv8
- Poco项目
- 所有贡献者