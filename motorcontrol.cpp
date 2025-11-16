#include "motorcontrol.h"
#include <QDebug>
#include <QDateTime>
#include <QRegularExpression>
#include <QtMath>
#include <QIODevice>

MotorControl::MotorControl(QObject *parent)
    : QObject(parent)
    , m_serialPort(nullptr)
    , m_controlTimer(nullptr)
    , m_K(0.0)
    , m_B(0.0)
    , m_G(0.0)
    , m_initialAngle1(0.0)
    , m_initialAngle2(0.0)
    , m_currentAngle1(0.0)
    , m_currentAngle2(0.0)
    , m_lastAngle1(0.0)
    , m_lastAngle2(0.0)
    , m_currentVelocity1(0.0)
    , m_currentVelocity2(0.0)
    , m_current1(0.0)
    , m_current2(0.0)
    , m_isControlling(false)
    , m_initialPositionCaptured(false)
    , m_lastTime(0)
{
    m_serialPort = new QSerialPort(this);
    m_controlTimer = new QTimer(this);
    
    connect(m_serialPort, &QSerialPort::readyRead, this, &MotorControl::readSerialData);
    connect(m_controlTimer, &QTimer::timeout, this, &MotorControl::controlLoop);
}

MotorControl::~MotorControl()
{
    closeSerialPort();
}

bool MotorControl::openSerialPort(const QString &portName, int baudRate)
{
    if (m_serialPort->isOpen()) {
        closeSerialPort();
    }
    
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        emit statusChanged("串口已连接: " + portName);
        return true;
    } else {
        emit errorOccurred("无法打开串口: " + m_serialPort->errorString());
        return false;
    }
}

void MotorControl::closeSerialPort()
{
    if (m_serialPort->isOpen()) {
        stopImpedanceControl();
        m_serialPort->close();
        emit statusChanged("串口已断开");
    }
}

bool MotorControl::isConnected() const
{
    return m_serialPort && m_serialPort->isOpen();
}

void MotorControl::setImpedanceParameters(double K, double B, double G)
{
    m_K = K;
    m_B = B;
    m_G = G;
    emit statusChanged(QString("参数已更新 - K: %1, B: %2, G: %3").arg(K).arg(B).arg(G));
}

void MotorControl::captureInitialPosition()
{
    if (!isConnected()) {
        emit errorOccurred("串口未连接，无法获取初始位置");
        return;
    }
    
    m_initialAngle1 = m_currentAngle1;
    m_initialAngle2 = m_currentAngle2;
    m_lastAngle1 = m_currentAngle1;
    m_lastAngle2 = m_currentAngle2;
    m_initialPositionCaptured = true;
    m_lastTime = QDateTime::currentMSecsSinceEpoch();
    
    emit statusChanged(QString("初始位置已捕获 - 角度1: %1°, 角度2: %2°")
                      .arg(m_initialAngle1, 0, 'f', 2)
                      .arg(m_initialAngle2, 0, 'f', 2));
}

void MotorControl::startImpedanceControl()
{
    if (!isConnected()) {
        emit errorOccurred("串口未连接，无法启动控制");
        return;
    }
    
    if (!m_initialPositionCaptured) {
        emit errorOccurred("请先捕获初始位置");
        return;
    }
    
    if (m_K == 0.0) {
        emit errorOccurred("刚度系数不能为零");
        return;
    }
    
    m_isControlling = true;
    m_lastTime = QDateTime::currentMSecsSinceEpoch();
    m_controlTimer->start(20);  // 50Hz控制频率
    emit statusChanged("阻抗控制已启动");
}

void MotorControl::stopImpedanceControl()
{
    if (m_isControlling) {
        m_controlTimer->stop();
        m_isControlling = false;
        emit statusChanged("阻抗控制已停止");
    }
}

void MotorControl::readSerialData()
{
    QByteArray data = m_serialPort->readAll();
    m_receivedData.append(QString::fromUtf8(data));
    
    // 检查是否有完整的数据包（以\n结尾）
    if (m_receivedData.contains('\n')) {
        QStringList lines = m_receivedData.split('\n');
        
        // 处理除最后一个元素外的所有完整行
        for (int i = 0; i < lines.size() - 1; ++i) {
            parseReceivedData(lines[i]);
        }
        
        // 保留最后一个不完整的部分
        m_receivedData = lines.last();
    }
}

