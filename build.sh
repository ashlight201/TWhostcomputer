#!/bin/bash

# 电机力矩标定系统 - 构建脚本

set -e  # 遇到错误立即退出

echo "=========================================="
echo "  电机力矩标定系统 - 构建脚本"
echo "=========================================="
echo ""

# 检查Qt是否安装
if ! command -v qmake &> /dev/null; then
    echo "警告: 未找到Qt安装，请确保已安装Qt开发库"
    echo ""
    echo "Ubuntu/Debian 安装命令:"
    echo "  sudo apt-get install qt6-base-dev qt6-serialport-dev qt6-charts-dev"
    echo "或:"
    echo "  sudo apt-get install qtbase5-dev libqt5serialport5-dev libqt5charts5-dev"
    echo ""
fi

# 创建并进入build目录
if [ -d "build" ]; then
    echo "清理已存在的build目录..."
    rm -rf build
fi

echo "创建build目录..."
mkdir -p build
cd build

# 配置项目
echo ""
echo "配置项目..."
cmake .. || {
    echo ""
    echo "错误: CMake配置失败"
    echo "请检查是否已正确安装Qt和CMake"
    exit 1
}

# 编译项目
echo ""
echo "编译项目..."
make -j$(nproc) || {
    echo ""
    echo "错误: 编译失败"
    exit 1
}

echo ""
echo "=========================================="
echo "  构建成功！"
echo "=========================================="
echo ""
echo "可执行文件位置: build/bin/MotorCalibration"
echo ""
echo "运行程序:"
echo "  ./bin/MotorCalibration"
echo ""
echo "或者在项目根目录运行:"
echo "  ./build/bin/MotorCalibration"
echo ""
