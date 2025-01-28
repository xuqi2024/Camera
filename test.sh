#!/bin/bash

# 定义颜色代码，用于输出着色
RED='\033[0;31m'      # 错误信息使用红色
GREEN='\033[0;32m'    # 成功信息使用绿色
NC='\033[0m'          # 恢复默认颜色

# 测试摄像头设备是否可访问
# 返回值：0-成功，1-失败
test_camera() {
    if [ -e /dev/video0 ]; then
        echo -e "${GREEN}摄像头设备存在${NC}"
    else
        echo -e "${RED}未找到摄像头设备${NC}"
        return 1
    fi
}

# 测试ONNX Runtime是否正确安装
# 返回值：0-成功，1-失败
test_onnx() {
    if [ -f "/usr/local/lib/libonnxruntime.so" ]; then
        echo -e "${GREEN}ONNX Runtime已安装${NC}"
    else
        echo -e "${RED}ONNX Runtime未安装${NC}"
        return 1
    fi
}

# 主测试流程
echo "开始测试..."

# 运行所有测试项
test_camera
test_onnx

# 检查测试结果
if [ $? -eq 0 ]; then
    echo -e "${GREEN}所有测试通过${NC}"
else
    echo -e "${RED}测试失败${NC}"
    exit 1
fi 