#include "mainwindow.h"
#include <QStatusBar>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QSplitter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_torqueSensorPort(new SerialPortHandler(this))
    , m_motorPort(new SerialPortHandler(this))
    , m_dataProcessor(new DataProcessor(this))
    , m_updateTimer(new QTimer(this))
    , m_dataPointCounter(0)
{
    initUI();
    initCharts();
    
    // 连接信号槽
    connect(m_torqueSensorPort, &SerialPortHandler::dataReceived,
            this, &MainWindow::onTorqueSensorDataReceived);
    connect(m_motorPort, &SerialPortHandler::dataReceived,
            this, &MainWindow::onMotorDataReceived);
    
    connect(m_dataProcessor, &DataProcessor::torqueDataProcessed,
            this, &MainWindow::onTorqueDataProcessed);
    connect(m_dataProcessor, &DataProcessor::currentDataProcessed,
            this, &MainWindow::onCurrentDataProcessed);
    connect(m_dataProcessor, &DataProcessor::coefficientUpdated,
            this, &MainWindow::onCoefficientUpdated);
    connect(m_dataProcessor, &DataProcessor::calibrationPointAdded,
            this, &MainWindow::onCalibrationPointAdded);
    
    // 图表更新定时器
    connect(m_updateTimer, &QTimer::timeout, this, &MainWindow::updateCharts);
    m_updateTimer->start(100); // 100ms更新一次图表
    
    // 初始化状态栏
    statusBar()->showMessage("就绪");
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    // 创建中心部件
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    
    // 左侧：控制面板
    QWidget *leftPanel = new QWidget();
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(10);
    
    setupTorqueSensorGroup();
    setupMotorGroup();
    setupCalibrationGroup();
    
    leftLayout->addWidget(m_torqueSensorGroup);
    leftLayout->addWidget(m_motorGroup);
    leftLayout->addWidget(m_calibrationGroup);
    leftLayout->addStretch();
    
    leftPanel->setMaximumWidth(450);
    
    // 右侧：图表显示
    QWidget *rightPanel = new QWidget();
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    
    setupChartsGroup();
    rightLayout->addWidget(m_chartsGroup);
    
    // 使用分隔器
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);
}

void MainWindow::setupTorqueSensorGroup()
{
    m_torqueSensorGroup = new QGroupBox("力矩传感器");
    QGridLayout *layout = new QGridLayout();
    
    // 串口选择
    layout->addWidget(new QLabel("串口:"), 0, 0);
    m_torquePortCombo = new QComboBox();
    layout->addWidget(m_torquePortCombo, 0, 1);
    
    m_torqueRefreshBtn = new QPushButton("刷新");
    layout->addWidget(m_torqueRefreshBtn, 0, 2);
    connect(m_torqueRefreshBtn, &QPushButton::clicked, this, &MainWindow::refreshPortList);
    
    // 波特率选择
    layout->addWidget(new QLabel("波特率:"), 1, 0);
    m_torqueBaudCombo = new QComboBox();
    m_torqueBaudCombo->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    m_torqueBaudCombo->setCurrentText("115200");
    layout->addWidget(m_torqueBaudCombo, 1, 1, 1, 2);
    
    // 打开/关闭按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_torqueOpenBtn = new QPushButton("打开串口");
    m_torqueCloseBtn = new QPushButton("关闭串口");
    m_torqueCloseBtn->setEnabled(false);
    btnLayout->addWidget(m_torqueOpenBtn);
    btnLayout->addWidget(m_torqueCloseBtn);
    layout->addLayout(btnLayout, 2, 0, 1, 3);
    
    connect(m_torqueOpenBtn, &QPushButton::clicked, 
            this, &MainWindow::onTorqueSensorPortOpenClicked);
    connect(m_torqueCloseBtn, &QPushButton::clicked, 
            this, &MainWindow::onTorqueSensorPortCloseClicked);
    
    // 状态标签
    m_torqueStatusLabel = new QLabel("未连接");
    m_torqueStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    layout->addWidget(m_torqueStatusLabel, 3, 0, 1, 3);
    
    // 原始数据显示
    layout->addWidget(new QLabel("原始数据:"), 4, 0, 1, 3);
    m_torqueRawDataText = new QTextEdit();
    m_torqueRawDataText->setMaximumHeight(80);
    m_torqueRawDataText->setReadOnly(true);
    layout->addWidget(m_torqueRawDataText, 5, 0, 1, 3);
    
    // 处理后数据显示
    layout->addWidget(new QLabel("处理后数据:"), 6, 0);
    m_torqueProcessedDataEdit = new QLineEdit();
    m_torqueProcessedDataEdit->setReadOnly(true);
    m_torqueProcessedDataEdit->setPlaceholderText("0.00 Nm");
    layout->addWidget(m_torqueProcessedDataEdit, 6, 1, 1, 2);
    
    m_torqueSensorGroup->setLayout(layout);
    
    // 初始化串口列表
    refreshPortList();
}

