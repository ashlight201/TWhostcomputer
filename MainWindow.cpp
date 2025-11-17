#include "MainWindow.h"
#include <QApplication>
#include <QDateTime>
#include <QMessageBox>
#include <QTextCursor>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_torquePortManager(new SerialPortManager(this))
    , m_motorPortManager(new SerialPortManager(this))
    , m_dataProcessor(new DataProcessor(this))
    , m_chart(new QChart())
    , m_chartView(new QChartView(m_chart))
    , m_torqueSeries(new QLineSeries())
    , m_currentSeries(new QLineSeries())
    , m_coefficientSeries(new QLineSeries())
    , m_axisY(new QValueAxis())
    , m_axisX(new QDateTimeAxis())
    , m_chartUpdateTimer(new QTimer(this))
    , m_calibrating(false)
{
    setupUI();
    setupChart();
    
    // 连接信号和槽
    connect(m_torquePortManager, &SerialPortManager::rawDataReceived,
            this, &MainWindow::onTorqueRawDataReceived);
    connect(m_torquePortManager, &SerialPortManager::errorOccurred,
            this, &MainWindow::onTorquePortError);
    connect(m_torquePortManager, &SerialPortManager::portStatusChanged,
            [this](bool isOpen) {
                m_torqueOpenBtn->setEnabled(!isOpen);
                m_torqueCloseBtn->setEnabled(isOpen);
            });

    connect(m_motorPortManager, &SerialPortManager::rawDataReceived,
            this, &MainWindow::onMotorRawDataReceived);
    connect(m_motorPortManager, &SerialPortManager::errorOccurred,
            this, &MainWindow::onMotorPortError);
    connect(m_motorPortManager, &SerialPortManager::portStatusChanged,
            [this](bool isOpen) {
                m_motorOpenBtn->setEnabled(!isOpen);
                m_motorCloseBtn->setEnabled(isOpen);
            });

    connect(m_dataProcessor, &DataProcessor::torqueDataProcessed,
            this, &MainWindow::onTorqueProcessed);
    connect(m_dataProcessor, &DataProcessor::currentDataProcessed,
            this, &MainWindow::onCurrentProcessed);
    connect(m_dataProcessor, &DataProcessor::coefficientCalculated,
            this, &MainWindow::onCoefficientCalculated);

    connect(m_chartUpdateTimer, &QTimer::timeout, this, &MainWindow::updateChart);
    m_chartUpdateTimer->start(100); // 每100ms更新一次图表

    refreshPortList();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("电机关节力矩标定系统");
    setMinimumSize(1200, 800);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 创建分割器
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    QSplitter *leftSplitter = new QSplitter(Qt::Vertical, this);
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, this);

    // === 左侧：串口配置和数据显示 ===
    
    // 力矩传感器串口组
    m_torquePortGroup = new QGroupBox("力矩传感器串口", this);
    QGridLayout *torqueLayout = new QGridLayout(m_torquePortGroup);
    
    torqueLayout->addWidget(new QLabel("串口:"), 0, 0);
    m_torquePortCombo = new QComboBox(this);
    torqueLayout->addWidget(m_torquePortCombo, 0, 1);
    
    torqueLayout->addWidget(new QLabel("波特率:"), 0, 2);
    m_torqueBaudCombo = new QComboBox(this);
    m_torqueBaudCombo->addItems({"9600", "19200", "38400", "57600", "115200"});
    m_torqueBaudCombo->setCurrentText("115200");
    torqueLayout->addWidget(m_torqueBaudCombo, 0, 3);
    
    m_torqueOpenBtn = new QPushButton("打开", this);
    m_torqueCloseBtn = new QPushButton("关闭", this);
    m_torqueCloseBtn->setEnabled(false);
    torqueLayout->addWidget(m_torqueOpenBtn, 0, 4);
    torqueLayout->addWidget(m_torqueCloseBtn, 0, 5);
    
    connect(m_torqueOpenBtn, &QPushButton::clicked, this, &MainWindow::onTorquePortOpen);
    connect(m_torqueCloseBtn, &QPushButton::clicked, this, &MainWindow::onTorquePortClose);
    
    torqueLayout->addWidget(new QLabel("原始数据:"), 1, 0, 1, 6);
    m_torqueRawDisplay = new QTextEdit(this);
    m_torqueRawDisplay->setMaximumHeight(100);
    m_torqueRawDisplay->setReadOnly(true);
    torqueLayout->addWidget(m_torqueRawDisplay, 2, 0, 1, 6);
    
    torqueLayout->addWidget(new QLabel("处理后数据:"), 3, 0, 1, 6);
    m_torqueProcessedDisplay = new QTextEdit(this);
    m_torqueProcessedDisplay->setMaximumHeight(80);
    m_torqueProcessedDisplay->setReadOnly(true);
    torqueLayout->addWidget(m_torqueProcessedDisplay, 4, 0, 1, 6);
    
    torqueLayout->addWidget(new QLabel("发送数据:"), 5, 0);
    m_torqueSendEdit = new QTextEdit(this);
    m_torqueSendEdit->setMaximumHeight(50);
    torqueLayout->addWidget(m_torqueSendEdit, 5, 1, 1, 4);
    QPushButton *torqueSendBtn = new QPushButton("发送", this);
    connect(torqueSendBtn, &QPushButton::clicked, this, &MainWindow::onTorquePortSend);
    torqueLayout->addWidget(torqueSendBtn, 5, 5);

    // 电机串口组
    m_motorPortGroup = new QGroupBox("电机串口", this);
    QGridLayout *motorLayout = new QGridLayout(m_motorPortGroup);
    
    motorLayout->addWidget(new QLabel("串口:"), 0, 0);
    m_motorPortCombo = new QComboBox(this);
    motorLayout->addWidget(m_motorPortCombo, 0, 1);
    
    motorLayout->addWidget(new QLabel("波特率:"), 0, 2);
    m_motorBaudCombo = new QComboBox(this);
    m_motorBaudCombo->addItems({"9600", "19200", "38400", "57600", "115200"});
    m_motorBaudCombo->setCurrentText("115200");
    motorLayout->addWidget(m_motorBaudCombo, 0, 3);
    
    m_motorOpenBtn = new QPushButton("打开", this);
    m_motorCloseBtn = new QPushButton("关闭", this);
    m_motorCloseBtn->setEnabled(false);
    motorLayout->addWidget(m_motorOpenBtn, 0, 4);
    motorLayout->addWidget(m_motorCloseBtn, 0, 5);
    
    connect(m_motorOpenBtn, &QPushButton::clicked, this, &MainWindow::onMotorPortOpen);
    connect(m_motorCloseBtn, &QPushButton::clicked, this, &MainWindow::onMotorPortClose);
    
    motorLayout->addWidget(new QLabel("原始数据:"), 1, 0, 1, 6);
    m_motorRawDisplay = new QTextEdit(this);
    m_motorRawDisplay->setMaximumHeight(100);
    m_motorRawDisplay->setReadOnly(true);
    motorLayout->addWidget(m_motorRawDisplay, 2, 0, 1, 6);
    
    motorLayout->addWidget(new QLabel("处理后数据:"), 3, 0, 1, 6);
    m_motorProcessedDisplay = new QTextEdit(this);
    m_motorProcessedDisplay->setMaximumHeight(80);
    m_motorProcessedDisplay->setReadOnly(true);
    motorLayout->addWidget(m_motorProcessedDisplay, 4, 0, 1, 6);
    
    motorLayout->addWidget(new QLabel("发送数据:"), 5, 0);
    m_motorSendEdit = new QTextEdit(this);
    m_motorSendEdit->setMaximumHeight(50);
    motorLayout->addWidget(m_motorSendEdit, 5, 1, 1, 4);
    QPushButton *motorSendBtn = new QPushButton("发送", this);
    connect(motorSendBtn, &QPushButton::clicked, this, &MainWindow::onMotorPortSend);
    motorLayout->addWidget(motorSendBtn, 5, 5);

    // 数据显示组
    m_dataDisplayGroup = new QGroupBox("实时数据", this);
    QGridLayout *dataLayout = new QGridLayout(m_dataDisplayGroup);
    
    dataLayout->addWidget(new QLabel("力矩值 (Nm):"), 0, 0);
    m_torqueValueLabel = new QLabel("0.000", this);
    m_torqueValueLabel->setStyleSheet("font-size: 18pt; font-weight: bold;");
    dataLayout->addWidget(m_torqueValueLabel, 0, 1);
    
    dataLayout->addWidget(new QLabel("电流值 (A):"), 1, 0);
    m_currentValueLabel = new QLabel("0.000", this);
    m_currentValueLabel->setStyleSheet("font-size: 18pt; font-weight: bold;");
    dataLayout->addWidget(m_currentValueLabel, 1, 1);
    
    dataLayout->addWidget(new QLabel("标定系数:"), 2, 0);
    m_coefficientLabel = new QLabel("0.000", this);
    m_coefficientLabel->setStyleSheet("font-size: 18pt; font-weight: bold; color: red;");
    dataLayout->addWidget(m_coefficientLabel, 2, 1);

    // 标定控制组
    m_calibrationGroup = new QGroupBox("标定控制", this);
    QHBoxLayout *calibrationLayout = new QHBoxLayout(m_calibrationGroup);
    m_startCalibrationBtn = new QPushButton("开始标定", this);
    m_stopCalibrationBtn = new QPushButton("停止标定", this);
    m_stopCalibrationBtn->setEnabled(false);
    m_clearDataBtn = new QPushButton("清空数据", this);
    calibrationLayout->addWidget(m_startCalibrationBtn);
    calibrationLayout->addWidget(m_stopCalibrationBtn);
    calibrationLayout->addWidget(m_clearDataBtn);
    
    connect(m_startCalibrationBtn, &QPushButton::clicked, this, &MainWindow::onStartCalibration);
    connect(m_stopCalibrationBtn, &QPushButton::clicked, this, &MainWindow::onStopCalibration);
    connect(m_clearDataBtn, &QPushButton::clicked, this, &MainWindow::onClearData);

    leftSplitter->addWidget(m_torquePortGroup);
    leftSplitter->addWidget(m_motorPortGroup);
    leftSplitter->addWidget(m_dataDisplayGroup);
    leftSplitter->addWidget(m_calibrationGroup);
    leftSplitter->setStretchFactor(0, 1);
    leftSplitter->setStretchFactor(1, 1);
    leftSplitter->setStretchFactor(2, 0);
    leftSplitter->setStretchFactor(3, 0);

    // === 右侧：图表 ===
    rightSplitter->addWidget(m_chartView);
    rightSplitter->setStretchFactor(0, 1);

    mainSplitter->addWidget(leftSplitter);
    mainSplitter->addWidget(rightSplitter);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 2);

    mainLayout->addWidget(mainSplitter);
}

