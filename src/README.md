# 源代码说明

## 文件结构

- `main.cpp` - 程序入口点
- `video_capture.cpp` - 视频捕获模块
- `image_processor.cpp` - 图像处理和目标检测
- `web_server.cpp` - WebSocket服务器
- `gui_viewer.cpp` - GUI显示界面

## 模块说明

### 视频捕获模块 (video_capture.cpp)
处理摄像头输入，支持V4L2接口。主要功能：
- 摄像头初始化
- 帧捕获
- 格式转换
- 错误处理

### 图像处理模块 (image_processor.cpp)
实现图像处理和目标检测。主要功能：
- YOLO模型加载
- 目标检测
- 后处理
- 性能优化

### WebSocket服务器 (web_server.cpp)
提供远程访问功能。主要功能：
- WebSocket连接管理
- 图像数据传输
- 并发处理
- 错误恢复

### GUI显示模块 (gui_viewer.cpp)
本地显示界面。主要功能：
- 实时视频显示
- 检测结果可视化
- 用户交互
- 性能监控 