#include "mainwindow.h"

#include "motorcontrol.h"

#include <QDateTime>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
constexpr double kDefaultStiffness = 50.0;
constexpr double kDefaultDamping = 5.0;
constexpr double kDefaultGravity = 0.0;
}  // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      motorControl_(new MotorControl(this)) {
    setupUi();
    connectSignals();
    autoConnectSerial();
}

MainWindow::~MainWindow() = default;

void MainWindow::handleConfirmClicked() {
    bool okK = false;
    bool okB = false;
    bool okG = false;

    const double stiffness = kCoefficientLineEdit_->text().toDouble(&okK);
    const double damping = bCoefficientLineEdit_->text().toDouble(&okB);
    const double gravity = gravityLineEdit_->text().toDouble(&okG);

    if (!okK || !okB || !okG) {
        QMessageBox::warning(this, tr("输入错误"), tr("请填写合法的数字参数。"));
        return;
    }

    if (stiffness <= 0.0) {
        QMessageBox::warning(this, tr("参数错误"), tr("刚度K必须为正。"));
        return;
    }

    motorControl_->setCoefficients(stiffness, damping, gravity);
    motorControl_->recalibrateInitialPosition();
}

void MainWindow::handleJointObservation(double angle, double velocity, double current) {
    jointDegreeLineEdit_->setText(QString::number(angle, 'f', 2));
    jointVelocityLineEdit_->setText(QString::number(velocity, 'f', 2));
    jointCurrentLineEdit_->setText(QString::number(current, 'f', 1));
}

void MainWindow::handleLogText(const QString &text) {
    const QString timeStamp = QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
    logViewer_->appendPlainText(QStringLiteral("[%1] %2").arg(timeStamp, text));
}

void MainWindow::setupUi() {
    setWindowTitle(tr("关节阻抗控制上位机"));

    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    auto *coeffGroup = new QGroupBox(tr("阻抗参数"), central);
    auto *coeffLayout = new QFormLayout(coeffGroup);

    auto *doubleValidator = new QDoubleValidator(coeffGroup);
    doubleValidator->setNotation(QDoubleValidator::StandardNotation);

    kCoefficientLineEdit_ = new QLineEdit(coeffGroup);
    kCoefficientLineEdit_->setObjectName(QStringLiteral("KCoefficient_lineEdit"));
    kCoefficientLineEdit_->setValidator(doubleValidator);
    kCoefficientLineEdit_->setText(QString::number(kDefaultStiffness));

    bCoefficientLineEdit_ = new QLineEdit(coeffGroup);
    bCoefficientLineEdit_->setObjectName(QStringLiteral("BCoefficient_lineEdit"));
    bCoefficientLineEdit_->setValidator(doubleValidator);
    bCoefficientLineEdit_->setText(QString::number(kDefaultDamping));

    gravityLineEdit_ = new QLineEdit(coeffGroup);
    gravityLineEdit_->setObjectName(QStringLiteral("Gravity_lineEdit"));
    gravityLineEdit_->setValidator(doubleValidator);
    gravityLineEdit_->setText(QString::number(kDefaultGravity));

    confirmButton_ = new QPushButton(tr("确认"), coeffGroup);
    confirmButton_->setObjectName(QStringLiteral("confirm_pushButton"));

    coeffLayout->addRow(tr("刚度K:"), kCoefficientLineEdit_);
    coeffLayout->addRow(tr("阻尼B:"), bCoefficientLineEdit_);
    coeffLayout->addRow(tr("重力补偿:"), gravityLineEdit_);
    coeffLayout->addRow(confirmButton_);

    auto *feedbackGroup = new QGroupBox(tr("实时反馈"), central);
    auto *feedbackLayout = new QFormLayout(feedbackGroup);

    jointDegreeLineEdit_ = new QLineEdit(feedbackGroup);
    jointDegreeLineEdit_->setObjectName(QStringLiteral("JointDegree_lineEdit"));
    jointDegreeLineEdit_->setReadOnly(true);

    jointVelocityLineEdit_ = new QLineEdit(feedbackGroup);
    jointVelocityLineEdit_->setObjectName(QStringLiteral("JointVelocity_lineEdit"));
    jointVelocityLineEdit_->setReadOnly(true);

    jointCurrentLineEdit_ = new QLineEdit(feedbackGroup);
    jointCurrentLineEdit_->setObjectName(QStringLiteral("Current_lineEdit"));
    jointCurrentLineEdit_->setReadOnly(true);

    feedbackLayout->addRow(tr("角度(°):"), jointDegreeLineEdit_);
    feedbackLayout->addRow(tr("速度(°/s):"), jointVelocityLineEdit_);
    feedbackLayout->addRow(tr("电流(mA):"), jointCurrentLineEdit_);

    logViewer_ = new QPlainTextEdit(central);
    logViewer_->setReadOnly(true);
    logViewer_->setPlaceholderText(tr("通信日志..."));

    mainLayout->addWidget(coeffGroup);
    mainLayout->addWidget(feedbackGroup);
    mainLayout->addWidget(logViewer_);

    setCentralWidget(central);
}

void MainWindow::connectSignals() {
    connect(confirmButton_, &QPushButton::clicked, this, &MainWindow::handleConfirmClicked);
    connect(motorControl_, &MotorControl::jointObservation, this, &MainWindow::handleJointObservation);
    connect(motorControl_, &MotorControl::logText, this, &MainWindow::handleLogText);
}

void MainWindow::autoConnectSerial() {
    if (!motorControl_->openFirstAvailable()) {
        handleLogText(tr("未能自动连接串口, 请检查设备。"));
    }
}
