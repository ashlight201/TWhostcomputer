#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include "motorcontrol.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConfirmClicked();
    void onDataReceived(const MotorData &data);
    void onControlTimer();
    void onErrorOccurred(const QString &error);

private:
    void setupUI();
    void calculateImpedanceControl();
    void updateDisplay();
    
    // UI组件
    QLineEdit *m_kCoefficientEdit;
    QLineEdit *m_bCoefficientEdit;
    QLineEdit *m_gravityEdit;
    QPushButton *m_confirmButton;
    
    QLineEdit *m_jointDegreeEdit;
    QLineEdit *m_jointVelocityEdit;
    QLineEdit *m_currentEdit;
    
    // 电机控制
    MotorControl *m_motorControl;
    
    // 控制参数
    double m_kCoefficient;      // 刚度系数
    double m_bCoefficient;      // 阻尼系数
    double m_gravityCompensation; // 重力补偿项
    
    // 状态变量
    double m_initialAngle1;      // 初始关节角度1
    double m_initialAngle2;      // 初始关节角度2
    double m_currentAngle1;      // 当前关节角度1
    double m_currentAngle2;      // 当前关节角度2
    double m_lastAngle1;         // 上一次关节角度1
    double m_lastAngle2;         // 上一次关节角度2
    double m_velocity1;          // 关节速度1
    double m_velocity2;          // 关节速度2
    double m_current1;           // 电流1 (mA)
    double m_current2;           // 电流2 (mA)
    
    bool m_initialPositionSet;  // 是否已设置初始位置
    bool m_controlActive;        // 控制是否激活
    
    QTimer *m_controlTimer;      // 控制定时器
    qint64 m_lastControlTime;    // 上次控制时间
    qint64 m_lastDataTime;        // 上次数据接收时间
};

#endif // MAINWINDOW_H
