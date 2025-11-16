#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "motorcontrol.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QLabel;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConfirmClicked();
    void onDataReceived(double angle1, double angle2, double current1, double current2);
    void onControlTimer();

private:
    void setupUI();
    void updateDisplay(double angle1, double angle2, double velocity1, double velocity2, 
                       double current1, double current2);
    double calculateControlCommand(double positionError, double velocityError, 
                                   double currentPosition, double kCoeff, double bCoeff, 
                                   double gravity);

    // UI组件
    QLineEdit *m_kCoefficientLineEdit;
    QLineEdit *m_bCoefficientLineEdit;
    QLineEdit *m_gravityLineEdit;
    QPushButton *m_confirmPushButton;
    
    QLineEdit *m_jointDegreeLineEdit;
    QLineEdit *m_jointVelocityLineEdit;
    QLineEdit *m_currentLineEdit;
    
    // 电机控制
    MotorControl *m_motorControl;
    
    // 控制参数
    double m_kCoefficient;
    double m_bCoefficient;
    double m_gravity;
    
    // 初始位置和当前状态
    double m_initialAngle1;
    double m_initialAngle2;
    double m_currentAngle1;
    double m_currentAngle2;
    double m_previousAngle1;
    double m_previousAngle2;
    double m_currentVelocity1;
    double m_currentVelocity2;
    
    // 控制标志
    bool m_controlActive;
    bool m_initialPositionSet;
    
    // 定时器
    QTimer *m_controlTimer;
    QElapsedTimer *m_elapsedTimer;
    qint64 m_lastTime;
};

#endif // MAINWINDOW_H
