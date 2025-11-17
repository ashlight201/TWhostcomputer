#pragma once

#include "DataProcessor.h"
#include "SerialChannelManager.h"

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QComboBox>
#include <QMainWindow>
#include <QPlainTextEdit>

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void handleTorqueConnectionToggle();
    void handleMotorConnectionToggle();
    void refreshPortList();
    void handleAutoCalibrate();
    void handleResetChart();
    void handleSmoothingChanged(double value);

private:
    void initializeUi();
    void setupChart();
    void bindSignals();
    void populateBaudRateCombo(QComboBox* combo);
    void appendFrameToConsole(QPlainTextEdit* console, const QByteArray& frame);
    void updateTorqueDisplay(const DataProcessor::Sample& sample);
    void updateMotorDisplay(const DataProcessor::Sample& sample);
    void notifyCoefficient(double coefficient);
    void appendChartPoint(double torqueNm, double currentAmp, double coefficient);
    void maybeUpdateChart();
    void trimSeries(QtCharts::QLineSeries* series);
    void updateConnectionLabels();
    void configureChannel(SerialChannelManager* channel, const QString& portName, qint32 baudRate);
    void handleTorqueFrame(const QByteArray& frame);
    void handleMotorFrame(const QByteArray& frame);

    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<SerialChannelManager> m_torqueChannel;
    std::unique_ptr<SerialChannelManager> m_motorChannel;
    std::unique_ptr<DataProcessor> m_processor;

    QtCharts::QChart* m_chart = nullptr;
    QtCharts::QLineSeries* m_torqueSeries = nullptr;
    QtCharts::QLineSeries* m_currentSeries = nullptr;
    QtCharts::QLineSeries* m_coefficientSeries = nullptr;

    qreal m_sampleIndex = 0.0;
    int m_maxDataPoints = 600;

    DataProcessor::Sample m_latestTorque;
    DataProcessor::Sample m_latestMotor;
};
