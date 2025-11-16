#include "motorcontrol.h"

#include <QtGlobal>
#include <QRegularExpression>

MotorControl::MotorControl(QObject *parent)
    : QObject(parent)
    , m_serial(new QSerialPort(this))
{
    connect(m_serial, &QSerialPort::readyRead, this, &MotorControl::handleReadyRead);
}

bool MotorControl::openPort(const QString &portName, qint32 baudRate)
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }

    m_serial->setPortName(portName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);
    m_serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred(tr("无法打开串口 %1：%2").arg(portName, m_serial->errorString()));
        return false;
    }

    return true;
}

void MotorControl::closePort()
{
    if (m_serial->isOpen()) {
        m_serial->close();
    }
}

void MotorControl::requestRealtimeData()
{
    if (!m_serial->isOpen()) {
        return;
    }

    const QString frame = buildCommandFrame(0.0, 0.0, true);
    m_serial->write(frame.toUtf8());
    emit controlFrameTransmitted(frame);
}

void MotorControl::setCoefficients(double stiffness, double damping, double gravity)
{
    m_stiffness = stiffness;
    m_damping = damping;
    m_gravity = gravity;
    emit coefficientsUpdated(m_stiffness, m_damping, m_gravity);
}

void MotorControl::resetInitialPose()
{
    m_hasInitialPose = false;
}

void MotorControl::handleReadyRead()
{
    m_rxBuffer.append(m_serial->readAll());

    int newlineIndex = -1;
    while ((newlineIndex = m_rxBuffer.indexOf('\n')) != -1) {
        const QByteArray line = m_rxBuffer.left(newlineIndex).trimmed();
        m_rxBuffer.remove(0, newlineIndex + 1);
        if (!line.isEmpty()) {
            processFeedbackLine(QString::fromUtf8(line));
        }
    }
}

void MotorControl::processFeedbackLine(const QString &line)
{
    static const QRegularExpression feedbackPattern(
        R"(angle:\s*(-?\d+(?:\.\d+)?)\s+(-?\d+(?:\.\d+)?)\s+current:\s*(-?\d+(?:\.\d+)?)\s*(-?\d+(?:\.\d+)?)mA)",
        QRegularExpression::CaseInsensitiveOption);

    const QRegularExpressionMatch match = feedbackPattern.match(line);
    if (!match.hasMatch()) {
        emit errorOccurred(tr("无法解析反馈数据：%1").arg(line));
        return;
    }

    JointFeedback feedback;
    feedback.angle1 = match.captured(1).toDouble();
    feedback.angle2 = match.captured(2).toDouble();
    feedback.current1 = match.captured(3).toDouble();
    feedback.current2 = match.captured(4).toDouble();
    feedback.timestamp = QDateTime::currentDateTime();

    computeAndTransmit(feedback);
}

void MotorControl::computeAndTransmit(const JointFeedback &feedback)
{
    if (!m_hasInitialPose) {
        m_initialAngle1 = feedback.angle1;
        m_initialAngle2 = feedback.angle2;
        m_hasInitialPose = true;
    }

    double dtSeconds = 0.0;
    if (m_hasLastFeedback) {
        const qint64 deltaMs = m_lastFeedback.timestamp.msecsTo(feedback.timestamp);
        if (deltaMs > 0) {
            dtSeconds = deltaMs / 1000.0;
        }
    }

    const double velocity1 = (m_hasLastFeedback && dtSeconds > 0.0)
        ? (feedback.angle1 - m_lastFeedback.angle1) / dtSeconds
        : 0.0;
    const double velocity2 = (m_hasLastFeedback && dtSeconds > 0.0)
        ? (feedback.angle2 - m_lastFeedback.angle2) / dtSeconds
        : 0.0;

    const double positionError1 = m_initialAngle1 - feedback.angle1;
    const double positionError2 = m_initialAngle2 - feedback.angle2;
    const double velocityError1 = -velocity1;
    const double velocityError2 = -velocity2;

    if (qFuzzyIsNull(m_stiffness) || qFuzzyCompare(qAbs(m_stiffness), 0.0)) {
        emit errorOccurred(tr("刚度系数为零，无法计算阻抗控制指令"));
        return;
    }

    const double impedance1 = m_stiffness * positionError1 + m_damping * velocityError1 + m_gravity;
    const double impedance2 = m_stiffness * positionError2 + m_damping * velocityError2 + m_gravity;

    const double targetAngle1 = feedback.angle1 + impedance1 / m_stiffness;
    const double targetAngle2 = feedback.angle2 + impedance2 / m_stiffness;

    if (m_serial->isOpen()) {
        const QString frame = buildCommandFrame(targetAngle1, targetAngle2, false);
        m_serial->write(frame.toUtf8());
        emit controlFrameTransmitted(frame);
    }

    emit jointStateUpdated(feedback.angle1, velocity1, feedback.current1);

    m_lastFeedback = feedback;
    m_hasLastFeedback = true;
}

QString MotorControl::buildCommandFrame(double motor1Deg, double motor2Deg, bool requestFeedback) const
{
    auto encodeAngle = [](double angle) -> QString {
        const int scaled = qRound(angle);
        if (scaled >= 0) {
            return QString::number(scaled);
        }
        return QStringLiteral("-") + QString::number(qAbs(scaled));
    };

    QString frame;
    if (requestFeedback) {
        frame.append('A');
    }
    frame.append('a');
    frame.append(encodeAngle(motor1Deg));
    frame.append(encodeAngle(motor2Deg));
    frame.append("+*");
    return frame;
}
