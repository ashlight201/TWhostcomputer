#!/bin/bash

echo "=========================================="
echo "  Qt 环境检查工具"
echo "=========================================="
echo ""

# 检查 qmake
echo "1. 检查 qmake..."
if command -v qmake &> /dev/null; then
    QMAKE_VERSION=$(qmake -v)
    echo "✓ 找到 qmake:"
    echo "$QMAKE_VERSION"
    
    # 获取 Qt 路径
    QT_PATH=$(qmake -query QT_INSTALL_PREFIX)
    echo "Qt 安装路径: $QT_PATH"
else
    echo "✗ 未找到 qmake"
    echo ""
    echo "请安装 Qt 开发库:"
    echo "  Ubuntu/Debian: sudo apt-get install qtbase5-dev libqt5serialport5-dev libqt5charts5-dev"
    echo "  或 Qt6: sudo apt-get install qt6-base-dev qt6-serialport-dev qt6-charts-dev"
fi

echo ""
echo "2. 检查 Qt Charts 库..."

# 检查 Qt5 Charts
if [ -d "/usr/include/qt5/QtCharts" ] || [ -d "/usr/include/x86_64-linux-gnu/qt5/QtCharts" ]; then
    echo "✓ 找到 Qt5 Charts 头文件"
elif [ -d "/usr/include/qt6/QtCharts" ] || [ -d "/usr/include/x86_64-linux-gnu/qt6/QtCharts" ]; then
    echo "✓ 找到 Qt6 Charts 头文件"
else
    echo "✗ 未找到 Qt Charts 头文件"
    echo ""
    echo "请安装 Qt Charts 模块:"
    echo "  Ubuntu/Debian (Qt5): sudo apt-get install libqt5charts5-dev"
    echo "  Ubuntu/Debian (Qt6): sudo apt-get install qt6-charts-dev"
    echo "  Fedora (Qt5): sudo dnf install qt5-qtcharts-devel"
    echo "  Fedora (Qt6): sudo dnf install qt6-qtcharts-devel"
fi

echo ""
echo "3. 检查 CMake..."
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n 1)
    echo "✓ 找到 CMake: $CMAKE_VERSION"
else
    echo "✗ 未找到 CMake"
    echo "请安装: sudo apt-get install cmake"
fi

echo ""
echo "=========================================="
echo "  环境检查完成"
echo "=========================================="