void MotorControl::parseReceivedData(const QString &data)
{
    // 解析格式: "angle: angle1  angle2 current: current1 current2mA"
    // 使用正则表达式解析
    QRegularExpression angleRegex(R"(angle:\s+([\d.-]+)\s+([\d.-]+))");
    QRegularExpression currentRegex(R"(current:\s+([\d.-]+)\s+([\d.-]+))");
    
    QRegularExpressionMatch angleMatch = angleRegex.match(data);
    QRegularExpressionMatch currentMatch = currentRegex.match(data);
    
    if (angleMatch.hasMatch()) {
        double newAngle1 = angleMatch.captured(1).toDouble();
        double newAngle2 = angleMatch.captured(2).toDouble();
        
        // 计算速度（差分）
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        double dt = (currentTime - m_lastTime) / 1000.0;  // 转换为秒
        
        if (dt > 0 && m_lastTime > 0) {
            m_currentVelocity1 = (newAngle1 - m_lastAngle1) / dt;
            m_currentVelocity2 = (newAngle2 - m_lastAngle2) / dt;
        }
        
        m_lastAngle1 = m_currentAngle1;
        m_lastAngle2 = m_currentAngle2;
        m_currentAngle1 = newAngle1;
        m_currentAngle2 = newAngle2;
        m_lastTime = currentTime;
    }
    
    if (currentMatch.hasMatch()) {
        m_current1 = currentMatch.captured(1).toDouble();
        m_current2 = currentMatch.captured(2).toDouble();
    }
    
    if (angleMatch.hasMatch() || currentMatch.hasMatch()) {
        emit dataUpdated();
    }
}

void MotorControl::controlLoop()
{
    if (!m_isControlling || !isConnected()) {
        return;
    }
    
    // 请求实时数据
    requestRealtimeData();
    
    // 计算位置误差
    double positionError1 = m_currentAngle1 - m_initialAngle1;
    double positionError2 = m_currentAngle2 - m_initialAngle2;
    
    // 速度误差（假设目标速度为0）
    double velocityError1 = m_currentVelocity1;
    double velocityError2 = m_currentVelocity2;
    
    // 计算阻抗力
    double impedanceForce1 = calculateImpedanceForce(positionError1, velocityError1);
    double impedanceForce2 = calculateImpedanceForce(positionError2, velocityError2);
    
    // 计算控制指令：θ_command = θ_current + F / K
    double commandAngle1 = m_currentAngle1 + impedanceForce1 / m_K;
    double commandAngle2 = m_currentAngle2 + impedanceForce2 / m_K;
    
    // 发送控制指令
    sendControlCommand(commandAngle1, commandAngle2);
    
    // 检查是否接近初始位置（可以设置一个阈值来判断是否回到初始状态）
    double threshold = 1.0;  // 1度的阈值
    if (qAbs(positionError1) < threshold && qAbs(positionError2) < threshold &&
        qAbs(velocityError1) < 0.5 && qAbs(velocityError2) < 0.5) {
        // 可以选择在这里停止控制或继续维持
        // stopImpedanceControl();
        // emit statusChanged("已回到初始位置");
    }
}

double MotorControl::calculateImpedanceForce(double positionError, double velocityError)
{
    // 阻抗力 = K * Δθ + B * Δω + G
    return m_K * positionError + m_B * velocityError + m_G;
}

void MotorControl::sendControlCommand(double angle1, double angle2)
{
    // 格式: "a160-150+*"
    // a表示控制板a，第一个数字是电机1的角度，第二个是电机2的角度
    // 将角度转换为整数
    int intAngle1 = qRound(angle1);
    int intAngle2 = qRound(angle2);
    
    // 构建命令字符串
    QString command = QString("a%1-%2+*").arg(intAngle1).arg(intAngle2);
    
    if (m_serialPort->isOpen()) {
        m_serialPort->write(command.toUtf8());
        m_serialPort->flush();
    }
}

void MotorControl::requestRealtimeData()
{
    // 请求实时数据：在a前面加大写字母A
    // 使用当前角度作为命令值
    int intAngle1 = qRound(m_currentAngle1);
    int intAngle2 = qRound(m_currentAngle2);
    
    QString command = QString("Aa%1-%2+*").arg(intAngle1).arg(intAngle2);
    
    if (m_serialPort->isOpen()) {
        m_serialPort->write(command.toUtf8());
        m_serialPort->flush();
    }
}
