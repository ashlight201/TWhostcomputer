#include "DataProcessor.h"

#include <QtEndian>
#include <QtMath>

#include <limits>
#include <cstring>

namespace
{
double parseAsciiNumber(const QByteArray &frame, bool &ok)
{
    ok = false;
    const QByteArray trimmed = frame.trimmed();
    if (trimmed.isEmpty()) {
        return 0.0;
    }

    const QString asText = QString::fromLatin1(trimmed);
    double value = asText.toDouble(&ok);
    if (ok) {
        return value;
    }

    bool signedOk = false;
    const qlonglong intValue = asText.toLongLong(&signedOk, 0);
    if (signedOk) {
        ok = true;
        return static_cast<double>(intValue);
    }

    return 0.0;
}
} // namespace

DataProcessor::DataProcessor(QObject *parent)
    : QObject(parent)
{
}

TorqueSample DataProcessor::processTorqueFrame(const QByteArray &frame)
{
    TorqueSample sample;
    sample.timestamp = QDateTime::currentDateTime();
    sample.rawFrame = frame;

    if (frame.isEmpty()) {
        return sample;
    }

    const double rawValue = extractSignedValue(frame, m_lastTorque);
    m_lastTorque = rawValue;
    sample.torqueNm = rawValue * m_torqueScale + m_torqueOffset;
    sample.valid = true;
    return sample;
}

MotorSample DataProcessor::processMotorFrame(const QByteArray &frame)
{
    MotorSample sample;
    sample.timestamp = QDateTime::currentDateTime();
    sample.rawFrame = frame;

    if (frame.isEmpty()) {
        return sample;
    }

    const double rawValue = extractSignedValue(frame, m_lastCurrent);
    m_lastCurrent = rawValue;
    sample.phaseCurrentA = rawValue * m_currentScale;
    sample.valid = true;
    return sample;
}

double DataProcessor::updateCoefficient(const TorqueSample &torque, const MotorSample &motor)
{
    if (!torque.valid || !motor.valid) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    if (qFuzzyIsNull(motor.phaseCurrentA)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    const double coefficient = torque.torqueNm / motor.phaseCurrentA;
    m_recentCoefficients.enqueue(coefficient);
    while (m_recentCoefficients.size() > m_smoothingWindow) {
        m_recentCoefficients.dequeue();
    }

    double sum = 0.0;
    for (double value : m_recentCoefficients) {
        sum += value;
    }
    const double averaged = sum / m_recentCoefficients.size();

    emit coefficientUpdated(averaged);
    return averaged;
}

void DataProcessor::reset()
{
    m_recentCoefficients.clear();
    m_lastTorque = 0.0;
    m_lastCurrent = 0.0;
}

void DataProcessor::setTorqueScale(double scale)
{
    m_torqueScale = scale;
}

void DataProcessor::setTorqueOffset(double offset)
{
    m_torqueOffset = offset;
}

void DataProcessor::setCurrentScale(double scale)
{
    m_currentScale = scale;
}

void DataProcessor::setSmoothingWindow(int window)
{
    m_smoothingWindow = qMax(1, window);
    while (m_recentCoefficients.size() > m_smoothingWindow) {
        m_recentCoefficients.dequeue();
    }
}

double DataProcessor::extractSignedValue(const QByteArray &frame, double fallback) const
{
    if (frame.isEmpty()) {
        return fallback;
    }

    bool ok = false;
    const double asciiValue = parseAsciiNumber(frame, ok);
    if (ok) {
        return asciiValue;
    }

    if (frame.size() >= static_cast<int>(sizeof(qint16))) {
        qint16 raw16 = 0;
        memcpy(&raw16, frame.constData(), sizeof(qint16));
        raw16 = qFromLittleEndian<qint16>(raw16);
        return static_cast<double>(raw16);
    }

    int sum = 0;
    for (const auto byte : frame) {
        sum += static_cast<unsigned char>(byte);
    }
    return sum == 0 ? fallback : static_cast<double>(sum);
}
