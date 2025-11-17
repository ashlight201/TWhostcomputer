# Windows 快速开始指南（5分钟）

## 步骤1：安装Qt（首次使用）⏱️ 30-60分钟

### 1.1 下载Qt安装器

**选项A - 官网下载（国外快）：**
- 访问：https://www.qt.io/download-qt-installer
- 下载：`qt-unified-windows-x64-online.exe`

**选项B - 国内镜像（推荐，更快）：**
- 清华镜像：https://mirrors.tuna.tsinghua.edu.cn/qt/official_releases/online_installers/
- 下载：`qt-unified-windows-x64-online.exe`

### 1.2 运行安装器

1. 双击运行 `qt-unified-windows-x64-online.exe`
2. 注册或登录Qt账号（免费）
3. 选择安装路径，例如：`C:\Qt`

### 1.3 选择组件（重要！）

在组件选择界面，必须勾选以下内容：

```
📦 Qt 6.5.x (LTS) 或 Qt 5.15.x
   ├─ MSVC 2019 64-bit                    ⬅️ 推荐
   ├─ MinGW 11.2.0 64-bit                 ⬅️ 备选
   └─ Additional Libraries
       ├─ ✅ Qt Charts                     ⬅️ 必选！
       └─ ✅ Qt Serial Port                ⬅️ 必选！

🛠️ Developer and Designer Tools
   ├─ Qt Creator                          ⬅️ 推荐
   ├─ CMake                               ⬅️ 推荐
   └─ MinGW 11.2.0 64-bit                 ⬅️ 如果选择了MinGW
```

**⚠️ 关键提醒：** 
- 必须勾选 `Qt Charts` 模块，否则无法编译！
- 必须勾选 `Qt Serial Port` 模块，否则无法使用串口！

### 1.4 等待安装完成

- 下载时间：15-40分钟（取决于网速）
- 安装时间：5-10分钟
- 总共大约：30-60分钟

---

## 步骤2：编译项目 ⏱️ 2-5分钟

### 方法A：使用Qt Creator（最简单）

1. **打开Qt Creator**
   - 位置：`C:\Qt\Tools\QtCreator\bin\qtcreator.exe`
   - 或从开始菜单搜索"Qt Creator"

2. **打开项目**
   - 点击：`文件` → `打开文件或项目`
   - 选择项目的 `CMakeLists.txt` 文件
   - 点击"打开"

3. **配置Kit**
   - 选择：`Desktop Qt 6.5.x MSVC2019 64bit`
   - 点击：`Configure Project`（配置项目）

4. **编译**
   - 点击左下角的 🔨 **构建**按钮
   - 或按快捷键：`Ctrl + B`
   - 等待1-3分钟

5. **运行**
   - 点击左下角的 ▶️ **运行**按钮
   - 或按快捷键：`Ctrl + R`

### 方法B：使用编译脚本（快捷）

1. **修改脚本中的Qt路径**
   
   用文本编辑器打开 `build_msvc.bat`，修改第8行：
   ```batch
   set QT_PATH=C:\Qt\6.5.0\msvc2019_64
   ```
   改为你的实际Qt安装路径。

2. **运行编译脚本**
   
   双击运行：`build_msvc.bat`

3. **等待编译完成**
   
   编译成功后会自动运行程序。

---

## 步骤3：使用程序 ⏱️ 1分钟

### 3.1 连接串口

1. 连接力矩传感器和电机到电脑
2. 在程序中点击"刷新"按钮
3. 选择对应的串口
4. 选择波特率（通常是115200）
5. 点击"打开串口"

### 3.2 查看数据

- 原始数据会在文本框中显示
- 处理后的数据会在输入框中显示
- 右侧图表会实时显示数据曲线

### 3.3 标定系数

1. 让电机运行在不同负载下
2. 每个稳定状态点击"添加标定点"
3. 采集5-10个数据点
4. 点击"计算系数"
5. 查看标定结果

---

