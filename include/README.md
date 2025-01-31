# API文档

## 头文件说明

### video_capture.h
```cpp
class VideoCapture {
    // 视频捕获类
    // 负责摄像头初始化和帧捕获
};
```

### image_processor.h
```cpp
class ImageProcessor {
    // 图像处理类
    // 实现目标检测和图像处理
};
```

### web_server.h
```cpp
class WebServer {
    // WebSocket服务器类
    // 处理远程连接和数据传输
};
```

### gui_viewer.h
```cpp
class GUIViewer {
    // GUI显示类
    // 实现本地显示界面
};
```

## 使用示例

```cpp
// 初始化视频捕获
VideoCapture capture;
capture.open(0);

// 创建图像处理器
ImageProcessor processor;
processor.loadModel("models/yolo.onnx");

// 处理帧
cv::Mat frame;
capture.read(frame);
processor.detect(frame);
``` 