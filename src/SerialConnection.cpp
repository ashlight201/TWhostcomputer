#include "SerialConnection.h"

#include <QSerialPortInfo>

SerialConnection::SerialConnection(QObject *parent)
    : QObject(parent)
{
    connect(&m_port, &QSerialPort::readyRead, this, &SerialConnection::handleReadyRead);
    connect(&m_port, &QSerialPort::errorOccurred, this, &SerialConnection::handleError);
}

bool SerialConnection::open(const SerialSettings &settings)
{
    if (settings.portName.isEmpty()) {
        const QString warning = tr("未选择串口，无法连接");
        emit statusChanged(warning, false);
        emit errorOccurred(warning);
        return false;
    }

    if (m_port.isOpen()) {
        m_port.close();
    }

    m_pendingBuffer.clear();
    m_port.setPortName(settings.portName);
    m_port.setBaudRate(settings.baudRate);
    m_port.setDataBits(settings.dataBits);
    m_port.setParity(settings.parity);
    m_port.setStopBits(settings.stopBits);

    if (!m_port.open(QIODevice::ReadWrite)) {
        const QString message = tr("无法打开串口 %1: %2")
                                    .arg(settings.portName, m_port.errorString());
        emit statusChanged(message, false);
        emit errorOccurred(message);
        return false;
    }

    emit statusChanged(tr("已连接 %1").arg(settings.portName), true);
    return true;
}

void SerialConnection::close()
{
    if (!m_port.isOpen()) {
        return;
    }

    const QString closedPort = m_port.portName();
    m_port.close();
    m_pendingBuffer.clear();
    emit statusChanged(tr("已断开 %1").arg(closedPort), false);
}

bool SerialConnection::isOpen() const
{
    return m_port.isOpen();
}

QString SerialConnection::portName() const
{
    return m_port.portName();
}

void SerialConnection::sendFrame(const QByteArray &frame)
{
    if (!m_port.isOpen() || frame.isEmpty()) {
        return;
    }

    m_port.write(frame);
}

void SerialConnection::handleReadyRead()
{
    m_pendingBuffer.append(m_port.readAll());

    int newlineIndex = -1;
    while ((newlineIndex = m_pendingBuffer.indexOf('\n')) != -1) {
        QByteArray frame = m_pendingBuffer.left(newlineIndex + 1);
        m_pendingBuffer.remove(0, newlineIndex + 1);
        emit frameReceived(frame);
    }

    if (m_pendingBuffer.size() > 512) {
        emit frameReceived(m_pendingBuffer);
        m_pendingBuffer.clear();
    }
}

void SerialConnection::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError || error == QSerialPort::TimeoutError) {
        return;
    }

    const QString message = tr("串口错误(%1): %2")
                                .arg(m_port.portName(), m_port.errorString());
    emit errorOccurred(message);
    emit statusChanged(message, m_port.isOpen());
}
