# Qt 5.12 配置指南

## 快速开始

### 方法1: 自动查找（推荐）

```bash
# 运行查找脚本
./find_qt5.sh

# 如果找到了Qt，脚本会显示如何设置环境变量
# 然后运行构建脚本
./build_with_qt5.sh
```

### 方法2: 手动指定Qt路径

如果您知道Qt 5.12的安装路径：

```bash
# 设置环境变量
export QT5_DIR="/path/to/qt5/lib/cmake/Qt5"
export CMAKE_PREFIX_PATH="/path/to/qt5/lib/cmake/Qt5:$CMAKE_PREFIX_PATH"

# 然后构建
./build.sh
```

或使用专用脚本：

```bash
./build_with_qt5.sh /path/to/qt5/lib/cmake/Qt5
```

### 方法3: 在CMake命令中指定

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_PREFIX_PATH="/path/to/qt5/lib/cmake/Qt5"
make
```

## 常见Qt 5.12安装路径

- `/opt/Qt/5.12.x/lib/cmake/Qt5` (Linux, 通过Qt安装器)
- `/usr/lib/x86_64-linux-gnu/cmake/Qt5` (Ubuntu/Debian包管理器)
- `~/Qt/5.12.x/lib/cmake/Qt5` (用户目录)
- `/usr/local/Qt/5.12.x/lib/cmake/Qt5` (手动编译安装)

## 验证Qt 5.12安装

### 检查qmake

```bash
# 如果qmake在PATH中
qmake --version

# 应该显示类似:
# QMake version 3.1
# Using Qt version 5.12.x in /path/to/qt5
```

### 检查CMake配置

```bash
# 查找Qt5Config.cmake
find / -name "Qt5Config.cmake" 2>/dev/null
```

### 检查Qt组件

确保以下组件已安装：
- Qt5Core
- Qt5Widgets  
- Qt5SerialPort
- Qt5Charts

```bash
# 如果通过包管理器安装
dpkg -l | grep qt5

# 应该看到:
# qtbase5-dev
# libqt5serialport5-dev
# qtcharts5-dev
```

## 如果Qt 5.12未安装

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    qtbase5-dev \
    libqt5serialport5-dev \
    qtcharts5-dev
```

### 通过Qt安装器

1. 下载Qt 5.12安装器: https://www.qt.io/download
2. 安装到 `/opt/Qt/` 或 `~/Qt/`
3. 确保选择以下组件:
   - Qt 5.12.x
   - Qt Charts
   - Qt Serial Port

## 环境变量设置（永久）

将以下内容添加到 `~/.bashrc` 或 `~/.profile`:

```bash
# Qt 5.12配置
export QT5_DIR="/path/to/qt5/lib/cmake/Qt5"
export CMAKE_PREFIX_PATH="$QT5_DIR:$CMAKE_PREFIX_PATH"
export PATH="/path/to/qt5/bin:$PATH"
```

然后重新加载:
```bash
source ~/.bashrc
```

## 故障排查

### 问题1: CMake找不到Qt5

**解决方案:**
```bash
# 运行查找脚本
./find_qt5.sh

# 根据输出设置环境变量
export QT5_DIR="<显示的路径>"
```

### 问题2: 找不到Qt Charts

**检查:**
```bash
ls /path/to/qt5/lib/cmake/Qt5Charts/
```

**如果不存在，需要安装Qt Charts模块**

### 问题3: 找不到Qt SerialPort

**检查:**
```bash
ls /path/to/qt5/lib/cmake/Qt5SerialPort/
```

## 测试配置

运行以下命令测试Qt配置是否正确:

```bash
cd build
rm -rf *
cmake .. -DCMAKE_PREFIX_PATH="/path/to/qt5/lib/cmake/Qt5"
```

如果看到:
```
-- 找到Qt5: 5.12.x
-- 使用Qt版本: 5
```

说明配置成功！
