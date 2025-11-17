@echo off
chcp 65001 >nul
echo ==========================================
echo   电机力矩标定系统 - MinGW编译脚本
echo ==========================================
echo.

:: 设置Qt路径（请根据实际安装路径修改）
set QT_PATH=C:\Qt\6.5.0\mingw_64
set MINGW_PATH=C:\Qt\Tools\mingw1120_64

:: 检查Qt是否存在
if not exist "%QT_PATH%" (
    echo ❌ 错误: 未找到Qt安装目录: %QT_PATH%
    echo.
    echo 请修改脚本中的 QT_PATH 变量为你的实际Qt安装路径
    echo 常见路径:
    echo   - C:\Qt\6.5.0\mingw_64
    echo   - C:\Qt\6.4.0\mingw_64
    echo   - C:\Qt\5.15.2\mingw81_64
    echo.
    pause
    exit /b 1
)

:: 检查MinGW是否存在
if not exist "%MINGW_PATH%" (
    echo ❌ 错误: 未找到MinGW安装目录: %MINGW_PATH%
    echo.
    echo 请修改脚本中的 MINGW_PATH 变量
    echo 常见路径:
    echo   - C:\Qt\Tools\mingw1120_64
    echo   - C:\Qt\Tools\mingw900_64
    echo.
    pause
    exit /b 1
)

echo ✓ 找到Qt安装: %QT_PATH%
echo ✓ 找到MinGW: %MINGW_PATH%

:: 设置环境变量
set PATH=%QT_PATH%\bin;%MINGW_PATH%\bin;%PATH%
set CMAKE_PREFIX_PATH=%QT_PATH%

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
cmake -G "MinGW Makefiles" .. 2>error.log
if errorlevel 1 (
    echo.
    echo ❌ 错误: CMake配置失败
    echo.
    echo 可能的原因:
    echo 1. 未安装Qt Charts模块
    echo    运行: C:\Qt\MaintenanceTool.exe
    echo    选择: Add or remove components
    echo    勾选: Qt Charts 和 Qt Serial Port
    echo.
    echo 2. MinGW路径不正确
    echo    检查: %MINGW_PATH%
    echo.
    type error.log
    cd ..
    pause
    exit /b 1
)

:: 编译项目
echo.
echo 编译项目...
mingw32-make -j4
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
echo 可执行文件位置: build\bin\MotorCalibration.exe
echo.
echo 部署Qt依赖文件（首次运行需要）...
cd build\bin
%QT_PATH%\bin\windeployqt.exe MotorCalibration.exe
cd ..\..

echo.
echo 按任意键运行程序...
pause >nul
start build\bin\MotorCalibration.exe
