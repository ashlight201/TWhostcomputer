#pragma once

#include "DataPlotWidget.h"
#include "DataProcessor.h"
#include "SerialEndpointWidget.h"
#include "SerialConnection.h"

#include <QMainWindow>
#include <limits>

class QLabel;
class QPlainTextEdit;
class QGroupBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    void buildUi();
    QGroupBox *createRawBox(const QString &title, QPlainTextEdit *&viewer);
    QGroupBox *createProcessedBox(const QString &title, QLabel *&valueLabel);
    void appendFrame(QPlainTextEdit *viewer, const QByteArray &frame);
    QString formatHex(const QByteArray &frame) const;
    void tryUpdateCoefficient();
    void updateCoefficientDisplay(double value);

private slots:
    void handleTorqueFrame(const QByteArray &frame);
    void handleMotorFrame(const QByteArray &frame);
    void handleTorqueStatus(const QString &status, bool connected);
    void handleMotorStatus(const QString &status, bool connected);
    void handleTorqueError(const QString &message);
    void handleMotorError(const QString &message);

private:
    SerialEndpointWidget *m_torqueEndpoint = nullptr;
    SerialEndpointWidget *m_motorEndpoint = nullptr;
    SerialConnection *m_torqueConnection = nullptr;
    SerialConnection *m_motorConnection = nullptr;
    DataProcessor *m_processor = nullptr;
    DataPlotWidget *m_plotWidget = nullptr;

    QPlainTextEdit *m_torqueRawViewer = nullptr;
    QPlainTextEdit *m_motorRawViewer = nullptr;
    QLabel *m_torqueProcessedLabel = nullptr;
    QLabel *m_motorProcessedLabel = nullptr;
    QLabel *m_coefficientValueLabel = nullptr;
    QLabel *m_lastUpdateLabel = nullptr;

    TorqueSample m_lastTorque;
    MotorSample m_lastMotor;
    double m_currentCoefficient = std::numeric_limits<double>::quiet_NaN();
};
