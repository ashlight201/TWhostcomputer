#include "mainwindow.h"
#include "motorcontrol.h"
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QGridLayout>
#include <QStringList>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_motorControl(nullptr)
{
    m_motorControl = new MotorControl(this);
    
    // 连接信号和槽
    connect(m_motorControl, &MotorControl::dataUpdated, this, &MainWindow::updateDisplayData);
    connect(m_motorControl, &MotorControl::errorOccurred, this, &MainWindow::handleError);
    connect(m_motorControl, &MotorControl::statusChanged, this, &MainWindow::handleStatusChange);
    
    setupUI();
    refreshSerialPorts();
    
    setWindowTitle("关节阻抗控制上位机");
    resize(800, 600);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    // 创建各个功能组
    createConnectionGroup();
    createParameterGroup();
    createDataDisplayGroup();
    createControlGroup();
    
    // 状态显示区域
    QGroupBox *statusGroup = new QGroupBox("状态信息", this);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    
    connectionStatus_label = new QLabel("未连接", this);
    connectionStatus_label->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    
    status_textEdit = new QTextEdit(this);
    status_textEdit->setReadOnly(true);
    status_textEdit->setMaximumHeight(150);
    
    statusLayout->addWidget(connectionStatus_label);
    statusLayout->addWidget(status_textEdit);
    
    // 添加所有组到主布局
    mainLayout->addWidget(statusGroup);
    
    appendStatus("程序已启动，请连接串口");
}

void MainWindow::createConnectionGroup()
{
    QGroupBox *connectionGroup = new QGroupBox("串口连接", this);
    QHBoxLayout *layout = new QHBoxLayout(connectionGroup);
    
    QLabel *portLabel = new QLabel("串口:", this);
    serialPort_comboBox = new QComboBox(this);
    serialPort_comboBox->setMinimumWidth(150);
    
    QLabel *baudLabel = new QLabel("波特率:", this);
    baudRate_comboBox = new QComboBox(this);
    baudRate_comboBox->addItems({"9600", "19200", "38400", "57600", "115200"});
    baudRate_comboBox->setCurrentText("115200");
    
    refresh_pushButton = new QPushButton("刷新", this);
    connect(refresh_pushButton, &QPushButton::clicked, this, &MainWindow::refreshSerialPorts);
    
    connect_pushButton = new QPushButton("连接", this);
    connect_pushButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }");
    connect(connect_pushButton, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
    
    disconnect_pushButton = new QPushButton("断开", this);
    disconnect_pushButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; }");
    disconnect_pushButton->setEnabled(false);
    connect(disconnect_pushButton, &QPushButton::clicked, this, &MainWindow::onDisconnectClicked);
    
    layout->addWidget(portLabel);
    layout->addWidget(serialPort_comboBox);
    layout->addWidget(baudLabel);
    layout->addWidget(baudRate_comboBox);
    layout->addWidget(refresh_pushButton);
    layout->addWidget(connect_pushButton);
    layout->addWidget(disconnect_pushButton);
    layout->addStretch();
    
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    if (mainLayout) {
        mainLayout->addWidget(connectionGroup);
    }
}

void MainWindow::createParameterGroup()
{
    QGroupBox *paramGroup = new QGroupBox("阻抗参数设置", this);
    QGridLayout *layout = new QGridLayout(paramGroup);
    
    QLabel *kLabel = new QLabel("刚度系数 (K):", this);
    KCoefficient_lineEdit = new QLineEdit(this);
    KCoefficient_lineEdit->setPlaceholderText("输入刚度系数");
    KCoefficient_lineEdit->setText("10.0");
    
    QLabel *bLabel = new QLabel("阻尼系数 (B):", this);
    BCoefficient_lineEdit = new QLineEdit(this);
    BCoefficient_lineEdit->setPlaceholderText("输入阻尼系数");
    BCoefficient_lineEdit->setText("0.5");
    
    QLabel *gLabel = new QLabel("重力补偿 (G):", this);
    Gravity_lineEdit = new QLineEdit(this);
    Gravity_lineEdit->setPlaceholderText("输入重力补偿项");
    Gravity_lineEdit->setText("0.0");
    
    confirm_pushButton = new QPushButton("确认参数", this);
    confirm_pushButton->setStyleSheet("QPushButton { background-color: #2196F3; color: white; padding: 5px; }");
    connect(confirm_pushButton, &QPushButton::clicked, this, &MainWindow::onConfirmClicked);
    
    layout->addWidget(kLabel, 0, 0);
    layout->addWidget(KCoefficient_lineEdit, 0, 1);
    layout->addWidget(bLabel, 0, 2);
    layout->addWidget(BCoefficient_lineEdit, 0, 3);
    layout->addWidget(gLabel, 1, 0);
    layout->addWidget(Gravity_lineEdit, 1, 1);
    layout->addWidget(confirm_pushButton, 1, 2, 1, 2);
    
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    if (mainLayout) {
        mainLayout->addWidget(paramGroup);
    }
}