void MainWindow::setupChart()
{
    m_torqueSeries->setName("力矩 (Nm)");
    m_currentSeries->setName("电流 (A)");
    m_coefficientSeries->setName("系数");

    m_chart->addSeries(m_torqueSeries);
    m_chart->addSeries(m_currentSeries);
    m_chart->addSeries(m_coefficientSeries);

    m_axisX->setFormat("hh:mm:ss");
    m_axisX->setTitleText("时间");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_torqueSeries->attachAxis(m_axisX);
    m_currentSeries->attachAxis(m_axisX);
    m_coefficientSeries->attachAxis(m_axisX);

    m_axisY->setTitleText("数值");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_torqueSeries->attachAxis(m_axisY);
    m_currentSeries->attachAxis(m_axisY);
    m_coefficientSeries->attachAxis(m_axisY);

    m_chart->setTitle("实时数据曲线");
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
}

void MainWindow::refreshPortList()
{
    QStringList ports = SerialPortManager::getAvailablePorts();
    
    QString currentTorquePort = m_torquePortCombo->currentText();
    QString currentMotorPort = m_motorPortCombo->currentText();
    
    m_torquePortCombo->clear();
    m_motorPortCombo->clear();
    
    m_torquePortCombo->addItems(ports);
    m_motorPortCombo->addItems(ports);
    
    int index = m_torquePortCombo->findText(currentTorquePort);
    if (index >= 0) m_torquePortCombo->setCurrentIndex(index);
    
    index = m_motorPortCombo->findText(currentMotorPort);
    if (index >= 0) m_motorPortCombo->setCurrentIndex(index);
}

