#include "ChartWidget.h"
#include <QVBoxLayout>
#include <QDateTime>
#include <QPainter>

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
    , chart(new QChart())
    , chartView(new QChartView(chart))
    , torqueSeries(new QLineSeries())
    , currentSeries(new QLineSeries())
    , coefficientSeries(new QLineSeries())
    , axisX(new QValueAxis())
    , axisY(new QValueAxis())
    , timeRangeSeconds(60)
    , startTime(QDateTime::currentMSecsSinceEpoch())
{
    setupChart();
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(chartView);
    
    // 设置定时器用于更新图表
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &ChartWidget::updateChart);
    updateTimer->start(100); // 每100ms更新一次
    
    setMinimumSize(600, 400);
}

ChartWidget::~ChartWidget()
{
}

void ChartWidget::setupChart()
{
    // 设置系列名称和颜色
    torqueSeries->setName("力矩 (N·m)");
    torqueSeries->setColor(QColor(255, 0, 0)); // 红色
    
    currentSeries->setName("电流 (A)");
    currentSeries->setColor(QColor(0, 255, 0)); // 绿色
    
    coefficientSeries->setName("标定系数");
    coefficientSeries->setColor(QColor(0, 0, 255)); // 蓝色
    
    // 添加到图表
    chart->addSeries(torqueSeries);
    chart->addSeries(currentSeries);
    chart->addSeries(coefficientSeries);
    
    // 设置坐标轴
    axisX->setTitleText("时间 (秒)");
    axisX->setLabelFormat("%.1f");
    axisX->setRange(0, timeRangeSeconds);
    
    axisY->setTitleText("数值");
    axisY->setLabelFormat("%.3f");
    axisY->setRange(-10, 10);
    
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    
    torqueSeries->attachAxis(axisX);
    torqueSeries->attachAxis(axisY);
    currentSeries->attachAxis(axisX);
    currentSeries->attachAxis(axisY);
    coefficientSeries->attachAxis(axisX);
    coefficientSeries->attachAxis(axisY);
    
    // 设置图表标题和图例
    chart->setTitle("实时数据监控");
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);
    
    // 设置图表视图
    chartView->setRenderHint(QPainter::Antialiasing);
}

void ChartWidget::addTorqueData(double torque, qint64 timestamp)
{
    DataPoint point;
    point.timestamp = timestamp;
    point.value = torque;
    torqueData.append(point);
    
    // 限制数据点数量，避免内存溢出
    if (torqueData.size() > 10000) {
        torqueData.removeFirst();
    }
}

void ChartWidget::addCurrentData(double current, qint64 timestamp)
{
    DataPoint point;
    point.timestamp = timestamp;
    point.value = current;
    currentData.append(point);
    
    // 限制数据点数量
    if (currentData.size() > 10000) {
        currentData.removeFirst();
    }
}

void ChartWidget::addCoefficientData(double coefficient, qint64 timestamp)
{
    DataPoint point;
    point.timestamp = timestamp;
    point.value = coefficient;
    coefficientData.append(point);
    
    // 限制数据点数量
    if (coefficientData.size() > 10000) {
        coefficientData.removeFirst();
    }
}

void ChartWidget::clearData()
{
    torqueData.clear();
    currentData.clear();
    coefficientData.clear();
    torqueSeries->clear();
    currentSeries->clear();
    coefficientSeries->clear();
    startTime = QDateTime::currentMSecsSinceEpoch();
}

void ChartWidget::setTimeRange(int seconds)
{
    timeRangeSeconds = seconds;
    axisX->setRange(0, timeRangeSeconds);
}

void ChartWidget::updateChart()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 timeOffset = currentTime - startTime;
    double currentTimeSeconds = timeOffset / 1000.0;
    
    // 更新X轴范围（滑动窗口）
    if (currentTimeSeconds > timeRangeSeconds) {
        double minTime = currentTimeSeconds - timeRangeSeconds;
        axisX->setRange(minTime, currentTimeSeconds);
    }
    
    // 更新力矩数据
    torqueSeries->clear();
    for (const auto &point : torqueData) {
        double timeSeconds = (point.timestamp - startTime) / 1000.0;
        if (timeSeconds >= axisX->min() && timeSeconds <= axisX->max()) {
            torqueSeries->append(timeSeconds, point.value);
        }
    }
    
    // 更新电流数据
    currentSeries->clear();
    for (const auto &point : currentData) {
        double timeSeconds = (point.timestamp - startTime) / 1000.0;
        if (timeSeconds >= axisX->min() && timeSeconds <= axisX->max()) {
            currentSeries->append(timeSeconds, point.value);
        }
    }
    
    // 更新标定系数数据
    coefficientSeries->clear();
    for (const auto &point : coefficientData) {
        double timeSeconds = (point.timestamp - startTime) / 1000.0;
        if (timeSeconds >= axisX->min() && timeSeconds <= axisX->max()) {
            coefficientSeries->append(timeSeconds, point.value);
        }
    }
    
    // 自动调整Y轴范围
    double minValue = 0, maxValue = 0;
    bool first = true;
    
    for (const auto &point : torqueData) {
        double timeSeconds = (point.timestamp - startTime) / 1000.0;
        if (timeSeconds >= axisX->min() && timeSeconds <= axisX->max()) {
            if (first) {
                minValue = maxValue = point.value;
                first = false;
            } else {
                minValue = qMin(minValue, point.value);
                maxValue = qMax(maxValue, point.value);
            }
        }
    }
    
    for (const auto &point : currentData) {
        double timeSeconds = (point.timestamp - startTime) / 1000.0;
        if (timeSeconds >= axisX->min() && timeSeconds <= axisX->max()) {
            minValue = qMin(minValue, point.value);
            maxValue = qMax(maxValue, point.value);
        }
    }
    
    for (const auto &point : coefficientData) {
        double timeSeconds = (point.timestamp - startTime) / 1000.0;
        if (timeSeconds >= axisX->min() && timeSeconds <= axisX->max()) {
            minValue = qMin(minValue, point.value);
            maxValue = qMax(maxValue, point.value);
        }
    }
    
    if (!first) {
        double range = maxValue - minValue;
        if (range < 0.1) range = 0.1; // 最小范围
        axisY->setRange(minValue - range * 0.1, maxValue + range * 0.1);
    }
}
