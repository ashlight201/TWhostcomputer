#include "MainWindow.h"

#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QTime>
#include <QWidget>

#include <cmath>

namespace
{
const int kMaxRawLines = 200;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_processor = new DataProcessor(this);
    m_torqueConnection = new SerialConnection(this);
    m_motorConnection = new SerialConnection(this);

    m_torqueEndpoint = new SerialEndpointWidget(tr("力矩传感器串口"), this);
    m_motorEndpoint = new SerialEndpointWidget(tr("电机串口"), this);
    m_plotWidget = new DataPlotWidget(this);

    buildUi();

    connect(m_torqueEndpoint, &SerialEndpointWidget::connectRequested, this, [this](const SerialSettings &settings) {
        if (m_torqueConnection->open(settings)) {
            m_torqueEndpoint->setConnected(true);
        }
    });
    connect(m_torqueEndpoint, &SerialEndpointWidget::disconnectRequested, this, [this]() {
        m_torqueConnection->close();
        m_torqueEndpoint->setConnected(false);
    });
    connect(m_torqueEndpoint, &SerialEndpointWidget::sendRequested, m_torqueConnection, &SerialConnection::sendFrame);
    connect(m_torqueConnection, &SerialConnection::frameReceived, this, &MainWindow::handleTorqueFrame);
    connect(m_torqueConnection, &SerialConnection::statusChanged, this, &MainWindow::handleTorqueStatus);
    connect(m_torqueConnection, &SerialConnection::errorOccurred, this, &MainWindow::handleTorqueError);

    connect(m_motorEndpoint, &SerialEndpointWidget::connectRequested, this, [this](const SerialSettings &settings) {
        if (m_motorConnection->open(settings)) {
            m_motorEndpoint->setConnected(true);
        }
    });
    connect(m_motorEndpoint, &SerialEndpointWidget::disconnectRequested, this, [this]() {
        m_motorConnection->close();
        m_motorEndpoint->setConnected(false);
    });
    connect(m_motorEndpoint, &SerialEndpointWidget::sendRequested, m_motorConnection, &SerialConnection::sendFrame);
    connect(m_motorConnection, &SerialConnection::frameReceived, this, &MainWindow::handleMotorFrame);
    connect(m_motorConnection, &SerialConnection::statusChanged, this, &MainWindow::handleMotorStatus);
    connect(m_motorConnection, &SerialConnection::errorOccurred, this, &MainWindow::handleMotorError);

    connect(m_processor, &DataProcessor::coefficientUpdated, this, &MainWindow::updateCoefficientDisplay);

    resize(1320, 900);
}

void MainWindow::buildUi()
{
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(12);

    auto *dataGrid = new QGridLayout();
    dataGrid->setSpacing(10);
    dataGrid->addWidget(createRawBox(tr("力矩原始帧"), m_torqueRawViewer), 0, 0);
    dataGrid->addWidget(createProcessedBox(tr("力矩处理数据"), m_torqueProcessedLabel), 0, 1);
    dataGrid->addWidget(createRawBox(tr("电机原始帧"), m_motorRawViewer), 1, 0);
    dataGrid->addWidget(createProcessedBox(tr("电机处理数据"), m_motorProcessedLabel), 1, 1);

    auto *chartBox = new QGroupBox(tr("实时曲线"), this);
    auto *chartLayout = new QVBoxLayout(chartBox);
    chartLayout->addWidget(m_plotWidget);

    auto *coefficientBox = new QGroupBox(tr("实时标定结果"), this);
    auto *coeffLayout = new QVBoxLayout(coefficientBox);
    m_coefficientValueLabel = new QLabel(tr("--"), coefficientBox);
    m_coefficientValueLabel->setAlignment(Qt::AlignCenter);
    m_coefficientValueLabel->setStyleSheet(QStringLiteral("font-size: 32px; font-weight: bold;"));
    m_lastUpdateLabel = new QLabel(tr("等待数据..."), coefficientBox);
    m_lastUpdateLabel->setAlignment(Qt::AlignCenter);
    coeffLayout->addWidget(m_coefficientValueLabel);
    coeffLayout->addWidget(m_lastUpdateLabel);

    auto *serialLayout = new QHBoxLayout();
    serialLayout->setSpacing(12);
    serialLayout->addWidget(m_torqueEndpoint);
    serialLayout->addWidget(m_motorEndpoint);
    serialLayout->setStretch(0, 1);
    serialLayout->setStretch(1, 1);

    mainLayout->addLayout(dataGrid);
    mainLayout->addWidget(chartBox);
    mainLayout->addWidget(coefficientBox);
    mainLayout->addLayout(serialLayout);

    setCentralWidget(central);
}

