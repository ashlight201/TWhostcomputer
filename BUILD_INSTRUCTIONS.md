# 编译说明

## 已解决的问题

✅ **编译器链接问题** - 已修复，现在使用g++而不是clang

## 当前状态

系统需要安装Qt开发库才能编译。请按照以下步骤操作：

## 快速开始

### 1. 安装依赖（Ubuntu/Debian）

```bash
# 运行自动安装脚本
./install_dependencies.sh

# 或手动安装Qt5
sudo apt-get update
sudo apt-get install -y \
    cmake \
    build-essential \
    qtbase5-dev \
    libqt5serialport5-dev \
    qtcharts5-dev

# 配置串口权限
sudo usermod -a -G dialout $USER
# 然后重新登录或运行: newgrp dialout
```

### 2. 编译项目

```bash
./build.sh
```

或手动编译：

```bash
mkdir -p build
cd build
cmake ..
make
```

### 3. 运行程序

```bash
./build/TorqueCalibration
```

## 已修复的问题

### 问题1: 链接器找不到libstdc++

**原因**: 系统默认使用clang，但clang的链接器配置有问题

**解决方案**: 在CMakeLists.txt中强制使用g++编译器

```cmake
set(CMAKE_CXX_COMPILER g++)
```

### 问题2: Qt找不到

**原因**: 系统未安装Qt开发库

**解决方案**: 
- 运行 `./install_dependencies.sh` 安装Qt
- 或手动安装Qt5/Qt6开发包

## 验证安装

运行依赖检查脚本：

```bash
./check_dependencies.sh
```

应该看到所有依赖都已安装。

## 如果仍有问题

1. **检查Qt安装**:
   ```bash
   qmake --version  # Qt5
   qmake6 --version  # Qt6
   ```

2. **检查CMake输出**:
   ```bash
   cd build
   cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
   ```

3. **查看详细错误**:
   ```bash
   make VERBOSE=1
   ```

## 编译选项

### 指定Qt版本

```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt5
```

### 使用Qt Creator

1. 打开Qt Creator
2. 文件 -> 打开文件或项目
3. 选择 `CMakeLists.txt`
4. 配置Kit（选择Qt版本）
5. 点击构建

## 常见问题

### Q: 编译时提示找不到Qt Charts

A: 安装qtcharts5-dev:
```bash
sudo apt-get install qtcharts5-dev
```

### Q: 编译时提示找不到Qt SerialPort

A: 安装libqt5serialport5-dev:
```bash
sudo apt-get install libqt5serialport5-dev
```

### Q: 运行时提示权限被拒绝（串口）

A: 添加用户到dialout组:
```bash
sudo usermod -a -G dialout $USER
newgrp dialout
```
