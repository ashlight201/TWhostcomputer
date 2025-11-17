#!/bin/bash

# 依赖检查脚本

echo "=========================================="
echo "电机关节力矩标定系统 - 依赖检查"
echo "=========================================="
echo ""

# 检查CMake
echo "1. 检查CMake..."
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "   ✓ CMake已安装: $CMAKE_VERSION"
    CMAKE_MAJOR=$(echo $CMAKE_VERSION | cut -d'.' -f1)
    CMAKE_MINOR=$(echo $CMAKE_VERSION | cut -d'.' -f2)
    if [ "$CMAKE_MAJOR" -lt 3 ] || ([ "$CMAKE_MAJOR" -eq 3 ] && [ "$CMAKE_MINOR" -lt 16 ]); then
        echo "   ✗ CMake版本过低，需要3.16或更高版本"
    else
        echo "   ✓ CMake版本符合要求"
    fi
else
    echo "   ✗ CMake未安装"
    echo "   安装方法: sudo apt-get install cmake"
fi
echo ""

# 检查C++编译器
echo "2. 检查C++编译器..."
if command -v g++ &> /dev/null; then
    GPP_VERSION=$(g++ --version | head -n1)
    echo "   ✓ g++已安装: $GPP_VERSION"
else
    echo "   ✗ g++未安装"
    echo "   安装方法: sudo apt-get install build-essential"
fi
echo ""

# 检查Qt5
echo "3. 检查Qt5..."
QT5_FOUND=false
if command -v qmake &> /dev/null; then
    QT_VERSION=$(qmake -v 2>&1 | grep -oP 'Qt version \K[0-9.]+' | head -n1)
    if [ ! -z "$QT_VERSION" ]; then
        echo "   ✓ Qt5已安装: 版本 $QT_VERSION"
        QT5_FOUND=true
        
        # 检查Qt5组件
        if pkg-config --exists Qt5Core 2>/dev/null; then
            echo "   ✓ Qt5Core已安装"
        else
            echo "   ⚠ Qt5Core未找到（可能通过其他方式安装）"
        fi
        
        if pkg-config --exists Qt5Widgets 2>/dev/null; then
            echo "   ✓ Qt5Widgets已安装"
        else
            echo "   ⚠ Qt5Widgets未找到"
            echo "   安装方法: sudo apt-get install qtbase5-dev"
        fi
        
        if pkg-config --exists Qt5SerialPort 2>/dev/null; then
            echo "   ✓ Qt5SerialPort已安装"
        else
            echo "   ✗ Qt5SerialPort未找到"
            echo "   安装方法: sudo apt-get install libqt5serialport5-dev"
        fi
        
        if pkg-config --exists Qt5Charts 2>/dev/null; then
            echo "   ✓ Qt5Charts已安装"
        else
            echo "   ✗ Qt5Charts未找到"
            echo "   安装方法: sudo apt-get install qtcharts5-dev"
        fi
    fi
fi

if [ "$QT5_FOUND" = false ]; then
    echo "   ⚠ Qt5未找到，检查Qt6..."
    
    # 检查Qt6
    if command -v qmake6 &> /dev/null; then
        QT6_VERSION=$(qmake6 -v 2>&1 | grep -oP 'Qt version \K[0-9.]+' | head -n1)
        if [ ! -z "$QT6_VERSION" ]; then
            echo "   ✓ Qt6已安装: 版本 $QT6_VERSION"
            QT6_FOUND=true
        fi
    fi
    
    if [ "$QT6_FOUND" != true ]; then
        echo "   ✗ 未找到Qt5或Qt6"
        echo "   安装方法:"
        echo "   - Qt5: sudo apt-get install qt5-default qtbase5-dev libqt5serialport5-dev qtcharts5-dev"
        echo "   - Qt6: 通过Qt安装器安装，或 sudo apt-get install qt6-base-dev qt6-serialport-dev qt6-charts-dev"
    fi
fi
echo ""

# 检查串口权限
echo "4. 检查串口权限..."
if [ -d "/dev" ]; then
    if groups | grep -q dialout; then
        echo "   ✓ 用户已在dialout组中"
    else
        echo "   ⚠ 用户不在dialout组中，可能无法访问串口"
        echo "   解决方法: sudo usermod -a -G dialout $USER"
        echo "   然后重新登录"
    fi
else
    echo "   ⚠ 无法检查串口权限（非Linux系统）"
fi
echo ""

# 总结
echo "=========================================="
echo "检查完成"
echo "=========================================="
echo ""
echo "如果所有依赖都已安装，可以运行:"
echo "  ./build.sh"
echo ""
