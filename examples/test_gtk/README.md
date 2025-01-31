# GTK+ 摄像头测试程序

这是一个使用 GTK+ 3.0 开发的简单摄像头查看器。

## 依赖项

在 Ubuntu 24.04 上需要安装以下依赖：

```bash
# 安装编译工具
sudo apt-get update
sudo apt-get install -y build-essential cmake

# 安装GTK+3开发库
sudo apt-get install -y libgtk-3-dev

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
cd examples/test_gtk
mkdir build
cd build
cmake ..
make
```

## 运行

```bash
./test_gtk
```

## 功能说明

- 实时显示摄像头画面
- 使用GTK+原生窗口
- 支持窗口缩放
- 使用单独线程处理视频捕获

## 故障排除

1. 如果出现 "无法打开摄像头" 错误：
   - 确认摄像头已正确连接
   - 检查设备权限：`ls -l /dev/video0`
   - 添加用户到video组：`sudo usermod -a -G video $USER`

2. 如果出现 GTK 相关错误：
   - 确认GTK+3已正确安装：`pkg-config --modversion gtk+-3.0`
   - 重新安装GTK+开发库：`sudo apt-get install --reinstall libgtk-3-dev`

## 注意事项

- 程序默认使用系统第一个摄像头（/dev/video0）
- 视频捕获使用30FPS的刷新率
- 程序退出时会自动释放摄像头资源 