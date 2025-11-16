#include "motorcontrol.h"
#include <QDebug>
#include <QDateTime>
#include <QRegExp>

MotorControl::MotorControl(QObject *parent)
    : QObject(parent)
    , m_serialPort(nullptr)
    , m_firstDataReceived(false)
    , m_lastAngle1(0.0)
    , m_lastAngle2(0.0)
    , m_lastTime(0)
{
    m_serialPort = new QSerialPort(this);
    connect(m_serialPort, &QSerialPort::readyRead, this, &MotorControl::handleReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error) {
        if (error != QSerialPort::NoError) {
            emit errorOccurred(m_serialPort->errorString());
        }
    });
}

MotorControl::~MotorControl()
{
    closeSerialPort();
}

bool MotorControl::openSerialPort(const QString &portName, qint32 baudRate)
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
    }

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        m_buffer.clear();
        m_firstDataReceived = false;
        return true;
    } else {
        emit errorOccurred(QString("无法打开串口: %1").arg(m_serialPort->errorString()));
        return false;
    }
}

void MotorControl::closeSerialPort()
{
    if (m_serialPort && m_serialPort->isOpen()) {
        m_serialPort->close();
    }
}

bool MotorControl::isPortOpen() const
{
    return m_serialPort && m_serialPort->isOpen();
}

void MotorControl::sendControlCommand(double angle1, double angle2)
{
    if (!isPortOpen()) {
        emit errorOccurred("串口未打开");
        return;
    }

    // 格式化数据帧: a160-150+*
    QString command = QString("a%1-%2+*").arg(angle1, 0, 'f', 1).arg(angle2, 0, 'f', 1);
    QByteArray data = command.toUtf8();
    
    qint64 bytesWritten = m_serialPort->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred(QString("发送数据失败: %1").arg(m_serialPort->errorString()));
    } else {
        m_serialPort->flush();
    }
}

void MotorControl::requestRealTimeData(double angle1, double angle2)
{
    if (!isPortOpen()) {
        emit errorOccurred("串口未打开");
        return;
    }

    // 格式化数据帧: Aa160-150+* (A表示请求实时数据)
    QString command = QString("Aa%1-%2+*").arg(angle1, 0, 'f', 1).arg(angle2, 0, 'f', 1);
    QByteArray data = command.toUtf8();
    
    qint64 bytesWritten = m_serialPort->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred(QString("发送数据失败: %1").arg(m_serialPort->errorString()));
    } else {
        m_serialPort->flush();
    }
}

void MotorControl::handleReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    m_buffer.append(data);

    // 查找完整的消息（以\n结尾）
    int newlineIndex = m_buffer.indexOf('\n');
    while (newlineIndex != -1) {
        QByteArray message = m_buffer.left(newlineIndex);
        m_buffer.remove(0, newlineIndex + 1);
        
        parseReceivedData(message);
        
        newlineIndex = m_buffer.indexOf('\n');
    }
}

void MotorControl::parseReceivedData(const QByteArray &data)
{
    QString message = QString::fromUtf8(data).trimmed();
    
    // 解析格式: angle: angle1  angle2 current: current1 current2mA
    // 例如: angle: 160.5  150.3 current: 100 120mA
    
    MotorData motorData;
    bool parseSuccess = false;
    
    // 使用正则表达式或字符串解析
    QStringList parts = message.split("current:");
    if (parts.size() == 2) {
        QString anglePart = parts[0].trimmed();
        QString currentPart = parts[1].trimmed();
        
        // 解析角度部分: angle: angle1  angle2
        if (anglePart.startsWith("angle:")) {
            QString angleStr = anglePart.mid(6).trimmed();
            QStringList angles = angleStr.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (angles.size() >= 2) {
                motorData.angle1 = angles[0].toDouble(&parseSuccess);
                if (parseSuccess) {
                    motorData.angle2 = angles[1].toDouble(&parseSuccess);
                }
            }
        }
        
        // 解析电流部分: current1 current2mA
        if (parseSuccess) {
            currentPart = currentPart.replace("mA", "").trimmed();
            QStringList currents = currentPart.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            if (currents.size() >= 2) {
                motorData.current1 = currents[0].toDouble(&parseSuccess);
                if (parseSuccess) {
                    motorData.current2 = currents[1].toDouble(&parseSuccess);
                }
            }
        }
    }
    
    if (parseSuccess) {
        m_lastData = motorData;
        emit dataReceived(motorData);
    } else {
        qDebug() << "数据解析失败:" << message;
    }
}
