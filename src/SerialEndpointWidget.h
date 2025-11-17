#pragma once

#include "SerialConnection.h"

#include <QGroupBox>

class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

class SerialEndpointWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit SerialEndpointWidget(const QString &title, QWidget *parent = nullptr);

    void setConnected(bool connected);
    void showStatusMessage(const QString &message);
    void appendLogLine(const QString &message);
    SerialSettings currentSettings() const;

signals:
    void connectRequested(const SerialSettings &settings);
    void disconnectRequested();
    void sendRequested(const QByteArray &payload);

private slots:
    void handleConnectButton();
    void handleSendButton();
    void refreshPorts();

private:
    QByteArray buildPayload(const QString &text) const;
    void setUiEnabled(bool enabled);
    void setupUi();

    QComboBox *m_portCombo = nullptr;
    QComboBox *m_baudCombo = nullptr;
    QPushButton *m_connectButton = nullptr;
    QPushButton *m_refreshButton = nullptr;
    QPushButton *m_sendButton = nullptr;
    QLineEdit *m_sendLine = nullptr;
    QPlainTextEdit *m_logViewer = nullptr;
    QLabel *m_statusLabel = nullptr;
    bool m_connected = false;
};
