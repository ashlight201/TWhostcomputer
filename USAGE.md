# 使用说明

## 自定义数据处理函数

### 力矩传感器数据处理

默认的数据处理函数在 `DataProcessor::defaultTorqueProcessor()` 中实现。您可以根据实际的力矩传感器通信协议来自定义处理函数。

#### 示例1：解析固定格式的浮点数

假设力矩传感器返回的数据格式为：前4个字节是IEEE 754单精度浮点数（小端序）

```cpp
// 在 MainWindow 构造函数或初始化函数中添加
dataProcessor->setTorqueSensorProcessor([](const QByteArray &data) -> double {
    if (data.size() >= 4) {
        union {
            float f;
            quint8 bytes[4];
        } converter;
        
        converter.bytes[0] = static_cast<quint8>(data[0]);
        converter.bytes[1] = static_cast<quint8>(data[1]);
        converter.bytes[2] = static_cast<quint8>(data[2]);
        converter.bytes[3] = static_cast<quint8>(data[3]);
        
        return static_cast<double>(converter.f);
    }
    return 0.0;
});
```

#### 示例2：解析Modbus RTU格式

假设力矩传感器使用Modbus RTU协议，力矩值在寄存器中：

```cpp
dataProcessor->setTorqueSensorProcessor([](const QByteArray &data) -> double {
    // Modbus RTU格式：设备地址(1字节) + 功能码(1字节) + 数据长度(1字节) + 数据(N字节) + CRC(2字节)
    if (data.size() >= 6) {
        // 假设数据从第3个字节开始，长度为2字节（16位整数）
        quint16 rawValue = (static_cast<quint8>(data[3]) << 8) | static_cast<quint8>(data[4]);
        
        // 根据传感器规格转换为实际力矩值
        // 例如：如果传感器量程是±100N·m，输出范围是0-65535
        double torque = (static_cast<int16_t>(rawValue) / 32768.0) * 100.0;
        
        return torque;
    }
    return 0.0;
});
```

#### 示例3：解析自定义协议

假设协议格式为：帧头(0xAA 0x55) + 数据长度(1字节) + 力矩值(4字节，大端序) + 校验和(1字节)

```cpp
dataProcessor->setTorqueSensorProcessor([](const QByteArray &data) -> double {
    // 查找帧头
    int frameStart = -1;
    for (int i = 0; i < data.size() - 1; ++i) {
        if (static_cast<quint8>(data[i]) == 0xAA && 
            static_cast<quint8>(data[i+1]) == 0x55) {
            frameStart = i;
            break;
        }
    }
    
    if (frameStart >= 0 && data.size() >= frameStart + 8) {
        // 跳过帧头(2字节)和数据长度(1字节)
        int dataOffset = frameStart + 3;
        
        // 读取4字节大端序浮点数
        union {
            float f;
            quint8 bytes[4];
        } converter;
        
        converter.bytes[0] = static_cast<quint8>(data[dataOffset + 3]);
        converter.bytes[1] = static_cast<quint8>(data[dataOffset + 2]);
        converter.bytes[2] = static_cast<quint8>(data[dataOffset + 1]);
        converter.bytes[3] = static_cast<quint8>(data[dataOffset]);
        
        return static_cast<double>(converter.f);
    }
    
    return 0.0;
});
```

### 电机数据处理

电机的数据处理函数设置方式相同，只需使用 `setMotorProcessor()` 方法：

```cpp
dataProcessor->setMotorProcessor([](const QByteArray &data) -> double {
    // 在这里实现您的电机电流数据解析逻辑
    // 返回电流值（单位：安培 A）
    return currentValue;
});
```

## 标定流程

1. **连接设备**
   - 连接力矩传感器串口
   - 连接电机串口

2. **数据采集**
   - 系统会自动接收并处理数据
   - 确保数据正常显示在界面上

3. **开始标定**
   - 点击"开始标定"按钮
   - 系统会使用已采集的所有数据点计算标定系数

4. **验证标定结果**
   - 查看标定系数值
   - 观察图表中的系数曲线
   - 可以多次标定以验证一致性

## 注意事项

1. **数据同步**：确保力矩和电流数据是同步采集的，即同一时刻的力矩和电流值对应

2. **数据量**：标定需要至少2个数据点，但建议采集更多数据点（10个以上）以获得更准确的标定结果

3. **数据范围**：尽量覆盖整个工作范围的数据，包括正负力矩和不同大小的电流值

4. **数据质量**：确保采集的数据稳定可靠，避免异常值影响标定结果

5. **实时更新**：如果需要实时更新标定系数，可以在 `MainWindow::updateCalibrationCoefficient()` 中实现自动标定逻辑

## 高级功能扩展

### 自动标定

可以在 `MainWindow::updateCalibrationCoefficient()` 中实现自动标定：

```cpp
void MainWindow::updateCalibrationCoefficient()
{
    // 每收集到一定数量的数据点就自动标定
    if (torqueValues.size() >= 10 && torqueValues.size() % 10 == 0) {
        int minSize = qMin(torqueValues.size(), currentValues.size());
        QVector<double> torques = torqueValues.mid(0, minSize);
        QVector<double> currents = currentValues.mid(0, minSize);
        
        double coefficient = dataProcessor->calculateCalibrationCoefficient(torques, currents);
        onCalibrationCoefficientCalculated(coefficient);
    }
}
```

### 数据保存和加载

可以添加数据保存功能，将采集的数据和标定系数保存到文件：

```cpp
// 保存数据
QJsonObject json;
QJsonArray torqueArray, currentArray;
for (int i = 0; i < torqueValues.size(); ++i) {
    torqueArray.append(torqueValues[i]);
    if (i < currentValues.size()) {
        currentArray.append(currentValues[i]);
    }
}
json["torques"] = torqueArray;
json["currents"] = currentArray;
// ... 保存到文件
```