void MainWindow::onTorquePortOpen()
{
    QString portName = m_torquePortCombo->currentText();
    if (portName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择串口！");
        return;
    }
    
    qint32 baudRate = m_torqueBaudCombo->currentText().toInt();
    if (m_torquePortManager->openPort(portName, baudRate)) {
        QMessageBox::information(this, "成功", "力矩传感器串口打开成功！");
    } else {
        QMessageBox::critical(this, "错误", "串口打开失败！");
    }
}

void MainWindow::onTorquePortClose()
{
    m_torquePortManager->closePort();
}

void MainWindow::onTorquePortSend()
{
    QString text = m_torqueSendEdit->toPlainText();
    QByteArray data = text.toUtf8();
    if (!m_torquePortManager->writeData(data)) {
        QMessageBox::warning(this, "警告", "数据发送失败！");
    }
}

void MainWindow::onTorqueRawDataReceived(const QByteArray &data)
{
    QString displayText = QString::fromUtf8(data);
    m_torqueRawDisplay->append(displayText);
    
    // 限制显示行数
    if (m_torqueRawDisplay->document()->blockCount() > 100) {
        QTextCursor cursor = m_torqueRawDisplay->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }
    
    // 处理数据
    double torque = m_dataProcessor->processTorqueSensorData(data);
}

