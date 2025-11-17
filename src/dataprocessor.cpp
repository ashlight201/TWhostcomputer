#include "dataprocessor.h"
#include <QDebug>
#include <QDateTime>
#include <cmath>

DataProcessor::DataProcessor(QObject *parent)
    : QObject(parent)
    , m_coefficient(1.0)
    , m_lastTorque(0.0)
    , m_lastCurrent(0.0)
{
}

DataProcessor::~DataProcessor()
{
}

DataProcessor::TorqueData DataProcessor::processTorqueSensorData(const QByteArray &rawData)
{
    TorqueData data;
    data.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    // 解析原始数据
    data.rawValue = parseTorqueRawData(rawData);
    
    // 处理数据（例如：滤波、单位转换等）
    data.processedValue = processTorqueValue(data.rawValue);
    
    m_lastTorque = data.processedValue;
    
    // 发送信号
    emit torqueDataProcessed(data.rawValue, data.processedValue);
    
    qDebug() << "力矩传感器数据 - 原始:" << data.rawValue << "处理后:" << data.processedValue;
    
    return data;
}

DataProcessor::CurrentData DataProcessor::processMotorCurrentData(const QByteArray &rawData)
{
    CurrentData data;
    data.timestamp = QDateTime::currentMSecsSinceEpoch();
    
    // 解析原始数据
    data.rawValue = parseCurrentRawData(rawData);
    
    // 处理数据（例如：滤波、单位转换等）
    data.processedValue = processCurrentValue(data.rawValue);
    
    m_lastCurrent = data.processedValue;
    
    // 发送信号
    emit currentDataProcessed(data.rawValue, data.processedValue);
    
    qDebug() << "电机电流数据 - 原始:" << data.rawValue << "处理后:" << data.processedValue;
    
    return data;
}

void DataProcessor::addCalibrationPoint(double torque, double current)
{
    // 过滤无效数据
    if (std::isnan(torque) || std::isnan(current) || 
        std::isinf(torque) || std::isinf(current)) {
        qDebug() << "无效的标定数据点，已忽略";
        return;
    }
    
    // 添加标定数据点
    m_torqueValues.append(torque);
    m_currentValues.append(current);
    
    qDebug() << "添加标定点 - 力矩:" << torque << "电流:" << current 
             << "总点数:" << m_torqueValues.size();
    
    emit calibrationPointAdded(m_torqueValues.size());
}

double DataProcessor::calculateCalibrationCoefficient()
{
    int n = m_torqueValues.size();
    
    if (n < 2) {
        qDebug() << "标定数据点不足（需要至少2个点）";
        return m_coefficient;
    }
    
    if (n != m_currentValues.size()) {
        qDebug() << "力矩和电流数据点数量不匹配";
        return m_coefficient;
    }
    
    // 使用最小二乘法计算系数
    // 力矩 = 系数 × 电流
    // 求解: min Σ(torque_i - k * current_i)^2
    // 结果: k = Σ(torque_i * current_i) / Σ(current_i^2)
    
    double sumTorqueCurrent = 0.0;
    double sumCurrentSquared = 0.0;
    
    for (int i = 0; i < n; ++i) {
        sumTorqueCurrent += m_torqueValues[i] * m_currentValues[i];
        sumCurrentSquared += m_currentValues[i] * m_currentValues[i];
    }
    
    if (std::abs(sumCurrentSquared) < 1e-10) {
        qDebug() << "电流数据过小，无法计算系数";
        return m_coefficient;
    }
    
    m_coefficient = sumTorqueCurrent / sumCurrentSquared;
    
    qDebug() << "标定系数计算完成:" << m_coefficient 
             << "使用数据点:" << n;
    
    emit coefficientUpdated(m_coefficient);
    
    return m_coefficient;
}

void DataProcessor::clearCalibrationData()
{
    m_torqueValues.clear();
    m_currentValues.clear();
    qDebug() << "标定数据已清除";
}

int DataProcessor::getCalibrationPointCount() const
{
    return m_torqueValues.size();
}

// ========== 私有函数（示例实现，需根据实际协议修改） ==========

double DataProcessor::parseTorqueRawData(const QByteArray &data)
{
    // TODO: 根据力矩传感器的实际通信协议解析数据
    // 这里提供一个示例实现
    
    if (data.size() < 4) {
        qDebug() << "力矩传感器数据长度不足";
        return 0.0;
    }
    
    // 示例：假设数据格式为4字节浮点数（小端）
    float value;
    memcpy(&value, data.constData(), sizeof(float));
    
    return static_cast<double>(value);
}

double DataProcessor::parseCurrentRawData(const QByteArray &data)
{
    // TODO: 根据电机的实际通信协议解析数据
    // 这里提供一个示例实现
    
    if (data.size() < 4) {
        qDebug() << "电机电流数据长度不足";
        return 0.0;
    }
    
    // 示例：假设数据格式为4字节浮点数（小端）
    float value;
    memcpy(&value, data.constData(), sizeof(float));
    
    return static_cast<double>(value);
}

double DataProcessor::processTorqueValue(double rawValue)
{
    // TODO: 添加数据处理逻辑
    // 例如：滤波、单位转换、零点校准等
    
    // 示例：简单的范围检查
    if (rawValue < -1000.0 || rawValue > 1000.0) {
        qDebug() << "力矩值超出合理范围:" << rawValue;
    }
    
    return rawValue;
}

double DataProcessor::processCurrentValue(double rawValue)
{
    // TODO: 添加数据处理逻辑
    // 例如：滤波、单位转换、零点校准等
    
    // 示例：简单的范围检查
    if (rawValue < -100.0 || rawValue > 100.0) {
        qDebug() << "电流值超出合理范围:" << rawValue;
    }
    
    return rawValue;
}