void MainWindow::setupMotorGroup()
{
    m_motorGroup = new QGroupBox("电机");
    QGridLayout *layout = new QGridLayout();
    
    // 串口选择
    layout->addWidget(new QLabel("串口:"), 0, 0);
    m_motorPortCombo = new QComboBox();
    layout->addWidget(m_motorPortCombo, 0, 1);
    
    m_motorRefreshBtn = new QPushButton("刷新");
    layout->addWidget(m_motorRefreshBtn, 0, 2);
    connect(m_motorRefreshBtn, &QPushButton::clicked, this, &MainWindow::refreshPortList);
    
    // 波特率选择
    layout->addWidget(new QLabel("波特率:"), 1, 0);
    m_motorBaudCombo = new QComboBox();
    m_motorBaudCombo->addItems({"9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"});
    m_motorBaudCombo->setCurrentText("115200");
    layout->addWidget(m_motorBaudCombo, 1, 1, 1, 2);
    
    // 打开/关闭按钮
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_motorOpenBtn = new QPushButton("打开串口");
    m_motorCloseBtn = new QPushButton("关闭串口");
    m_motorCloseBtn->setEnabled(false);
    btnLayout->addWidget(m_motorOpenBtn);
    btnLayout->addWidget(m_motorCloseBtn);
    layout->addLayout(btnLayout, 2, 0, 1, 3);
    
    connect(m_motorOpenBtn, &QPushButton::clicked, 
            this, &MainWindow::onMotorPortOpenClicked);
    connect(m_motorCloseBtn, &QPushButton::clicked, 
            this, &MainWindow::onMotorPortCloseClicked);
    
    // 状态标签
    m_motorStatusLabel = new QLabel("未连接");
    m_motorStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    layout->addWidget(m_motorStatusLabel, 3, 0, 1, 3);
    
    // 原始数据显示
    layout->addWidget(new QLabel("原始数据:"), 4, 0, 1, 3);
    m_motorRawDataText = new QTextEdit();
    m_motorRawDataText->setMaximumHeight(80);
    m_motorRawDataText->setReadOnly(true);
    layout->addWidget(m_motorRawDataText, 5, 0, 1, 3);
    
    // 处理后数据显示
    layout->addWidget(new QLabel("处理后数据:"), 6, 0);
    m_motorProcessedDataEdit = new QLineEdit();
    m_motorProcessedDataEdit->setReadOnly(true);
    m_motorProcessedDataEdit->setPlaceholderText("0.00 A");
    layout->addWidget(m_motorProcessedDataEdit, 6, 1, 1, 2);
    
    m_motorGroup->setLayout(layout);
}

