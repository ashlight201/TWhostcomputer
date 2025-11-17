# 电机关节力矩标定系统

这是一个基于Qt（C++）的上位机界面程序，用于标定电机关节力矩与电流之间的关系系数。

## 功能特性

1. **双串口通信**
   - 力矩传感器串口：接收力矩传感器反馈的原始数据
   - 电机串口：接收电机反馈的电流数据
   - 每个串口都支持独立的配置（端口、波特率）和调试功能

2. **数据显示**
   - 力矩传感器原始数据帧显示（十六进制格式）
   - 力矩传感器处理后的力矩值显示
   - 电机原始数据帧显示（十六进制格式）
   - 电机处理后的电流值显示

3. **数据标定**
   - 实时采集力矩和电流数据
   - 使用最小二乘法计算标定系数（力矩 = 系数 × 电流）
   - 显示计算得到的标定系数

4. **实时图表**
   - 实时显示力矩、电流和标定系数的变化曲线
   - 支持时间滑动窗口显示
   - 自动调整Y轴范围

## 项目结构

```
.
├── CMakeLists.txt          # CMake构建配置文件
├── src/
│   ├── main.cpp            # 程序入口
│   ├── MainWindow.h/cpp    # 主窗口类
│   ├── SerialPortManager.h/cpp  # 串口管理类
│   ├── DataProcessor.h/cpp  # 数据处理类
│   └── ChartWidget.h/cpp   # 图表显示类
└── ui/
    └── MainWindow.ui        # UI界面文件
```

## 编译要求

- CMake 3.16 或更高版本
- Qt 5.12 或更高版本（或 Qt 6）
- C++17 编译器

### Qt组件要求
- Qt Core
- Qt Widgets
- Qt SerialPort
- Qt Charts

## 编译步骤

### Linux

```bash
mkdir build
cd build
cmake ..
make
```

### Windows (使用Visual Studio)

```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### macOS

```bash
mkdir build
cd build
cmake ..
make
```

## 使用方法

1. **连接串口**
   - 在"力矩传感器串口调试"区域选择串口和波特率，点击"连接"
   - 在"电机串口调试"区域选择串口和波特率，点击"连接"

2. **查看数据**
   - 原始数据帧会显示在对应的文本框中（十六进制格式）
   - 处理后的数据会显示在下方（力矩值或电流值）

3. **发送数据**
   - 在发送输入框中输入十六进制数据（如：01 02 03），点击"发送"

4. **标定系数**
   - 系统会自动采集力矩和电流数据
   - 点击"开始标定"按钮计算标定系数
   - 标定系数会显示在界面上，并实时更新到图表中

5. **实时图表**
   - 右侧图表实时显示力矩、电流和标定系数的变化
   - 图表支持自动缩放和时间滑动窗口

## 自定义数据处理

数据处理函数可以在 `DataProcessor` 类中自定义：

```cpp
// 在 MainWindow 中设置自定义处理函数
dataProcessor->setTorqueSensorProcessor([](const QByteArray &data) -> double {
    // 在这里实现您的力矩传感器数据解析逻辑
    // 返回解析得到的力矩值（单位：N·m）
    return torqueValue;
});

dataProcessor->setMotorProcessor([](const QByteArray &data) -> double {
    // 在这里实现您的电机数据解析逻辑
    // 返回解析得到的电流值（单位：A）
    return currentValue;
});
```

## 标定算法

系统使用最小二乘法计算标定系数：

```
系数 = Σ(力矩 × 电流) / Σ(电流²)
```

这确保了在给定电流值下，计算得到的力矩值与实际力矩值的误差平方和最小。

## 注意事项

1. 默认的数据处理函数是示例实现，需要根据实际的串口通信协议进行修改
2. 确保串口没有被其他程序占用
3. 标定需要至少2个数据点才能计算系数
4. 数据会自动存储，点击"清空数据"可以清除所有历史数据

## 许可证

请查看 LICENSE 文件了解许可证信息。