void MainWindow::onTorquePortError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        QMessageBox::critical(this, "串口错误", 
                             QString("力矩传感器串口错误: %1").arg(error));
    }
}

void MainWindow::onMotorPortOpen()
{
    QString portName = m_motorPortCombo->currentText();
    if (portName.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择串口！");
        return;
    }
    
    qint32 baudRate = m_motorBaudCombo->currentText().toInt();
    if (m_motorPortManager->openPort(portName, baudRate)) {
        QMessageBox::information(this, "成功", "电机串口打开成功！");
    } else {
        QMessageBox::critical(this, "错误", "串口打开失败！");
    }
}

void MainWindow::onMotorPortClose()
{
    m_motorPortManager->closePort();
}

void MainWindow::onMotorPortSend()
{
    QString text = m_motorSendEdit->toPlainText();
    QByteArray data = text.toUtf8();
    if (!m_motorPortManager->writeData(data)) {
        QMessageBox::warning(this, "警告", "数据发送失败！");
    }
}

void MainWindow::onMotorRawDataReceived(const QByteArray &data)
{
    QString displayText = QString::fromUtf8(data);
    m_motorRawDisplay->append(displayText);
    
    // 限制显示行数
    if (m_motorRawDisplay->document()->blockCount() > 100) {
        QTextCursor cursor = m_motorRawDisplay->textCursor();
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, 1);
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    }
    
    // 处理数据
    double current = m_dataProcessor->processMotorCurrentData(data);
}

void MainWindow::onMotorPortError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        QMessageBox::critical(this, "串口错误", 
                             QString("电机串口错误: %1").arg(error));
    }
}

void MainWindow::onTorqueProcessed(double torque)
{
    m_torqueProcessedDisplay->setPlainText(QString::number(torque, 'f', 3));
    m_torqueValueLabel->setText(QString::number(torque, 'f', 3));
    
    // 保存数据用于图表
    m_torqueValues.append(torque);
    m_timeStamps.append(QDateTime::currentDateTime());
    
    // 如果正在标定，保存数据
    if (m_calibrating) {
        m_calibrationTorques.append(torque);
    }
    
    // 计算系数
    if (!m_currentValues.isEmpty()) {
        double current = m_currentValues.last();
        double coefficient = m_dataProcessor->calculateCalibrationCoefficient(torque, current);
        m_coefficientValues.append(coefficient);
    }
}

void MainWindow::onCurrentProcessed(double current)
{
    m_motorProcessedDisplay->setPlainText(QString::number(current, 'f', 3));
    m_currentValueLabel->setText(QString::number(current, 'f', 3));
    
    // 保存数据用于图表
    m_currentValues.append(current);
    
    // 如果正在标定，保存数据
    if (m_calibrating) {
        m_calibrationCurrents.append(current);
    }
    
    // 计算系数
    if (!m_torqueValues.isEmpty()) {
        double torque = m_torqueValues.last();
        double coefficient = m_dataProcessor->calculateCalibrationCoefficient(torque, current);
        m_coefficientValues.append(coefficient);
    }
}