void MainWindow::setupCalibrationGroup()
{
    m_calibrationGroup = new QGroupBox("标定控制");
    QGridLayout *layout = new QGridLayout();
    
    // 标定系数显示
    layout->addWidget(new QLabel("标定系数:"), 0, 0);
    m_coefficientEdit = new QLineEdit();
    m_coefficientEdit->setReadOnly(true);
    m_coefficientEdit->setText("1.000000");
    layout->addWidget(m_coefficientEdit, 0, 1);
    
    // 标定点数量
    layout->addWidget(new QLabel("标定点数:"), 1, 0);
    m_calibrationPointsEdit = new QLineEdit();
    m_calibrationPointsEdit->setReadOnly(true);
    m_calibrationPointsEdit->setText("0");
    layout->addWidget(m_calibrationPointsEdit, 1, 1);
    
    // 操作按钮
    m_addPointBtn = new QPushButton("添加标定点");
    m_calculateBtn = new QPushButton("计算系数");
    m_clearDataBtn = new QPushButton("清除数据");
    
    layout->addWidget(m_addPointBtn, 2, 0, 1, 2);
    layout->addWidget(m_calculateBtn, 3, 0, 1, 2);
    layout->addWidget(m_clearDataBtn, 4, 0, 1, 2);
    
    connect(m_addPointBtn, &QPushButton::clicked, 
            this, &MainWindow::onAddCalibrationPointClicked);
    connect(m_calculateBtn, &QPushButton::clicked, 
            this, &MainWindow::onCalculateCoefficientClicked);
    connect(m_clearDataBtn, &QPushButton::clicked, 
            this, &MainWindow::onClearCalibrationDataClicked);
    
    // 标定日志
    layout->addWidget(new QLabel("标定日志:"), 5, 0, 1, 2);
    m_calibrationLogText = new QTextEdit();
    m_calibrationLogText->setMaximumHeight(100);
    m_calibrationLogText->setReadOnly(true);
    layout->addWidget(m_calibrationLogText, 6, 0, 1, 2);
    
    m_calibrationGroup->setLayout(layout);
}

void MainWindow::setupChartsGroup()
{
    m_chartsGroup = new QGroupBox("实时数据曲线");
    QVBoxLayout *layout = new QVBoxLayout();
    
    // 创建图表视图
    m_torqueChartView = new QChartView();
    m_currentChartView = new QChartView();
    m_coefficientChartView = new QChartView();
    
    m_torqueChartView->setRenderHint(QPainter::Antialiasing);
    m_currentChartView->setRenderHint(QPainter::Antialiasing);
    m_coefficientChartView->setRenderHint(QPainter::Antialiasing);
    
    layout->addWidget(m_torqueChartView);
    layout->addWidget(m_currentChartView);
    layout->addWidget(m_coefficientChartView);
    
    m_chartsGroup->setLayout(layout);
}

