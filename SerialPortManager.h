#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QTimer>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    // 串口配置
    bool openPort(const QString &portName, qint32 baudRate = 115200,
                  QSerialPort::DataBits dataBits = QSerialPort::Data8,
                  QSerialPort::Parity parity = QSerialPort::NoParity,
                  QSerialPort::StopBits stopBits = QSerialPort::OneStop);
    void closePort();
    bool isOpen() const;

    // 发送数据
    bool writeData(const QByteArray &data);

    // 获取串口信息
    QString getPortName() const;
    QSerialPort::SerialPortError getError() const;

    // 获取可用串口列表
    static QStringList getAvailablePorts();

signals:
    // 接收到原始数据
    void rawDataReceived(const QByteArray &data);
    // 串口错误
    void errorOccurred(QSerialPort::SerialPortError error);
    // 串口状态变化
    void portStatusChanged(bool isOpen);

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
    QByteArray m_buffer;
};

#endif // SERIALPORTMANAGER_H
