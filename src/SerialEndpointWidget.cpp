#include "SerialEndpointWidget.h"

#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSerialPortInfo>
#include <QTextCursor>
#include <QTime>

namespace
{
const int kMaxLogLines = 200;
}

SerialEndpointWidget::SerialEndpointWidget(const QString &title, QWidget *parent)
    : QGroupBox(title, parent)
{
    setupUi();
    refreshPorts();
}

void SerialEndpointWidget::setConnected(bool connected)
{
    if (m_connected == connected) {
        return;
    }
    m_connected = connected;

    m_connectButton->setText(connected ? tr("断开") : tr("连接"));
    setUiEnabled(!connected);
    m_sendButton->setEnabled(connected);
}

void SerialEndpointWidget::showStatusMessage(const QString &message)
{
    if (m_statusLabel) {
        m_statusLabel->setText(message);
    }
    appendLogLine(message);
}

void SerialEndpointWidget::appendLogLine(const QString &message)
{
    if (!m_logViewer) {
        return;
    }

    const QString line = QStringLiteral("[%1] %2")
                             .arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss.zzz")), message);
    m_logViewer->appendPlainText(line);

    while (m_logViewer->blockCount() > kMaxLogLines) {
        QTextCursor cursor(m_logViewer->document());
        cursor.movePosition(QTextCursor::Start);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.removeSelectedText();
        cursor.deleteChar();
    }
}

SerialSettings SerialEndpointWidget::currentSettings() const
{
    SerialSettings settings;
    settings.portName = m_portCombo->currentText();
    settings.baudRate = m_baudCombo->currentText().toInt();
    return settings;
}

void SerialEndpointWidget::handleConnectButton()
{
    if (m_connected) {
        emit disconnectRequested();
    } else {
        emit connectRequested(currentSettings());
    }
}

void SerialEndpointWidget::handleSendButton()
{
    const QString text = m_sendLine->text();
    if (text.isEmpty()) {
        return;
    }

    const QByteArray payload = buildPayload(text);
    if (payload.isEmpty()) {
        appendLogLine(tr("发送内容为空或格式错误"));
        return;
    }

    emit sendRequested(payload);
    appendLogLine(tr("TX %1").arg(QString::fromLatin1(payload.toHex(' ').toUpper())));
}

void SerialEndpointWidget::refreshPorts()
{
    const QString currentSelection = m_portCombo->currentText();

    m_portCombo->blockSignals(true);
    m_portCombo->clear();
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : ports) {
        m_portCombo->addItem(info.portName());
    }
    m_portCombo->blockSignals(false);

    const int index = m_portCombo->findText(currentSelection);
    if (index >= 0) {
        m_portCombo->setCurrentIndex(index);
    }
}

QByteArray SerialEndpointWidget::buildPayload(const QString &text) const
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    const bool looksLikeHex = trimmed.contains(QRegularExpression(QStringLiteral("[,\\s-]"))) ||
                              trimmed.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive);
    if (looksLikeHex) {
        QByteArray payload;
        const auto tokens = trimmed.split(QRegularExpression(QStringLiteral("[,\\s-]+")),
                                          Qt::SkipEmptyParts);
        for (const QString &token : tokens) {
            bool ok = false;
            const int value = token.toInt(&ok, 16);
            if (!ok) {
                continue;
            }
            payload.append(static_cast<char>(value & 0xFF));
        }
        if (!payload.isEmpty()) {
            return payload;
        }
    }

    QByteArray payload = trimmed.toLatin1();
    if (!payload.endsWith('\n')) {
        payload.append('\n');
    }
    return payload;
}

void SerialEndpointWidget::setUiEnabled(bool enabled)
{
    m_portCombo->setEnabled(enabled);
    m_baudCombo->setEnabled(enabled);
    m_refreshButton->setEnabled(enabled);
}

void SerialEndpointWidget::setupUi()
{
    auto *layout = new QGridLayout(this);

    m_portCombo = new QComboBox(this);
    m_baudCombo = new QComboBox(this);
    m_connectButton = new QPushButton(tr("连接"), this);
    m_refreshButton = new QPushButton(tr("刷新"), this);
    m_sendButton = new QPushButton(tr("发送"), this);
    m_sendLine = new QLineEdit(this);
    m_logViewer = new QPlainTextEdit(this);
    m_statusLabel = new QLabel(tr("未连接"), this);

    m_logViewer->setReadOnly(true);
    m_sendLine->setPlaceholderText(tr("输入ASCII或空格分隔的16进制(如: 01 03 FF)"));

    const QList<int> baudRates = {115200, 921600, 460800, 256000, 128000,
                                  57600, 38400, 19200, 9600};
    for (int baud : baudRates) {
        m_baudCombo->addItem(QString::number(baud));
    }
    m_baudCombo->setCurrentText(QString::number(115200));

    int row = 0;
    layout->addWidget(new QLabel(tr("端口")), row, 0);
    layout->addWidget(m_portCombo, row, 1);
    layout->addWidget(m_refreshButton, row, 2);

    ++row;
    layout->addWidget(new QLabel(tr("波特率")), row, 0);
    layout->addWidget(m_baudCombo, row, 1);
    layout->addWidget(m_connectButton, row, 2);

    ++row;
    layout->addWidget(new QLabel(tr("发送")), row, 0);
    layout->addWidget(m_sendLine, row, 1);
    layout->addWidget(m_sendButton, row, 2);

    ++row;
    layout->addWidget(new QLabel(tr("状态")), row, 0);
    layout->addWidget(m_statusLabel, row, 1, 1, 2);

    ++row;
    layout->addWidget(m_logViewer, row, 0, 1, 3);

    connect(m_connectButton, &QPushButton::clicked, this, &SerialEndpointWidget::handleConnectButton);
    connect(m_sendButton, &QPushButton::clicked, this, &SerialEndpointWidget::handleSendButton);
    connect(m_refreshButton, &QPushButton::clicked, this, &SerialEndpointWidget::refreshPorts);
    m_sendButton->setEnabled(false);
}
