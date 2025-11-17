# 故障排除指南

## 问题：找不到 QtCharts 头文件

### 错误信息
```
QtCharts/QChartView: No such file or directory
'QtCharts/QChartView' file not found
unknown type name 'QT_CHARTS_USE_NAMESPACE'
```

### 原因
系统中没有安装 Qt Charts 开发库。

### 解决方案

#### 方案1：安装 Qt Charts 模块（推荐）

##### Ubuntu/Debian 系统

**安装 Qt5 版本（推荐，更稳定）：**
```bash
sudo apt-get update
sudo apt-get install -y qtbase5-dev libqt5serialport5-dev libqt5charts5-dev cmake g++
```

**或安装 Qt6 版本：**
```bash
sudo apt-get update
sudo apt-get install -y qt6-base-dev qt6-serialport-dev qt6-charts-dev cmake g++
```

##### Fedora/RHEL 系统

**Qt5：**
```bash
sudo dnf install qt5-qtbase-devel qt5-qtserialport-devel qt5-qtcharts-devel cmake gcc-c++
```

**Qt6：**
```bash
sudo dnf install qt6-qtbase-devel qt6-qtserialport-devel qt6-qtcharts-devel cmake gcc-c++
```

##### Arch Linux

```bash
sudo pacman -S qt5-base qt5-serialport qt5-charts cmake gcc
# 或
sudo pacman -S qt6-base qt6-serialport qt6-charts cmake gcc
```

##### 验证安装

安装完成后，运行检查脚本：
```bash
./check_qt.sh
```

如果显示 "✓ 找到 Qt Charts 头文件"，说明安装成功。

#### 方案2：使用不带图表功能的精简版本

如果无法安装 Qt Charts 或暂时不需要图表功能，我可以为你创建一个不使用 Qt Charts 的精简版本。

精简版本功能：
- ✓ 双串口通信
- ✓ 数据接收和显示
- ✓ 数据处理
- ✓ 标定计算
- ✗ 实时图表（移除）

如需精简版本，请告诉我，我会立即创建。

---

## 编译步骤

### 1. 清理旧的编译文件（如果存在）
```bash
rm -rf build
```

### 2. 重新编译
```bash
./build.sh
```

或手动编译：
```bash
mkdir build && cd build
cmake ..
make -j4
```

### 3. 运行程序
```bash
./build/bin/MotorCalibration
```

---

## 其他常见问题

### 问题：串口权限不足

**错误信息：**
```
Permission denied
```

**解决方案：**
```bash
# 将当前用户添加到 dialout 组
sudo usermod -a -G dialout $USER

# 注销并重新登录，或执行：
newgrp dialout
```

### 问题：找不到 Qt 库

**错误信息：**
```
Could not find Qt6 or Qt5
```

**解决方案：**

1. 确认已安装 Qt：
```bash
qmake -v
```

2. 如果未安装，参考上面的安装命令

3. 如果已安装但 CMake 找不到，设置 Qt 路径：
```bash
# Qt5
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt5

# 或 Qt6
export CMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6

# 然后重新编译
./build.sh
```

### 问题：编译器版本过低

**错误信息：**
```
C++17 required
```

**解决方案：**

安装较新的编译器：
```bash
# Ubuntu/Debian
sudo apt-get install g++-9

# 设置为默认编译器
export CXX=g++-9

# 重新编译
./build.sh
```

### 问题：CMake 版本过低

**错误信息：**
```
CMake 3.16 or higher is required
```

**解决方案：**

从官方网站安装最新版 CMake：
```bash
# 下载并安装最新版本
wget https://github.com/Kitware/CMake/releases/download/v3.28.0/cmake-3.28.0-linux-x86_64.sh
chmod +x cmake-3.28.0-linux-x86_64.sh
sudo ./cmake-3.28.0-linux-x86_64.sh --prefix=/usr/local --skip-license

# 验证
cmake --version
```

---

## 快速参考

### 完整安装命令（一键复制）

**Ubuntu/Debian + Qt5（推荐）：**
```bash
sudo apt-get update && \
sudo apt-get install -y qtbase5-dev libqt5serialport5-dev libqt5charts5-dev cmake g++ && \
sudo usermod -a -G dialout $USER && \
echo "安装完成！请注销并重新登录以应用权限更改。"
```

**Ubuntu/Debian + Qt6：**
```bash
sudo apt-get update && \
sudo apt-get install -y qt6-base-dev qt6-serialport-dev qt6-charts-dev cmake g++ && \
sudo usermod -a -G dialout $USER && \
echo "安装完成！请注销并重新登录以应用权限更改。"
```

### 编译和运行命令

```bash
# 检查环境
./check_qt.sh

# 编译
./build.sh

# 运行
./build/bin/MotorCalibration
```

---

## 获取帮助

如果以上方案都无法解决问题：

1. 查看详细的编译日志：
```bash
cat build/cmake_output.log
```

2. 查看系统信息：
```bash
uname -a
lsb_release -a
qmake -v
cmake --version
g++ --version
```

3. 将以上信息和错误消息一起提供，以便进一步诊断。
