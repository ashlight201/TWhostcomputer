# 电机力矩标定系统

这是一个基于Qt（C++）开发的上位机界面程序，用于标定电机关节力矩与电机反馈电流之间的关系。

## 功能概述

该系统实现了以下主要功能：

### 1. 双串口通信
- **力矩传感器串口**：接收力矩传感器的实时数据
- **电机串口**：接收电机的实时电流数据
- 支持多种波特率配置（9600~921600）
- 实时显示原始数据帧（十六进制格式）

### 2. 数据处理
- 力矩传感器数据解析和处理
- 电机电流数据解析和处理
- 处理后数据实时显示
- 数据处理接口函数可根据实际协议自定义

### 3. 标定功能
- 手动添加标定数据点（力矩-电流对）
- 基于最小二乘法自动计算标定系数
- 标定系数显示和历史记录
- 标定日志记录

### 4. 实时图表显示
- 力矩传感器数据曲线
- 电机电流数据曲线
- 标定系数变化曲线
- 自动缩放和滚动显示

## 系统原理

电机关节力矩与电机电流的关系为：

```
力矩 (Nm) = 标定系数 × 电流 (A)
```

本系统通过采集多组力矩和电流数据，使用最小二乘法计算最优标定系数。

## 项目结构

```
workspace/
├── CMakeLists.txt              # CMake构建配置
├── README.md                   # 项目说明文档
├── src/
│   ├── main.cpp               # 主程序入口
│   ├── mainwindow.h           # 主窗口头文件
│   ├── mainwindow.cpp         # 主窗口实现
│   ├── serialporthandler.h    # 串口处理类头文件
│   ├── serialporthandler.cpp  # 串口处理类实现
│   ├── dataprocessor.h        # 数据处理类头文件
│   └── dataprocessor.cpp      # 数据处理类实现
└── build/                      # 编译输出目录（需自行创建）
```

## 环境要求

### 开发环境
- CMake 3.16 或更高版本
- C++17 编译器
- Qt 5.15 或 Qt 6.x

### Qt模块依赖
- Qt::Core
- Qt::Widgets
- Qt::SerialPort
- Qt::Charts

## 编译安装

### Windows（推荐使用Qt Creator）

**📖 详细安装指南：** 请查看 [INSTALL_WINDOWS.md](INSTALL_WINDOWS.md)

#### 快速步骤：

1. **安装Qt（包含Charts模块）**
   - 下载Qt在线安装器：https://www.qt.io/download-qt-installer
   - 安装时必须勾选：`Qt Charts` 和 `Qt Serial Port` 模块
   - 推荐安装 Qt 6.5 LTS MSVC版本

2. **使用Qt Creator编译**
   ```
   1. 打开 Qt Creator
   2. 文件 → 打开文件或项目 → 选择 CMakeLists.txt
   3. 配置Kit（选择MSVC或MinGW）
   4. 点击"构建"按钮（Ctrl+B）
   5. 点击"运行"按钮（Ctrl+R）
   ```

3. **或使用命令行编译**
   
   **MSVC编译（推荐）：**
   ```cmd
   :: 双击运行编译脚本
   build_msvc.bat
   ```
   
   **MinGW编译：**
   ```cmd
   :: 双击运行编译脚本
   build_mingw.bat
   ```
   
   **或手动编译：**
   ```cmd
   :: 设置Qt路径
   set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64
   
   :: 编译
   mkdir build && cd build
   cmake -G "Visual Studio 16 2019" -A x64 ..
   cmake --build . --config Release
   
   :: 运行
   bin\Release\MotorCalibration.exe
   ```

### Linux / macOS

```bash
# 安装Qt开发库（以Ubuntu为例）
sudo apt-get install qtbase5-dev libqt5serialport5-dev libqt5charts5-dev
# 或者使用Qt6
# sudo apt-get install qt6-base-dev qt6-serialport-dev qt6-charts-dev

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j4

# 运行
./bin/MotorCalibration
```

## 使用说明

### 1. 连接串口设备

#### 力矩传感器串口
1. 将力矩传感器连接到计算机
2. 在"力矩传感器"组中选择对应的串口
3. 选择正确的波特率（默认115200）
4. 点击"打开串口"按钮

#### 电机串口
1. 将电机控制器连接到计算机
2. 在"电机"组中选择对应的串口
3. 选择正确的波特率（默认115200）
4. 点击"打开串口"按钮

