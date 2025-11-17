#ifndef SERIALPORTHANDLER_H
#define SERIALPORTHANDLER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QTimer>

/**
 * @brief 串口处理类
 * 负责串口的打开、关闭、数据接收和发送
 */
class SerialPortHandler : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortHandler(QObject *parent = nullptr);
    ~SerialPortHandler();

    // 打开串口
    bool openPort(const QString &portName, 
                  qint32 baudRate = QSerialPort::Baud115200,
                  QSerialPort::DataBits dataBits = QSerialPort::Data8,
                  QSerialPort::Parity parity = QSerialPort::NoParity,
                  QSerialPort::StopBits stopBits = QSerialPort::OneStop);
    
    // 关闭串口
    void closePort();
    
    // 检查串口是否打开
    bool isOpen() const;
    
    // 发送数据
    qint64 sendData(const QByteArray &data);
    
    // 获取可用串口列表
    static QStringList getAvailablePorts();
    
    // 清空接收缓冲区
    void clearBuffer();

signals:
    // 接收到数据
    void dataReceived(const QByteArray &data);
    
    // 串口错误
    void errorOccurred(const QString &error);
    
    // 串口状态变化
    void connectionStatusChanged(bool connected);

private slots:
    // 读取串口数据
    void onReadyRead();
    
    // 处理串口错误
    void onErrorOccurred(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
    QByteArray m_receiveBuffer;
};

#endif // SERIALPORTHANDLER_H
