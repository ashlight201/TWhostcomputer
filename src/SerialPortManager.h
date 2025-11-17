#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();
    
    bool openPort(const QString &portName, qint32 baudRate);
    void closePort();
    bool isOpen() const;
    QString portName() const;
    qint32 baudRate() const;
    
    bool sendData(const QByteArray &data);
    QList<QSerialPortInfo> availablePorts() const;
    
signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);
    void portOpened();
    void portClosed();

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    QSerialPort *serialPort;
    QString currentPortName;
    qint32 currentBaudRate;
};

#endif // SERIALPORTMANAGER_H
