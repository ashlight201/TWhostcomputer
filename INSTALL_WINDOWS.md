# Windows 安装指南

## Qt Charts 安装方法

### 方法1：使用Qt在线安装器（推荐，最简单）

#### 步骤1：下载Qt在线安装器

访问Qt官网下载页面：
- 官网：https://www.qt.io/download-qt-installer
- 国内镜像（推荐，更快）：
  - 清华镜像：https://mirrors.tuna.tsinghua.edu.cn/qt/official_releases/online_installers/
  - 中科大镜像：https://mirrors.ustc.edu.cn/qtproject/official_releases/online_installers/

下载 `qt-unified-windows-x64-online.exe`

#### 步骤2：运行安装器

1. 双击运行安装器
2. 登录或注册Qt账号（可以使用免费的开源版本）
3. 选择安装路径（例如：`C:\Qt`）

#### 步骤3：选择组件（重要！）

在组件选择界面，**必须选择以下组件**：

```
Qt 6.x.x（或 Qt 5.15.x）
├── MSVC 2019 64-bit  （如果使用Visual Studio）
├── MinGW 11.2.0 64-bit （如果使用MinGW）
└── Additional Libraries
    ├── Qt Charts  ⬅️ 必选！
    ├── Qt Serial Port  ⬅️ 必选！
    └── Qt 5 Compatibility Module（仅Qt6需要）

Developer and Designer Tools
├── CMake 3.x.x  ⬅️ 推荐
├── Ninja  ⬅️ 推荐
└── MinGW 11.2.0 64-bit（如果选择了MinGW）
```

**关键提醒：** 必须勾选 `Qt Charts` 和 `Qt Serial Port` 模块！

#### 步骤4：完成安装

1. 点击"下一步"开始安装
2. 等待安装完成（可能需要30分钟到1小时，取决于网速）
3. 安装完成后，Qt会安装在 `C:\Qt\6.x.x` 或 `C:\Qt\5.15.x`

---

### 方法2：使用Qt维护工具（已安装Qt但缺少Charts）

如果你已经安装了Qt，但是没有Charts模块：

1. 打开 Qt Maintenance Tool
   - 位置：`C:\Qt\MaintenanceTool.exe`

2. 选择 "Add or remove components"

3. 找到你已安装的Qt版本，展开 "Additional Libraries"

4. 勾选：
   - ✅ Qt Charts
   - ✅ Qt Serial Port

5. 点击"Update"，等待下载安装完成

---

### 方法3：使用vcpkg包管理器（适合高级用户）

```powershell
# 安装vcpkg（如果还没有）
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安装Qt Charts
.\vcpkg install qt5-charts:x64-windows
.\vcpkg install qt5-serialport:x64-windows

# 或Qt6
.\vcpkg install qt6-charts:x64-windows
.\vcpkg install qt6-serialport:x64-windows
```

---

## 编译项目

### 使用Qt Creator（推荐，最简单）

#### 步骤1：启动Qt Creator

打开 `C:\Qt\Tools\QtCreator\bin\qtcreator.exe`

#### 步骤2：打开项目

1. 点击 "文件" → "打开文件或项目"
2. 选择项目的 `CMakeLists.txt` 文件
3. 在 "Configure Project" 界面，选择合适的Kit：
   - Desktop Qt 6.x.x MSVC2019 64bit
   - 或 Desktop Qt 5.15.x MinGW 64bit

#### 步骤3：构建项目

1. 点击左下角的"构建"按钮（锤子图标）
2. 或按快捷键 `Ctrl+B`
3. 等待编译完成

#### 步骤4：运行程序

1. 点击左下角的"运行"按钮（绿色三角形）
2. 或按快捷键 `Ctrl+R`

---

### 使用命令行编译

#### 前提条件

1. **安装Visual Studio 2019或更新版本**（如果使用MSVC）
   - 下载：https://visualstudio.microsoft.com/downloads/
   - 必须安装"使用C++的桌面开发"工作负荷

2. **或安装MinGW**（包含在Qt安装中）

#### 使用MSVC编译

打开 "x64 Native Tools Command Prompt for VS 2019"：

```cmd
:: 设置Qt路径（根据你的实际安装路径修改）
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64

:: 创建构建目录
mkdir build
cd build

:: 配置项目
cmake -G "Visual Studio 16 2019" -A x64 ..

:: 编译
cmake --build . --config Release

:: 运行
bin\Release\MotorCalibration.exe
```

#### 使用MinGW编译

打开"命令提示符"或"PowerShell"：

```cmd
:: 设置Qt和MinGW路径（根据你的实际安装路径修改）
set PATH=C:\Qt\6.5.0\mingw_64\bin;C:\Qt\Tools\mingw1120_64\bin;%PATH%
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\mingw_64

:: 创建构建目录
mkdir build
cd build

:: 配置项目
cmake -G "MinGW Makefiles" ..

:: 编译
mingw32-make -j4

:: 运行
bin\MotorCalibration.exe
```

---

## 创建便捷的编译脚本

### build_msvc.bat（使用Visual Studio）

创建 `build_msvc.bat` 文件：