void MainWindow::initCharts()
{
    // 力矩图表
    m_torqueChart = new QChart();
    m_torqueChart->setTitle("力矩传感器数据");
    m_torqueChart->setAnimationOptions(QChart::NoAnimation);
    
    m_torqueSeries = new QLineSeries();
    m_torqueSeries->setName("力矩 (Nm)");
    m_torqueChart->addSeries(m_torqueSeries);
    
    m_torqueAxisX = new QValueAxis();
    m_torqueAxisX->setTitleText("数据点");
    m_torqueAxisX->setRange(0, MAX_CHART_POINTS);
    m_torqueAxisY = new QValueAxis();
    m_torqueAxisY->setTitleText("力矩 (Nm)");
    m_torqueAxisY->setRange(-10, 10);
    
    m_torqueChart->addAxis(m_torqueAxisX, Qt::AlignBottom);
    m_torqueChart->addAxis(m_torqueAxisY, Qt::AlignLeft);
    m_torqueSeries->attachAxis(m_torqueAxisX);
    m_torqueSeries->attachAxis(m_torqueAxisY);
    
    m_torqueChartView->setChart(m_torqueChart);
    
    // 电流图表
    m_currentChart = new QChart();
    m_currentChart->setTitle("电机电流数据");
    m_currentChart->setAnimationOptions(QChart::NoAnimation);
    
    m_currentSeries = new QLineSeries();
    m_currentSeries->setName("电流 (A)");
    m_currentChart->addSeries(m_currentSeries);
    
    m_currentAxisX = new QValueAxis();
    m_currentAxisX->setTitleText("数据点");
    m_currentAxisX->setRange(0, MAX_CHART_POINTS);
    m_currentAxisY = new QValueAxis();
    m_currentAxisY->setTitleText("电流 (A)");
    m_currentAxisY->setRange(-10, 10);
    
    m_currentChart->addAxis(m_currentAxisX, Qt::AlignBottom);
    m_currentChart->addAxis(m_currentAxisY, Qt::AlignLeft);
    m_currentSeries->attachAxis(m_currentAxisX);
    m_currentSeries->attachAxis(m_currentAxisY);
    
    m_currentChartView->setChart(m_currentChart);
    
    // 标定系数图表
    m_coefficientChart = new QChart();
    m_coefficientChart->setTitle("标定系数变化");
    m_coefficientChart->setAnimationOptions(QChart::NoAnimation);
    
    m_coefficientSeries = new QLineSeries();
    m_coefficientSeries->setName("系数");
    m_coefficientChart->addSeries(m_coefficientSeries);
    
    m_coeffAxisX = new QValueAxis();
    m_coeffAxisX->setTitleText("计算次数");
    m_coeffAxisX->setRange(0, 50);
    m_coeffAxisY = new QValueAxis();
    m_coeffAxisY->setTitleText("系数值");
    m_coeffAxisY->setRange(0, 2);
    
    m_coefficientChart->addAxis(m_coeffAxisX, Qt::AlignBottom);
    m_coefficientChart->addAxis(m_coeffAxisY, Qt::AlignLeft);
    m_coefficientSeries->attachAxis(m_coeffAxisX);
    m_coefficientSeries->attachAxis(m_coeffAxisY);
    
    m_coefficientChartView->setChart(m_coefficientChart);
}

void MainWindow::onTorqueSensorPortOpenClicked()
{
    QString portName = m_torquePortCombo->currentText();
    int baudRate = m_torqueBaudCombo->currentText().toInt();
    
    if (portName.isEmpty()) {
        QMessageBox::warning(this, "错误", "请选择串口");
        return;
    }
    
    if (m_torqueSensorPort->openPort(portName, baudRate)) {
        m_torqueOpenBtn->setEnabled(false);
        m_torqueCloseBtn->setEnabled(true);
        m_torqueStatusLabel->setText("已连接");
        m_torqueStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        statusBar()->showMessage(QString("力矩传感器串口已打开: %1").arg(portName));
    } else {
        QMessageBox::critical(this, "错误", "无法打开串口");
    }
}

void MainWindow::onTorqueSensorPortCloseClicked()
{
    m_torqueSensorPort->closePort();
    m_torqueOpenBtn->setEnabled(true);
    m_torqueCloseBtn->setEnabled(false);
    m_torqueStatusLabel->setText("未连接");
    m_torqueStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    statusBar()->showMessage("力矩传感器串口已关闭");
}

