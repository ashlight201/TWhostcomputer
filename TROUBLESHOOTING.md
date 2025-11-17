# 问题排查指南

## 常见编译问题及解决方案

### 1. Qt Charts 找不到或编译错误

**问题现象：**
```
CMake Error: Could not find Qt5Charts (or Qt6Charts)
```

**解决方案：**

#### 方案A：安装Qt Charts模块
- **Ubuntu/Debian**: `sudo apt-get install qtcharts5-dev` (Qt5) 或 `sudo apt-get install qt6-charts-dev` (Qt6)
- **Windows**: 在Qt安装器中选择安装Charts组件
- **macOS**: `brew install qt@5` 或通过Qt安装器安装

#### 方案B：修改CMakeLists.txt，使Charts可选
如果不需要图表功能，可以修改代码使其可选。

### 2. 找不到串口相关头文件

**问题现象：**
```
error: 'QSerialPort' was not declared
```

**解决方案：**
- 确保安装了Qt SerialPort模块
- **Ubuntu/Debian**: `sudo apt-get install libqt5serialport5-dev` (Qt5) 或 `sudo apt-get install qt6-serialport-dev` (Qt6)
- **Windows/macOS**: 通过Qt安装器安装SerialPort组件

### 3. CMake找不到Qt

**问题现象：**
```
CMake Error: Could not find Qt5 (or Qt6)
```

**解决方案：**
```bash
# 设置Qt路径（根据实际安装路径调整）
export Qt5_DIR=/path/to/qt5/lib/cmake/Qt5
# 或
export Qt6_DIR=/path/to/qt6/lib/cmake/Qt6

# 然后重新运行cmake
cd build
cmake ..
```

### 4. 编译时出现C++17相关错误

**问题现象：**
```
error: 'auto' keyword not supported
```

**解决方案：**
- 确保使用支持C++17的编译器（GCC 7+, Clang 5+, MSVC 2017+）
- 检查CMakeLists.txt中的C++标准设置

### 5. 链接错误：找不到Qt库

**问题现象：**
```
undefined reference to `QApplication::QApplication(int&, char**, int)'
```

**解决方案：**
```bash
# 确保Qt库在系统路径中
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/path/to/qt/lib

# 或在CMakeLists.txt中设置
set(CMAKE_PREFIX_PATH "/path/to/qt")
```

### 6. Qt Charts命名空间问题

**问题现象：**
```
error: 'QChart' was not declared in this scope
```

**解决方案：**
修改 `ChartWidget.h`，确保正确使用命名空间：

```cpp
// Qt 5
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE

// Qt 6
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
using namespace QtCharts;
```

### 7. MOC（Meta-Object Compiler）错误

**问题现象：**
```
moc_MainWindow.cpp: error: 'MainWindow' has no member named 'qt_metacast'
```

**解决方案：**
- 确保所有包含Q_OBJECT宏的头文件都在CMakeLists.txt的HEADERS列表中
- 确保设置了 `CMAKE_AUTOMOC ON`

## 运行时问题

### 1. 程序启动后立即崩溃

**可能原因：**
- Qt库版本不匹配
- 缺少必要的Qt插件

**解决方案：**
```bash
# 检查Qt库版本
ldd ./TorqueCalibration | grep Qt

# 设置Qt插件路径
export QT_PLUGIN_PATH=/path/to/qt/plugins
```

### 2. 串口无法打开

**可能原因：**
- 权限不足（Linux）
- 串口被其他程序占用
- 串口名称不正确

**解决方案：**
```bash
# Linux: 添加用户到dialout组
sudo usermod -a -G dialout $USER
# 然后重新登录

# 检查串口是否被占用
lsof /dev/ttyUSB0  # 或 /dev/ttyACM0

# 列出可用串口
ls -l /dev/tty* | grep -E "USB|ACM"
```

### 3. 图表不显示或显示异常

**可能原因：**
- Qt Charts模块未正确链接
- OpenGL支持问题

**解决方案：**
```bash
# 检查是否安装了OpenGL
sudo apt-get install libgl1-mesa-dev

# 如果使用Qt6，可能需要
sudo apt-get install qt6-base-dev
```

### 4. 数据不更新或更新缓慢

**可能原因：**
- 数据处理函数过于复杂
- 图表更新频率过高

**解决方案：**
- 优化数据处理函数
- 调整ChartWidget中的更新定时器间隔（默认100ms）

## 调试技巧

### 1. 启用详细编译输出
```bash
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
make VERBOSE=1
```

### 2. 检查Qt版本
```bash
qmake --version  # Qt5
qmake6 --version  # Qt6
```

### 3. 使用Qt Creator
- 在Qt Creator中打开CMakeLists.txt
- 配置Kit（选择正确的Qt版本）
- 直接编译和调试

### 4. 检查依赖库
```bash
# Linux
ldd ./TorqueCalibration

# macOS
otool -L ./TorqueCalibration
```

## 快速修复脚本

如果遇到常见问题，可以尝试运行以下检查：

```bash
#!/bin/bash
echo "检查Qt安装..."
qmake --version 2>/dev/null || qmake6 --version 2>/dev/null || echo "Qt未找到"

echo "检查Qt Charts..."
pkg-config --exists Qt5Charts 2>/dev/null && echo "Qt5 Charts已安装" || \
pkg-config --exists Qt6Charts 2>/dev/null && echo "Qt6 Charts已安装" || \
echo "Qt Charts未找到"

echo "检查Qt SerialPort..."
pkg-config --exists Qt5SerialPort 2>/dev/null && echo "Qt5 SerialPort已安装" || \
pkg-config --exists Qt6SerialPort 2>/dev/null && echo "Qt6 SerialPort已安装" || \
echo "Qt SerialPort未找到"

echo "检查编译器..."
g++ --version | head -1
```

## 如果问题仍未解决

1. 检查完整的错误信息
2. 确认Qt版本和安装路径
3. 检查CMake输出信息
4. 查看系统日志（`journalctl` 或 `dmesg`）

请提供具体的错误信息，以便进一步诊断问题。
