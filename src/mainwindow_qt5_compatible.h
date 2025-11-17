#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QGroupBox>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

// Qt Charts 头文件 - Qt 5.12 兼容方式
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "serialporthandler.h"
#include "dataprocessor.h"

// 使用 Qt Charts 命名空间 - Qt 5.12 兼容
QT_CHARTS_USE_NAMESPACE

/**
 * @brief 主窗口类（Qt 5.12 兼容版本）
 * 实现电机力矩标定系统的主界面
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 力矩传感器串口操作
    void onTorqueSensorPortOpenClicked();
    void onTorqueSensorPortCloseClicked();
    void onTorqueSensorDataReceived(const QByteArray &data);
    
    // 电机串口操作
    void onMotorPortOpenClicked();
    void onMotorPortCloseClicked();
    void onMotorDataReceived(const QByteArray &data);
    
    // 标定操作
    void onAddCalibrationPointClicked();
    void onCalculateCoefficientClicked();
    void onClearCalibrationDataClicked();
    
    // 数据更新
    void onTorqueDataProcessed(double rawValue, double processedValue);
    void onCurrentDataProcessed(double rawValue, double processedValue);
    void onCoefficientUpdated(double coefficient);
    void onCalibrationPointAdded(int totalPoints);
    
    // 图表更新
    void updateCharts();
    
    // 刷新串口列表
    void refreshPortList();

private:
    // 初始化UI
    void initUI();
    void setupTorqueSensorGroup();
    void setupMotorGroup();
    void setupCalibrationGroup();
    void setupChartsGroup();
    
    // 初始化图表
    void initCharts();
    
    // 更新状态栏
    void updateStatusBar();

private:
    // 串口处理器
    SerialPortHandler *m_torqueSensorPort;
    SerialPortHandler *m_motorPort;
    
    // 数据处理器
    DataProcessor *m_dataProcessor;
    
    // 更新定时器
    QTimer *m_updateTimer;
    
    // ========== 力矩传感器组件 ==========
    QGroupBox *m_torqueSensorGroup;
    QComboBox *m_torquePortCombo;
    QComboBox *m_torqueBaudCombo;
    QPushButton *m_torqueOpenBtn;
    QPushButton *m_torqueCloseBtn;
    QPushButton *m_torqueRefreshBtn;
    QTextEdit *m_torqueRawDataText;
    QLineEdit *m_torqueProcessedDataEdit;
    QLabel *m_torqueStatusLabel;
    
    // ========== 电机组件 ==========
    QGroupBox *m_motorGroup;
    QComboBox *m_motorPortCombo;
    QComboBox *m_motorBaudCombo;
    QPushButton *m_motorOpenBtn;
    QPushButton *m_motorCloseBtn;
    QPushButton *m_motorRefreshBtn;
    QTextEdit *m_motorRawDataText;
    QLineEdit *m_motorProcessedDataEdit;
    QLabel *m_motorStatusLabel;
    
    // ========== 标定组件 ==========
    QGroupBox *m_calibrationGroup;
    QLineEdit *m_coefficientEdit;
    QLineEdit *m_calibrationPointsEdit;
    QPushButton *m_addPointBtn;
    QPushButton *m_calculateBtn;
    QPushButton *m_clearDataBtn;
    QTextEdit *m_calibrationLogText;
    
    // ========== 图表组件 ==========
    QGroupBox *m_chartsGroup;
    QChartView *m_torqueChartView;
    QChartView *m_currentChartView;
    QChartView *m_coefficientChartView;
    
    QChart *m_torqueChart;
    QChart *m_currentChart;
    QChart *m_coefficientChart;
    
    QLineSeries *m_torqueSeries;
    QLineSeries *m_currentSeries;
    QLineSeries *m_coefficientSeries;
    
    QValueAxis *m_torqueAxisX;
    QValueAxis *m_torqueAxisY;
    QValueAxis *m_currentAxisX;
    QValueAxis *m_currentAxisY;
    QValueAxis *m_coeffAxisX;
    QValueAxis *m_coeffAxisY;
    
    // 数据存储（用于图表显示）
    QVector<QPointF> m_torquePoints;
    QVector<QPointF> m_currentPoints;
    QVector<QPointF> m_coefficientPoints;
    
    int m_dataPointCounter;
    static const int MAX_CHART_POINTS = 100;
};

#endif // MAINWINDOW_H
