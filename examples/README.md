# 测试示例程序

本目录包含了各个模块的独立测试程序。

## WebSocket测试 (test_websocket)

测试Poco的WebSocket功能：
```bash
# 编译
cd build/examples/test_websocket
make

# 运行
./test_websocket
```

然后打开浏览器访问 http://localhost:8080 进行测试。

## V4L2测试 (test_v4l2)

测试V4L2摄像头捕获功能：
```bash
# 编译
cd build/examples/test_v4l2
make

# 运行
./test_v4l2
```

## ONNX Runtime测试 (test_onnx)

测试ONNX Runtime推理功能：
```bash
# 编译
cd build/examples/test_onnx
make

# 运行
./test_onnx
``` 