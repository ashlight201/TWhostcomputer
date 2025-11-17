#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QChartView>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>

#include "SerialPortManager.h"
#include "DataProcessor.h"

QT_BEGIN_NAMESPACE
class QChart;
class QChartView;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 串口1（力矩传感器）相关槽函数
    void onTorquePortOpen();
    void onTorquePortClose();
    void onTorquePortSend();
    void onTorqueRawDataReceived(const QByteArray &data);
    void onTorquePortError(QSerialPort::SerialPortError error);

    // 串口2（电机）相关槽函数
    void onMotorPortOpen();
    void onMotorPortClose();
    void onMotorPortSend();
    void onMotorRawDataReceived(const QByteArray &data);
    void onMotorPortError(QSerialPort::SerialPortError error);

    // 数据处理相关槽函数
    void onTorqueProcessed(double torque);
    void onCurrentProcessed(double current);
    void onCoefficientCalculated(double coefficient);

    // 更新图表
    void updateChart();

    // 标定相关
    void onStartCalibration();
    void onStopCalibration();
    void onClearData();

private:
    void setupUI();
    void setupChart();
    void refreshPortList();
    void updateCoefficientDisplay();

    // 串口管理器
    SerialPortManager *m_torquePortManager;
    SerialPortManager *m_motorPortManager;
    
    // 数据处理器
    DataProcessor *m_dataProcessor;

    // UI组件 - 力矩传感器串口
    QGroupBox *m_torquePortGroup;
    QComboBox *m_torquePortCombo;
    QComboBox *m_torqueBaudCombo;
    QPushButton *m_torqueOpenBtn;
    QPushButton *m_torqueCloseBtn;
    QTextEdit *m_torqueRawDisplay;
    QTextEdit *m_torqueProcessedDisplay;
    QTextEdit *m_torqueSendEdit;

    // UI组件 - 电机串口
    QGroupBox *m_motorPortGroup;
    QComboBox *m_motorPortCombo;
    QComboBox *m_motorBaudCombo;
    QPushButton *m_motorOpenBtn;
    QPushButton *m_motorCloseBtn;
    QTextEdit *m_motorRawDisplay;
    QTextEdit *m_motorProcessedDisplay;
    QTextEdit *m_motorSendEdit;

    // UI组件 - 数据显示
    QGroupBox *m_dataDisplayGroup;
    QLabel *m_torqueValueLabel;
    QLabel *m_currentValueLabel;
    QLabel *m_coefficientLabel;

    // UI组件 - 图表
    QChart *m_chart;
    QChartView *m_chartView;
    QLineSeries *m_torqueSeries;
    QLineSeries *m_currentSeries;
    QLineSeries *m_coefficientSeries;
    QValueAxis *m_axisY;
    QDateTimeAxis *m_axisX;

    // UI组件 - 标定控制
    QGroupBox *m_calibrationGroup;
    QPushButton *m_startCalibrationBtn;
    QPushButton *m_stopCalibrationBtn;
    QPushButton *m_clearDataBtn;

    // 数据存储
    QVector<double> m_torqueValues;
    QVector<double> m_currentValues;
    QVector<double> m_coefficientValues;
    QVector<QDateTime> m_timeStamps;

    // 定时器
    QTimer *m_chartUpdateTimer;
    
    // 标定状态
    bool m_calibrating;
    QVector<double> m_calibrationTorques;
    QVector<double> m_calibrationCurrents;
};

#endif // MAINWINDOW_H
