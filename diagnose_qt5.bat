@echo off
chcp 65001 >nul
echo ===========================================
echo   Qt 5.12 问题诊断工具
echo ===========================================
echo.

:: 设置你的Qt路径
set QT_PATH=C:\Qt\5.12.0\mingw73_64

echo [1/5] 检查Qt安装...
if exist "%QT_PATH%" (
    echo ✓ 找到Qt安装: %QT_PATH%
) else (
    echo ❌ 未找到Qt: %QT_PATH%
    echo 请修改此脚本第8行的QT_PATH为你的实际Qt路径
    pause
    exit /b 1
)

echo.
echo [2/5] 检查Qt Charts模块...
if exist "%QT_PATH%\include\QtCharts" (
    echo ✓ 找到Qt Charts头文件
    dir "%QT_PATH%\include\QtCharts" | find "QChartView"
) else (
    echo ❌ 未找到Qt Charts头文件
    echo.
    echo 解决方案:
    echo 1. 运行 C:\Qt\MaintenanceTool.exe
    echo 2. 选择 "Add or remove components"
    echo 3. 勾选 Qt Charts 模块
    echo 4. 点击 Update
    pause
    exit /b 1
)

echo.
echo [3/5] 检查Qt Charts库文件...
if exist "%QT_PATH%\lib\libQt5Charts*" (
    echo ✓ 找到Qt Charts库文件
    dir "%QT_PATH%\lib\libQt5Charts*"
) else (
    echo ❌ 未找到Qt Charts库文件
    pause
    exit /b 1
)

echo.
echo [4/5] 检查MinGW编译器...
if exist "%QT_PATH%\bin\g++.exe" (
    echo ✓ 找到MinGW编译器
    "%QT_PATH%\bin\g++.exe" --version | find "g++"
) else (
    echo ⚠ MinGW可能在单独的目录
)

echo.
echo [5/5] 检查必要的DLL...
set DLL_LIST=Qt5Core Qt5Gui Qt5Widgets Qt5SerialPort Qt5Charts
for %%d in (%DLL_LIST%) do (
    if exist "%QT_PATH%\bin\%%d.dll" (
        echo ✓ %%d.dll
    ) else (
        echo ❌ 缺少 %%d.dll
    )
)

echo.
echo ===========================================
echo   诊断完成
echo ===========================================
echo.
pause
