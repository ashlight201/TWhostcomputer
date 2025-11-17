# 快速入门指南

## 5分钟快速开始

### 1. 安装依赖（首次使用）

#### Ubuntu/Debian
```bash
# 安装Qt6（推荐）
sudo apt-get update
sudo apt-get install qt6-base-dev qt6-serialport-dev qt6-charts-dev cmake g++

# 或安装Qt5
sudo apt-get install qtbase5-dev libqt5serialport5-dev libqt5charts5-dev cmake g++
```

#### Fedora/RHEL
```bash
sudo dnf install qt6-qtbase-devel qt6-qtserialport-devel qt6-qtcharts-devel cmake gcc-c++
```

#### Arch Linux
```bash
sudo pacman -S qt6-base qt6-serialport qt6-charts cmake gcc
```

### 2. 编译项目

使用提供的构建脚本（最简单）：
```bash
./build.sh
```

或手动编译：
```bash
mkdir build && cd build
cmake ..
make -j4
```

### 3. 运行程序

```bash
./build/bin/MotorCalibration
```

### 4. 串口权限设置（Linux）

如果提示串口权限不足：
```bash
sudo usermod -a -G dialout $USER
# 注销并重新登录生效
```

## 使用流程

### 步骤1：连接硬件
1. 将力矩传感器通过USB转串口连接到电脑
2. 将电机控制器通过USB转串口连接到电脑
3. 确保设备供电正常

### 步骤2：打开串口
1. 点击"刷新"按钮，扫描可用串口
2. 分别为力矩传感器和电机选择正确的串口
3. 选择正确的波特率（通常为115200）
4. 点击"打开串口"按钮

### 步骤3：验证数据
1. 查看"原始数据"区域，确认有数据接收
2. 查看"处理后数据"，确认数值正常
3. 观察右侧的实时曲线图

### 步骤4：标定系数
1. 让电机在不同负载下运行
2. 每个稳定状态点击"添加标定点"
3. 至少采集5-10个不同负载的数据点
4. 点击"计算系数"得到标定结果

### 步骤5：验证结果
1. 查看标定系数值是否合理
2. 观察标定系数变化曲线是否收敛
3. 在标定日志中查看详细信息

## 数据协议定制

如果你的设备使用特殊的通信协议，需要修改数据解析函数：

### 修改文件：`src/dataprocessor.cpp`

#### 力矩传感器数据解析
找到函数 `parseTorqueRawData`：
```cpp
double DataProcessor::parseTorqueRawData(const QByteArray &data)
{
    // 示例1：假设数据是ASCII格式 "T:123.45\r\n"
    QString str = QString::fromLatin1(data);
    QStringList parts = str.split(':');
    if (parts.size() >= 2) {
        return parts[1].trimmed().toDouble();
    }
    
    // 示例2：假设数据是4字节float（小端）
    if (data.size() >= 4) {
        float value;
        memcpy(&value, data.constData(), sizeof(float));
        return static_cast<double>(value);
    }
    
    // 示例3：假设数据是2字节整数（需要转换）
    if (data.size() >= 2) {
        qint16 rawValue = qFromLittleEndian<qint16>(
            reinterpret_cast<const uchar*>(data.constData()));
        return rawValue / 100.0;  // 转换为实际单位
    }
    
    return 0.0;
}
```

#### 电机电流数据解析
找到函数 `parseCurrentRawData`，方法类似。

### 重新编译
```bash
cd build
make
```

## 常见问题排查

### 问题1：找不到串口
- 检查设备是否正确连接
- 运行 `ls /dev/ttyUSB*` 或 `ls /dev/ttyACM*` 查看串口设备
- 检查USB线缆是否正常

### 问题2：串口打开失败
- 确认串口没有被其他程序占用
- 检查用户权限：`groups` 命令确认是否在dialout组
- 尝试使用sudo运行（不推荐，应该添加权限）

### 问题3：接收不到数据
- 确认波特率设置正确
- 使用串口调试工具（如minicom）验证设备是否正常发送
- 检查设备是否需要特殊的初始化命令

### 问题4：数据显示异常
- 检查数据解析函数是否正确
- 在`dataprocessor.cpp`中添加调试输出
- 查看原始数据的十六进制格式

### 问题5：标定系数不合理
- 确保采集的数据点覆盖了整个工作范围
- 增加标定点数量（建议10个以上）
- 检查力矩和电流数据是否同步
- 排除异常数据点后重新计算

## 调试技巧

### 查看调试信息
程序运行时会在终端输出调试信息：
```bash
./build/bin/MotorCalibration 2>&1 | tee debug.log
```

### 测试数据解析
可以创建测试程序验证数据解析：
```cpp
// test_parser.cpp
#include "dataprocessor.h"
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    DataProcessor processor;
    
    // 测试数据
    QByteArray testData = QByteArray::fromHex("01 02 03 04");
    auto result = processor.processTorqueSensorData(testData);
    
    qDebug() << "Raw:" << result.rawValue;
    qDebug() << "Processed:" << result.processedValue;
    
    return 0;
}
```

## 进阶功能

### 1. 数据记录
可以添加数据记录功能，修改`mainwindow.cpp`：
```cpp
void MainWindow::onTorqueDataProcessed(double rawValue, double processedValue)
{
    // 原有代码...
    
    // 添加数据记录
    QFile logFile("data_log.csv");
    if (logFile.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&logFile);
        out << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")
            << "," << rawValue << "," << processedValue << "\n";
        logFile.close();
    }
}
```

### 2. 自动标定
可以实现自动采集标定点：
```cpp
void MainWindow::startAutoCalibration()
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this, timer]() {
        static int count = 0;
        if (count < 10) {
            onAddCalibrationPointClicked();
            count++;
        } else {
            timer->stop();
            onCalculateCoefficientClicked();
        }
    });
    timer->start(2000);  // 每2秒采集一次
}
```

### 3. 数据导出
导出标定结果到文件：
```cpp
void MainWindow::exportCalibrationData()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, "导出标定数据", "", "CSV Files (*.csv)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "力矩(Nm),电流(A)\n";
            // 写入数据...
            file.close();
        }
    }
}
```

## 技术支持

如有问题，请检查：
1. README.md - 完整文档
2. 源代码注释 - 详细说明
3. Qt官方文档 - https://doc.qt.io/

祝你使用愉快！ 🚀
