#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 数据处理类
 * 负责处理力矩传感器和电机数据，计算标定系数
 */
class DataProcessor : public QObject
{
    Q_OBJECT

public:
    struct TorqueData {
        double rawValue;      // 原始值
        double processedValue; // 处理后的值
        qint64 timestamp;      // 时间戳
    };
    
    struct CurrentData {
        double rawValue;      // 原始值
        double processedValue; // 处理后的值
        qint64 timestamp;      // 时间戳
    };

    explicit DataProcessor(QObject *parent = nullptr);
    ~DataProcessor();

    // 处理力矩传感器数据
    TorqueData processTorqueSensorData(const QByteArray &rawData);
    
    // 处理电机电流数据
    CurrentData processMotorCurrentData(const QByteArray &rawData);
    
    // 添加数据点用于标定
    void addCalibrationPoint(double torque, double current);
    
    // 计算标定系数（最小二乘法）
    double calculateCalibrationCoefficient();
    
    // 清除标定数据
    void clearCalibrationData();
    
    // 获取标定数据点数量
    int getCalibrationPointCount() const;
    
    // 获取最后处理的力矩值
    double getLastTorque() const { return m_lastTorque; }
    
    // 获取最后处理的电流值
    double getLastCurrent() const { return m_lastCurrent; }
    
    // 获取标定系数
    double getCoefficient() const { return m_coefficient; }
    
    // 设置标定系数
    void setCoefficient(double coeff) { m_coefficient = coeff; }

signals:
    // 力矩数据已处理
    void torqueDataProcessed(double rawValue, double processedValue);
    
    // 电流数据已处理
    void currentDataProcessed(double rawValue, double processedValue);
    
    // 标定系数已更新
    void coefficientUpdated(double coefficient);
    
    // 添加了新的标定点
    void calibrationPointAdded(int totalPoints);

private:
    // 解析力矩传感器原始数据（示例函数，需根据实际协议修改）
    double parseTorqueRawData(const QByteArray &data);
    
    // 解析电机电流原始数据（示例函数，需根据实际协议修改）
    double parseCurrentRawData(const QByteArray &data);
    
    // 处理力矩传感器数据（示例函数，可添加滤波等）
    double processTorqueValue(double rawValue);
    
    // 处理电机电流数据（示例函数，可添加滤波等）
    double processCurrentValue(double rawValue);

private:
    // 标定数据存储
    QVector<double> m_torqueValues;
    QVector<double> m_currentValues;
    
    // 当前标定系数
    double m_coefficient;
    
    // 最后处理的数据
    double m_lastTorque;
    double m_lastCurrent;
};

#endif // DATAPROCESSOR_H
