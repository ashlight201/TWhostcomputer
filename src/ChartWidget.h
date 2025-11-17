#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QTimer>

// Qt Charts 兼容性处理
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
QT_CHARTS_USE_NAMESPACE
#else
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
QT_CHARTS_USE_NAMESPACE
#endif

class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = nullptr);
    ~ChartWidget();
    
    void addTorqueData(double torque, qint64 timestamp);
    void addCurrentData(double current, qint64 timestamp);
    void addCoefficientData(double coefficient, qint64 timestamp);
    void clearData();
    void setTimeRange(int seconds);

private slots:
    void updateChart();

private:
    void setupChart();
    
    QChart *chart;
    QChartView *chartView;
    
    QLineSeries *torqueSeries;
    QLineSeries *currentSeries;
    QLineSeries *coefficientSeries;
    
    QValueAxis *axisX;
    QValueAxis *axisY;
    
    QTimer *updateTimer;
    
    struct DataPoint {
        qint64 timestamp;
        double value;
    };
    
    QVector<DataPoint> torqueData;
    QVector<DataPoint> currentData;
    QVector<DataPoint> coefficientData;
    
    int timeRangeSeconds;
    qint64 startTime;
};

#endif // CHARTWIDGET_H
