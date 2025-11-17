#include "MainWindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QScrollBar>
#include <QSplitter>
#include <QSerialPortInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , torqueSensorSerial(nullptr)
    , motorSerial(nullptr)
    , dataProcessor(nullptr)
{
    // 初始化串口管理器
    torqueSensorSerial = new SerialPortManager(this);
    motorSerial = new SerialPortManager(this);
    dataProcessor = new DataProcessor(this);
    
    setupUI();
    setupConnections();
    
    // 刷新可用串口列表
    QTimer::singleShot(100, [this]() {
        auto ports = QSerialPortInfo::availablePorts();
        for (const auto &port : ports) {
            torqueSensorPortCombo->addItem(port.portName());
            motorPortCombo->addItem(port.portName());
        }
    });
    
    // 设置定时器用于更新图表
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &MainWindow::updateCalibrationCoefficient);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建分割器
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    
    // 左侧：串口调试区域
    QWidget *leftWidget = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    
    // 力矩传感器串口调试组
    torqueSensorGroup = new QGroupBox("力矩传感器串口调试", this);
    QVBoxLayout *torqueSensorLayout = new QVBoxLayout;
    
    QHBoxLayout *torqueSensorConfigLayout = new QHBoxLayout;
    torqueSensorPortCombo = new QComboBox;
    torqueSensorBaudCombo = new QComboBox;
    torqueSensorBaudCombo->addItems({"9600", "19200", "38400", "57600", "115200"});
    torqueSensorBaudCombo->setCurrentText("115200");
    torqueSensorConnectBtn = new QPushButton("连接");
    torqueSensorDisconnectBtn = new QPushButton("断开");
    torqueSensorDisconnectBtn->setEnabled(false);
    torqueSensorConfigLayout->addWidget(new QLabel("端口:"));
    torqueSensorConfigLayout->addWidget(torqueSensorPortCombo);
    torqueSensorConfigLayout->addWidget(new QLabel("波特率:"));
    torqueSensorConfigLayout->addWidget(torqueSensorBaudCombo);
    torqueSensorConfigLayout->addWidget(torqueSensorConnectBtn);
    torqueSensorConfigLayout->addWidget(torqueSensorDisconnectBtn);
    torqueSensorConfigLayout->addStretch();
    
    torqueSensorRawData = new QTextEdit;
    torqueSensorRawData->setReadOnly(true);
    torqueSensorRawData->setMaximumHeight(150);
    torqueSensorRawData->setPlaceholderText("原始数据帧显示区域");
    
    torqueSensorProcessedData = new QTextEdit;
    torqueSensorProcessedData->setReadOnly(true);
    torqueSensorProcessedData->setMaximumHeight(100);
    torqueSensorProcessedData->setPlaceholderText("处理后的力矩数据显示");
    
    QHBoxLayout *torqueSensorSendLayout = new QHBoxLayout;
    torqueSensorSendEdit = new QLineEdit;
    torqueSensorSendEdit->setPlaceholderText("输入要发送的数据（十六进制，如: 01 02 03）");
    torqueSensorSendBtn = new QPushButton("发送");
    torqueSensorSendLayout->addWidget(torqueSensorSendEdit);
    torqueSensorSendLayout->addWidget(torqueSensorSendBtn);
    
    torqueSensorLayout->addLayout(torqueSensorConfigLayout);
    torqueSensorLayout->addWidget(new QLabel("原始数据帧:"));
    torqueSensorLayout->addWidget(torqueSensorRawData);
    torqueSensorLayout->addWidget(new QLabel("处理后数据:"));
    torqueSensorLayout->addWidget(torqueSensorProcessedData);
    torqueSensorLayout->addLayout(torqueSensorSendLayout);
    torqueSensorGroup->setLayout(torqueSensorLayout);
    
    // 电机串口调试组
    motorGroup = new QGroupBox("电机串口调试", this);
    QVBoxLayout *motorLayout = new QVBoxLayout;
    
    QHBoxLayout *motorConfigLayout = new QHBoxLayout;
    motorPortCombo = new QComboBox;
    motorBaudCombo = new QComboBox;
    motorBaudCombo->addItems({"9600", "19200", "38400", "57600", "115200"});
    motorBaudCombo->setCurrentText("115200");
    motorConnectBtn = new QPushButton("连接");
    motorDisconnectBtn = new QPushButton("断开");
    motorDisconnectBtn->setEnabled(false);
    motorConfigLayout->addWidget(new QLabel("端口:"));
    motorConfigLayout->addWidget(motorPortCombo);
    motorConfigLayout->addWidget(new QLabel("波特率:"));
    motorConfigLayout->addWidget(motorBaudCombo);
    motorConfigLayout->addWidget(motorConnectBtn);
    motorConfigLayout->addWidget(motorDisconnectBtn);
    motorConfigLayout->addStretch();
    
    motorRawData = new QTextEdit;
    motorRawData->setReadOnly(true);
    motorRawData->setMaximumHeight(150);
    motorRawData->setPlaceholderText("原始数据帧显示区域");
    
    motorProcessedData = new QTextEdit;
    motorProcessedData->setReadOnly(true);
    motorProcessedData->setMaximumHeight(100);
    motorProcessedData->setPlaceholderText("处理后的电流数据显示");
    
    QHBoxLayout *motorSendLayout = new QHBoxLayout;
    motorSendEdit = new QLineEdit;
    motorSendEdit->setPlaceholderText("输入要发送的数据（十六进制，如: 01 02 03）");
    motorSendBtn = new QPushButton("发送");
    motorSendLayout->addWidget(motorSendEdit);
    motorSendLayout->addWidget(motorSendBtn);
    
    motorLayout->addLayout(motorConfigLayout);
    motorLayout->addWidget(new QLabel("原始数据帧:"));
    motorLayout->addWidget(motorRawData);
    motorLayout->addWidget(new QLabel("处理后数据:"));
    motorLayout->addWidget(motorProcessedData);
    motorLayout->addLayout(motorSendLayout);
    motorGroup->setLayout(motorLayout);
    
    // 标定系数组
    calibrationGroup = new QGroupBox("标定系数", this);
    QVBoxLayout *calibrationLayout = new QVBoxLayout;
    coefficientLabel = new QLabel("标定系数:");
    coefficientValueLabel = new QLabel("未计算");
    coefficientValueLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: blue;");
    QHBoxLayout *calibrationBtnLayout = new QHBoxLayout;
    startCalibrationBtn = new QPushButton("开始标定");
    clearDataBtn = new QPushButton("清空数据");
    calibrationBtnLayout->addWidget(startCalibrationBtn);
    calibrationBtnLayout->addWidget(clearDataBtn);
    calibrationBtnLayout->addStretch();
    calibrationLayout->addWidget(coefficientLabel);
    calibrationLayout->addWidget(coefficientValueLabel);
    calibrationLayout->addLayout(calibrationBtnLayout);
    calibrationGroup->setLayout(calibrationLayout);
    
    leftLayout->addWidget(torqueSensorGroup);
    leftLayout->addWidget(motorGroup);
    leftLayout->addWidget(calibrationGroup);
    leftLayout->addStretch();
    
    // 右侧：实时图表
    chartWidget = new ChartWidget(this);
    chartWidget->setMinimumWidth(600);
    
    splitter->addWidget(leftWidget);
    splitter->addWidget(chartWidget);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    
    mainLayout->addWidget(splitter);
    
    setWindowTitle("电机关节力矩标定系统");
    resize(1400, 900);
}

