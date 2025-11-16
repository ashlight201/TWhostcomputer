#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QElapsedTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_kCoefficient(0.0)
    , m_bCoefficient(0.0)
    , m_gravity(0.0)
    , m_initialAngle1(0.0)
    , m_initialAngle2(0.0)
    , m_currentAngle1(0.0)
    , m_currentAngle2(0.0)
    , m_previousAngle1(0.0)
    , m_previousAngle2(0.0)
    , m_currentVelocity1(0.0)
    , m_currentVelocity2(0.0)
    , m_controlActive(false)
    , m_initialPositionSet(false)
    , m_lastTime(0)
{
    m_motorControl = new MotorControl(this);
    connect(m_motorControl, &MotorControl::dataReceived, 
            this, &MainWindow::onDataReceived);
    
    m_controlTimer = new QTimer(this);
    connect(m_controlTimer, &QTimer::timeout, this, &MainWindow::onControlTimer);
    
    m_elapsedTimer = new QElapsedTimer();
    
    setupUI();
    
    // 尝试打开串口（这里可以改为用户选择）
    // m_motorControl->openSerialPort("/dev/ttyUSB0", 115200);
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
    QGridLayout *paramLayout = new QGridLayout(paramGroup);
    
    paramLayout->addWidget(new QLabel("刚度系数 K:"), 0, 0);
    m_kCoefficientLineEdit = new QLineEdit(this);
    m_kCoefficientLineEdit->setPlaceholderText("输入刚度系数");
    paramLayout->addWidget(m_kCoefficientLineEdit, 0, 1);
    
    paramLayout->addWidget(new QLabel("阻尼系数 B:"), 1, 0);
    m_bCoefficientLineEdit = new QLineEdit(this);
    m_bCoefficientLineEdit->setPlaceholderText("输入阻尼系数");
    paramLayout->addWidget(m_bCoefficientLineEdit, 1, 1);
    
    paramLayout->addWidget(new QLabel("重力补偿项:"), 2, 0);
    m_gravityLineEdit = new QLineEdit(this);
    m_gravityLineEdit->setPlaceholderText("输入重力补偿项");
    paramLayout->addWidget(m_gravityLineEdit, 2, 1);
    
    m_confirmPushButton = new QPushButton("确认", this);
    connect(m_confirmPushButton, &QPushButton::clicked, this, &MainWindow::onConfirmClicked);
    paramLayout->addWidget(m_confirmPushButton, 3, 0, 1, 2);
    
    mainLayout->addWidget(paramGroup);
    
    // 实时数据显示组
    QGroupBox *dataGroup = new QGroupBox("实时数据", this);
    QGridLayout *dataLayout = new QGridLayout(dataGroup);
    
    dataLayout->addWidget(new QLabel("关节角度:"), 0, 0);
    m_jointDegreeLineEdit = new QLineEdit(this);
    m_jointDegreeLineEdit->setReadOnly(true);
    m_jointDegreeLineEdit->setPlaceholderText("等待数据...");
    dataLayout->addWidget(m_jointDegreeLineEdit, 0, 1);
    
    dataLayout->addWidget(new QLabel("关节速度:"), 1, 0);
    m_jointVelocityLineEdit = new QLineEdit(this);
    m_jointVelocityLineEdit->setReadOnly(true);
    m_jointVelocityLineEdit->setPlaceholderText("等待数据...");
    dataLayout->addWidget(m_jointVelocityLineEdit, 1, 1);
    
    dataLayout->addWidget(new QLabel("电流:"), 2, 0);
    m_currentLineEdit = new QLineEdit(this);
    m_currentLineEdit->setReadOnly(true);
    m_currentLineEdit->setPlaceholderText("等待数据...");
    dataLayout->addWidget(m_currentLineEdit, 2, 1);
    
    mainLayout->addWidget(dataGroup);
    
    mainLayout->addStretch();
    
    setWindowTitle("关节阻抗控制上位机");
    resize(500, 400);
}

void MainWindow::onConfirmClicked()
{
    bool ok;
    
    m_kCoefficient = m_kCoefficientLineEdit->text().toDouble(&ok);
    if (!ok || m_kCoefficient <= 0) {
        QMessageBox::warning(this, "错误", "请输入有效的刚度系数（大于0）");
        return;
    }
    
    m_bCoefficient = m_bCoefficientLineEdit->text().toDouble(&ok);
    if (!ok || m_bCoefficient < 0) {
        QMessageBox::warning(this, "错误", "请输入有效的阻尼系数（大于等于0）");
        return;
    }
    
    m_gravity = m_gravityLineEdit->text().toDouble(&ok);
    if (!ok) {
        QMessageBox::warning(this, "错误", "请输入有效的重力补偿项");
        return;
    }
    
    // 检查串口是否打开
    if (!m_motorControl->isPortOpen()) {
        QMessageBox::warning(this, "错误", "请先打开串口连接");
        return;
    }
    
    // 请求初始位置
    m_initialPositionSet = false;
    m_controlActive = true;
    
    // 请求数据以获取初始位置
    m_motorControl->requestData(0, 0);
    
    // 启动控制定时器（10ms周期）
    m_controlTimer->start(10);
    m_elapsedTimer->start();
    m_lastTime = m_elapsedTimer->elapsed();
    
    QMessageBox::information(this, "成功", "参数已设置，开始获取初始位置");
}

