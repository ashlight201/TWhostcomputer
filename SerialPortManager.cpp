#include "SerialPortManager.h"
#include <QDebug>

SerialPortManager::SerialPortManager(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortManager::handleReadyRead);
    connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::error),
            this, &SerialPortManager::handleError);
}

SerialPortManager::~SerialPortManager()
{
    closePort();
}

bool SerialPortManager::openPort(const QString &portName, qint32 baudRate,
                                 QSerialPort::DataBits dataBits,
                                 QSerialPort::Parity parity,
                                 QSerialPort::StopBits stopBits)
{
    if (m_serialPort->isOpen()) {
        closePort();
    }

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(dataBits);
    m_serialPort->setParity(parity);
    m_serialPort->setStopBits(stopBits);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite)) {
        emit portStatusChanged(true);
        return true;
    } else {
        emit portStatusChanged(false);
        return false;
    }
}

void SerialPortManager::closePort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        emit portStatusChanged(false);
    }
}

bool SerialPortManager::isOpen() const
{
    return m_serialPort->isOpen();
}

bool SerialPortManager::writeData(const QByteArray &data)
{
    if (!m_serialPort->isOpen()) {
        return false;
    }
    qint64 bytesWritten = m_serialPort->write(data);
    return bytesWritten == data.size();
}

QString SerialPortManager::getPortName() const
{
    return m_serialPort->portName();
}

QSerialPort::SerialPortError SerialPortManager::getError() const
{
    return m_serialPort->error();
}

QStringList SerialPortManager::getAvailablePorts()
{
    QStringList ports;
    const auto portInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : portInfos) {
        ports << info.portName();
    }
    return ports;
}

void SerialPortManager::handleReadyRead()
{
    QByteArray data = m_serialPort->readAll();
    emit rawDataReceived(data);
}

void SerialPortManager::handleError(QSerialPort::SerialPortError error)
{
    if (error != QSerialPort::NoError) {
        emit errorOccurred(error);
    }
}
