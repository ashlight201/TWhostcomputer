# 快速问题解决指南

## 如果您遇到编译错误

### 问题1: "找不到Qt Charts"

**快速解决：**
```bash
# Ubuntu/Debian
sudo apt-get install qtcharts5-dev        # Qt5
# 或
sudo apt-get install qt6-charts-dev       # Qt6

# 然后重新编译
cd build
cmake ..
make
```

### 问题2: "找不到Qt SerialPort"

**快速解决：**
```bash
# Ubuntu/Debian
sudo apt-get install libqt5serialport5-dev    # Qt5
# 或
sudo apt-get install qt6-serialport-dev       # Qt6

# 然后重新编译
cd build
cmake ..
make
```

### 问题3: "CMake找不到Qt"

**快速解决：**
```bash
# 查找Qt安装路径
find /usr -name "Qt5Config.cmake" 2>/dev/null
# 或
find /usr -name "Qt6Config.cmake" 2>/dev/null

# 设置Qt路径（替换为实际路径）
export Qt5_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt5
# 或
export Qt6_DIR=/usr/lib/x86_64-linux-gnu/cmake/Qt6

# 然后重新运行cmake
cd build
cmake ..
make
```

### 问题4: "权限被拒绝"（串口无法打开）

**快速解决：**
```bash
# 添加用户到dialout组
sudo usermod -a -G dialout $USER

# 重新登录或执行
newgrp dialout

# 验证
groups | grep dialout
```

## 一键安装所有依赖（Ubuntu/Debian）

```bash
# Qt5版本
sudo apt-get update
sudo apt-get install -y \
    cmake \
    build-essential \
    qtbase5-dev \
    libqt5serialport5-dev \
    qtcharts5-dev

# 或者Qt6版本
sudo apt-get update
sudo apt-get install -y \
    cmake \
    build-essential \
    qt6-base-dev \
    qt6-serialport-dev \
    qt6-charts-dev

# 添加串口权限
sudo usermod -a -G dialout $USER
```

## 运行诊断脚本

```bash
# 检查所有依赖
./check_dependencies.sh

# 如果依赖都正常，编译项目
./build.sh
```

## 如果仍然有问题

1. **查看完整错误信息**
   ```bash
   cd build
   cmake .. 2>&1 | tee cmake_output.log
   make 2>&1 | tee make_output.log
   ```

2. **检查Qt版本**
   ```bash
   qmake --version    # Qt5
   qmake6 --version   # Qt6
   ```

3. **手动指定Qt路径**
   编辑 `CMakeLists.txt`，在开头添加：
   ```cmake
   set(CMAKE_PREFIX_PATH "/path/to/qt")
   ```

4. **使用Qt Creator**
   - 打开Qt Creator
   - 文件 -> 打开文件或项目 -> 选择 `CMakeLists.txt`
   - 配置Kit（选择正确的Qt版本）
   - 点击构建

## 常见错误代码对照

| 错误信息 | 解决方案 |
|---------|---------|
| `Could not find Qt5Charts` | 安装 `qtcharts5-dev` |
| `Could not find Qt6Charts` | 安装 `qt6-charts-dev` |
| `QSerialPort: No such file` | 安装 `libqt5serialport5-dev` 或 `qt6-serialport-dev` |
| `Permission denied` (串口) | 运行 `sudo usermod -a -G dialout $USER` |
| `undefined reference to QApplication` | 检查Qt库是否正确链接 |
| `moc_*.cpp` 错误 | 确保头文件在CMakeLists.txt的HEADERS列表中 |

## 最小化版本（无图表）

如果不需要图表功能，可以修改代码使其可选。但这需要修改多个文件，建议直接安装Qt Charts模块。
