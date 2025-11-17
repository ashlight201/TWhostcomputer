#include "DataProcessor.h"
#include <QDebug>
#include <algorithm>
#include <numeric>
#include <cmath>

DataProcessor::DataProcessor(QObject *parent)
    : QObject(parent)
{
    // 设置默认处理函数
    torqueProcessor = [this](const QByteArray &data) {
        return defaultTorqueProcessor(data);
    };
    
    motorProcessor = [this](const QByteArray &data) {
        return defaultMotorProcessor(data);
    };
}

double DataProcessor::processTorqueSensorData(const QByteArray &rawData)
{
    double torque = torqueProcessor(rawData);
    emit torqueProcessed(torque);
    return torque;
}

double DataProcessor::processMotorData(const QByteArray &rawData)
{
    double current = motorProcessor(rawData);
    emit currentProcessed(current);
    return current;
}

double DataProcessor::calculateCalibrationCoefficient(const QVector<double> &torques, 
                                                      const QVector<double> &currents)
{
    if (torques.size() != currents.size() || torques.size() < 2) {
        return 0.0;
    }
    
    // 使用最小二乘法计算标定系数
    // 力矩 = 系数 × 电流
    // 使用线性回归: coefficient = Σ(torque * current) / Σ(current^2)
    
    double sumTorqueCurrent = 0.0;
    double sumCurrentSquared = 0.0;
    
    for (int i = 0; i < torques.size(); ++i) {
        if (currents[i] != 0.0) {
            sumTorqueCurrent += torques[i] * currents[i];
            sumCurrentSquared += currents[i] * currents[i];
        }
    }
    
    if (sumCurrentSquared == 0.0) {
        return 0.0;
    }
    
    double coefficient = sumTorqueCurrent / sumCurrentSquared;
    emit coefficientCalculated(coefficient);
    return coefficient;
}

void DataProcessor::setTorqueSensorProcessor(std::function<double(const QByteArray&)> processor)
{
    if (processor) {
        torqueProcessor = processor;
    }
}

void DataProcessor::setMotorProcessor(std::function<double(const QByteArray&)> processor)
{
    if (processor) {
        motorProcessor = processor;
    }
}

double DataProcessor::defaultTorqueProcessor(const QByteArray &data)
{
    // 默认处理函数：简单示例
    // 这里假设数据格式，实际使用时需要根据具体协议修改
    // 示例：假设前4个字节是浮点数（小端序）
    
    if (data.size() >= 4) {
        // 这里是一个示例，实际需要根据力矩传感器的数据协议来解析
        // 假设数据是IEEE 754单精度浮点数（小端序）
        union {
            float f;
            quint8 bytes[4];
        } converter;
        
        for (int i = 0; i < 4 && i < data.size(); ++i) {
            converter.bytes[i] = static_cast<quint8>(data[i]);
        }
        
        return static_cast<double>(converter.f);
    }
    
    // 如果数据格式不匹配，返回一个基于数据长度的模拟值（仅用于测试）
    return data.size() * 0.1;
}

double DataProcessor::defaultMotorProcessor(const QByteArray &data)
{
    // 默认处理函数：简单示例
    // 这里假设数据格式，实际使用时需要根据具体协议修改
    // 示例：假设前4个字节是浮点数（小端序）
    
    if (data.size() >= 4) {
        // 这里是一个示例，实际需要根据电机的数据协议来解析
        // 假设数据是IEEE 754单精度浮点数（小端序）
        union {
            float f;
            quint8 bytes[4];
        } converter;
        
        for (int i = 0; i < 4 && i < data.size(); ++i) {
            converter.bytes[i] = static_cast<quint8>(data[i]);
        }
        
        return static_cast<double>(converter.f);
    }
    
    // 如果数据格式不匹配，返回一个基于数据长度的模拟值（仅用于测试）
    return data.size() * 0.05;
}
