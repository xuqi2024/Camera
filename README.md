# 视频流处理系统

这是一个基于OpenCV和ONNX Runtime的视频流处理系统，支持实时视频捕获、目标检测和远程查看。

## 项目结构

```
.
├── CMakeLists.txt              # 主CMake配置文件
├── include/                    # 头文件目录
├── src/                        # 源代码目录
├── examples/                   # 测试示例程序
├── models/                     # 模型文件目录
├── prebuild/                   # 预编译库目录
└── 3rdparty/                  # 第三方库源码
```

## 功能特性

- 实时视频捕获（支持V4L2）
- YOLO目标检测
- WebSocket远程查看
- GUI本地查看
- 支持x86_64和aarch64架构

## 构建说明

### 依赖项

```bash
# 安装基本构建工具
sudo apt-get update
sudo apt-get install -y build-essential cmake

# 安装OpenCV
sudo apt-get install -y libopencv-dev

# 安装Poco库
sudo apt-get install -y libpoco-dev

# 安装GUI依赖（可选）
sudo apt-get install -y qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
```

### 编译

```bash
# 创建构建目录
mkdir build && cd build

# 配置
cmake ..

# 编译
make -j$(nproc)
```

## 使用说明

1. 启动主程序：
```bash
./bin/video_streaming_app
```

2. 运行测试程序：
```bash
# WebSocket测试
./bin/test_websocket

# GUI测试
./bin/test_gui
```

## 开发指南

详细的开发文档请参考各目录下的README文件：
- [examples/README.md](examples/README.md) - 测试程序说明
- [include/README.md](include/README.md) - API文档
- [src/README.md](src/README.md) - 实现说明

## 故障排除

常见问题及解决方法请参考 [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

## 许可证

[License信息]