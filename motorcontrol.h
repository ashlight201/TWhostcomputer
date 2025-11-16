#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QTimer>

class MotorControl : public QObject
{
    Q_OBJECT
public:
    explicit MotorControl(QObject *parent = nullptr);
    ~MotorControl();

    // 串口操作
    bool openSerialPort(const QString &portName, int baudRate = 115200);
    void closeSerialPort();
    bool isConnected() const;

    // 阻抗控制参数设置
    void setImpedanceParameters(double K, double B, double G);
    
    // 控制操作
    void startImpedanceControl();
    void stopImpedanceControl();
    void captureInitialPosition();
    
    // 获取当前状态
    double getCurrentAngle1() const { return m_currentAngle1; }
    double getCurrentAngle2() const { return m_currentAngle2; }
    double getCurrentVelocity1() const { return m_currentVelocity1; }
    double getCurrentVelocity2() const { return m_currentVelocity2; }
    double getCurrent1() const { return m_current1; }
    double getCurrent2() const { return m_current2; }

signals:
    void dataUpdated();  // 数据更新信号
    void errorOccurred(const QString &error);  // 错误信号
    void statusChanged(const QString &status);  // 状态变化信号

private slots:
    void readSerialData();
    void controlLoop();

private:
    // 串口
    QSerialPort *m_serialPort;
    
    // 定时器
    QTimer *m_controlTimer;  // 控制循环定时器
    
    // 阻抗控制参数
    double m_K;  // 刚度系数
    double m_B;  // 阻尼系数
    double m_G;  // 重力补偿
    
    // 位置和速度
    double m_initialAngle1;   // 初始关节1角度
    double m_initialAngle2;   // 初始关节2角度
    double m_currentAngle1;   // 当前关节1角度
    double m_currentAngle2;   // 当前关节2角度
    double m_lastAngle1;      // 上一次关节1角度（用于计算速度）
    double m_lastAngle2;      // 上一次关节2角度（用于计算速度）
    double m_currentVelocity1;  // 当前关节1速度
    double m_currentVelocity2;  // 当前关节2速度
    
    // 电流
    double m_current1;
    double m_current2;
    
    // 控制标志
    bool m_isControlling;
    bool m_initialPositionCaptured;
    
    // 时间控制
    qint64 m_lastTime;
    
    // 接收数据缓冲区
    QString m_receivedData;
    
    // 私有方法
    void parseReceivedData(const QString &data);
    void sendControlCommand(double angle1, double angle2);
    void requestRealtimeData();
    double calculateImpedanceForce(double positionError, double velocityError);
};

#endif // MOTORCONTROL_H