QGroupBox *MainWindow::createRawBox(const QString &title, QPlainTextEdit *&viewer)
{
    auto *box = new QGroupBox(title, this);
    auto *layout = new QVBoxLayout(box);
    viewer = new QPlainTextEdit(box);
    viewer->setReadOnly(true);
    viewer->setMaximumBlockCount(kMaxRawLines);
    layout->addWidget(viewer);
    return box;
}

QGroupBox *MainWindow::createProcessedBox(const QString &title, QLabel *&valueLabel)
{
    auto *box = new QGroupBox(title, this);
    auto *layout = new QVBoxLayout(box);
    valueLabel = new QLabel(tr("--"), box);
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setStyleSheet(QStringLiteral("font-size: 28px; font-weight: 600;"));
    layout->addWidget(valueLabel);
    return box;
}

void MainWindow::appendFrame(QPlainTextEdit *viewer, const QByteArray &frame)
{
    if (!viewer) {
        return;
    }

    const QString line = QStringLiteral("[%1] %2")
                             .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss.zzz")),
                                  formatHex(frame));
    viewer->appendPlainText(line);

    if (viewer->blockCount() > kMaxRawLines) {
        QTextCursor cursor(viewer->document());
        cursor.movePosition(QTextCursor::Start);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar();
    }
}

QString MainWindow::formatHex(const QByteArray &frame) const
{
    return frame.isEmpty() ? QStringLiteral("(空)") : QString::fromLatin1(frame.toHex(' ').toUpper());
}

void MainWindow::tryUpdateCoefficient()
{
    if (!m_lastTorque.valid || !m_lastMotor.valid) {
        return;
    }

    const double coefficient = m_processor->updateCoefficient(m_lastTorque, m_lastMotor);
    if (std::isfinite(coefficient)) {
        m_currentCoefficient = coefficient;
        updateCoefficientDisplay(coefficient);
    }

    m_plotWidget->appendSample(
        m_lastTorque.valid ? m_lastTorque.torqueNm : std::numeric_limits<double>::quiet_NaN(),
        m_lastMotor.valid ? m_lastMotor.phaseCurrentA : std::numeric_limits<double>::quiet_NaN(),
        std::isfinite(m_currentCoefficient) ? m_currentCoefficient : std::numeric_limits<double>::quiet_NaN());
}

void MainWindow::updateCoefficientDisplay(double value)
{
    if (std::isfinite(value)) {
        m_coefficientValueLabel->setText(QString::number(value, 'f', 5));
        m_lastUpdateLabel->setText(tr("最后刷新: %1").arg(QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss"))));
    } else {
        m_coefficientValueLabel->setText(QStringLiteral("--"));
        m_lastUpdateLabel->setText(tr("等待数据..."));
    }
}

void MainWindow::handleTorqueFrame(const QByteArray &frame)
{
    appendFrame(m_torqueRawViewer, frame);
    m_torqueEndpoint->appendLogLine(tr("RX %1").arg(formatHex(frame)));

    m_lastTorque = m_processor->processTorqueFrame(frame);
    if (m_lastTorque.valid) {
        m_torqueProcessedLabel->setText(QString::number(m_lastTorque.torqueNm, 'f', 3) + QStringLiteral(" N·m"));
    }

    tryUpdateCoefficient();
}

void MainWindow::handleMotorFrame(const QByteArray &frame)
{
    appendFrame(m_motorRawViewer, frame);
    m_motorEndpoint->appendLogLine(tr("RX %1").arg(formatHex(frame)));

    m_lastMotor = m_processor->processMotorFrame(frame);
    if (m_lastMotor.valid) {
        m_motorProcessedLabel->setText(QString::number(m_lastMotor.phaseCurrentA, 'f', 3) + QStringLiteral(" A"));
    }

    tryUpdateCoefficient();
}

void MainWindow::handleTorqueStatus(const QString &status, bool connected)
{
    m_torqueEndpoint->showStatusMessage(status);
    m_torqueEndpoint->setConnected(connected);
}

void MainWindow::handleMotorStatus(const QString &status, bool connected)
{
    m_motorEndpoint->showStatusMessage(status);
    m_motorEndpoint->setConnected(connected);
}

void MainWindow::handleTorqueError(const QString &message)
{
    m_torqueEndpoint->appendLogLine(message);
    QMessageBox::warning(this, tr("力矩串口错误"), message);
}

void MainWindow::handleMotorError(const QString &message)
{
    m_motorEndpoint->appendLogLine(message);
    QMessageBox::warning(this, tr("电机串口错误"), message);
}