void MainWindow::onCoefficientCalculated(double coefficient)
{
    updateCoefficientDisplay();
}

void MainWindow::updateCoefficientDisplay()
{
    if (!m_torqueValues.isEmpty() && !m_currentValues.isEmpty()) {
        double torque = m_torqueValues.last();
        double current = m_currentValues.last();
        if (qAbs(current) > 1e-6) {
            double coefficient = torque / current;
            m_coefficientLabel->setText(QString::number(coefficient, 'f', 6));
        }
    }
}

void MainWindow::updateChart()
{
    QDateTime now = QDateTime::currentDateTime();
    
    // 更新数据序列（只保留最近1000个点）
    const int maxPoints = 1000;
    
    if (m_torqueValues.size() > maxPoints) {
        m_torqueValues.removeFirst();
        m_currentValues.removeFirst();
        m_coefficientValues.removeFirst();
        m_timeStamps.removeFirst();
    }
    
    // 更新图表
    if (!m_torqueValues.isEmpty() && !m_timeStamps.isEmpty()) {
        m_torqueSeries->clear();
        m_currentSeries->clear();
        m_coefficientSeries->clear();
        
        for (int i = 0; i < m_torqueValues.size(); ++i) {
            qint64 timestamp = m_timeStamps[i].toMSecsSinceEpoch();
            m_torqueSeries->append(timestamp, m_torqueValues[i]);
            if (i < m_currentValues.size()) {
                m_currentSeries->append(timestamp, m_currentValues[i]);
            }
            if (i < m_coefficientValues.size()) {
                m_coefficientSeries->append(timestamp, m_coefficientValues[i]);
            }
        }
        
        // 更新坐标轴范围
        if (!m_timeStamps.isEmpty()) {
            m_axisX->setRange(m_timeStamps.first(), m_timeStamps.last());
        }
        
        double minY = 0, maxY = 0;
        for (double v : m_torqueValues) {
            if (v < minY) minY = v;
            if (v > maxY) maxY = v;
        }
        for (double v : m_currentValues) {
            if (v < minY) minY = v;
            if (v > maxY) maxY = v;
        }
        for (double v : m_coefficientValues) {
            if (v < minY) minY = v;
            if (v > maxY) maxY = v;
        }
        
        if (maxY > minY) {
            m_axisY->setRange(minY - (maxY - minY) * 0.1, maxY + (maxY - minY) * 0.1);
        }
    }
}

void MainWindow::onStartCalibration()
{
    m_calibrating = true;
    m_calibrationTorques.clear();
    m_calibrationCurrents.clear();
    m_startCalibrationBtn->setEnabled(false);
    m_stopCalibrationBtn->setEnabled(true);
    QMessageBox::information(this, "提示", "标定已开始，请采集数据点...");
}

void MainWindow::onStopCalibration()
{
    m_calibrating = false;
    m_startCalibrationBtn->setEnabled(true);
    m_stopCalibrationBtn->setEnabled(false);
    
    if (m_calibrationTorques.size() > 0 && m_calibrationTorques.size() == m_calibrationCurrents.size()) {
        double coefficient = m_dataProcessor->calculateCalibrationCoefficient(
            m_calibrationTorques, m_calibrationCurrents);
        m_coefficientLabel->setText(QString::number(coefficient, 'f', 6));
        QMessageBox::information(this, "标定完成", 
                                 QString("标定系数: %1\n数据点数: %2")
                                 .arg(coefficient, 0, 'f', 6)
                                 .arg(m_calibrationTorques.size()));
    } else {
        QMessageBox::warning(this, "警告", "标定数据不完整！");
    }
}

void MainWindow::onClearData()
{
    m_torqueValues.clear();
    m_currentValues.clear();
    m_coefficientValues.clear();
    m_timeStamps.clear();
    m_torqueSeries->clear();
    m_currentSeries->clear();
    m_coefficientSeries->clear();
    m_torqueRawDisplay->clear();
    m_motorRawDisplay->clear();
    m_torqueProcessedDisplay->clear();
    m_motorProcessedDisplay->clear();
    m_torqueValueLabel->setText("0.000");
    m_currentValueLabel->setText("0.000");
    m_coefficientLabel->setText("0.000");
}