## 常见问题速查

### ❌ 问题1：找不到Qt Charts头文件

**错误信息：**
```
QtCharts/QChartView: No such file or directory
```

**原因：** 安装Qt时没有勾选Qt Charts模块

**解决方案：**
1. 运行 `C:\Qt\MaintenanceTool.exe`
2. 选择 "Add or remove components"
3. 找到你安装的Qt版本，展开 "Additional Libraries"
4. 勾选 ✅ `Qt Charts`
5. 勾选 ✅ `Qt Serial Port`
6. 点击"Update"

### ❌ 问题2：找不到Visual Studio

**错误信息：**
```
Could not find Visual Studio
```

**解决方案：**

**选项A：** 安装Visual Studio（如果你想用MSVC）
1. 下载：https://visualstudio.microsoft.com/downloads/
2. 选择"Community"版本（免费）
3. 安装时勾选"使用C++的桌面开发"

**选项B：** 使用MinGW（不需要Visual Studio）
1. 运行 `build_mingw.bat` 而不是 `build_msvc.bat`
2. 或在Qt Creator中选择MinGW的Kit

### ❌ 问题3：运行时缺少DLL

**错误信息：**
```
找不到 Qt6Core.dll / Qt5Core.dll
```

**解决方案：**

运行windeployqt工具（会自动复制所有DLL）：
```cmd
cd build\bin\Release
C:\Qt\6.5.0\msvc2019_64\bin\windeployqt.exe MotorCalibration.exe
```

或者使用编译脚本，它会自动执行这个步骤。

### ❌ 问题4：串口打不开

**可能原因和解决方案：**

1. **串口被占用**
   - 关闭Arduino IDE、串口助手等程序
   - 重新插拔USB

2. **驱动未安装**
   - 打开设备管理器
   - 查看"端口(COM和LPT)"
   - 如果有黄色感叹号，需要安装驱动

3. **串口号选择错误**
   - 在设备管理器中确认正确的COM端口号
   - 在程序中选择对应的端口

---

## 快速参考卡片

### 文件说明

| 文件 | 用途 |
|------|------|
| `CMakeLists.txt` | 项目构建配置 |
| `build_msvc.bat` | MSVC编译脚本 |
| `build_mingw.bat` | MinGW编译脚本 |
| `INSTALL_WINDOWS.md` | 详细安装文档 |
| `README.md` | 完整项目文档 |

### 常用路径

| 说明 | 路径 |
|------|------|
| Qt安装目录 | `C:\Qt\6.5.0\msvc2019_64\` |
| Qt Creator | `C:\Qt\Tools\QtCreator\bin\qtcreator.exe` |
| Qt维护工具 | `C:\Qt\MaintenanceTool.exe` |
| 可执行文件 | `build\bin\Release\MotorCalibration.exe` |

### 快捷键（Qt Creator）

| 功能 | 快捷键 |
|------|--------|
| 构建 | `Ctrl + B` |
| 运行 | `Ctrl + R` |
| 停止 | `Ctrl + Shift + R` |
| 清理 | `Alt + C` |
| 查找 | `Ctrl + F` |

---

## 下载链接汇总

- **Qt官方下载：** https://www.qt.io/download-qt-installer
- **Qt清华镜像：** https://mirrors.tuna.tsinghua.edu.cn/qt/
- **Visual Studio：** https://visualstudio.microsoft.com/downloads/
- **CMake下载：** https://cmake.org/download/

---

## 需要帮助？

1. 查看详细文档：
   - [INSTALL_WINDOWS.md](INSTALL_WINDOWS.md) - 详细安装指南
   - [README.md](README.md) - 完整项目文档
   - [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - 故障排除

2. 检查环境：
   ```cmd
   qmake -v
   cmake --version
   ```

3. 验证Qt Charts：
   ```cmd
   dir C:\Qt\6.5.0\msvc2019_64\include\QtCharts
   ```

祝你使用愉快！ 🎉
