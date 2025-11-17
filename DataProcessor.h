#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QByteArray>
#include <QVector>

class DataProcessor : public QObject
{
    Q_OBJECT

public:
    explicit DataProcessor(QObject *parent = nullptr);

    // 处理力矩传感器原始数据帧
    // 返回处理后的力矩值（单位：Nm）
    double processTorqueSensorData(const QByteArray &rawData);

    // 处理电机原始数据
    // 返回处理后的电流值（单位：A）
    double processMotorCurrentData(const QByteArray &rawData);

    // 计算标定系数
    // 系数 = 力矩 / 电流
    double calculateCalibrationCoefficient(double torque, double current);

    // 批量计算系数（用于标定）
    // 使用最小二乘法或其他方法
    double calculateCalibrationCoefficient(const QVector<double> &torques, const QVector<double> &currents);

    // 设置数据解析格式（可根据实际协议调整）
    void setTorqueSensorProtocol(const QString &protocol);
    void setMotorProtocol(const QString &protocol);

signals:
    // 数据解析完成
    void torqueDataProcessed(double torque);
    void currentDataProcessed(double current);
    void coefficientCalculated(double coefficient);

private:
    // 解析函数（根据实际协议实现）
    double parseTorqueSensorFrame(const QByteArray &frame);
    double parseMotorDataFrame(const QByteArray &frame);

    QString m_torqueSensorProtocol;
    QString m_motorProtocol;
    
    // 数据缓冲区（用于帧解析）
    QByteArray m_torqueBuffer;
    QByteArray m_motorBuffer;
};

#endif // DATAPROCESSOR_H
