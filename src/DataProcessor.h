#pragma once

#include <QObject>
#include <QByteArray>
#include <QDateTime>
#include <QQueue>

struct TorqueSample
{
    QDateTime timestamp;
    double torqueNm = 0.0;
    QByteArray rawFrame;
    bool valid = false;
};

struct MotorSample
{
    QDateTime timestamp;
    double phaseCurrentA = 0.0;
    QByteArray rawFrame;
    bool valid = false;
};

class DataProcessor : public QObject
{
    Q_OBJECT

public:
    explicit DataProcessor(QObject *parent = nullptr);

    TorqueSample processTorqueFrame(const QByteArray &frame);
    MotorSample processMotorFrame(const QByteArray &frame);
    double updateCoefficient(const TorqueSample &torque, const MotorSample &motor);
    void reset();

    void setTorqueScale(double scale);
    void setTorqueOffset(double offset);
    void setCurrentScale(double scale);
    void setSmoothingWindow(int window);

signals:
    void coefficientUpdated(double value);

private:
    double extractSignedValue(const QByteArray &frame, double fallback) const;

    double m_torqueScale = 0.001;
    double m_torqueOffset = 0.0;
    double m_currentScale = 0.001;
    int m_smoothingWindow = 32;
    QQueue<double> m_recentCoefficients;
    double m_lastTorque = 0.0;
    double m_lastCurrent = 0.0;
};
