#include "motorcontrol.h"
#include <QDebug>
#include <QRegularExpression>

MotorControl::MotorControl(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &MotorControl::readData);
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error),
            this, &MotorControl::handleError);
}

MotorControl::~MotorControl()
{
    closeSerialPort();
}

bool MotorControl::openSerialPort(const QString &portName, qint32 baudRate)
{
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_buffer.clear();
        return true;
    } else {
        qDebug() << "打开串口失败:" << m_serialPort->errorString();
        return false;
    }
}

void MotorControl::closeSerialPort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool MotorControl::isPortOpen() const
{
    return m_serialPort->isOpen();
}

QString MotorControl::formatCommand(double angleA, double angleB, bool requestData)
{
    // 将角度转换为整数（假设角度单位是度）
    int angleAInt = static_cast<int>(angleA);
    int angleBInt = static_cast<int>(angleB);
    
    QString command;
    if (requestData) {
        command = QString("A%1-%2+*").arg(angleAInt).arg(angleBInt);
    } else {
        command = QString("a%1-%2+*").arg(angleAInt).arg(angleBInt);
    }
    
    return command;
}

void MotorControl::sendControlCommand(double angleA, double angleB)
{
    if (!m_serialPort->isOpen()) {
        qDebug() << "串口未打开";
        return;
    }

    QString command = formatCommand(angleA, angleB, false);
    QByteArray data = command.toUtf8();
    
    qint64 bytesWritten = m_serialPort->write(data);
    if (bytesWritten == -1) {
        qDebug() << "发送数据失败:" << m_serialPort->errorString();
    } else {
        qDebug() << "发送控制指令:" << command;
    }
}

void MotorControl::requestData(double angleA, double angleB)
{
    if (!m_serialPort->isOpen()) {
        qDebug() << "串口未打开";
        return;
    }

    QString command = formatCommand(angleA, angleB, true);
    QByteArray data = command.toUtf8();
    
    qint64 bytesWritten = m_serialPort->write(data);
    if (bytesWritten == -1) {
        qDebug() << "发送请求失败:" << m_serialPort->errorString();
    } else {
        qDebug() << "请求数据:" << command;
    }
}

void MotorControl::readData()
{
    QByteArray data = m_serialPort->readAll();
    m_buffer.append(QString::fromUtf8(data));
    
    // 查找完整的数据帧（以\n结尾）
    int newlineIndex = m_buffer.indexOf('\n');
    while (newlineIndex != -1) {
        QString line = m_buffer.left(newlineIndex).trimmed();
        m_buffer.remove(0, newlineIndex + 1);
        
        double angle1 = 0, angle2 = 0, current1 = 0, current2 = 0;
        if (parseData(line, angle1, angle2, current1, current2)) {
            emit dataReceived(angle1, angle2, current1, current2);
        }
        
        newlineIndex = m_buffer.indexOf('\n');
    }
}

bool MotorControl::parseData(const QString &data, double &angle1, double &angle2, 
                              double &current1, double &current2)
{
    // 解析格式: angle: angle1  angle2 current: current1 current2mA
    QRegularExpression regex(R"(angle:\s*([-\d.]+)\s+([-\d.]+)\s+current:\s*([-\d.]+)\s+([-\d.]+)mA)");
    QRegularExpressionMatch match = regex.match(data);
    
    if (match.hasMatch()) {
        angle1 = match.captured(1).toDouble();
        angle2 = match.captured(2).toDouble();
        current1 = match.captured(3).toDouble();
        current2 = match.captured(4).toDouble();
        return true;
    }
    
    qDebug() << "数据解析失败:" << data;
    return false;
}

void MotorControl::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        qDebug() << "串口错误:" << m_serialPort->errorString();
        closeSerialPort();
    }
}
