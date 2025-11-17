#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTimer>
#include "SerialPortManager.h"
#include "DataProcessor.h"
#include "ChartWidget.h"

QT_BEGIN_NAMESPACE
class QSerialPort;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTorqueSensorDataReceived(const QByteArray &data);
    void onMotorDataReceived(const QByteArray &data);
    void onTorqueSensorProcessedData(double torque);
    void onMotorProcessedData(double current);
    void onCalibrationCoefficientCalculated(double coefficient);
    
    void onTorqueSensorConnect();
    void onTorqueSensorDisconnect();
    void onMotorConnect();
    void onMotorDisconnect();
    void onTorqueSensorSend();
    void onMotorSend();
    void onStartCalibration();
    void onClearData();

private:
    void setupUI();
    void setupConnections();
    void updateCalibrationCoefficient();
    
    // UI组件
    QWidget *centralWidget;
    
    // 力矩传感器串口调试区域
    QGroupBox *torqueSensorGroup;
    QComboBox *torqueSensorPortCombo;
    QComboBox *torqueSensorBaudCombo;
    QPushButton *torqueSensorConnectBtn;
    QPushButton *torqueSensorDisconnectBtn;
    QTextEdit *torqueSensorRawData;
    QTextEdit *torqueSensorProcessedData;
    QLineEdit *torqueSensorSendEdit;
    QPushButton *torqueSensorSendBtn;
    
    // 电机串口调试区域
    QGroupBox *motorGroup;
    QComboBox *motorPortCombo;
    QComboBox *motorBaudCombo;
    QPushButton *motorConnectBtn;
    QPushButton *motorDisconnectBtn;
    QTextEdit *motorRawData;
    QTextEdit *motorProcessedData;
    QLineEdit *motorSendEdit;
    QPushButton *motorSendBtn;
    
    // 标定系数显示
    QGroupBox *calibrationGroup;
    QLabel *coefficientLabel;
    QLabel *coefficientValueLabel;
    QPushButton *startCalibrationBtn;
    QPushButton *clearDataBtn;
    
    // 实时图表
    ChartWidget *chartWidget;
    
    // 功能模块
    SerialPortManager *torqueSensorSerial;
    SerialPortManager *motorSerial;
    DataProcessor *dataProcessor;
    
    // 数据存储（用于标定）
    QVector<double> torqueValues;
    QVector<double> currentValues;
    
    QTimer *updateTimer;
};

#endif // MAINWINDOW_H
