# FLTK 摄像头测试程序

这是一个使用 FLTK (Fast Light Toolkit) 开发的轻量级摄像头查看器。

## 依赖项

在 Ubuntu 24.04 上需要安装以下依赖：

```bash
# 安装编译工具
sudo apt-get update
sudo apt-get install -y build-essential cmake

# 安装FLTK开发库
sudo apt-get install -y libfltk1.3-dev

# 安装OpenCV
sudo apt-get install -y libopencv-dev
```

## 编译

```bash
# 在项目根目录下
mkdir -p build
cd build
cmake ..
make

# 或者直接编译测试程序
cd examples/test_fltk
mkdir build
cd build
cmake ..
make
```

## 运行

```bash
./test_fltk
```

## 功能说明

- 实时显示摄像头画面
- 极简界面设计
- 低资源占用
- 30FPS刷新率

## 故障排除

1. 如果出现 "无法打开摄像头" 错误：
   - 检查摄像头连接
   - 验证设备权限
   - 运行 `v4l2-ctl --list-devices` 查看可用设备

2. 如果出现 FLTK 相关错误：
   - 检查FLTK安装：`fltk-config --version`
   - 重新安装FLTK：`sudo apt-get install --reinstall libfltk1.3-dev`

3. 如果程序启动但没有显示图像：
   - 检查OpenCV安装：`pkg-config --modversion opencv4`
   - 确认摄像头支持的分辨率：`v4l2-ctl --list-formats-ext`

## 注意事项

- 程序使用固定的640x480分辨率
- 自动选择系统默认摄像头
- 关闭窗口时自动释放资源 