void MainWindow::setupConnections()
{
    // 力矩传感器串口连接
    connect(torqueSensorConnectBtn, &QPushButton::clicked, this, &MainWindow::onTorqueSensorConnect);
    connect(torqueSensorDisconnectBtn, &QPushButton::clicked, this, &MainWindow::onTorqueSensorDisconnect);
    connect(torqueSensorSendBtn, &QPushButton::clicked, this, &MainWindow::onTorqueSensorSend);
    connect(torqueSensorSerial, &SerialPortManager::dataReceived, this, &MainWindow::onTorqueSensorDataReceived);
    
    // 电机串口连接
    connect(motorConnectBtn, &QPushButton::clicked, this, &MainWindow::onMotorConnect);
    connect(motorDisconnectBtn, &QPushButton::clicked, this, &MainWindow::onMotorDisconnect);
    connect(motorSendBtn, &QPushButton::clicked, this, &MainWindow::onMotorSend);
    connect(motorSerial, &SerialPortManager::dataReceived, this, &MainWindow::onMotorDataReceived);
    
    // 数据处理连接
    connect(dataProcessor, &DataProcessor::torqueProcessed, this, &MainWindow::onTorqueSensorProcessedData);
    connect(dataProcessor, &DataProcessor::currentProcessed, this, &MainWindow::onMotorProcessedData);
    connect(dataProcessor, &DataProcessor::coefficientCalculated, this, &MainWindow::onCalibrationCoefficientCalculated);
    
    // 标定按钮
    connect(startCalibrationBtn, &QPushButton::clicked, this, &MainWindow::onStartCalibration);
    connect(clearDataBtn, &QPushButton::clicked, this, &MainWindow::onClearData);
}

