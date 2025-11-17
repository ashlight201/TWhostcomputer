#include "DataPlotWidget.h"

#include <QPainter>
#include <QVBoxLayout>

#include <limits>
#include <cmath>

using namespace QtCharts;

DataPlotWidget::DataPlotWidget(QWidget *parent)
    : QWidget(parent)
{
    setupChart();
}

void DataPlotWidget::appendSample(double torqueNm, double currentA, double coefficient)
{
    const qreal x = m_clock.isValid() ? m_clock.elapsed() / 1000.0 : 0.0;

    if (std::isfinite(torqueNm)) {
        m_torqueSeries->append(x, torqueNm);
    }

    if (std::isfinite(currentA)) {
        m_currentSeries->append(x, currentA);
    }

    if (std::isfinite(coefficient)) {
        m_coefficientSeries->append(x, coefficient);
    }

    trimSeries(m_torqueSeries);
    trimSeries(m_currentSeries);
    trimSeries(m_coefficientSeries);

    const double start = qMax(0.0, x - m_viewDurationSec);
    m_axisX->setRange(start, start + m_viewDurationSec);

    updateAxisRanges();
}

void DataPlotWidget::clear()
{
    m_torqueSeries->clear();
    m_currentSeries->clear();
    m_coefficientSeries->clear();
    m_clock.restart();
    m_axisX->setRange(0.0, m_viewDurationSec);
    m_axisYLeft->setRange(-10.0, 10.0);
    m_axisYRight->setRange(0.0, 10.0);
}

void DataPlotWidget::setupChart()
{
    m_chart = new QChart();
    m_chart->legend()->setVisible(true);
    m_chart->setTitle(tr("实时数据曲线"));

    m_torqueSeries = new QLineSeries(this);
    m_torqueSeries->setName(tr("力矩 (N·m)"));

    m_currentSeries = new QLineSeries(this);
    m_currentSeries->setName(tr("电流 (A)"));

    m_coefficientSeries = new QLineSeries(this);
    m_coefficientSeries->setName(tr("系数"));

    m_chart->addSeries(m_torqueSeries);
    m_chart->addSeries(m_currentSeries);
    m_chart->addSeries(m_coefficientSeries);

    m_axisX = new QValueAxis(this);
    m_axisX->setTitleText(tr("时间 (s)"));
    m_axisX->setRange(0.0, m_viewDurationSec);

    m_axisYLeft = new QValueAxis(this);
    m_axisYLeft->setTitleText(tr("力矩/电流"));
    m_axisYLeft->setRange(-10.0, 10.0);

    m_axisYRight = new QValueAxis(this);
    m_axisYRight->setTitleText(tr("系数"));
    m_axisYRight->setRange(0.0, 10.0);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisYLeft, Qt::AlignLeft);
    m_chart->addAxis(m_axisYRight, Qt::AlignRight);

    m_torqueSeries->attachAxis(m_axisX);
    m_torqueSeries->attachAxis(m_axisYLeft);

    m_currentSeries->attachAxis(m_axisX);
    m_currentSeries->attachAxis(m_axisYLeft);

    m_coefficientSeries->attachAxis(m_axisX);
    m_coefficientSeries->attachAxis(m_axisYRight);

    m_chartView = new QChartView(m_chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_chartView);

    m_clock.start();
}

void DataPlotWidget::trimSeries(QLineSeries *series)
{
    if (!series) {
        return;
    }

    if (series->count() <= m_maxSamples) {
        return;
    }

    const int removeCount = series->count() - m_maxSamples;
    series->removePoints(0, removeCount);
}

std::pair<double, double> DataPlotWidget::findRange(const QLineSeries *series) const
{
    if (!series || series->isEmpty()) {
        const double nan = std::numeric_limits<double>::quiet_NaN();
        return {nan, nan};
    }

    double minV = std::numeric_limits<double>::max();
    double maxV = std::numeric_limits<double>::lowest();

    for (const QPointF &point : series->points()) {
        if (!std::isfinite(point.y())) {
            continue;
        }
        minV = std::min(minV, point.y());
        maxV = std::max(maxV, point.y());
    }

    if (minV == std::numeric_limits<double>::max()) {
        const double nan = std::numeric_limits<double>::quiet_NaN();
        return {nan, nan};
    }

    return {minV, maxV};
}

void DataPlotWidget::updateAxisRanges()
{
    const auto torqueRange = findRange(m_torqueSeries);
    const auto currentRange = findRange(m_currentSeries);

    double minLeft = std::numeric_limits<double>::quiet_NaN();
    double maxLeft = std::numeric_limits<double>::quiet_NaN();

    if (std::isfinite(torqueRange.first)) {
        minLeft = torqueRange.first;
        maxLeft = torqueRange.second;
    }

    if (std::isfinite(currentRange.first)) {
        if (std::isfinite(minLeft)) {
            minLeft = std::min(minLeft, currentRange.first);
            maxLeft = std::max(maxLeft, currentRange.second);
        } else {
            minLeft = currentRange.first;
            maxLeft = currentRange.second;
        }
    }

    if (std::isfinite(minLeft) && std::isfinite(maxLeft)) {
        double span = maxLeft - minLeft;
        if (qFuzzyIsNull(span)) {
            span = qMax(1.0, qAbs(maxLeft));
        }
        const double padding = qMax(1.0, span * 0.1);
        m_axisYLeft->setRange(minLeft - padding, maxLeft + padding);
    }

    const auto coeffRange = findRange(m_coefficientSeries);
    if (std::isfinite(coeffRange.first) && std::isfinite(coeffRange.second)) {
        double span = coeffRange.second - coeffRange.first;
        if (qFuzzyIsNull(span)) {
            span = qMax(0.1, qAbs(coeffRange.second));
        }
        const double padding = qMax(0.1, span * 0.1);
        m_axisYRight->setRange(coeffRange.first - padding, coeffRange.second + padding);
    }
}
