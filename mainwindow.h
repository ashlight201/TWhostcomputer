#pragma once

#include <QMainWindow>

class QLineEdit;
class QPushButton;
class QPlainTextEdit;

class MotorControl;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void handleConfirmClicked();
    void handleJointObservation(double angle, double velocity, double current);
    void handleLogText(const QString &text);

private:
    void setupUi();
    void connectSignals();
    void autoConnectSerial();

    QLineEdit *kCoefficientLineEdit_{nullptr};
    QLineEdit *bCoefficientLineEdit_{nullptr};
    QLineEdit *gravityLineEdit_{nullptr};
    QPushButton *confirmButton_{nullptr};

    QLineEdit *jointDegreeLineEdit_{nullptr};
    QLineEdit *jointVelocityLineEdit_{nullptr};
    QLineEdit *jointCurrentLineEdit_{nullptr};

    QPlainTextEdit *logViewer_{nullptr};

    MotorControl *motorControl_{nullptr};
};
