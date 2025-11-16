#pragma once

#include <QObject>
#include <QDateTime>
#include <QSerialPort>
#include <QByteArray>

struct JointFeedback
{
    double angle1 {0.0};
    double angle2 {0.0};
    double current1 {0.0};
    double current2 {0.0};
    QDateTime timestamp;
};

class MotorControl : public QObject
{
    Q_OBJECT

public:
    explicit MotorControl(QObject *parent = nullptr);

    bool openPort(const QString &portName, qint32 baudRate = 115200);
    void closePort();

    void requestRealtimeData();
    void setCoefficients(double stiffness, double damping, double gravity);
    void resetInitialPose();

    double stiffness() const { return m_stiffness; }
    double damping() const { return m_damping; }
    double gravity() const { return m_gravity; }

signals:
    void jointStateUpdated(double jointAngleDeg, double jointVelocityDegPerS, double jointCurrentmA);
    void controlFrameTransmitted(const QString &frame);
    void errorOccurred(const QString &message);
    void coefficientsUpdated(double stiffness, double damping, double gravity);

private slots:
    void handleReadyRead();

private:
    void processFeedbackLine(const QString &line);
    void computeAndTransmit(const JointFeedback &feedback);
    QString buildCommandFrame(double motor1Deg, double motor2Deg, bool requestFeedback) const;

    QSerialPort *m_serial {nullptr};
    QByteArray m_rxBuffer;

    double m_stiffness {50.0};
    double m_damping {2.5};
    double m_gravity {0.0};

    bool m_hasInitialPose {false};
    double m_initialAngle1 {0.0};
    double m_initialAngle2 {0.0};

    bool m_hasLastFeedback {false};
    JointFeedback m_lastFeedback;
};
