#include "SerialPortManager.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
    , serialPort(new QSerialPort(this))
    , currentBaudRate(115200)
{
    connect(serialPort, &QSerialPort::readyRead, this, &SerialPortManager::handleReadyRead);
    connect(serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error),
            this, &SerialPortManager::handleError);
}

SerialPortManager::~SerialPortManager()
{
    closePort();
}

bool SerialPortManager::openPort(const QString &portName, qint32 baudRate)
{
    if (serialPort->isOpen()) {
        closePort();
    }
    
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    if (serialPort->open(QIODevice::ReadWrite)) {
        currentPortName = portName;
        currentBaudRate = baudRate;
        emit portOpened();
        return true;
    } else {
        emit errorOccurred(serialPort->errorString());
        return false;
    }
}

void SerialPortManager::closePort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
        emit portClosed();
    }
}

bool SerialPortManager::isOpen() const
{
    return serialPort->isOpen();
}

QString SerialPortManager::portName() const
{
    return currentPortName;
}

qint32 SerialPortManager::baudRate() const
{
    return currentBaudRate;
}

bool SerialPortManager::sendData(const QByteArray &data)
{
    if (!serialPort->isOpen()) {
        emit errorOccurred("串口未打开");
        return false;
    }
    
    qint64 bytesWritten = serialPort->write(data);
    if (bytesWritten == -1) {
        emit errorOccurred(serialPort->errorString());
        return false;
    }
    
    return serialPort->waitForBytesWritten(1000);
}

QList<QSerialPortInfo> SerialPortManager::availablePorts() const
{
    return QSerialPortInfo::availablePorts();
}

void SerialPortManager::handleReadyRead()
{
    QByteArray data = serialPort->readAll();
    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void SerialPortManager::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        emit errorOccurred(serialPort->errorString());
        closePort();
    }
}
