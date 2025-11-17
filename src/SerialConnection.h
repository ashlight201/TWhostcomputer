#pragma once

#include <QObject>
#include <QSerialPort>

struct SerialSettings
{
    QString portName;
    int baudRate = 115200;
    QSerialPort::DataBits dataBits = QSerialPort::Data8;
    QSerialPort::Parity parity = QSerialPort::NoParity;
    QSerialPort::StopBits stopBits = QSerialPort::OneStop;
};

Q_DECLARE_METATYPE(SerialSettings)

class SerialConnection : public QObject
{
    Q_OBJECT

public:
    explicit SerialConnection(QObject *parent = nullptr);

    bool open(const SerialSettings &settings);
    void close();
    [[nodiscard]] bool isOpen() const;
    [[nodiscard]] QString portName() const;

public slots:
    void sendFrame(const QByteArray &frame);

signals:
    void frameReceived(const QByteArray &frame);
    void statusChanged(const QString &status, bool connected);
    void errorOccurred(const QString &message);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort m_port;
    QByteArray m_pendingBuffer;
};
