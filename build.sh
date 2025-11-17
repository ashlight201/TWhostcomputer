#!/bin/bash

# 电机力矩标定系统 - 构建脚本

echo "=========================================="
echo "  电机力矩标定系统 - 构建脚本"
echo "=========================================="
echo ""

# 检查Qt是否安装
if ! command -v qmake &> /dev/null; then
    echo "⚠️  警告: 未找到Qt安装，请确保已安装Qt开发库"
    echo ""
    echo "Ubuntu/Debian 安装命令:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install qtbase5-dev libqt5serialport5-dev libqt5charts5-dev"
    echo ""
    echo "或安装 Qt6:"
    echo "  sudo apt-get install qt6-base-dev qt6-serialport-dev qt6-charts-dev"
    echo ""
    read -p "是否继续尝试编译? (y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
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
cmake .. 2>&1 | tee cmake_output.log || {
    echo ""
    echo "❌ 错误: CMake配置失败"
    echo ""
    echo "常见问题排查:"
    echo "1. 检查是否安装了 Qt Charts 模块"
    echo "   运行: ./check_qt.sh"
    echo ""
    echo "2. 如果使用 Qt5，确保安装了 libqt5charts5-dev"
    echo "   sudo apt-get install libqt5charts5-dev"
    echo ""
    echo "3. 如果使用 Qt6，确保安装了 qt6-charts-dev"
    echo "   sudo apt-get install qt6-charts-dev"
    echo ""
    echo "详细错误信息已保存到: build/cmake_output.log"
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
