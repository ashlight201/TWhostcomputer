# 电机关节力矩标定系统

## 项目简介

这是一个基于Qt6（C++）开发的上位机界面程序，用于电机关节力矩的标定。系统通过串口接收力矩传感器和电机的数据，实时显示并计算标定系数。

## 功能特性

1. **双串口通信**
   - 串口1：力矩传感器通信
   - 串口2：电机通信
   - 支持串口配置（端口、波特率等）
   - 实时显示原始数据帧和处理后的数据

2. **数据处理**
   - 力矩传感器数据解析（可自定义协议）
   - 电机电流数据解析（可自定义协议）
   - 实时计算标定系数：系数 = 力矩 / 电流

3. **实时图表显示**
   - 力矩值曲线
   - 电流值曲线
   - 标定系数曲线
   - 支持实时更新和历史数据查看

4. **标定功能**
   - 开始/停止标定
   - 批量数据采集
   - 使用最小二乘法计算最优标定系数
   - 清空数据功能

## 系统要求

- Qt6（Core, Widgets, SerialPort, Charts）
- CMake 3.16或更高版本
- C++17编译器
- Linux/Windows/macOS

## 编译说明

### 1. 安装Qt6

确保已安装Qt6及以下组件：
- Qt6::Core
- Qt6::Widgets
- Qt6::SerialPort
- Qt6::Charts

### 2. 编译项目

```bash
mkdir build
cd build
cmake ..
make
```

### 3. 运行程序

```bash
./bin/TorqueCalibrationApp
```

## 使用说明

### 1. 配置串口

- **力矩传感器串口**：
  - 选择串口端口
  - 设置波特率（默认115200）
  - 点击"打开"按钮

- **电机串口**：
  - 选择串口端口
  - 设置波特率（默认115200）
  - 点击"打开"按钮

### 2. 数据解析配置

在 `DataProcessor.cpp` 中修改以下函数以适配实际的数据协议：

- `parseTorqueSensorFrame()`: 解析力矩传感器数据帧
- `parseMotorDataFrame()`: 解析电机数据帧

### 3. 标定流程

1. 打开两个串口
2. 确认数据正常接收和显示
3. 点击"开始标定"
4. 采集多个数据点（不同力矩和电流值）
5. 点击"停止标定"
6. 系统自动计算并显示标定系数

## 项目结构

```
.
├── CMakeLists.txt          # CMake构建文件
├── main.cpp                # 程序入口
├── MainWindow.h/cpp        # 主窗口类
├── MainWindow.ui           # UI设计文件
├── SerialPortManager.h/cpp # 串口管理类
├── DataProcessor.h/cpp     # 数据处理类
└── README.md               # 项目说明文档
```

## 自定义数据协议

### 力矩传感器协议

修改 `DataProcessor::parseTorqueSensorFrame()` 函数：

```cpp
double DataProcessor::parseTorqueSensorFrame(const QByteArray &frame)
{
    // 根据实际协议实现数据解析
    // 示例：假设数据格式为十六进制 "AA BB CC DD"（大端序）
    // 实际使用时需要根据真实协议修改
    
    if (frame.size() >= 4) {
        // 解析示例
        // ...
    }
    
    return torqueValue;
}
```

### 电机协议

修改 `DataProcessor::parseMotorDataFrame()` 函数：

```cpp
double DataProcessor::parseMotorDataFrame(const QByteArray &frame)
{
    // 根据实际协议实现数据解析
    // 示例：假设数据格式为ASCII "CURRENT:XX.XX\r\n"
    // 实际使用时需要根据真实协议修改
    
    // 解析示例
    // ...
    
    return currentValue;
}
```

## 注意事项

1. **数据同步**：确保力矩传感器和电机的数据采集时间同步，以获得准确的标定系数
2. **数据质量**：标定时应采集多个不同工况下的数据点，以提高标定精度
3. **串口权限**：Linux系统可能需要将用户添加到dialout组以获得串口访问权限：
   ```bash
   sudo usermod -a -G dialout $USER
   ```
4. **数据格式**：当前实现使用示例数据解析，实际使用时需要根据设备协议修改

## 开发计划

- [ ] 支持更多串口参数配置（数据位、停止位、校验位）
- [ ] 数据导出功能（CSV/Excel）
- [ ] 标定数据保存和加载
- [ ] 多组标定数据对比
- [ ] 标定精度评估（R²值等）

## 许可证

详见 LICENSE 文件
