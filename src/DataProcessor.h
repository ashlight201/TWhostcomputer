#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <QObject>
#include <QVector>
#include <functional>

class DataProcessor : public QObject
{
    Q_OBJECT

public:
    explicit DataProcessor(QObject *parent = nullptr);
    
    // 力矩传感器数据处理接口
    double processTorqueSensorData(const QByteArray &rawData);
    
    // 电机数据处理接口
    double processMotorData(const QByteArray &rawData);
    
    // 标定系数计算
    double calculateCalibrationCoefficient(const QVector<double> &torques, 
                                          const QVector<double> &currents);
    
    // 设置自定义处理函数（可选）
    void setTorqueSensorProcessor(std::function<double(const QByteArray&)> processor);
    void setMotorProcessor(std::function<double(const QByteArray&)> processor);

signals:
    void torqueProcessed(double torque);
    void currentProcessed(double current);
    void coefficientCalculated(double coefficient);

private:
    // 默认处理函数（简单示例）
    double defaultTorqueProcessor(const QByteArray &data);
    double defaultMotorProcessor(const QByteArray &data);
    
    std::function<double(const QByteArray&)> torqueProcessor;
    std::function<double(const QByteArray&)> motorProcessor;
};

#endif // DATAPROCESSOR_H
