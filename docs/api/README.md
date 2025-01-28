# API文档

## 核心类接口说明

### 1. CaptureInterface 类

视频捕获的抽象接口类，定义了与摄像头交互的基本操作。

#### 公共方法

##### `bool start(int device_id = 0)`
启动视频捕获。
- 参数：
  - device_id: 设备ID号，默认为0
- 返回值：
  - true: 启动成功
  - false: 启动失败

##### `void stop()`
停止视频捕获。

##### `std::string getLatestFrame()`
获取最新的视频帧。
- 返回值：JPEG格式的图像数据

### 2. V4L2Capture 类

Video4Linux2 (V4L2) API的具体实现类。

#### 公共方法

除继承自CaptureInterface的方法外，还包括：

##### `bool initDevice(int device_id)`
初始化视频设备。
- 参数：
  - device_id: 设备ID号
- 返回值：
  - true: 初始化成功
  - false: 初始化失败

### 3. ImageProcessor 类

图像处理和目标检测类。

#### 公共方法

##### `std::vector<DetectionResult> processFrame(const cv::Mat& frame)`
处理视频帧并进行目标检测。
- 参数：
  - frame: OpenCV格式的图像帧
- 返回值：
  - 检测结果数组

##### `void setDetectionEnabled(bool enabled)`
启用或禁用目标检测。
- 参数：
  - enabled: 是否启用检测

##### `void setConfidenceThreshold(float threshold)`
设置检测置信度阈值。
- 参数：
  - threshold: 置信度阈值 (0.0-1.0)

### 4. WebServer 类

Web服务器类，处理HTTP和WebSocket请求。

#### 公共方法

##### `void start(int port = 8080)`
启动Web服务器。
- 参数：
  - port: 监听端口号

##### `void stop()`
停止Web服务器。

## WebSocket API

### 连接

```
ws://localhost:8080/ws
```

### 消息格式

#### 1. 视频帧消息
```json
{
    "type": "frame",
    "data": "base64编码的JPEG数据"
}
```

#### 2. 检测结果消息
```json
{
    "type": "detections",
    "detections": [
        {
            "label": "person",
            "confidence": 0.95,
            "bbox": {
                "x": 100,
                "y": 200,
                "width": 50,
                "height": 100
            }
        }
    ]
}
```

#### 3. 配置消息
```json
{
    "type": "config",
    "detection_enabled": true,
    "confidence_threshold": 0.5
}
``` 