void MainWindow::onTorqueSensorDataReceived(const QByteArray &data)
{
    // 显示原始数据（十六进制）
    QString hexStr = data.toHex(' ').toUpper();
    m_torqueRawDataText->append(QString("[%1] %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
        .arg(hexStr));
    
    // 自动滚动到底部
    m_torqueRawDataText->moveCursor(QTextCursor::End);
    
    // 处理数据
    m_dataProcessor->processTorqueSensorData(data);
}

void MainWindow::onMotorPortOpenClicked()
{
    QString portName = m_motorPortCombo->currentText();
    int baudRate = m_motorBaudCombo->currentText().toInt();
    
    if (portName.isEmpty()) {
        QMessageBox::warning(this, "错误", "请选择串口");
        return;
    }
    
    if (m_motorPort->openPort(portName, baudRate)) {
        m_motorOpenBtn->setEnabled(false);
        m_motorCloseBtn->setEnabled(true);
        m_motorStatusLabel->setText("已连接");
        m_motorStatusLabel->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        statusBar()->showMessage(QString("电机串口已打开: %1").arg(portName));
    } else {
        QMessageBox::critical(this, "错误", "无法打开串口");
    }
}

void MainWindow::onMotorPortCloseClicked()
{
    m_motorPort->closePort();
    m_motorOpenBtn->setEnabled(true);
    m_motorCloseBtn->setEnabled(false);
    m_motorStatusLabel->setText("未连接");
    m_motorStatusLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    statusBar()->showMessage("电机串口已关闭");
}

void MainWindow::onMotorDataReceived(const QByteArray &data)
{
    // 显示原始数据（十六进制）
    QString hexStr = data.toHex(' ').toUpper();
    m_motorRawDataText->append(QString("[%1] %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
        .arg(hexStr));
    
    // 自动滚动到底部
    m_motorRawDataText->moveCursor(QTextCursor::End);
    
    // 处理数据
    m_dataProcessor->processMotorCurrentData(data);
}

void MainWindow::onAddCalibrationPointClicked()
{
    double torque = m_dataProcessor->getLastTorque();
    double current = m_dataProcessor->getLastCurrent();
    
    if (qAbs(torque) < 1e-6 && qAbs(current) < 1e-6) {
        QMessageBox::warning(this, "警告", "当前数据无效，请确保已接收到有效的力矩和电流数据");
        return;
    }
    
    m_dataProcessor->addCalibrationPoint(torque, current);
    
    QString logMsg = QString("[%1] 添加标定点: 力矩=%.4f Nm, 电流=%.4f A")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(torque)
        .arg(current);
    m_calibrationLogText->append(logMsg);
    m_calibrationLogText->moveCursor(QTextCursor::End);
}

void MainWindow::onCalculateCoefficientClicked()
{
    int pointCount = m_dataProcessor->getCalibrationPointCount();
    
    if (pointCount < 2) {
        QMessageBox::warning(this, "警告", "标定点数量不足，至少需要2个数据点");
        return;
    }
    
    double coefficient = m_dataProcessor->calculateCalibrationCoefficient();
    
    QString logMsg = QString("[%1] 计算完成: 系数=%.6f (使用%2个数据点)")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"))
        .arg(coefficient)
        .arg(pointCount);
    m_calibrationLogText->append(logMsg);
    m_calibrationLogText->moveCursor(QTextCursor::End);
}

void MainWindow::onClearCalibrationDataClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认", "确定要清除所有标定数据吗？",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_dataProcessor->clearCalibrationData();
        m_calibrationPointsEdit->setText("0");
        m_coefficientPoints.clear();
        m_coefficientSeries->clear();
        
        QString logMsg = QString("[%1] 已清除所有标定数据")
            .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
        m_calibrationLogText->append(logMsg);
        m_calibrationLogText->moveCursor(QTextCursor::End);
    }
}

void MainWindow::onTorqueDataProcessed(double rawValue, double processedValue)
{
    m_torqueProcessedDataEdit->setText(QString::number(processedValue, 'f', 4) + " Nm");
    
    // 添加到图表数据
    m_torquePoints.append(QPointF(m_dataPointCounter, processedValue));
    
    // 限制数据点数量
    if (m_torquePoints.size() > MAX_CHART_POINTS) {
        m_torquePoints.removeFirst();
    }
}

void MainWindow::onCurrentDataProcessed(double rawValue, double processedValue)
{
    m_motorProcessedDataEdit->setText(QString::number(processedValue, 'f', 4) + " A");
    
    // 添加到图表数据
    m_currentPoints.append(QPointF(m_dataPointCounter, processedValue));
    m_dataPointCounter++;
    
    // 限制数据点数量
    if (m_currentPoints.size() > MAX_CHART_POINTS) {
        m_currentPoints.removeFirst();
    }
}

