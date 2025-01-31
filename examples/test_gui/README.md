# Qt 摄像头测试程序

这是一个使用 Qt 开发的功能完整的摄像头查看器。

## 依赖项

在 Ubuntu 24.04 上需要安装以下依赖：

```bash
# 安装编译工具
sudo apt-get update
sudo apt-get install -y build-essential cmake

# 安装Qt开发库
sudo apt-get install -y qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools

# 安装OpenCV
sudo apt-get install -y libopencv-dev
```

如果需要设置Qt5为默认版本：
```bash
# 设置Qt5为默认版本
mkdir -p ~/.config/qtchooser
echo "/usr/lib/qt5/bin" > ~/.config/qtchooser/default.conf
echo "/usr/lib/x86_64-linux-gnu" >> ~/.config/qtchooser/default.conf

# 或者直接使用环境变量（推荐）
export QT_SELECT=qt5

# 验证Qt版本
qmake --version
```

## 编译

```bash
# 在项目根目录下
mkdir -p build
cd build
cmake ..
make

# 或者直接编译测试程序
cd examples/test_gui
mkdir build
cd build
cmake ..
make
```

## 运行

```bash
./test_gui
```

## 功能说明

- 实时视频显示
- FPS计数器
- 目标检测开关
- 置信度调节
- 截图功能

## 界面布局

- 主视频显示区域
- 控制面板
  - 检测开关
  - 置信度滑块
  - 截图按钮
- FPS显示

## 故障排除

1. Qt相关问题：
   - 检查Qt安装：`qmake --version`
   - 确认环境变量：`echo $LD_LIBRARY_PATH`
   - 可能需要安装：`sudo apt-get install qtbase5-dev-tools`

2. 编译问题：
   - 确认CMake版本：`cmake --version`
   - 检查Qt模块：`pkg-config --list-all | grep Qt`
   - 验证moc工具：`which moc`

3. 运行问题：
   - 检查摄像头权限
   - 验证OpenCV：`pkg-config --libs opencv4`
   - 确认Qt插件：`ls /usr/lib/x86_64-linux-gnu/qt5/plugins`

## 性能优化

- 使用Qt事件循环
- 优化图像转换
- 减少内存拷贝
- 使用定时器控制帧率

## 注意事项

- 默认使用系统第一个摄像头
- 图像格式为RGB
- 自动释放资源
- 支持窗口调整

## 开发扩展

- 可添加录制功能
- 支持多摄像头
- 可集成其他算法
- 支持网络传输 