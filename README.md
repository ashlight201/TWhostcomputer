# TWhostcomputer

一个使用 Qt Widgets+C++ 编写的电机力矩标定上位机示例，包含以下核心能力：

- 同时管理力矩传感器与电机两个串口（调试、发送/接收监视）
- 展示原始数据帧、处理后的力矩/电流数值
- 在线计算 `力矩 = 系数 × 电流` 中的未知系数，并进行滑动平均平滑
- 使用 Qt Charts 将力矩、电流与实时系数三条曲线绘制在同一屏幕中

> ⚠️ 当前的数据解析/处理逻辑为占位实现，只提供接口与示例算法，实际项目请根据真实通讯协议自行调整 `DataProcessor` 类的解析方式、系数计算策略以及 UI 表达。

## 目录结构

```
├── CMakeLists.txt
├── LICENSE
├── README.md
└── src
    ├── DataPlotWidget.*      # 封装 Qt Charts 的实时曲线控件
    ├── DataProcessor.*       # 原始帧解析、力矩/电流换算与系数平滑
    ├── MainWindow.*          # 主界面，整合所有控件与业务逻辑
    ├── SerialConnection.*    # QSerialPort 封装，负责底层连接/接收
    ├── SerialEndpointWidget.*# 串口调试面板（端口选择、发送、日志）
    └── main.cpp
```

## 构建与运行

1. 确保已经安装 Qt6（推荐 6.5+）并包含 `Widgets`、`SerialPort`、`Charts` 模块；也可以在 Qt5 环境下编译。
2. 在仓库根目录执行：

   ```bash
   cmake -S . -B build
   cmake --build build
   ./build/TWhostcomputer
   ```

3. 打开程序后，分别在“力矩传感器串口”和“电机串口”板块中选择端口、波特率，点击“连接”即可开始接收数据。

## 二次开发指南

- **协议解析**：修改 `DataProcessor::processTorqueFrame` / `processMotorFrame`，将占位解析替换为真实协议（例如校验、帧头帧尾、量程换算等）。
- **系数算法**：在 `DataProcessor::updateCoefficient` 中自由扩展滤波/优化策略，或将求解过程迁移到上位机以外的系统。
- **图表/显示**：`DataPlotWidget` 已提供追加与清空接口，可根据需要控制显示窗口长度、曲线样式等。
- **串口调试**：`SerialEndpointWidget` 自带 ASCII 与 16 进制发送模式，可作为后续扩展（例如保存日志、导入脚本） 的基础。

欢迎根据实际项目需求进行裁剪或增强。若需在无 GUI 环境下运行，可将核心逻辑抽离至独立的库，再封装不同前端。