```batch
@echo off
echo ==========================================
echo   电机力矩标定系统 - MSVC编译脚本
echo ==========================================
echo.

:: 设置Qt路径（请根据实际安装路径修改）
set QT_PATH=C:\Qt\6.5.0\msvc2019_64
set CMAKE_PREFIX_PATH=%QT_PATH%

:: 检查Qt是否存在
if not exist "%QT_PATH%" (
    echo 错误: 未找到Qt安装目录: %QT_PATH%
    echo 请修改脚本中的 QT_PATH 变量
    pause
    exit /b 1
)

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
cmake -G "Visual Studio 16 2019" -A x64 .. || (
    echo.
    echo 错误: CMake配置失败
    echo 请确保已安装Visual Studio 2019和Qt Charts模块
    cd ..
    pause
    exit /b 1
)

:: 编译项目
echo.
echo 编译项目...
cmake --build . --config Release || (
    echo.
    echo 错误: 编译失败
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ==========================================
echo   编译成功！
echo ==========================================
echo.
echo 可执行文件位置: build\bin\Release\MotorCalibration.exe
echo.
echo 按任意键运行程序...
pause
start build\bin\Release\MotorCalibration.exe
```

### build_mingw.bat（使用MinGW）

创建 `build_mingw.bat` 文件：

```batch
@echo off
echo ==========================================
echo   电机力矩标定系统 - MinGW编译脚本
echo ==========================================
echo.

:: 设置Qt路径（请根据实际安装路径修改）
set QT_PATH=C:\Qt\6.5.0\mingw_64
set MINGW_PATH=C:\Qt\Tools\mingw1120_64
set PATH=%QT_PATH%\bin;%MINGW_PATH%\bin;%PATH%
set CMAKE_PREFIX_PATH=%QT_PATH%

:: 检查Qt是否存在
if not exist "%QT_PATH%" (
    echo 错误: 未找到Qt安装目录: %QT_PATH%
    echo 请修改脚本中的 QT_PATH 变量
    pause
    exit /b 1
)

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
cmake -G "MinGW Makefiles" .. || (
    echo.
    echo 错误: CMake配置失败
    echo 请确保已安装MinGW和Qt Charts模块
    cd ..
    pause
    exit /b 1
)

:: 编译项目
echo.
echo 编译项目...
mingw32-make -j4 || (
    echo.
    echo 错误: 编译失败
    cd ..
    pause
    exit /b 1
)

cd ..

echo.
echo ==========================================
echo   编译成功！
echo ==========================================
echo.
echo 可执行文件位置: build\bin\MotorCalibration.exe
echo.
echo 按任意键运行程序...
pause
start build\bin\MotorCalibration.exe
```

---

## 常见问题

### Q1: 提示找不到Qt Charts头文件？

**原因：** 安装Qt时没有勾选Qt Charts模块

**解决方案：**
1. 运行 `C:\Qt\MaintenanceTool.exe`
2. 选择 "Add or remove components"
3. 在你的Qt版本下，勾选 "Qt Charts"
4. 点击"Update"重新安装

### Q2: CMake找不到Qt？

**解决方案：**

确保设置了正确的 `CMAKE_PREFIX_PATH`：

```cmd
:: MSVC版本
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64

:: MinGW版本
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\mingw_64
```

### Q3: 运行时提示缺少DLL文件？

**原因：** Qt的DLL文件不在系统PATH中

**解决方案1：** 添加Qt到系统PATH

1. 右键"此电脑" → "属性" → "高级系统设置"
2. 点击"环境变量"
3. 在"系统变量"中找到"Path"，点击"编辑"
4. 添加Qt的bin目录：
   - MSVC: `C:\Qt\6.5.0\msvc2019_64\bin`
   - MinGW: `C:\Qt\6.5.0\mingw_64\bin`

**解决方案2：** 使用windeployqt工具

```cmd
cd build\bin\Release
C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe MotorCalibration.exe
```

这会自动复制所有需要的DLL到程序目录。

### Q4: 串口无法打开？

**原因：** Windows不需要特殊权限，但可能是串口被占用

**解决方案：**
1. 检查设备管理器中串口是否正常
2. 关闭其他可能使用串口的程序（如Arduino IDE、串口调试助手等）
3. 尝试重新插拔USB设备

---

## 验证安装

### 检查Qt安装

```cmd
:: 检查qmake版本
C:\Qt\6.5.0\msvc2019_64\bin\qmake.exe -v

:: 检查Qt Charts模块是否存在
dir C:\Qt\6.5.0\msvc2019_64\include\QtCharts
```

如果能看到QtCharts目录和文件，说明安装成功。

---

## 推荐的Qt版本

- **Qt 6.5 LTS** - 最新长期支持版本（推荐）
- **Qt 5.15 LTS** - 稳定版本，兼容性好
- **编译器：**
  - MSVC 2019（推荐，与Visual Studio集成好）
  - MinGW（不需要Visual Studio，适合快速开发）

---

## 完整示例

假设你安装Qt 6.5.0 MSVC版本到 `C:\Qt`：

```cmd
:: 1. 设置环境
set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64

:: 2. 打开VS命令提示符
:: 开始菜单 → Visual Studio 2019 → x64 Native Tools Command Prompt

:: 3. 进入项目目录
cd C:\Users\YourName\workspace

:: 4. 编译
mkdir build && cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
cmake --build . --config Release

:: 5. 部署Qt依赖
cd bin\Release
C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe MotorCalibration.exe

:: 6. 运行
MotorCalibration.exe
```

祝你编译成功！🎉
