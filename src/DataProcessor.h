#pragma once

#include <QObject>
#include <QByteArray>
#include <optional>

class DataProcessor final : public QObject
{
    Q_OBJECT

public:
    struct Sample
    {
        double rawValue = 0.0;
        double filteredValue = 0.0;
    };

    explicit DataProcessor(QObject* parent = nullptr);

    Sample processTorqueFrame(const QByteArray& frame);
    Sample processMotorFrame(const QByteArray& frame);
    double computeCoefficient(double torqueNm, double currentAmp) const;

    void setSmoothingFactor(double alpha);
    double smoothingFactor() const noexcept { return m_alpha; }

signals:
    void coefficientUpdated(double value);

private:
    Sample processGenericFrame(const QByteArray& frame, Sample& state, const QString& channelName);
    static std::optional<double> extractFirstNumber(const QByteArray& frame);

    Sample m_torqueSample;
    Sample m_motorSample;
    double m_alpha = 0.2;
};
