#!/bin/bash

# 查找Qt 5.12安装路径的脚本

echo "=========================================="
echo "查找Qt 5.12安装路径"
echo "=========================================="
echo ""

QT_FOUND=false

# 方法1: 查找Qt5Config.cmake
echo "1. 搜索Qt5Config.cmake文件..."
QT_CONFIG=$(find /usr /opt /home -name "Qt5Config.cmake" 2>/dev/null | head -1)
if [ ! -z "$QT_CONFIG" ]; then
    QT_DIR=$(dirname $(dirname "$QT_CONFIG"))
    echo "   找到: $QT_CONFIG"
    echo "   Qt安装目录: $QT_DIR"
    QT_FOUND=true
else
    echo "   未找到Qt5Config.cmake"
fi

echo ""

# 方法2: 查找qmake
echo "2. 搜索qmake..."
QMAKE_PATH=$(which qmake 2>/dev/null || find /usr /opt /home -name "qmake" -type f 2>/dev/null | head -1)
if [ ! -z "$QMAKE_PATH" ]; then
    echo "   找到qmake: $QMAKE_PATH"
    if [ -x "$QMAKE_PATH" ]; then
        QT_VERSION=$($QMAKE_PATH -v 2>&1 | grep -oP 'Qt version \K[0-9.]+' | head -1)
        if [ ! -z "$QT_VERSION" ]; then
            echo "   Qt版本: $QT_VERSION"
            # 尝试获取Qt安装路径
            QT_PREFIX=$($QMAKE_PATH -query QT_INSTALL_PREFIX 2>/dev/null)
            if [ ! -z "$QT_PREFIX" ]; then
                echo "   Qt安装路径: $QT_PREFIX"
                QT_DIR="$QT_PREFIX/lib/cmake/Qt5"
                if [ -d "$QT_DIR" ]; then
                    echo "   CMake配置路径: $QT_DIR"
                    QT_FOUND=true
                fi
            fi
        fi
    fi
else
    echo "   未找到qmake"
fi

echo ""

# 方法3: 查找常见的Qt安装目录
echo "3. 搜索常见安装目录..."
COMMON_PATHS=(
    "/opt/Qt/5.12"
    "/opt/Qt5.12"
    "$HOME/Qt/5.12"
    "$HOME/Qt5.12"
    "/usr/local/Qt/5.12"
    "/usr/local/Qt5.12"
    "/usr/lib/x86_64-linux-gnu/cmake/Qt5"
    "/usr/lib/cmake/Qt5"
)

for path in "${COMMON_PATHS[@]}"; do
    if [ -d "$path" ]; then
        echo "   找到目录: $path"
        if [ -f "$path/lib/cmake/Qt5/Qt5Config.cmake" ] || [ -f "$path/Qt5Config.cmake" ]; then
            if [ -f "$path/lib/cmake/Qt5/Qt5Config.cmake" ]; then
                QT_DIR="$path/lib/cmake/Qt5"
            else
                QT_DIR="$path"
            fi
            echo "   ✓ 确认Qt5配置: $QT_DIR"
            QT_FOUND=true
            break
        fi
    fi
done

echo ""

if [ "$QT_FOUND" = true ]; then
    echo "=========================================="
    echo "✓ 找到Qt安装路径"
    echo "=========================================="
    echo ""
    echo "请设置环境变量或CMake变量:"
    echo ""
    echo "方法1: 设置环境变量（推荐）"
    echo "  export QT5_DIR=\"$QT_DIR\""
    echo "  export CMAKE_PREFIX_PATH=\"$QT_DIR:\$CMAKE_PREFIX_PATH\""
    echo ""
    echo "方法2: 在CMake命令中指定"
    echo "  cmake .. -DCMAKE_PREFIX_PATH=\"$QT_DIR\""
    echo ""
    echo "方法3: 在CMakeLists.txt中设置（已自动配置）"
    echo ""
else
    echo "=========================================="
    echo "✗ 未找到Qt 5.12"
    echo "=========================================="
    echo ""
    echo "请手动指定Qt 5.12的安装路径:"
    echo ""
    echo "1. 找到Qt 5.12的安装目录（包含lib/cmake/Qt5的目录）"
    echo "2. 设置环境变量:"
    echo "   export QT5_DIR=\"/path/to/qt5/lib/cmake/Qt5\""
    echo "3. 或运行cmake时指定:"
    echo "   cmake .. -DCMAKE_PREFIX_PATH=\"/path/to/qt5/lib/cmake/Qt5\""
    echo ""
fi
