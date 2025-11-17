#include "DataProcessor.h"

#include <QLoggingCategory>
#include <QRegularExpression>

#include <cmath>

Q_LOGGING_CATEGORY(dataProcessorLog, "data.processor")

namespace
{
constexpr double kEpsilon = 1e-6;
}

DataProcessor::DataProcessor(QObject* parent)
    : QObject(parent)
{
}

DataProcessor::Sample DataProcessor::processTorqueFrame(const QByteArray& frame)
{
    return processGenericFrame(frame, m_torqueSample, QStringLiteral("torque"));
}

DataProcessor::Sample DataProcessor::processMotorFrame(const QByteArray& frame)
{
    return processGenericFrame(frame, m_motorSample, QStringLiteral("motor"));
}

double DataProcessor::computeCoefficient(double torqueNm, double currentAmp) const
{
    if (std::abs(currentAmp) < kEpsilon)
    {
        return 0.0;
    }

    return torqueNm / currentAmp;
}

void DataProcessor::setSmoothingFactor(double alpha)
{
    if (alpha < 0.0)
    {
        alpha = 0.0;
    }
    else if (alpha > 1.0)
    {
        alpha = 1.0;
    }
    m_alpha = alpha;
}

DataProcessor::Sample DataProcessor::processGenericFrame(const QByteArray& frame, Sample& state, const QString& channelName)
{
    const auto maybeValue = extractFirstNumber(frame);
    if (!maybeValue.has_value())
    {
        qCWarning(dataProcessorLog) << "未能在" << channelName << "帧中解析出数值, frame=" << frame;
        return state;
    }

    state.rawValue = maybeValue.value();
    state.filteredValue = m_alpha * state.rawValue + (1.0 - m_alpha) * state.filteredValue;
    return state;
}

std::optional<double> DataProcessor::extractFirstNumber(const QByteArray& frame)
{
    static const QRegularExpression numberRegex(QStringLiteral(R"([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)"));
    const QString payload = QString::fromUtf8(frame);
    const auto match = numberRegex.match(payload);
    if (!match.hasMatch())
    {
        return std::nullopt;
    }

    bool ok = false;
    const double value = match.captured(0).toDouble(&ok);
    if (!ok)
    {
        return std::nullopt;
    }
    return value;
}
