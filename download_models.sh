#!/bin/bash

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

# 创建模型目录
mkdir -p models

# 下载并验证模型文件
download_model() {
    local url="$1"
    local output="$2"
    local max_retries=3
    local retry_count=0
    
    while [ $retry_count -lt $max_retries ]; do
        echo "下载模型文件 (尝试 $((retry_count + 1))/$max_retries)..."
        if wget --no-verbose --show-progress -c -O "${output}.tmp" "$url"; then
            # 移动临时文件到目标位置
            mv "${output}.tmp" "$output"
            return 0
        fi
        
        retry_count=$((retry_count + 1))
        if [ $retry_count -lt $max_retries ]; then
            echo "下载失败，等待 5 秒后重试..."
            sleep 5
        fi
    done
    
    return 1
}

# 下载并转换YOLOv11n模型
download_yolov11() {
    echo "正在下载YOLOv11n模型..."
    
    # 检查ONNX模型是否已存在
    if [ -f "models/yolov11n.onnx" ]; then
        echo -e "${GREEN}ONNX模型已存在，跳过下载和转换${NC}"
        return 0
    fi
    
    # 检查PT模型是否已存在
    if [ ! -f "models/yolov11n.pt" ]; then
        # 下载PT模型
        if ! download_model \
            "https://github.com/ultralytics/assets/releases/download/v8.3.0/yolo11n.pt" \
            "models/yolov11n.pt"; then
            echo -e "${RED}错误: YOLOv11n PT模型下载失败${NC}"
            return 1
        fi
    else
        echo "PT模型已存在，跳过下载"
    fi
    
    echo "正在转换模型为ONNX格式..."
    
    # 检查是否已存在虚拟环境
    if [ ! -d "venv" ]; then
        echo "创建新的虚拟环境..."
        python3 -m venv venv
    else
        echo "使用已存在的虚拟环境"
    fi
    
    # 激活虚拟环境
    source venv/bin/activate
    
    # 安装所需包
    echo "安装必要的Python包..."
    python3 -m pip install --upgrade pip
    python3 -m pip install wheel setuptools  
    # 安装特定版本的依赖
    python3 -m pip install onnxruntime==1.17.0

    python3 -m pip install ultralytics onnx
    python3 -m pip install onnxslim
    
    # 验证安装
    if ! python3 -c "import ultralytics; import onnxruntime; import onnx" 2>/dev/null; then
        echo -e "${RED}错误: Python包安装失败${NC}"
        deactivate
        return 1
    fi
    
    # 转换模型
    python3 -c "
from ultralytics import YOLO
# 加载并导出模型
model = YOLO('models/yolov11n.pt')
success = model.export(format='onnx', dynamic=True, simplify=False, opset=11)
if not success:
    raise RuntimeError('Model export failed')
"
    
    # 检查转换结果
    if [ -f "yolov11n.onnx" ]; then
        mv yolov11n.onnx models/yolov11n.onnx
        echo -e "${GREEN}模型转换成功${NC}"
        rm -f models/yolov11n.pt
        deactivate
        return 0
    else
        echo -e "${RED}错误: 模型转换失败${NC}"
        deactivate
        return 1
    fi
}

# 下载COCO类别名称
download_coco_names() {
    echo "正在下载COCO类别名称..."
    cat > models/coco.names << 'EOF'
person
bicycle
car
motorcycle
airplane
bus
train
truck
boat
traffic light
fire hydrant
stop sign
parking meter
bench
bird
cat
dog
horse
sheep
cow
elephant
bear
zebra
giraffe
backpack
umbrella
handbag
tie
suitcase
frisbee
skis
snowboard
sports ball
kite
baseball bat
baseball glove
skateboard
surfboard
tennis racket
bottle
wine glass
cup
fork
knife
spoon
bowl
banana
apple
sandwich
orange
broccoli
carrot
hot dog
pizza
donut
cake
chair
couch
potted plant
bed
dining table
toilet
tv
laptop
mouse
remote
keyboard
cell phone
microwave
oven
toaster
sink
refrigerator
book
clock
vase
scissors
teddy bear
hair drier
toothbrush
EOF
    
    if [ -f models/coco.names ]; then
        echo -e "${GREEN}COCO类别名称创建成功${NC}"
        return 0
    else
        echo -e "${RED}错误: COCO类别名称创建失败${NC}"
        return 1
    fi
}

# 安装依赖
install_dependencies() {
    echo "正在检查并安装依赖..."
    
    # 检查是否有sudo权限
    if ! command -v sudo &> /dev/null; then
        echo -e "${RED}错误: 需要sudo权限来安装依赖${NC}"
        return 1
    fi
    
    # 更新包列表
    sudo apt-get update
    
    # 安装必要的包
    PACKAGES=(
        "python3"
        "python3-pip"
        "python3-venv"
        "wget"
    )
    
    for pkg in "${PACKAGES[@]}"; do
        if ! dpkg -l | grep -q "^ii  $pkg "; then
            echo "正在安装 $pkg..."
            sudo apt-get install -y "$pkg"
            if [ $? -ne 0 ]; then
                echo -e "${RED}错误: 安装 $pkg 失败${NC}"
                return 1
            fi
        fi
    done
    
    echo -e "${GREEN}所有依赖安装完成${NC}"
    return 0
}

# 检查Python3和pip是否安装
check_dependencies() {
    local missing_deps=0
    
    if ! command -v python3 &> /dev/null; then
        echo -e "${RED}错误: 未找到python3${NC}"
        missing_deps=1
    fi
    
    if ! command -v pip &> /dev/null; then
        echo -e "${RED}错误: 未找到pip${NC}"
        missing_deps=1
    fi
    
    if ! command -v python3 -m venv &> /dev/null; then
        echo -e "${RED}错误: 未找到python3-venv${NC}"
        missing_deps=1
    fi
    
    if [ $missing_deps -eq 1 ]; then
        echo "是否要安装缺失的依赖? [Y/n]"
        read -r response
        if [[ "$response" =~ ^([yY][eE][sS]|[yY]|)$ ]]; then
            install_dependencies
            return $?
        else
            return 1
        fi
    fi
    
    return 0
}

# 主流程
echo "开始下载模型文件..."

# 检查并安装依赖
check_dependencies
if [ $? -ne 0 ]; then
    echo -e "${RED}错误: 依赖检查失败，请手动安装所需依赖${NC}"
    echo "需要安装的包："
    echo "  sudo apt-get install python3 python3-pip python3-venv wget"
    exit 1
fi

# 下载并转换YOLOv11n模型
download_yolov11
if [ $? -ne 0 ]; then
    exit 1
fi

# 下载COCO类别名称
download_coco_names
if [ $? -ne 0 ]; then
    exit 1
fi

echo -e "${GREEN}所有模型文件下载完成${NC}" 