#include "DataProcessor.h"
#include <QDebug>
#include <QRegularExpression>
#include <cmath>

DataProcessor::DataProcessor(QObject *parent)
    : QObject(parent)
    , m_torqueSensorProtocol("default")
    , m_motorProtocol("default")
{
}

double DataProcessor::processTorqueSensorData(const QByteArray &rawData)
{
    // TODO: 根据实际协议实现数据解析
    // 这里提供一个示例实现，实际使用时需要根据协议修改
    
    m_torqueBuffer.append(rawData);
    
    // 示例：假设数据格式为 "TORQUE:XX.XX\r\n"
    // 实际使用时需要根据真实协议修改
    if (m_torqueBuffer.contains('\n')) {
        QByteArray frame = m_torqueBuffer.left(m_torqueBuffer.indexOf('\n') + 1);
        m_torqueBuffer.remove(0, frame.size());
        
        double torque = parseTorqueSensorFrame(frame);
        emit torqueDataProcessed(torque);
        return torque;
    }
    
    return 0.0;
}

double DataProcessor::processMotorCurrentData(const QByteArray &rawData)
{
    // TODO: 根据实际协议实现数据解析
    // 这里提供一个示例实现，实际使用时需要根据协议修改
    
    m_motorBuffer.append(rawData);
    
    // 示例：假设数据格式为 "CURRENT:XX.XX\r\n"
    // 实际使用时需要根据真实协议修改
    if (m_motorBuffer.contains('\n')) {
        QByteArray frame = m_motorBuffer.left(m_motorBuffer.indexOf('\n') + 1);
        m_motorBuffer.remove(0, frame.size());
        
        double current = parseMotorDataFrame(frame);
        emit currentDataProcessed(current);
        return current;
    }
    
    return 0.0;
}

double DataProcessor::calculateCalibrationCoefficient(double torque, double current)
{
    if (qAbs(current) < 1e-6) {
        return 0.0; // 避免除零
    }
    double coefficient = torque / current;
    emit coefficientCalculated(coefficient);
    return coefficient;
}

double DataProcessor::calculateCalibrationCoefficient(const QVector<double> &torques, const QVector<double> &currents)
{
    if (torques.size() != currents.size() || torques.isEmpty()) {
        return 0.0;
    }
    
    // 使用最小二乘法计算系数：y = kx，其中y是力矩，x是电流
    // k = Σ(xy) / Σ(x²)
    double sumXY = 0.0;
    double sumX2 = 0.0;
    
    for (int i = 0; i < torques.size(); ++i) {
        if (qAbs(currents[i]) > 1e-6) {
            sumXY += currents[i] * torques[i];
            sumX2 += currents[i] * currents[i];
        }
    }
    
    if (qAbs(sumX2) < 1e-6) {
        return 0.0;
    }
    
    double coefficient = sumXY / sumX2;
    emit coefficientCalculated(coefficient);
    return coefficient;
}

void DataProcessor::setTorqueSensorProtocol(const QString &protocol)
{
    m_torqueSensorProtocol = protocol;
}

void DataProcessor::setMotorProtocol(const QString &protocol)
{
    m_motorProtocol = protocol;
}

double DataProcessor::parseTorqueSensorFrame(const QByteArray &frame)
{
    // TODO: 根据实际协议实现
    // 示例：从字符串中提取数值
    QString str = QString::fromUtf8(frame);
    
    // 示例解析：查找数字
    QRegularExpression rx("([-+]?[0-9]*\\.?[0-9]+)");
    QRegularExpressionMatch match = rx.match(str);
    if (match.hasMatch()) {
        return match.captured(1).toDouble();
    }
    
    return 0.0;
}

double DataProcessor::parseMotorDataFrame(const QByteArray &frame)
{
    // TODO: 根据实际协议实现
    // 示例：从字符串中提取数值
    QString str = QString::fromUtf8(frame);
    
    // 示例解析：查找数字
    QRegularExpression rx("([-+]?[0-9]*\\.?[0-9]+)");
    QRegularExpressionMatch match = rx.match(str);
    if (match.hasMatch()) {
        return match.captured(1).toDouble();
    }
    
    return 0.0;
}
