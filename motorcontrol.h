#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>
#include <QString>

struct MotorData {
    double angle1;      // 关节角度1
    double angle2;      // 关节角度2
    double current1;    // 电流1 (mA)
    double current2;    // 电流2 (mA)
};

class MotorControl : public QObject
{
    Q_OBJECT

public:
    explicit MotorControl(QObject *parent = nullptr);
    ~MotorControl();

    // 串口操作
    bool openSerialPort(const QString &portName, qint32 baudRate = 115200);
    void closeSerialPort();
    bool isPortOpen() const;

    // 发送控制指令
    void sendControlCommand(double angle1, double angle2);
    
    // 请求实时数据
    void requestRealTimeData(double angle1, double angle2);

signals:
    // 数据接收信号
    void dataReceived(const MotorData &data);
    // 错误信号
    void errorOccurred(const QString &error);

private slots:
    void handleReadyRead();
    void parseReceivedData(const QByteArray &data);

private:
    QSerialPort *m_serialPort;
    QByteArray m_buffer;
    MotorData m_lastData;
    double m_lastAngle1;
    double m_lastAngle2;
    qint64 m_lastTime;
    bool m_firstDataReceived;
};

#endif // MOTORCONTROL_H