void MainWindow::onTorqueSensorDataReceived(const QByteArray &data)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString hexData = data.toHex(' ').toUpper();
    torqueSensorRawData->append(QString("[%1] %2").arg(timestamp).arg(hexData));
    
    // 自动滚动到底部
    QScrollBar *scrollBar = torqueSensorRawData->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    // 处理数据（会通过信号槽自动调用onTorqueSensorProcessedData）
    dataProcessor->processTorqueSensorData(data);
}

void MainWindow::onMotorDataReceived(const QByteArray &data)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString hexData = data.toHex(' ').toUpper();
    motorRawData->append(QString("[%1] %2").arg(timestamp).arg(hexData));
    
    // 自动滚动到底部
    QScrollBar *scrollBar = motorRawData->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    // 处理数据（会通过信号槽自动调用onMotorProcessedData）
    dataProcessor->processMotorData(data);
}

void MainWindow::onTorqueSensorProcessedData(double torque)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    torqueSensorProcessedData->append(QString("[%1] 力矩: %2 N·m").arg(timestamp).arg(torque, 0, 'f', 3));
    
    // 自动滚动到底部
    QScrollBar *scrollBar = torqueSensorProcessedData->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    // 添加到图表
    chartWidget->addTorqueData(torque, QDateTime::currentMSecsSinceEpoch());
    
    // 存储用于标定
    torqueValues.append(torque);
}

void MainWindow::onMotorProcessedData(double current)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    motorProcessedData->append(QString("[%1] 电流: %2 A").arg(timestamp).arg(current, 0, 'f', 3));
    
    // 自动滚动到底部
    QScrollBar *scrollBar = motorProcessedData->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
    
    // 添加到图表
    chartWidget->addCurrentData(current, QDateTime::currentMSecsSinceEpoch());
    
    // 存储用于标定
    currentValues.append(current);
}

void MainWindow::onCalibrationCoefficientCalculated(double coefficient)
{
    coefficientValueLabel->setText(QString::number(coefficient, 'f', 6));
    chartWidget->addCoefficientData(coefficient, QDateTime::currentMSecsSinceEpoch());
}

void MainWindow::onTorqueSensorConnect()
{
    QString portName = torqueSensorPortCombo->currentText();
    qint32 baudRate = torqueSensorBaudCombo->currentText().toInt();
    
    if (torqueSensorSerial->openPort(portName, baudRate)) {
        torqueSensorConnectBtn->setEnabled(false);
        torqueSensorDisconnectBtn->setEnabled(true);
        torqueSensorPortCombo->setEnabled(false);
        torqueSensorBaudCombo->setEnabled(false);
        QMessageBox::information(this, "成功", QString("力矩传感器串口 %1 连接成功").arg(portName));
    } else {
        QMessageBox::warning(this, "错误", QString("无法打开串口 %1").arg(portName));
    }
}

