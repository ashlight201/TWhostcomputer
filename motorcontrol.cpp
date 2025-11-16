#include "motorcontrol.h"

#include <QMetaType>
#include <QRegularExpression>
#include <QSerialPortInfo>
#include <QtMath>

#include <algorithm>

namespace {
constexpr int kDefaultRequestIntervalMs = 50;

QString formatAngleWithSign(int value) {
    if (value >= 0) {
        return QStringLiteral("+") + QString::number(value);
    }
    return QString::number(value);
}
}  // namespace

MotorControl::MotorControl(QObject *parent)
    : QObject(parent) {
    qRegisterMetaType<double>("double");

    requestTimer_.setInterval(kDefaultRequestIntervalMs);
    requestTimer_.setSingleShot(false);
    connect(&requestTimer_, &QTimer::timeout, this, &MotorControl::requestRealtimeData);
    connect(&serial_, &QSerialPort::readyRead, this, &MotorControl::handleReadyRead);
}

bool MotorControl::open(const QString &portName, qint32 baudRate) {
    if (serial_.isOpen()) {
        serial_.close();
    }

    serial_.setPortName(portName);
    serial_.setBaudRate(baudRate);
    serial_.setDataBits(QSerialPort::Data8);
    serial_.setParity(QSerialPort::NoParity);
    serial_.setStopBits(QSerialPort::OneStop);
    serial_.setFlowControl(QSerialPort::NoFlowControl);

    if (!serial_.open(QIODevice::ReadWrite)) {
        emit logText(tr("串口 %1 打开失败: %2").arg(portName, serial_.errorString()));
        return false;
    }

    emit logText(tr("串口 %1 已打开, 波特率 %2").arg(portName).arg(baudRate));
    elapsedTimer_.restart();
    requestTimer_.start();
    return true;
}

bool MotorControl::openFirstAvailable(qint32 baudRate) {
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        const QString location = !info.systemLocation().isEmpty() ? info.systemLocation() : info.portName();
        if (open(location, baudRate)) {
            return true;
        }
    }

    emit logText(tr("没有可用串口, 请手动调用 open()."));
    return false;
}

void MotorControl::close() {
    requestTimer_.stop();
    if (serial_.isOpen()) {
        serial_.close();
        emit logText(tr("串口已关闭"));
    }
}

bool MotorControl::isOpen() const {
    return serial_.isOpen();
}

void MotorControl::setCoefficients(double stiffness, double damping, double gravity) {
    stiffness_ = stiffness;
    damping_ = damping;
    gravityComp_ = gravity;
    emit logText(tr("参数已更新: K=%1, B=%2, G=%3").arg(stiffness_).arg(damping_).arg(gravityComp_));
}

void MotorControl::setSecondaryMotorAngle(double angle) {
    secondaryMotorAngle_ = std::clamp(angle, minAngleDeg_, maxAngleDeg_);
}

void MotorControl::setRequestInterval(int intervalMs) {
    const int safeInterval = std::max(10, intervalMs);
    requestTimer_.setInterval(safeInterval);
}

void MotorControl::recalibrateInitialPosition() {
    initialAngle_ = std::numeric_limits<double>::quiet_NaN();
    lastAngle_ = std::numeric_limits<double>::quiet_NaN();
    elapsedTimer_.restart();
    emit logText(tr("初始角度将根据下一帧反馈重新标定"));
}

void MotorControl::requestRealtimeData() {
    if (!serial_.isOpen()) {
        return;
    }

    if (awaitingFeedback_) {
        return;
    }

    // 仅请求实时数据, 维持当前目标.
    const QString frame = buildFrame(lastTargetAngle_, secondaryMotorAngle_, true);
    writeFrame(frame);
}

void MotorControl::handleReadyRead() {
    readBuffer_.append(QString::fromUtf8(serial_.readAll()));

    int newlineIndex = -1;
    while ((newlineIndex = readBuffer_.indexOf('\n')) != -1) {
        const QString line = readBuffer_.left(newlineIndex).trimmed();
        readBuffer_.remove(0, newlineIndex + 1);
        if (!line.isEmpty()) {
            processFeedbackLine(line);
        }
    }
}

void MotorControl::processFeedbackLine(const QString &line) {
    awaitingFeedback_ = false;

    static const QRegularExpression regex(
        QStringLiteral(R"(angle:\s*(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+current:\s*(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)mA)"),
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = regex.match(line);
    if (!match.hasMatch()) {
        emit logText(tr("无法解析反馈: %1").arg(line));
        return;
    }

    const double angle1 = match.captured(1).toDouble();
    const double angle2 = match.captured(2).toDouble();
    const double current1 = match.captured(3).toDouble();
    const double current2 = match.captured(4).toDouble();
    Q_UNUSED(angle2);
    Q_UNUSED(current2);

    double dt = 0.0;
    if (elapsedTimer_.isValid()) {
        dt = elapsedTimer_.restart() / 1000.0;
    } else {
        elapsedTimer_.start();
    }

    double velocity = 0.0;
    if (std::isfinite(lastAngle_) && dt > 1e-6) {
        velocity = (angle1 - lastAngle_) / dt;
    }
    lastAngle_ = angle1;

    emit jointObservation(angle1, velocity, current1);
    executeImpedanceControl(angle1, velocity);
}

void MotorControl::executeImpedanceControl(double currentAngle, double currentVelocity) {
    if (!std::isfinite(stiffness_) || qFuzzyIsNull(stiffness_)) {
        emit logText(tr("刚度为0或非法, 跳过阻抗计算"));
        return;
    }

    if (!std::isfinite(initialAngle_)) {
        initialAngle_ = currentAngle;
        emit logText(tr("记录初始角度: %1 度").arg(initialAngle_));
        return;
    }

    const double positionError = initialAngle_ - currentAngle;
    const double velocityError = -currentVelocity;
    const double impedanceForce = stiffness_ * positionError + damping_ * velocityError + gravityComp_;

    double targetAngle = currentAngle + (impedanceForce / stiffness_);
    targetAngle = std::clamp(targetAngle, minAngleDeg_, maxAngleDeg_);
    lastTargetAngle_ = targetAngle;

    const QString frame = buildFrame(targetAngle, secondaryMotorAngle_, true);
    writeFrame(frame);
}

QString MotorControl::buildFrame(double primaryAngleDeg, double secondaryAngleDeg, bool requestRealtime) const {
    const int primaryInt = qRound(std::clamp(primaryAngleDeg, minAngleDeg_, maxAngleDeg_));
    const int secondaryInt = qRound(std::clamp(secondaryAngleDeg, minAngleDeg_, maxAngleDeg_));

    QString frame = requestRealtime ? QStringLiteral("A") : QString();
    frame += QStringLiteral("a");
    frame += QString::number(primaryInt);
    frame += formatAngleWithSign(secondaryInt);
    frame += QStringLiteral("+*");
    frame += QLatin1Char('\n');
    return frame;
}

void MotorControl::writeFrame(const QString &frame) {
    if (!serial_.isOpen()) {
        emit logText(tr("串口未打开, 无法发送: %1").arg(frame.trimmed()));
        return;
    }

    const QByteArray payload = frame.toUtf8();
    if (serial_.write(payload) == -1) {
        emit logText(tr("发送失败: %1").arg(serial_.errorString()));
    } else {
        emit logText(tr("发送: %1").arg(QString::fromUtf8(payload).trimmed()));
        awaitingFeedback_ = true;
    }

    serial_.flush();
}