void MainWindow::onCoefficientUpdated(double coefficient)
{
    m_coefficientEdit->setText(QString::number(coefficient, 'f', 6));
    
    // 添加到系数图表
    int calcCount = m_coefficientPoints.size();
    m_coefficientPoints.append(QPointF(calcCount, coefficient));
    
    // 更新系数图表
    m_coefficientSeries->clear();
    for (const QPointF &point : m_coefficientPoints) {
        m_coefficientSeries->append(point);
    }
    
    // 自动调整Y轴范围
    if (!m_coefficientPoints.isEmpty()) {
        double minCoeff = coefficient;
        double maxCoeff = coefficient;
        for (const QPointF &point : m_coefficientPoints) {
            minCoeff = qMin(minCoeff, point.y());
            maxCoeff = qMax(maxCoeff, point.y());
        }
        double margin = (maxCoeff - minCoeff) * 0.1 + 0.1;
        m_coeffAxisY->setRange(minCoeff - margin, maxCoeff + margin);
    }
    
    // 更新X轴范围
    if (calcCount > 50) {
        m_coeffAxisX->setRange(0, calcCount);
    }
}

void MainWindow::onCalibrationPointAdded(int totalPoints)
{
    m_calibrationPointsEdit->setText(QString::number(totalPoints));
}

void MainWindow::updateCharts()
{
    // 更新力矩图表
    m_torqueSeries->clear();
    for (int i = 0; i < m_torquePoints.size(); ++i) {
        m_torqueSeries->append(i, m_torquePoints[i].y());
    }
    
    // 更新电流图表
    m_currentSeries->clear();
    for (int i = 0; i < m_currentPoints.size(); ++i) {
        m_currentSeries->append(i, m_currentPoints[i].y());
    }
    
    // 自动调整Y轴范围
    if (!m_torquePoints.isEmpty()) {
        double minTorque = m_torquePoints.first().y();
        double maxTorque = m_torquePoints.first().y();
        for (const QPointF &point : m_torquePoints) {
            minTorque = qMin(minTorque, point.y());
            maxTorque = qMax(maxTorque, point.y());
        }
        double margin = (maxTorque - minTorque) * 0.1 + 1.0;
        m_torqueAxisY->setRange(minTorque - margin, maxTorque + margin);
    }
    
    if (!m_currentPoints.isEmpty()) {
        double minCurrent = m_currentPoints.first().y();
        double maxCurrent = m_currentPoints.first().y();
        for (const QPointF &point : m_currentPoints) {
            minCurrent = qMin(minCurrent, point.y());
            maxCurrent = qMax(maxCurrent, point.y());
        }
        double margin = (maxCurrent - minCurrent) * 0.1 + 1.0;
        m_currentAxisY->setRange(minCurrent - margin, maxCurrent + margin);
    }
}

void MainWindow::refreshPortList()
{
    QStringList ports = SerialPortHandler::getAvailablePorts();
    
    QString currentTorquePort = m_torquePortCombo->currentText();
    QString currentMotorPort = m_motorPortCombo->currentText();
    
    m_torquePortCombo->clear();
    m_motorPortCombo->clear();
    
    m_torquePortCombo->addItems(ports);
    m_motorPortCombo->addItems(ports);
    
    // 恢复之前的选择
    int torqueIndex = m_torquePortCombo->findText(currentTorquePort);
    if (torqueIndex >= 0) {
        m_torquePortCombo->setCurrentIndex(torqueIndex);
    }
    
    int motorIndex = m_motorPortCombo->findText(currentMotorPort);
    if (motorIndex >= 0) {
        m_motorPortCombo->setCurrentIndex(motorIndex);
    }
    
    statusBar()->showMessage(QString("找到 %1 个串口").arg(ports.size()), 2000);
}
