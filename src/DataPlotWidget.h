#pragma once

#include <QElapsedTimer>
#include <QWidget>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

class DataPlotWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataPlotWidget(QWidget *parent = nullptr);

    void appendSample(double torqueNm, double currentA, double coefficient);
    void clear();

private:
    void setupChart();
    void trimSeries(QtCharts::QLineSeries *series);
    std::pair<double, double> findRange(const QtCharts::QLineSeries *series) const;
    void updateAxisRanges();

    QtCharts::QChart *m_chart = nullptr;
    QtCharts::QChartView *m_chartView = nullptr;
    QtCharts::QLineSeries *m_torqueSeries = nullptr;
    QtCharts::QLineSeries *m_currentSeries = nullptr;
    QtCharts::QLineSeries *m_coefficientSeries = nullptr;
    QtCharts::QValueAxis *m_axisX = nullptr;
    QtCharts::QValueAxis *m_axisYLeft = nullptr;
    QtCharts::QValueAxis *m_axisYRight = nullptr;
    QElapsedTimer m_clock;
    int m_maxSamples = 600;
    double m_viewDurationSec = 60.0;
};
