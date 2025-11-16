#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_kCoefficient(0.0)
    , m_bCoefficient(0.0)
    , m_gravityCompensation(0.0)
    , m_initialAngle1(0.0)
    , m_initialAngle2(0.0)
    , m_currentAngle1(0.0)
    , m_currentAngle2(0.0)
    , m_lastAngle1(0.0)
    , m_lastAngle2(0.0)
    , m_velocity1(0.0)
    , m_velocity2(0.0)
    , m_current1(0.0)
    , m_current2(0.0)
    , m_initialPositionSet(false)
    , m_controlActive(false)
    , m_lastControlTime(0)
    , m_lastDataTime(0)
{
    m_motorControl = new MotorControl(this);
    connect(m_motorControl, &MotorControl::dataReceived, this, &MainWindow::onDataReceived);
    connect(m_motorControl, &MotorControl::errorOccurred, this, &MainWindow::onErrorOccurred);
    
    m_controlTimer = new QTimer(this);
    connect(m_controlTimer, &QTimer::timeout, this, &MainWindow::onControlTimer);
    
    setupUI();
    
    // 尝试打开串口（这里默认使用COM1，实际使用时需要根据实际情况修改）
    // m_motorControl->openSerialPort("COM1", 115200);
}

MainWindow::~MainWindow()
{
    if (m_controlTimer->isActive()) {
        m_controlTimer->stop();
    }
    m_motorControl->closeSerialPort();
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 参数输入组
    QGroupBox *paramGroup = new QGroupBox("阻抗控制参数", this);
    QVBoxLayout *paramLayout = new QVBoxLayout(paramGroup);
    
    QHBoxLayout *kLayout = new QHBoxLayout();
    kLayout->addWidget(new QLabel("刚度系数 K:", this));
    m_kCoefficientEdit = new QLineEdit(this);
    m_kCoefficientEdit->setPlaceholderText("输入刚度系数");
    kLayout->addWidget(m_kCoefficientEdit);
    paramLayout->addLayout(kLayout);
    
    QHBoxLayout *bLayout = new QHBoxLayout();
    bLayout->addWidget(new QLabel("阻尼系数 B:", this));
    m_bCoefficientEdit = new QLineEdit(this);
    m_bCoefficientEdit->setPlaceholderText("输入阻尼系数");
    bLayout->addWidget(m_bCoefficientEdit);
    paramLayout->addLayout(bLayout);
    
    QHBoxLayout *gLayout = new QHBoxLayout();
    gLayout->addWidget(new QLabel("重力补偿项:", this));
    m_gravityEdit = new QLineEdit(this);
    m_gravityEdit->setPlaceholderText("输入重力补偿项");
    gLayout->addWidget(m_gravityEdit);
    paramLayout->addLayout(gLayout);
    
    m_confirmButton = new QPushButton("确认", this);
    connect(m_confirmButton, &QPushButton::clicked, this, &MainWindow::onConfirmClicked);
    paramLayout->addWidget(m_confirmButton);
    
    mainLayout->addWidget(paramGroup);
    
    // 实时数据显示组
    QGroupBox *dataGroup = new QGroupBox("实时数据", this);
    QVBoxLayout *dataLayout = new QVBoxLayout(dataGroup);
    
    QHBoxLayout *angleLayout = new QHBoxLayout();
    angleLayout->addWidget(new QLabel("关节角度:", this));
    m_jointDegreeEdit = new QLineEdit(this);
    m_jointDegreeEdit->setReadOnly(true);
    m_jointDegreeEdit->setPlaceholderText("实时关节角度");
    angleLayout->addWidget(m_jointDegreeEdit);
    dataLayout->addLayout(angleLayout);
    
    QHBoxLayout *velocityLayout = new QHBoxLayout();
    velocityLayout->addWidget(new QLabel("关节速度:", this));
    m_jointVelocityEdit = new QLineEdit(this);
    m_jointVelocityEdit->setReadOnly(true);
    m_jointVelocityEdit->setPlaceholderText("实时关节速度");
    velocityLayout->addWidget(m_jointVelocityEdit);
    dataLayout->addLayout(velocityLayout);
    
    QHBoxLayout *currentLayout = new QHBoxLayout();
    currentLayout->addWidget(new QLabel("电流:", this));
    m_currentEdit = new QLineEdit(this);
    m_currentEdit->setReadOnly(true);
    m_currentEdit->setPlaceholderText("实时电流");
    currentLayout->addWidget(m_currentEdit);
    dataLayout->addLayout(currentLayout);
    
    mainLayout->addWidget(dataGroup);
    
    // 状态标签
    QLabel *statusLabel = new QLabel("状态: 等待设置参数", this);
    statusLabel->setStyleSheet("QLabel { color: blue; font-weight: bold; }");
    mainLayout->addWidget(statusLabel);
    
    setWindowTitle("关节阻抗控制上位机");
    resize(500, 400);
}

