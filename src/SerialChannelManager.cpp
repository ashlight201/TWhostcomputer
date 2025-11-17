#include "SerialChannelManager.h"

#include <QLoggingCategory>

#include <utility>

Q_LOGGING_CATEGORY(serialChannelLog, "serial.channel")

SerialChannelManager::SerialChannelManager(QObject* parent)
    : QObject(parent)
{
    QObject::connect(&m_serialPort, &QSerialPort::readyRead, this, &SerialChannelManager::handleReadyRead);
    QObject::connect(&m_serialPort, &QSerialPort::errorOccurred, this, &SerialChannelManager::handleSerialError);
    m_frameSplitter = defaultSplitter;
}

void SerialChannelManager::setSettings(const Settings& settings)
{
    m_settings = settings;
}

bool SerialChannelManager::open()
{
    if (m_settings.portName.isEmpty())
    {
        emit serialError(tr("串口名称为空，无法建立连接"));
        return false;
    }

    if (m_serialPort.isOpen())
    {
        m_serialPort.close();
    }

    m_serialPort.setPortName(m_settings.portName);
    m_serialPort.setBaudRate(m_settings.baudRate);
    m_serialPort.setDataBits(m_settings.dataBits);
    m_serialPort.setParity(m_settings.parity);
    m_serialPort.setStopBits(m_settings.stopBits);
    m_serialPort.setFlowControl(m_settings.flowControl);

    if (!m_serialPort.open(QIODevice::ReadWrite))
    {
        const auto errorMsg = tr("打开串口%1失败: %2").arg(m_settings.portName, m_serialPort.errorString());
        qCWarning(serialChannelLog) << errorMsg;
        emit serialError(errorMsg);
        return false;
    }

    emit connectionStateChanged(true);
    qCInfo(serialChannelLog) << "串口已打开:" << m_settings.portName;
    return true;
}

void SerialChannelManager::close()
{
    if (!m_serialPort.isOpen())
    {
        return;
    }

    m_serialPort.close();
    emit connectionStateChanged(false);
    qCInfo(serialChannelLog) << "串口已关闭:" << m_settings.portName;
}

bool SerialChannelManager::isOpen() const
{
    return m_serialPort.isOpen();
}

void SerialChannelManager::writeFrame(const QByteArray& payload)
{
    if (!m_serialPort.isOpen())
    {
        qCWarning(serialChannelLog) << "串口未打开，写入被忽略";
        return;
    }

    m_serialPort.write(payload);
}

void SerialChannelManager::setFrameSplitter(FrameSplitter splitter)
{
    if (splitter)
    {
        m_frameSplitter = std::move(splitter);
    }
    else
    {
        m_frameSplitter = defaultSplitter;
    }
}

void SerialChannelManager::setFrameHandler(FrameHandler handler)
{
    m_frameHandler = std::move(handler);
}

void SerialChannelManager::handleReadyRead()
{
    m_buffer.append(m_serialPort.readAll());
    const auto frames = m_frameSplitter ? m_frameSplitter(m_buffer) : QList<QByteArray>{};
    if (frames.isEmpty() || !m_frameHandler)
    {
        return;
    }

    for (const auto& frame : frames)
    {
        m_frameHandler(frame);
    }
}

void SerialChannelManager::handleSerialError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
    {
        return;
    }

    const auto message = tr("串口错误(%1): %2").arg(m_settings.portName, m_serialPort.errorString());
    qCWarning(serialChannelLog) << message;
    emit serialError(message);
}

QList<QByteArray> SerialChannelManager::defaultSplitter(QByteArray& buffer)
{
    QList<QByteArray> frames;
    int index = -1;
    while ((index = buffer.indexOf('\n')) != -1)
    {
        const QByteArray frame = buffer.left(index).trimmed();
        if (!frame.isEmpty())
        {
            frames.push_back(frame);
        }
        buffer.remove(0, index + 1);
    }
    return frames;
}