void MainWindow::createDataDisplayGroup()
{
    QGroupBox *dataGroup = new QGroupBox("实时数据显示", this);
    QGridLayout *layout = new QGridLayout(dataGroup);
    
    QLabel *degreeLabel = new QLabel("关节角度 (度):", this);
    JointDegree_lineEdit = new QLineEdit(this);
    JointDegree_lineEdit->setReadOnly(true);
    JointDegree_lineEdit->setStyleSheet("QLineEdit { background-color: #f0f0f0; }");
    
    QLabel *velocityLabel = new QLabel("关节速度 (度/秒):", this);
    JointVelocity_lineEdit = new QLineEdit(this);
    JointVelocity_lineEdit->setReadOnly(true);
    JointVelocity_lineEdit->setStyleSheet("QLineEdit { background-color: #f0f0f0; }");
    
    QLabel *currentLabel = new QLabel("电流 (mA):", this);
    Current_lineEdit = new QLineEdit(this);
    Current_lineEdit->setReadOnly(true);
    Current_lineEdit->setStyleSheet("QLineEdit { background-color: #f0f0f0; }");
    
    layout->addWidget(degreeLabel, 0, 0);
    layout->addWidget(JointDegree_lineEdit, 0, 1);
    layout->addWidget(velocityLabel, 1, 0);
    layout->addWidget(JointVelocity_lineEdit, 1, 1);
    layout->addWidget(currentLabel, 2, 0);
    layout->addWidget(Current_lineEdit, 2, 1);
    
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    if (mainLayout) {
        mainLayout->addWidget(dataGroup);
    }
}

void MainWindow::createControlGroup()
{
    QGroupBox *controlGroup = new QGroupBox("控制操作", this);
    QHBoxLayout *layout = new QHBoxLayout(controlGroup);
    
    captureInitial_pushButton = new QPushButton("捕获初始位置", this);
    captureInitial_pushButton->setStyleSheet("QPushButton { background-color: #FF9800; color: white; padding: 10px; }");
    captureInitial_pushButton->setEnabled(false);
    connect(captureInitial_pushButton, &QPushButton::clicked, this, &MainWindow::onCaptureInitialClicked);
    
    startControl_pushButton = new QPushButton("启动阻抗控制", this);
    startControl_pushButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 10px; font-weight: bold; }");
    startControl_pushButton->setEnabled(false);
    connect(startControl_pushButton, &QPushButton::clicked, this, &MainWindow::onStartControlClicked);
    
    stopControl_pushButton = new QPushButton("停止控制", this);
    stopControl_pushButton->setStyleSheet("QPushButton { background-color: #f44336; color: white; padding: 10px; font-weight: bold; }");
    stopControl_pushButton->setEnabled(false);
    connect(stopControl_pushButton, &QPushButton::clicked, this, &MainWindow::onStopControlClicked);
    
    layout->addWidget(captureInitial_pushButton);
    layout->addWidget(startControl_pushButton);
    layout->addWidget(stopControl_pushButton);
    
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(centralWidget()->layout());
    if (mainLayout) {
        mainLayout->addWidget(controlGroup);
    }
}

void MainWindow::onConfirmClicked()
{
    bool okK, okB, okG;
    double K = KCoefficient_lineEdit->text().toDouble(&okK);
    double B = BCoefficient_lineEdit->text().toDouble(&okB);
    double G = Gravity_lineEdit->text().toDouble(&okG);
    
    if (!okK || !okB || !okG) {
        QMessageBox::warning(this, "参数错误", "请输入有效的数值参数！");
        return;
    }
    
    if (K == 0.0) {
        QMessageBox::warning(this, "参数错误", "刚度系数不能为零！");
        return;
    }
    
    m_motorControl->setImpedanceParameters(K, B, G);
    appendStatus(QString("参数已设置 - K: %1, B: %2, G: %3").arg(K).arg(B).arg(G));
}

