# wxWidgets 摄像头测试程序

这是一个使用 wxWidgets 开发的跨平台摄像头查看器。

## 依赖项

在 Ubuntu 24.04 上需要安装以下依赖：

```bash
# 安装编译工具
sudo apt-get update
sudo apt-get install -y build-essential cmake

# 安装wxWidgets开发库
sudo apt-get install -y libwxgtk3.0-gtk3-dev

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
cd examples/test_wx
mkdir build
cd build
cmake ..
make
```

## 运行

```bash
./test_wx
```

## 功能说明

- 实时摄像头画面显示
- 原生系统界面风格
- 自动适应窗口大小
- 高效的图像刷新机制

## 故障排除

1. 编译错误：
   - 确认wxWidgets版本：`wx-config --version`
   - 检查开发库安装：`dpkg -l | grep wxgtk`
   - 可能需要安装额外组件：`sudo apt-get install libwxbase3.0-dev`

2. 运行时错误：
   - 检查摄像头权限
   - 验证OpenCV安装：`pkg-config --libs opencv4`
   - 检查显示服务器：`echo $DISPLAY`

3. 图像显示问题：
   - 检查色彩空间转换
   - 验证图像数据格式
   - 确认内存分配

## 注意事项

- 使用事件驱动的刷新机制
- 自动管理摄像头资源
- 支持窗口大小调整
- 程序关闭时自动清理资源

## 开发说明

- 使用wxPanel作为绘图表面
- 采用定时器触发画面更新
- 实现了基本的错误处理
- 支持自定义扩展 