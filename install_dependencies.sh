#!/bin/bash

# Qt依赖安装脚本

set -e

echo "=========================================="
echo "安装Qt开发库依赖"
echo "=========================================="
echo ""

# 检测系统类型
if [ -f /etc/debian_version ]; then
    echo "检测到Debian/Ubuntu系统"
    
    # 更新包列表
    echo "更新包列表..."
    sudo apt-get update
    
    # 询问用户选择Qt版本
    echo ""
    echo "请选择要安装的Qt版本:"
    echo "1) Qt5 (推荐，更稳定)"
    echo "2) Qt6 (最新版本)"
    read -p "请输入选择 (1 或 2): " qt_choice
    
    if [ "$qt_choice" = "1" ] || [ -z "$qt_choice" ]; then
        echo ""
        echo "安装Qt5开发库..."
        sudo apt-get install -y \
            cmake \
            build-essential \
            qtbase5-dev \
            libqt5serialport5-dev \
            qtcharts5-dev \
            qttools5-dev \
            qttools5-dev-tools
        echo ""
        echo "✓ Qt5安装完成"
    elif [ "$qt_choice" = "2" ]; then
        echo ""
        echo "安装Qt6开发库..."
        sudo apt-get install -y \
            cmake \
            build-essential \
            qt6-base-dev \
            qt6-serialport-dev \
            qt6-charts-dev \
            qt6-tools-dev \
            qt6-tools-dev-tools
        echo ""
        echo "✓ Qt6安装完成"
    else
        echo "无效的选择"
        exit 1
    fi
    
    # 添加串口权限
    echo ""
    echo "配置串口权限..."
    if ! groups | grep -q dialout; then
        sudo usermod -a -G dialout $USER
        echo "✓ 已将用户添加到dialout组"
        echo "⚠ 请重新登录或运行 'newgrp dialout' 使权限生效"
    else
        echo "✓ 用户已在dialout组中"
    fi
    
elif [ -f /etc/redhat-release ]; then
    echo "检测到RedHat/CentOS系统"
    echo "安装Qt5开发库..."
    sudo yum install -y \
        cmake \
        gcc-c++ \
        qt5-qtbase-devel \
        qt5-qtserialport-devel \
        qt5-qtcharts-devel
    echo "✓ Qt5安装完成"
    
elif [[ "$OSTYPE" == "darwin"* ]]; then
    echo "检测到macOS系统"
    echo "请使用Homebrew安装Qt:"
    echo "  brew install qt@5"
    echo "或通过Qt安装器安装: https://www.qt.io/download"
    
else
    echo "未识别的系统，请手动安装Qt开发库"
fi

echo ""
echo "=========================================="
echo "安装完成！"
echo "=========================================="
echo ""
echo "现在可以运行:"
echo "  ./build.sh"
echo ""