void MainWindow::onTorqueSensorDisconnect()
{
    torqueSensorSerial->closePort();
    torqueSensorConnectBtn->setEnabled(true);
    torqueSensorDisconnectBtn->setEnabled(false);
    torqueSensorPortCombo->setEnabled(true);
    torqueSensorBaudCombo->setEnabled(true);
    QMessageBox::information(this, "提示", "力矩传感器串口已断开");
}

void MainWindow::onMotorConnect()
{
    QString portName = motorPortCombo->currentText();
    qint32 baudRate = motorBaudCombo->currentText().toInt();
    
    if (motorSerial->openPort(portName, baudRate)) {
        motorConnectBtn->setEnabled(false);
        motorDisconnectBtn->setEnabled(true);
        motorPortCombo->setEnabled(false);
        motorBaudCombo->setEnabled(false);
        QMessageBox::information(this, "成功", QString("电机串口 %1 连接成功").arg(portName));
    } else {
        QMessageBox::warning(this, "错误", QString("无法打开串口 %1").arg(portName));
    }
}

void MainWindow::onMotorDisconnect()
{
    motorSerial->closePort();
    motorConnectBtn->setEnabled(true);
    motorDisconnectBtn->setEnabled(false);
    motorPortCombo->setEnabled(true);
    motorBaudCombo->setEnabled(true);
    QMessageBox::information(this, "提示", "电机串口已断开");
}

void MainWindow::onTorqueSensorSend()
{
    QString text = torqueSensorSendEdit->text();
    if (text.isEmpty()) {
        return;
    }
    
    // 解析十六进制字符串
    QByteArray data;
    QStringList hexBytes = text.split(' ', Qt::SkipEmptyParts);
    for (const QString &hex : hexBytes) {
        bool ok;
        quint8 byte = hex.toInt(&ok, 16);
        if (ok) {
            data.append(byte);
        }
    }
    
    if (!data.isEmpty() && torqueSensorSerial->sendData(data)) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        torqueSensorRawData->append(QString("[%1] 发送: %2").arg(timestamp).arg(text.toUpper()));
    } else {
        QMessageBox::warning(this, "错误", "发送数据失败");
    }
}

void MainWindow::onMotorSend()
{
    QString text = motorSendEdit->text();
    if (text.isEmpty()) {
        return;
    }
    
    // 解析十六进制字符串
    QByteArray data;
    QStringList hexBytes = text.split(' ', Qt::SkipEmptyParts);
    for (const QString &hex : hexBytes) {
        bool ok;
        quint8 byte = hex.toInt(&ok, 16);
        if (ok) {
            data.append(byte);
        }
    }
    
    if (!data.isEmpty() && motorSerial->sendData(data)) {
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        motorRawData->append(QString("[%1] 发送: %2").arg(timestamp).arg(text.toUpper()));
    } else {
        QMessageBox::warning(this, "错误", "发送数据失败");
    }
}

void MainWindow::onStartCalibration()
{
    if (torqueValues.size() < 2 || currentValues.size() < 2) {
        QMessageBox::warning(this, "警告", "数据点不足，至少需要2个数据点才能进行标定");
        return;
    }
    
    int minSize = qMin(torqueValues.size(), currentValues.size());
    QVector<double> torques = torqueValues.mid(0, minSize);
    QVector<double> currents = currentValues.mid(0, minSize);
    
    double coefficient = dataProcessor->calculateCalibrationCoefficient(torques, currents);
    onCalibrationCoefficientCalculated(coefficient);
    
    QMessageBox::information(this, "标定完成", 
        QString("标定系数: %1\n使用数据点: %2").arg(coefficient, 0, 'f', 6).arg(minSize));
}

void MainWindow::onClearData()
{
    torqueValues.clear();
    currentValues.clear();
    torqueSensorRawData->clear();
    torqueSensorProcessedData->clear();
    motorRawData->clear();
    motorProcessedData->clear();
    chartWidget->clearData();
    coefficientValueLabel->setText("未计算");
    QMessageBox::information(this, "提示", "数据已清空");
}

void MainWindow::updateCalibrationCoefficient()
{
    // 可以在这里实现自动更新标定系数的逻辑
}
