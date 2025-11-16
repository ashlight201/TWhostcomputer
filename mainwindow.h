#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextEdit>
#include "motorcontrol.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConfirmClicked();
    void onConnectClicked();
    void onDisconnectClicked();
    void onCaptureInitialClicked();
    void onStartControlClicked();
    void onStopControlClicked();
    void updateDisplayData();
    void handleError(const QString &error);
    void handleStatusChange(const QString &status);

private:
    void setupUI();
    void createParameterGroup();
    void createDataDisplayGroup();
    void createControlGroup();
    void createConnectionGroup();
    
    // UI组件 - 参数输入
    QLineEdit *KCoefficient_lineEdit;
    QLineEdit *BCoefficient_lineEdit;
    QLineEdit *Gravity_lineEdit;
    QPushButton *confirm_pushButton;
    
    // UI组件 - 数据显示
    QLineEdit *JointDegree_lineEdit;
    QLineEdit *JointVelocity_lineEdit;
    QLineEdit *Current_lineEdit;
    
    // UI组件 - 控制按钮
    QPushButton *captureInitial_pushButton;
    QPushButton *startControl_pushButton;
    QPushButton *stopControl_pushButton;
    
    // UI组件 - 连接控制
    QComboBox *serialPort_comboBox;
    QComboBox *baudRate_comboBox;
    QPushButton *connect_pushButton;
    QPushButton *disconnect_pushButton;
    QPushButton *refresh_pushButton;
    
    // 状态显示
    QTextEdit *status_textEdit;
    QLabel *connectionStatus_label;
    
    // 电机控制对象
    MotorControl *m_motorControl;
    
    // 私有方法
    void refreshSerialPorts();
    void appendStatus(const QString &message);
};

#endif // MAINWINDOW_H
