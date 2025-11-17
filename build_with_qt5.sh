#!/bin/bash

# 使用Qt 5.12构建的脚本

set -e

echo "=========================================="
echo "使用Qt 5.12构建项目"
echo "=========================================="
echo ""

# 如果用户提供了Qt路径作为参数
if [ ! -z "$1" ]; then
    QT_PATH="$1"
    echo "使用指定的Qt路径: $QT_PATH"
    export CMAKE_PREFIX_PATH="$QT_PATH:$CMAKE_PREFIX_PATH"
    export QT5_DIR="$QT_PATH"
elif [ ! -z "$QT5_DIR" ]; then
    echo "使用环境变量QT5_DIR: $QT5_DIR"
    export CMAKE_PREFIX_PATH="$QT5_DIR:$CMAKE_PREFIX_PATH"
elif [ ! -z "$CMAKE_PREFIX_PATH" ]; then
    echo "使用环境变量CMAKE_PREFIX_PATH: $CMAKE_PREFIX_PATH"
else
    echo "尝试自动查找Qt 5.12..."
    ./find_qt5.sh
    echo ""
    echo "如果自动查找失败，请提供Qt路径:"
    echo "  ./build_with_qt5.sh /path/to/qt5/lib/cmake/Qt5"
    echo ""
    read -p "按Enter继续尝试构建，或Ctrl+C取消..." 
fi

echo ""
echo "开始构建..."
echo ""

# 创建构建目录
mkdir -p build
cd build

# 运行CMake
if [ ! -z "$QT_PATH" ] || [ ! -z "$QT5_DIR" ] || [ ! -z "$CMAKE_PREFIX_PATH" ]; then
    cmake .. -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"
else
    cmake ..
fi

# 编译
cmake --build . -j$(nproc)

echo ""
echo "=========================================="
echo "构建完成！"
echo "=========================================="
echo ""
echo "可执行文件: build/TorqueCalibration"
echo ""