void MainWindow::onConfirmClicked()
{
    bool ok;
    
    // 读取刚度系数
    QString kStr = m_kCoefficientEdit->text().trimmed();
    if (kStr.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入刚度系数");
        return;
    }
    m_kCoefficient = kStr.toDouble(&ok);
    if (!ok || m_kCoefficient <= 0) {
        QMessageBox::warning(this, "警告", "刚度系数必须为正数");
        return;
    }
    
    // 读取阻尼系数
    QString bStr = m_bCoefficientEdit->text().trimmed();
    if (bStr.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入阻尼系数");
        return;
    }
    m_bCoefficient = bStr.toDouble(&ok);
    if (!ok || m_bCoefficient < 0) {
        QMessageBox::warning(this, "警告", "阻尼系数必须为非负数");
        return;
    }
    
    // 读取重力补偿项
    QString gStr = m_gravityEdit->text().trimmed();
    if (gStr.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入重力补偿项");
        return;
    }
    m_gravityCompensation = gStr.toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "警告", "重力补偿项格式错误");
        return;
    }
    
    // 检查串口是否打开
    if (!m_motorControl->isPortOpen()) {
        QMessageBox::warning(this, "警告", "请先打开串口连接");
        return;
    }
    
    // 如果还没有设置初始位置，现在设置
    if (!m_initialPositionSet) {
        // 请求一次数据来获取初始位置
        m_motorControl->requestRealTimeData(0.0, 0.0);
        QMessageBox::information(this, "提示", "参数已设置，等待获取初始关节位置...");
    } else {
        // 重新开始控制
        m_controlActive = true;
        m_lastControlTime = QDateTime::currentMSecsSinceEpoch();
        if (!m_controlTimer->isActive()) {
            m_controlTimer->start(50); // 20Hz控制频率
        }
        QMessageBox::information(this, "提示", "参数已更新，控制已启动");
    }
}

void MainWindow::onDataReceived(const MotorData &data)
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    // 更新当前角度
    m_lastAngle1 = m_currentAngle1;
    m_lastAngle2 = m_currentAngle2;
    
    m_currentAngle1 = data.angle1;
    m_currentAngle2 = data.angle2;
    m_current1 = data.current1;
    m_current2 = data.current2;
    
    // 计算速度（使用数据接收时间间隔）
    if (m_lastDataTime > 0) {
        double dt = (currentTime - m_lastDataTime) / 1000.0; // 转换为秒
        if (dt > 0) {
            m_velocity1 = (m_currentAngle1 - m_lastAngle1) / dt;
            m_velocity2 = (m_currentAngle2 - m_lastAngle2) / dt;
        }
    }
    m_lastDataTime = currentTime;
    
    // 如果是第一次接收数据，设置为初始位置
    if (!m_initialPositionSet) {
        m_initialAngle1 = m_currentAngle1;
        m_initialAngle2 = m_currentAngle2;
        m_lastAngle1 = m_currentAngle1;
        m_lastAngle2 = m_currentAngle2;
        m_velocity1 = 0.0;
        m_velocity2 = 0.0;
        m_initialPositionSet = true;
        m_controlActive = true;
        m_lastControlTime = currentTime;
        
        if (!m_controlTimer->isActive()) {
            m_controlTimer->start(50); // 20Hz控制频率
        }
        
        QMessageBox::information(this, "提示", 
            QString("初始位置已设置:\n关节1: %1°\n关节2: %2°\n控制已启动")
            .arg(m_initialAngle1).arg(m_initialAngle2));
    }
    
    // 更新显示
    updateDisplay();
    
    // 如果控制激活，计算并发送控制指令
    if (m_controlActive) {
        calculateImpedanceControl();
    }
}

void MainWindow::onControlTimer()
{
    // 定时请求实时数据
    if (m_controlActive && m_motorControl->isPortOpen()) {
        // 使用当前控制角度请求数据（这里使用当前角度作为占位符）
        m_motorControl->requestRealTimeData(m_currentAngle1, m_currentAngle2);
    }
}

void MainWindow::calculateImpedanceControl()
{
    if (!m_initialPositionSet) {
        return;
    }
    
    // 计算位置差
    double positionError1 = m_currentAngle1 - m_initialAngle1;
    double positionError2 = m_currentAngle2 - m_initialAngle2;
    
    // 速度已经在onDataReceived中计算好了，这里直接使用
    
    // 计算阻抗力
    // 阻抗力 = 位置差 × 刚度系数 + 速度差 × 阻尼系数 + 重力补偿项
    double impedanceForce1 = positionError1 * m_kCoefficient + m_velocity1 * m_bCoefficient + m_gravityCompensation;
    double impedanceForce2 = positionError2 * m_kCoefficient + m_velocity2 * m_bCoefficient + m_gravityCompensation;
    
    // 计算控制指令
    // 控制指令 = 阻抗力 / 刚度系数 + 实时关节位置
    double controlAngle1 = impedanceForce1 / m_kCoefficient + m_currentAngle1;
    double controlAngle2 = impedanceForce2 / m_kCoefficient + m_currentAngle2;
    
    // 发送控制指令
    m_motorControl->sendControlCommand(controlAngle1, controlAngle2);
    
    // 检查是否回到初始位置（允许一定误差）
    double tolerance = 0.5; // 0.5度误差
    if (qAbs(positionError1) < tolerance && qAbs(positionError2) < tolerance &&
        qAbs(m_velocity1) < 0.1 && qAbs(m_velocity2) < 0.1) {
        // 可以停止控制或保持控制
        // m_controlActive = false;
        // m_controlTimer->stop();
    }
}

void MainWindow::updateDisplay()
{
    // 更新关节角度显示
    m_jointDegreeEdit->setText(QString("关节1: %1°  关节2: %2°")
                                .arg(m_currentAngle1, 0, 'f', 2)
                                .arg(m_currentAngle2, 0, 'f', 2));
    
    // 更新关节速度显示
    m_jointVelocityEdit->setText(QString("关节1: %1°/s  关节2: %2°/s")
                                  .arg(m_velocity1, 0, 'f', 2)
                                  .arg(m_velocity2, 0, 'f', 2));
    
    // 更新电流显示
    m_currentEdit->setText(QString("电流1: %1mA  电流2: %2mA")
                           .arg(m_current1, 0, 'f', 2)
                           .arg(m_current2, 0, 'f', 2));
}

void MainWindow::onErrorOccurred(const QString &error)
{
    QMessageBox::critical(this, "错误", error);
    qDebug() << "错误:" << error;
}
