#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtMath>
#include <QtGlobal>

#include <QComboBox>
#include <QDateTime>
#include <QDoubleSpinBox>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QStatusBar>
#include <QStringList>

#include <algorithm>

namespace
{
constexpr int kStatusMessageDurationMs = 4000;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(std::make_unique<Ui::MainWindow>())
    , m_torqueChannel(std::make_unique<SerialChannelManager>(this))
    , m_motorChannel(std::make_unique<SerialChannelManager>(this))
    , m_processor(std::make_unique<DataProcessor>(this))
{
    ui->setupUi(this);
    initializeUi();
    setupChart();
    bindSignals();
    refreshPortList();
}

MainWindow::~MainWindow() = default;

void MainWindow::initializeUi()
{
    populateBaudRateCombo(ui->torqueBaudCombo);
    populateBaudRateCombo(ui->motorBaudCombo);
    ui->torqueProcessedLabel->setText(QStringLiteral("处理结果: -"));
    ui->motorProcessedLabel->setText(QStringLiteral("处理结果: -"));
    ui->coefficientDisplay->display(0.0);
    statusBar()->showMessage(QStringLiteral("请先连接串口并开始接收数据"));
}

void MainWindow::setupChart()
{
    m_chart = new QtCharts::QChart();
    m_chart->setTitle(QStringLiteral("力矩/电流/系数实时走势"));
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    m_torqueSeries = new QtCharts::QLineSeries(m_chart);
    m_torqueSeries->setName(QStringLiteral("力矩(Nm)"));

    m_currentSeries = new QtCharts::QLineSeries(m_chart);
    m_currentSeries->setName(QStringLiteral("电流(A)"));

    m_coefficientSeries = new QtCharts::QLineSeries(m_chart);
    m_coefficientSeries->setName(QStringLiteral("系数k(Nm/A)"));

    m_chart->addSeries(m_torqueSeries);
    m_chart->addSeries(m_currentSeries);
    m_chart->addSeries(m_coefficientSeries);

    auto* axisX = new QtCharts::QValueAxis();
    axisX->setTitleText(QStringLiteral("样本序号"));
    axisX->setRange(0.0, static_cast<double>(m_maxDataPoints));

    auto* axisY = new QtCharts::QValueAxis();
    axisY->setTitleText(QStringLiteral("数值"));
    axisY->setRange(-10.0, 10.0);

    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_chart->addAxis(axisY, Qt::AlignLeft);

    for (auto* series : {m_torqueSeries, m_currentSeries, m_coefficientSeries})
    {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    ui->chartView->setChart(m_chart);
}

void MainWindow::bindSignals()
{
    connect(ui->torqueConnectButton, &QPushButton::clicked, this, &MainWindow::handleTorqueConnectionToggle);
    connect(ui->motorConnectButton, &QPushButton::clicked, this, &MainWindow::handleMotorConnectionToggle);
    connect(ui->torqueRefreshButton, &QPushButton::clicked, this, &MainWindow::refreshPortList);
    connect(ui->motorRefreshButton, &QPushButton::clicked, this, &MainWindow::refreshPortList);
    connect(ui->autoCalibrateButton, &QPushButton::clicked, this, &MainWindow::handleAutoCalibrate);
    connect(ui->resetChartButton, &QPushButton::clicked, this, &MainWindow::handleResetChart);
    connect(ui->smoothingSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &MainWindow::handleSmoothingChanged);

    connect(m_torqueChannel.get(), &SerialChannelManager::serialError, this, [this](const QString& message) {
        QMessageBox::warning(this, QStringLiteral("力矩传感器串口错误"), message);
    });
    connect(m_motorChannel.get(), &SerialChannelManager::serialError, this, [this](const QString& message) {
        QMessageBox::warning(this, QStringLiteral("电机串口错误"), message);
    });

    connect(m_torqueChannel.get(), &SerialChannelManager::connectionStateChanged, this, &MainWindow::updateConnectionLabels);
    connect(m_motorChannel.get(), &SerialChannelManager::connectionStateChanged, this, &MainWindow::updateConnectionLabels);

    m_torqueChannel->setFrameHandler([this](const QByteArray& frame) {
        QMetaObject::invokeMethod(this, [this, frame]() { handleTorqueFrame(frame); }, Qt::QueuedConnection);
    });

    m_motorChannel->setFrameHandler([this](const QByteArray& frame) {
        QMetaObject::invokeMethod(this, [this, frame]() { handleMotorFrame(frame); }, Qt::QueuedConnection);
    });
}

void MainWindow::populateBaudRateCombo(QComboBox* combo)
{
    if (!combo)
    {
        return;
    }

    combo->clear();
    const QList<int> baudRates = {921600, 460800, 230400, 115200, 57600, 38400, 19200, 9600};
    for (int rate : baudRates)
    {
        combo->addItem(QString::number(rate), rate);
    }
    combo->setCurrentIndex(3); // 默认115200
}

void MainWindow::handleTorqueConnectionToggle()
{
    if (m_torqueChannel->isOpen())
    {
        m_torqueChannel->close();
        return;
    }

    const QString portName = ui->torquePortCombo->currentText();
    const int baudRate = ui->torqueBaudCombo->currentData().toInt();

    configureChannel(m_torqueChannel.get(), portName, baudRate);
    if (m_torqueChannel->open())
    {
        statusBar()->showMessage(QStringLiteral("力矩传感器串口已连接"), kStatusMessageDurationMs);
    }
}

void MainWindow::handleMotorConnectionToggle()
{
    if (m_motorChannel->isOpen())
    {
        m_motorChannel->close();
        return;
    }

    const QString portName = ui->motorPortCombo->currentText();
    const int baudRate = ui->motorBaudCombo->currentData().toInt();

    configureChannel(m_motorChannel.get(), portName, baudRate);
    if (m_motorChannel->open())
    {
        statusBar()->showMessage(QStringLiteral("电机串口已连接"), kStatusMessageDurationMs);
    }
}

void MainWindow::refreshPortList()
{
    const QString currentTorque = ui->torquePortCombo->currentText();
    const QString currentMotor = ui->motorPortCombo->currentText();

    ui->torquePortCombo->clear();
    ui->motorPortCombo->clear();

    const auto ports = QSerialPortInfo::availablePorts();
    for (const auto& info : ports)
    {
        const QString name = info.portName();
        ui->torquePortCombo->addItem(name);
        ui->motorPortCombo->addItem(name);
    }

    const auto setCurrentIfAvailable = [](QComboBox* combo, const QString& value) {
        const int index = combo->findText(value);
        if (index != -1)
        {
            combo->setCurrentIndex(index);
        }
    };

    setCurrentIfAvailable(ui->torquePortCombo, currentTorque);
    setCurrentIfAvailable(ui->motorPortCombo, currentMotor);
}

void MainWindow::handleAutoCalibrate()
{
    maybeUpdateChart();
}

void MainWindow::handleResetChart()
{
    for (auto* series : {m_torqueSeries, m_currentSeries, m_coefficientSeries})
    {
        series->clear();
    }

    m_sampleIndex = 0.0;
    statusBar()->showMessage(QStringLiteral("曲线已清空"), kStatusMessageDurationMs);
}

void MainWindow::handleSmoothingChanged(double value)
{
    m_processor->setSmoothingFactor(value);
}

void MainWindow::appendFrameToConsole(QPlainTextEdit* console, const QByteArray& frame)
{
    if (!console)
    {
        return;
    }

    const QString timestamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
    const QString hexPayload = QString::fromLatin1(frame.toHex(' ')).toUpper();
    const QString asciiPayload = QString::fromUtf8(frame);
    const QString message = QStringLiteral("[%1] %2 | HEX: %3").arg(timestamp, asciiPayload, hexPayload);
    console->appendPlainText(message);
}

void MainWindow::updateTorqueDisplay(const DataProcessor::Sample& sample)
{
    const QString text = QStringLiteral("处理结果: 原始=%1 Nm, 滤波=%2 Nm")
                             .arg(sample.rawValue, 0, 'f', 4)
                             .arg(sample.filteredValue, 0, 'f', 4);
    ui->torqueProcessedLabel->setText(text);
}

void MainWindow::updateMotorDisplay(const DataProcessor::Sample& sample)
{
    const QString text = QStringLiteral("处理结果: 原始=%1 A, 滤波=%2 A")
                             .arg(sample.rawValue, 0, 'f', 4)
                             .arg(sample.filteredValue, 0, 'f', 4);
    ui->motorProcessedLabel->setText(text);
}

void MainWindow::notifyCoefficient(double coefficient)
{
    ui->coefficientDisplay->display(coefficient);
    statusBar()->showMessage(QStringLiteral("最新系数 k = %1 Nm/A").arg(coefficient, 0, 'f', 4), kStatusMessageDurationMs);
}

void MainWindow::appendChartPoint(double torqueNm, double currentAmp, double coefficient)
{
    const double index = m_sampleIndex++;
    m_torqueSeries->append(index, torqueNm);
    m_currentSeries->append(index, currentAmp);
    m_coefficientSeries->append(index, coefficient);

    trimSeries(m_torqueSeries);
    trimSeries(m_currentSeries);
    trimSeries(m_coefficientSeries);

    if (m_torqueSeries->count() > 1)
    {
        const double minIndex = std::max(0.0, index - m_maxDataPoints);
        const double maxIndex = minIndex + m_maxDataPoints;
        auto* axisX = qobject_cast<QtCharts::QValueAxis*>(m_chart->axisX());
        if (axisX)
        {
            axisX->setRange(minIndex, maxIndex);
        }
    }
}

void MainWindow::maybeUpdateChart()
{
    const double torque = m_latestTorque.filteredValue;
    const double current = m_latestMotor.filteredValue;

    if (qFuzzyIsNull(current) || qFuzzyIsNull(torque))
    {
        return;
    }

    const double coefficient = m_processor->computeCoefficient(torque, current);
    notifyCoefficient(coefficient);
    appendChartPoint(torque, current, coefficient);
}

void MainWindow::trimSeries(QtCharts::QLineSeries* series)
{
    if (!series)
    {
        return;
    }

    const int extraPoints = series->count() - m_maxDataPoints;
    if (extraPoints > 0)
    {
        series->removePoints(0, extraPoints);
    }
}

void MainWindow::updateConnectionLabels()
{
    ui->torqueStatusLabel->setText(m_torqueChannel->isOpen() ? QStringLiteral("已连接") : QStringLiteral("未连接"));
    ui->motorStatusLabel->setText(m_motorChannel->isOpen() ? QStringLiteral("已连接") : QStringLiteral("未连接"));
}

void MainWindow::configureChannel(SerialChannelManager* channel, const QString& portName, qint32 baudRate)
{
    if (!channel)
    {
        return;
    }

    SerialChannelManager::Settings settings = channel->settings();
    settings.portName = portName;
    settings.baudRate = baudRate;
    channel->setSettings(settings);
}

void MainWindow::handleTorqueFrame(const QByteArray& frame)
{
    appendFrameToConsole(ui->torqueRawText, frame);
    m_latestTorque = m_processor->processTorqueFrame(frame);
    updateTorqueDisplay(m_latestTorque);
    maybeUpdateChart();
}

void MainWindow::handleMotorFrame(const QByteArray& frame)
{
    appendFrameToConsole(ui->motorRawText, frame);
    m_latestMotor = m_processor->processMotorFrame(frame);
    updateMotorDisplay(m_latestMotor);
    maybeUpdateChart();
}