### 2. 查看实时数据

- **原始数据**：在各自的"原始数据"文本框中以十六进制格式显示
- **处理后数据**：在"处理后数据"输入框中显示解析后的数值
- **实时曲线**：右侧三个图表实时显示数据变化

### 3. 标定系数

#### 添加标定点
1. 确保力矩传感器和电机都已连接并接收到数据
2. 让电机在某个稳定状态下运行
3. 点击"添加标定点"按钮，将当前的力矩和电流值保存为一个标定点
4. 重复以上步骤，在不同的运行状态下采集多个标定点（建议至少5-10个点）

#### 计算系数
1. 确保已添加足够的标定点（至少2个）
2. 点击"计算系数"按钮
3. 系统将使用最小二乘法计算标定系数
4. 计算结果显示在"标定系数"输入框中
5. 标定系数的变化曲线显示在右下角图表中

#### 清除数据
- 点击"清除数据"按钮可以清除所有已保存的标定点

### 4. 数据协议定制

系统提供了数据解析的接口函数，可根据实际的通信协议进行修改：

#### 修改力矩传感器数据解析
编辑 `src/dataprocessor.cpp` 中的以下函数：

```cpp
double DataProcessor::parseTorqueRawData(const QByteArray &data)
{
    // 在这里实现你的数据解析逻辑
    // 例如：解析特定格式的数据帧
}

double DataProcessor::processTorqueValue(double rawValue)
{
    // 在这里实现数据处理逻辑
    // 例如：滤波、单位转换、零点校准等
}
```

#### 修改电机电流数据解析
编辑 `src/dataprocessor.cpp` 中的以下函数：

```cpp
double DataProcessor::parseCurrentRawData(const QByteArray &data)
{
    // 在这里实现你的数据解析逻辑
}

double DataProcessor::processCurrentValue(double rawValue)
{
    // 在这里实现数据处理逻辑
}
```

## 技术特点

### 1. 模块化设计
- **SerialPortHandler**：封装串口通信功能，支持多串口独立管理
- **DataProcessor**：封装数据处理和标定计算逻辑
- **MainWindow**：UI界面和用户交互

### 2. 实时性能优化
- 使用Qt信号槽机制实现异步数据处理
- 图表更新采用定时器控制（100ms刷新）
- 限制图表数据点数量（最多100点）避免性能问题

### 3. 用户体验
- 直观的界面布局
- 实时数据可视化
- 详细的操作日志
- 错误提示和状态反馈

## 常见问题

### Q1: 串口打开失败？
- 检查串口是否被其他程序占用
- 确认串口设备是否正确连接
- 在Linux下可能需要添加串口权限：`sudo usermod -a -G dialout $USER`

### Q2: 接收不到数据？
- 检查波特率设置是否正确
- 确认数据位、停止位、校验位配置
- 使用串口调试工具验证设备是否正常发送数据

### Q3: 标定系数异常？
- 确保采集的数据点覆盖足够的工作范围
- 检查力矩和电流数据是否同步
- 增加标定点数量以提高精度
- 排除异常数据点

### Q4: 图表显示异常？
- 检查数据解析是否正确
- 确认数据范围是否合理
- 图表会自动调整Y轴范围

## 开发和扩展

### 添加数据保存功能
可以在 `MainWindow` 中添加数据保存功能，将标定数据和结果保存到文件：

```cpp
void MainWindow::saveCalibrationData()
{
    QFile file("calibration_data.csv");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "Torque(Nm),Current(A)\n";
        // 写入数据...
        file.close();
    }
}
```

### 添加自动标定功能
可以实现自动采集和计算：

```cpp
void MainWindow::startAutoCalibration()
{
    // 定时自动采集数据点
    // 达到设定数量后自动计算系数
}
```

### 添加数据滤波
在 `DataProcessor` 中实现各种滤波算法：

```cpp
double DataProcessor::applyKalmanFilter(double newValue)
{
    // 实现卡尔曼滤波
}
```

## 许可证

本项目采用 MIT 许可证。详见 LICENSE 文件。

## 作者

根据用户需求开发的电机力矩标定系统。

## 更新日志

### v1.0 (2025-11-17)
- 初始版本发布
- 实现双串口通信功能
- 实现数据处理和标定计算
- 实现实时图表显示
- 提供数据解析接口函数