#include "mainwindow.h"

#include "motorcontrol.h"

#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTextEdit>
#include <QTimer>
#include <QWidget>
#include <QtGlobal>

namespace {
constexpr int kRequestIntervalMs = 25;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_motorControl(new MotorControl(this))
    , m_requestTimer(new QTimer(this))
{
    buildUi();
    bindSignals();

    const QString defaultPort = qEnvironmentVariable("IMPEDANCE_PORT", "ttyUSB0");
    if (m_motorControl->openPort(defaultPort, 115200)) {
        statusBar()->showMessage(tr("串口 %1 已连接").arg(defaultPort), 3000);
        startRealtimeRequests();
    } else {
        statusBar()->showMessage(tr("串口连接失败"), 3000);
    }
}

void MainWindow::handleConfirmClicked()
{
    bool okK = false;
    bool okB = false;
    bool okG = false;

    const double k = m_kEdit->text().toDouble(&okK);
    const double b = m_bEdit->text().toDouble(&okB);
    const double g = m_gEdit->text().toDouble(&okG);

    if (!(okK && okB && okG)) {
        QMessageBox::warning(this, tr("输入错误"), tr("请输入合法的阻抗参数"));
        return;
    }

    if (qFuzzyIsNull(k)) {
        QMessageBox::warning(this, tr("参数错误"), tr("刚度系数不可为0"));
        return;
    }

    m_motorControl->setCoefficients(k, b, g);
    statusBar()->showMessage(tr("阻抗参数已更新"), 2000);
}

void MainWindow::updateJointTelemetry(double angleDeg, double velocityDegPerS, double currentmA)
{
    m_angleEdit->setText(QString::number(angleDeg, 'f', 2));
    m_velEdit->setText(QString::number(velocityDegPerS, 'f', 2));
    m_currentEdit->setText(QString::number(currentmA, 'f', 1));
}

void MainWindow::showError(const QString &message)
{
    statusBar()->showMessage(message, 4000);
}

void MainWindow::appendControlFrame(const QString &frame)
{
    if (!m_logView) {
        return;
    }
    m_logView->append(frame);
}

void MainWindow::buildUi()
{
    setWindowTitle(tr("关节阻抗控制面板"));

    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    auto *paramLayout = new QGridLayout();
    paramLayout->addWidget(new QLabel(tr("刚度 K")), 0, 0);
    m_kEdit = new QLineEdit(QStringLiteral("50"));
    paramLayout->addWidget(m_kEdit, 0, 1);

    paramLayout->addWidget(new QLabel(tr("阻尼 B")), 1, 0);
    m_bEdit = new QLineEdit(QStringLiteral("2.5"));
    paramLayout->addWidget(m_bEdit, 1, 1);

    paramLayout->addWidget(new QLabel(tr("重力补偿")) , 2, 0);
    m_gEdit = new QLineEdit(QStringLiteral("0"));
    paramLayout->addWidget(m_gEdit, 2, 1);

    m_confirmButton = new QPushButton(tr("确认"), this);
    paramLayout->addWidget(m_confirmButton, 3, 0, 1, 2);

    mainLayout->addLayout(paramLayout);

    auto *telemetryLayout = new QGridLayout();
    telemetryLayout->addWidget(new QLabel(tr("关节角度 (°)")), 0, 0);
    m_angleEdit = new QLineEdit(this);
    m_angleEdit->setReadOnly(true);
    telemetryLayout->addWidget(m_angleEdit, 0, 1);

    telemetryLayout->addWidget(new QLabel(tr("关节速度 (°/s)")), 1, 0);
    m_velEdit = new QLineEdit(this);
    m_velEdit->setReadOnly(true);
    telemetryLayout->addWidget(m_velEdit, 1, 1);

    telemetryLayout->addWidget(new QLabel(tr("电流 (mA)")), 2, 0);
    m_currentEdit = new QLineEdit(this);
    m_currentEdit->setReadOnly(true);
    telemetryLayout->addWidget(m_currentEdit, 2, 1);

    mainLayout->addLayout(telemetryLayout);

    m_logView = new QTextEdit(this);
    m_logView->setReadOnly(true);
    m_logView->setPlaceholderText(tr("命令和反馈日志..."));
    mainLayout->addWidget(m_logView);

    setCentralWidget(central);

    if (!statusBar()) {
        setStatusBar(new QStatusBar(this));
    }
}

void MainWindow::bindSignals()
{
    connect(m_confirmButton, &QPushButton::clicked, this, &MainWindow::handleConfirmClicked);
    connect(m_motorControl, &MotorControl::jointStateUpdated, this, &MainWindow::updateJointTelemetry);
    connect(m_motorControl, &MotorControl::errorOccurred, this, &MainWindow::showError);
    connect(m_motorControl, &MotorControl::controlFrameTransmitted, this, &MainWindow::appendControlFrame);

    connect(m_requestTimer, &QTimer::timeout, m_motorControl, &MotorControl::requestRealtimeData);
}

void MainWindow::startRealtimeRequests()
{
    m_requestTimer->setInterval(kRequestIntervalMs);
    m_requestTimer->start();
}
