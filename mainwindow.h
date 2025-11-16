#pragma once

#include <QMainWindow>

class QLineEdit;
class QPushButton;
class QTextEdit;
class QTimer;
class MotorControl;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    void handleConfirmClicked();
    void updateJointTelemetry(double angleDeg, double velocityDegPerS, double currentmA);
    void showError(const QString &message);
    void appendControlFrame(const QString &frame);

private:
    void buildUi();
    void bindSignals();
    void startRealtimeRequests();

    MotorControl *m_motorControl {nullptr};
    QTimer *m_requestTimer {nullptr};

    QLineEdit *m_kEdit {nullptr};
    QLineEdit *m_bEdit {nullptr};
    QLineEdit *m_gEdit {nullptr};

    QLineEdit *m_angleEdit {nullptr};
    QLineEdit *m_velEdit {nullptr};
    QLineEdit *m_currentEdit {nullptr};

    QPushButton *m_confirmButton {nullptr};
    QTextEdit *m_logView {nullptr};
};
