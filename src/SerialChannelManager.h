#pragma once

#include <QObject>
#include <QSerialPort>
#include <functional>

class SerialChannelManager final : public QObject
{
    Q_OBJECT

public:
    struct Settings
    {
        QString portName;
        qint32 baudRate = QSerialPort::Baud115200;
        QSerialPort::DataBits dataBits = QSerialPort::Data8;
        QSerialPort::Parity parity = QSerialPort::NoParity;
        QSerialPort::StopBits stopBits = QSerialPort::OneStop;
        QSerialPort::FlowControl flowControl = QSerialPort::NoFlowControl;
    };

    using FrameSplitter = std::function<QList<QByteArray>(QByteArray& buffer)>;
    using FrameHandler = std::function<void(const QByteArray& frame)>;

    explicit SerialChannelManager(QObject* parent = nullptr);

    void setSettings(const Settings& settings);
    Settings settings() const { return m_settings; }

    bool open();
    void close();
    bool isOpen() const;

    void writeFrame(const QByteArray& payload);

    void setFrameSplitter(FrameSplitter splitter);
    void setFrameHandler(FrameHandler handler);

signals:
    void connectionStateChanged(bool connected);
    void serialError(const QString& description);

private slots:
    void handleReadyRead();
    void handleSerialError(QSerialPort::SerialPortError error);

private:
    static QList<QByteArray> defaultSplitter(QByteArray& buffer);

    QSerialPort m_serialPort;
    QByteArray m_buffer;
    Settings m_settings;
    FrameSplitter m_frameSplitter;
    FrameHandler m_frameHandler;
};
