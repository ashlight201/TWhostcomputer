#!/bin/bash

# 电机关节力矩标定系统构建脚本

set -e

echo "开始构建项目..."

# 创建构建目录
if [ ! -d "build" ]; then
    mkdir build
fi

cd build

# 运行CMake配置
echo "运行CMake配置..."
cmake ..

# 编译项目
echo "编译项目..."
make -j$(nproc)

echo "构建完成！"
echo "可执行文件位置: build/bin/TorqueCalibrationApp"
