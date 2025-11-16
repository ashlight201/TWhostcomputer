#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QSerialPort>
#include <QTimer>

#include <limits>

class MotorControl : public QObject {
    Q_OBJECT

public:
    explicit MotorControl(QObject *parent = nullptr);

    bool open(const QString &portName, qint32 baudRate = 115200);
    bool openFirstAvailable(qint32 baudRate = 115200);
    void close();
    bool isOpen() const;

    void setCoefficients(double stiffness, double damping, double gravity);
    void setSecondaryMotorAngle(double angle);
    void setRequestInterval(int intervalMs);
    void recalibrateInitialPosition();

signals:
    void jointObservation(double angleDeg, double velocityDegPerSec, double currentMilliAmp);
    void logText(const QString &text);

public slots:
    void requestRealtimeData();

private slots:
    void handleReadyRead();

private:
    void processFeedbackLine(const QString &line);
    void executeImpedanceControl(double currentAngle, double currentVelocity);
    QString buildFrame(double primaryAngleDeg, double secondaryAngleDeg, bool requestRealtime) const;
    void writeFrame(const QString &frame);

    QSerialPort serial_;
    QTimer requestTimer_;
    QElapsedTimer elapsedTimer_;
    QString readBuffer_;

    double stiffness_{50.0};
    double damping_{5.0};
    double gravityComp_{0.0};
    double initialAngle_{std::numeric_limits<double>::quiet_NaN()};
    double lastAngle_{std::numeric_limits<double>::quiet_NaN()};
    double secondaryMotorAngle_{0.0};
    double lastTargetAngle_{0.0};
    bool awaitingFeedback_{false};

    static constexpr double minAngleDeg_{-360.0};
    static constexpr double maxAngleDeg_{360.0};
};
