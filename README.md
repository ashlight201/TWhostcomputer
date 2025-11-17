# 电机关节力矩标定上位机

## 功能概览
- Qt Widgets 上位机界面，包含力矩传感器、电机两路串口调试面板。
- 原始数据帧实时滚动显示 + 滤波后的数值展示。
- 实时求解 `力矩 = k × 电流` 的系数 `k`，并在 LCD 以及状态栏提示。
- QtCharts 曲线实时绘制力矩、电流、系数三条曲线，可一键清空。
- 数据处理、串口帧解析接口模块化封装，便于后续替换实际协议及算法。

## 构建 & 运行
```bash
cmake -S . -B build
cmake --build build
./build/TWhostcomputer
```

> 需要安装 Qt5.15+ 或 Qt6（Widgets、SerialPort、Charts 模块）。Qt6 优先，若未找到会自动回退到 Qt5。

## 后续扩展建议
- 根据实际力矩/电流帧格式，实现自定义 `SerialChannelManager::FrameSplitter` 与 `DataProcessor` 解析逻辑。
- 若需更复杂的标定算法，可在 `DataProcessor` 中补充窗口拟合/最小二乘等处理，并通过信号实时推送结果。