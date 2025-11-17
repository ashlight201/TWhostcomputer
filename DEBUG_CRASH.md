# 程序崩溃调试指南

## 你的情况分析

**环境信息：**
- Qt版本：5.12.0
- 编译器：MinGW 64-bit
- 构建类型：Debug
- 现象：程序启动3秒后崩溃

## 可能的原因和解决方案

### 原因1：Qt 5.12 中 Qt Charts 命名空间问题（最可能）

Qt 5.x 和 Qt 6.x 在 Charts 模块的使用上有差异。

**解决方案：** 修改代码以兼容 Qt 5.12

需要修改 `src/mainwindow.h` 文件。

**当前代码（Qt 6风格）：**
```cpp
#include <QtCharts>
QT_CHARTS_USE_NAMESPACE
```

**应改为（Qt 5兼容）：**
```cpp
#include <QtCharts/QtCharts>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE
```

我会立即为你创建修复版本。

---

### 原因2：缺少Qt Charts模块

**检查方法：**

打开命令提示符，检查是否安装了Qt Charts：
```cmd
dir C:\Qt\5.12.0\mingw73_64\include\QtCharts
dir C:\Qt\5.12.0\mingw73_64\lib\libQt5Charts*
```

如果显示"找不到文件"，说明没有安装Qt Charts。

**解决方案：**

运行Qt维护工具添加Charts模块：
1. 打开 `C:\Qt\MaintenanceTool.exe`
2. 选择 "Add or remove components"
3. 找到 Qt 5.12.0 → Additional Libraries
4. 勾选 ✅ Qt Charts
5. 点击 Update

---

### 原因3：缺少运行时DLL

**解决方案：**

在构建目录中部署Qt依赖：
```cmd
cd E:\05_forReferenceCode\TW\build-TWcurrent-Desktop_Qt_5_12_0_MinGW_64_bit-Debug\debug

C:\Qt\5.12.0\mingw73_64\bin\windeployqt.exe TWcurrent.exe
```

---

### 原因4：代码中的空指针或初始化错误

**调试方法：**

#### 方法A：使用Qt Creator的调试器

1. 在Qt Creator中打开项目
2. 点击"调试"按钮（F5）而不是"运行"
3. 程序崩溃时会停在崩溃位置
4. 查看调用栈和变量值

#### 方法B：添加调试输出

在 `main.cpp` 中添加：
```cpp
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    qDebug() << "程序启动...";
    
    QApplication app(argc, argv);
    qDebug() << "QApplication创建成功";
    
    try {
        MainWindow window;
        qDebug() << "MainWindow创建成功";
        
        window.setWindowTitle("电机力矩标定系统 v1.0");
        window.resize(1400, 900);
        qDebug() << "窗口设置完成";
        
        window.show();
        qDebug() << "窗口显示成功";
        
        return app.exec();
    } catch (const std::exception &e) {
        QMessageBox::critical(nullptr, "错误", 
            QString("程序异常: %1").arg(e.what()));
        return -1;
    }
}
```

#### 方法C：注释掉图表初始化（快速定位）

临时注释掉图表相关代码，看是否是图表导致的崩溃：

在 `mainwindow.cpp` 的构造函数中：
```cpp
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    // , m_updateTimer(new QTimer(this))  // 先注释掉
    , m_dataPointCounter(0)
{
    initUI();
    // initCharts();  // 先注释掉图表初始化
    
    // 注释掉图表相关的连接
    // connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateCharts);
    // m_updateTimer->start(100);
}
```

---

## 立即尝试的步骤

### 步骤1：快速修复（5分钟）

我会立即为你创建一个Qt 5.12兼容的版本。

### 步骤2：部署DLL（1分钟）

```cmd
cd E:\05_forReferenceCode\TW\build-TWcurrent-Desktop_Qt_5_12_0_MinGW_64_bit-Debug\debug
C:\Qt\5.12.0\mingw73_64\bin\windeployqt.exe --debug TWcurrent.exe
```

### 步骤3：使用调试器运行（2分钟）

1. 在Qt Creator中点击"调试"按钮（虫子图标）
2. 等待程序崩溃
3. 查看"应用程序输出"和"调试器"窗口
4. 截图并告诉我错误信息

---

## 调试信息收集

如果上述方法都不行，请提供以下信息：

### 1. 检查Qt Charts是否存在
```cmd
dir C:\Qt\5.12.0\mingw73_64\include\QtCharts
```

### 2. 查看应用程序输出
在Qt Creator底部的"应用程序输出"窗口，复制所有输出信息。

### 3. 使用调试器
按F5调试运行，崩溃时查看调用栈（Call Stack）。

### 4. 检查依赖DLL
使用 Dependency Walker 或运行：
```cmd
dumpbin /dependents TWcurrent.exe
```

---

## 紧急方案：使用无图表版本

如果Qt Charts一直有问题，我可以立即为你创建一个不使用图表的精简版本：

**精简版功能：**
- ✅ 双串口通信
- ✅ 数据接收和显示
- ✅ 数据处理
- ✅ 标定计算
- ✅ 文本形式的数据记录
- ❌ 实时图表（移除）

这个版本不依赖Qt Charts，可以立即运行。

---

## 下一步

请告诉我：
1. 按F5调试运行后的错误信息
2. 或者需要我创建Qt 5.12兼容版本
3. 或者需要无图表的精简版本

我会立即帮你解决！
