#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

print_step() {
    echo -e "${GREEN}==== $1 ====${NC}"
}

print_error() {
    echo -e "${RED}错误: $1${NC}"
}

# 设置变量
PACKAGE_NAME="video_streaming_app"
VERSION="1.0.0"
PLATFORM="linux-x86_64"
PACKAGE_DIR="${PACKAGE_NAME}-${VERSION}-${PLATFORM}"

# 创建打包目录
print_step "创建打包目录"
rm -rf ${PACKAGE_DIR}
mkdir -p ${PACKAGE_DIR}/{bin,lib,models,config}

# 复制可执行文件
print_step "复制可执行文件"
cp build/bin/video_streaming_app ${PACKAGE_DIR}/bin/
cp build/bin/test_* ${PACKAGE_DIR}/bin/ 2>/dev/null || true

# 复制模型文件
print_step "复制模型文件"
cp -r models/* ${PACKAGE_DIR}/models/

# 复制依赖库
print_step "复制依赖库"
cp -r prebuild/${PLATFORM}/lib/* ${PACKAGE_DIR}/lib/

# 创建运行脚本
cat > ${PACKAGE_DIR}/run.sh << 'EOF'
#!/bin/bash
# 设置环境变量
export LD_LIBRARY_PATH="$(dirname "$0")/lib:$LD_LIBRARY_PATH"
# 运行程序
"$(dirname "$0")/bin/video_streaming_app" "$@"
EOF

# 创建测试脚本
cat > ${PACKAGE_DIR}/test.sh << 'EOF'
#!/bin/bash
# 设置环境变量
export LD_LIBRARY_PATH="$(dirname "$0")/lib:$LD_LIBRARY_PATH"
# 运行测试程序
for test in $(dirname "$0")/bin/test_*; do
    if [ -x "$test" ]; then
        echo "运行测试: $(basename "$test")"
        "$test"
    fi
done
EOF

# 添加执行权限
chmod +x ${PACKAGE_DIR}/run.sh
chmod +x ${PACKAGE_DIR}/test.sh

# 创建README
cat > ${PACKAGE_DIR}/README.md << 'EOF'
# 视频流处理系统

## 运行方法
1. 确保摄像头已连接
2. 运行 ./run.sh 启动程序
3. 打开浏览器访问 http://localhost:8080

## 测试程序
运行 ./test.sh 执行测试程序

## 目录结构
- bin/: 可执行文件
- lib/: 依赖库
- models/: 模型文件
- config/: 配置文件

## 注意事项
- 确保摄像头权限正确
- 程序默认使用8080端口
EOF

# 打包
print_step "创建压缩包"
tar -czf ${PACKAGE_DIR}.tar.gz ${PACKAGE_DIR}

# 清理
print_step "清理临时文件"
rm -rf ${PACKAGE_DIR}

print_step "打包完成"
echo "生成的文件: ${PACKAGE_DIR}.tar.gz"
echo "使用方法:"
echo "1. 解压: tar -xzf ${PACKAGE_DIR}.tar.gz"
echo "2. 进入目录: cd ${PACKAGE_DIR}"
echo "3. 运行程序: ./run.sh" 