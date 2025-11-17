@echo off
chcp 65001 >nul
echo ==========================================
echo   电机力矩标定系统 - MSVC编译脚本
echo ==========================================
echo.

:: 设置Qt路径（请根据实际安装路径修改）
set QT_PATH=C:\Qt\6.5.0\msvc2019_64
set CMAKE_PREFIX_PATH=%QT_PATH%

:: 检查Qt是否存在
if not exist "%QT_PATH%" (
    echo ❌ 错误: 未找到Qt安装目录: %QT_PATH%
    echo.
    echo 请修改脚本中的 QT_PATH 变量为你的实际Qt安装路径
    echo 常见路径:
    echo   - C:\Qt\6.5.0\msvc2019_64
    echo   - C:\Qt\6.4.0\msvc2019_64
    echo   - C:\Qt\5.15.2\msvc2019_64
    echo.
    pause
    exit /b 1
)

echo ✓ 找到Qt安装: %QT_PATH%

:: 清理旧的构建
if exist build (
    echo 清理旧的构建目录...
    rmdir /s /q build
)

:: 创建构建目录
echo 创建构建目录...
mkdir build
cd build

:: 配置项目
echo.
echo 配置项目...
cmake -G "Visual Studio 16 2019" -A x64 .. 2>error.log
if errorlevel 1 (
    echo.
    echo ❌ 错误: CMake配置失败
    echo.
    echo 可能的原因:
    echo 1. 未安装Visual Studio 2019或更新版本
    echo    下载: https://visualstudio.microsoft.com/downloads/
    echo.
    echo 2. 未安装Qt Charts模块
    echo    运行: C:\Qt\MaintenanceTool.exe
    echo    选择: Add or remove components
    echo    勾选: Qt Charts 和 Qt Serial Port
    echo.
    echo 3. Visual Studio版本不匹配
    echo    尝试修改为: cmake -G "Visual Studio 17 2022" -A x64 ..
    echo.
    type error.log
    cd ..
    pause
    exit /b 1
)

:: 编译项目
echo.
echo 编译项目...
cmake --build . --config Release
if errorlevel 1 (
    echo.
    echo ❌ 错误: 编译失败
    echo 请查看上面的错误信息
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ==========================================
echo   ✅ 编译成功！
echo ==========================================
echo.
echo 可执行文件位置: build\bin\Release\MotorCalibration.exe
echo.
echo 部署Qt依赖文件（首次运行需要）...
cd build\bin\Release
%QT_PATH%\bin\windeployqt.exe MotorCalibration.exe
cd ..\..\..

echo.
echo 按任意键运行程序...
pause >nul
start build\bin\Release\MotorCalibration.exe