void MainWindow::onConnectClicked()
{
    QString portName = serialPort_comboBox->currentText();
    int baudRate = baudRate_comboBox->currentText().toInt();
    
    if (portName.isEmpty()) {
        QMessageBox::warning(this, "连接错误", "请选择串口！");
        return;
    }
    
    if (m_motorControl->openSerialPort(portName, baudRate)) {
        connectionStatus_label->setText("已连接: " + portName);
        connectionStatus_label->setStyleSheet("QLabel { color: green; font-weight: bold; }");
        connect_pushButton->setEnabled(false);
        disconnect_pushButton->setEnabled(true);
        captureInitial_pushButton->setEnabled(true);
        serialPort_comboBox->setEnabled(false);
        baudRate_comboBox->setEnabled(false);
        appendStatus("串口连接成功: " + portName);
    } else {
        QMessageBox::critical(this, "连接失败", "无法打开串口！");
    }
}

void MainWindow::onDisconnectClicked()
{
    m_motorControl->closeSerialPort();
    connectionStatus_label->setText("未连接");
    connectionStatus_label->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    connect_pushButton->setEnabled(true);
    disconnect_pushButton->setEnabled(false);
    captureInitial_pushButton->setEnabled(false);
    startControl_pushButton->setEnabled(false);
    stopControl_pushButton->setEnabled(false);
    serialPort_comboBox->setEnabled(true);
    baudRate_comboBox->setEnabled(true);
    appendStatus("串口已断开");
}

void MainWindow::onCaptureInitialClicked()
{
    m_motorControl->captureInitialPosition();
    startControl_pushButton->setEnabled(true);
    appendStatus("初始位置已捕获");
}

void MainWindow::onStartControlClicked()
{
    m_motorControl->startImpedanceControl();
    startControl_pushButton->setEnabled(false);
    stopControl_pushButton->setEnabled(true);
    captureInitial_pushButton->setEnabled(false);
    appendStatus("阻抗控制已启动");
}

void MainWindow::onStopControlClicked()
{
    m_motorControl->stopImpedanceControl();
    startControl_pushButton->setEnabled(true);
    stopControl_pushButton->setEnabled(false);
    captureInitial_pushButton->setEnabled(true);
    appendStatus("阻抗控制已停止");
}

void MainWindow::updateDisplayData()
{
    // 更新关节角度显示
    QString degreeText = QString("角度1: %1°  |  角度2: %2°")
                         .arg(m_motorControl->getCurrentAngle1(), 0, 'f', 2)
                         .arg(m_motorControl->getCurrentAngle2(), 0, 'f', 2);
    JointDegree_lineEdit->setText(degreeText);
    
    // 更新速度显示
    QString velocityText = QString("速度1: %1°/s  |  速度2: %2°/s")
                           .arg(m_motorControl->getCurrentVelocity1(), 0, 'f', 2)
                           .arg(m_motorControl->getCurrentVelocity2(), 0, 'f', 2);
    JointVelocity_lineEdit->setText(velocityText);
    
    // 更新电流显示
    QString currentText = QString("电流1: %1 mA  |  电流2: %2 mA")
                          .arg(m_motorControl->getCurrent1(), 0, 'f', 2)
                          .arg(m_motorControl->getCurrent2(), 0, 'f', 2);
    Current_lineEdit->setText(currentText);
}

void MainWindow::handleError(const QString &error)
{
    appendStatus("错误: " + error);
    QMessageBox::warning(this, "错误", error);
}

void MainWindow::handleStatusChange(const QString &status)
{
    appendStatus(status);
}

void MainWindow::refreshSerialPorts()
{
    serialPort_comboBox->clear();
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    
    for (const QSerialPortInfo &info : ports) {
        serialPort_comboBox->addItem(info.portName());
    }
    
    if (serialPort_comboBox->count() == 0) {
        appendStatus("未检测到可用串口");
    } else {
        appendStatus(QString("已检测到 %1 个串口").arg(serialPort_comboBox->count()));
    }
}

void MainWindow::appendStatus(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    status_textEdit->append(QString("[%1] %2").arg(timestamp).arg(message));
}