void MainWindow::onDataReceived(double angle1, double angle2, double current1, double current2)
{
    // 更新当前角度
    m_previousAngle1 = m_currentAngle1;
    m_previousAngle2 = m_currentAngle2;
    
    m_currentAngle1 = angle1;
    m_currentAngle2 = angle2;
    
    // 计算速度（差分）
    if (m_controlActive && m_initialPositionSet) {
        qint64 currentTime = m_elapsedTimer->elapsed();
        if (m_lastTime > 0) {
            double dt = (currentTime - m_lastTime) / 1000.0; // 转换为秒
            if (dt > 0 && dt < 1.0) { // 防止异常大的时间间隔
                m_currentVelocity1 = (m_currentAngle1 - m_previousAngle1) / dt;
                m_currentVelocity2 = (m_currentAngle2 - m_previousAngle2) / dt;
            }
        }
        m_lastTime = currentTime;
    }
    
    // 如果是第一次接收数据，设置为初始位置
    if (!m_initialPositionSet && m_controlActive) {
        m_initialAngle1 = m_currentAngle1;
        m_initialAngle2 = m_currentAngle2;
        m_previousAngle1 = m_currentAngle1;
        m_previousAngle2 = m_currentAngle2;
        m_initialPositionSet = true;
        qDebug() << "初始位置设置: 角度1=" << m_initialAngle1 << ", 角度2=" << m_initialAngle2;
    }
    
    // 更新显示
    updateDisplay(m_currentAngle1, m_currentAngle2, 
                  m_currentVelocity1, m_currentVelocity2,
                  current1, current2);
}

void MainWindow::onControlTimer()
{
    if (!m_controlActive || !m_initialPositionSet) {
        return;
    }
    
    // 计算位置误差
    double positionError1 = m_currentAngle1 - m_initialAngle1;
    double positionError2 = m_currentAngle2 - m_initialAngle2;
    
    // 计算控制指令
    double controlCmd1 = calculateControlCommand(positionError1, m_currentVelocity1,
                                                  m_currentAngle1, m_kCoefficient,
                                                  m_bCoefficient, m_gravity);
    double controlCmd2 = calculateControlCommand(positionError2, m_currentVelocity2,
                                                  m_currentAngle2, m_kCoefficient,
                                                  m_bCoefficient, m_gravity);
    
    // 发送控制指令
    m_motorControl->sendControlCommand(controlCmd1, controlCmd2);
    
    // 请求下一次数据
    m_motorControl->requestData(controlCmd1, controlCmd2);
    
    // 检查是否回到初始位置（误差小于阈值）
    const double threshold = 0.5; // 0.5度的误差阈值
    if (qAbs(positionError1) < threshold && qAbs(positionError2) < threshold &&
        qAbs(m_currentVelocity1) < 0.1 && qAbs(m_currentVelocity2) < 0.1) {
        // 可以在这里添加回到初始位置的提示
        // qDebug() << "关节已回到初始位置";
    }
}

double MainWindow::calculateControlCommand(double positionError, double velocityError,
                                            double currentPosition, double kCoeff, 
                                            double bCoeff, double gravity)
{
    // 计算阻抗力
    double impedanceForce = kCoeff * positionError + bCoeff * velocityError + gravity;
    
    // 除以刚度系数得到控制量
    double controlDelta = impedanceForce / kCoeff;
    
    // 与当前位置相加得到控制指令
    double controlCommand = currentPosition + controlDelta;
    
    return controlCommand;
}

void MainWindow::updateDisplay(double angle1, double angle2, double velocity1, 
                               double velocity2, double current1, double current2)
{
    // 更新关节角度显示
    QString angleText = QString("角度1: %1°  角度2: %2°")
                        .arg(angle1, 0, 'f', 2)
                        .arg(angle2, 0, 'f', 2);
    m_jointDegreeLineEdit->setText(angleText);
    
    // 更新关节速度显示
    QString velocityText = QString("速度1: %1°/s  速度2: %2°/s")
                           .arg(velocity1, 0, 'f', 2)
                           .arg(velocity2, 0, 'f', 2);
    m_jointVelocityLineEdit->setText(velocityText);
    
    // 更新电流显示
    QString currentText = QString("电流1: %1mA  电流2: %2mA")
                          .arg(current1, 0, 'f', 2)
                          .arg(current2, 0, 'f', 2);
    m_currentLineEdit->setText(currentText);
}
