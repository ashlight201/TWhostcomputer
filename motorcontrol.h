#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <QObject>
#include <QSerialPort>
#include <QTimer>
#include <QString>

class MotorControl : public QObject
{
    Q_OBJECT

public:
    explicit MotorControl(QObject *parent = nullptr);
    ~MotorControl();

    bool openSerialPort(const QString &portName, qint32 baudRate = 115200);
    void closeSerialPort();
    bool isPortOpen() const;

    // 发送控制指令
    void sendControlCommand(double angleA, double angleB);
    
    // 请求实时数据
    void requestData(double angleA, double angleB);

signals:
    // 数据接收信号
    void dataReceived(double angle1, double angle2, double current1, double current2);

private slots:
    void readData();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *m_serialPort;
    QString m_buffer;
    
    // 解析接收到的数据
    bool parseData(const QString &data, double &angle1, double &angle2, 
                   double &current1, double &current2);
    
    // 格式化角度为控制指令
    QString formatCommand(double angleA, double angleB, bool requestData = false);
};

#endif // MOTORCONTROL_H
