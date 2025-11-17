#include "serialporthandler.h"
#include <QDebug>

SerialPortHandler::SerialPortHandler(QObject *parent)
    : QObject(parent)
    , m_serialPort(new QSerialPort(this))
{
    // 连接信号槽
    connect(m_serialPort, &QSerialPort::readyRead, 
            this, &SerialPortHandler::onReadyRead);
    connect(m_serialPort, &QSerialPort::errorOccurred, 
            this, &SerialPortHandler::onErrorOccurred);
}

SerialPortHandler::~SerialPortHandler()
{
    closePort();
}

bool SerialPortHandler::openPort(const QString &portName, 
                                  qint32 baudRate,
                                  QSerialPort::DataBits dataBits,
                                  QSerialPort::Parity parity,
                                  QSerialPort::StopBits stopBits)
{
    // 如果已经打开，先关闭
    if (m_serialPort->isOpen()) {
        closePort();
    }
    
    // 设置串口参数
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(dataBits);
    m_serialPort->setParity(parity);
    m_serialPort->setStopBits(stopBits);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    
    // 尝试打开串口
    if (m_serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "串口打开成功:" << portName;
        emit connectionStatusChanged(true);
        return true;
    } else {
        QString errorMsg = QString("串口打开失败: %1 - %2")
                          .arg(portName)
                          .arg(m_serialPort->errorString());
        qDebug() << errorMsg;
        emit errorOccurred(errorMsg);
        emit connectionStatusChanged(false);
        return false;
    }
}

void SerialPortHandler::closePort()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        qDebug() << "串口已关闭";
        emit connectionStatusChanged(false);
    }
}

bool SerialPortHandler::isOpen() const
{
    return m_serialPort->isOpen();
}

qint64 SerialPortHandler::sendData(const QByteArray &data)
{
    if (!m_serialPort->isOpen()) {
        qDebug() << "串口未打开，无法发送数据";
        return -1;
    }
    
    qint64 bytesWritten = m_serialPort->write(data);
    m_serialPort->flush();
    
    qDebug() << "发送数据:" << data.toHex(' ') << "字节数:" << bytesWritten;
    return bytesWritten;
}

QStringList SerialPortHandler::getAvailablePorts()
{
    QStringList portList;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        portList.append(info.portName());
    }
    return portList;
}

void SerialPortHandler::clearBuffer()
{
    m_receiveBuffer.clear();
    if (m_serialPort->isOpen()) {
        m_serialPort->clear();
    }
}

void SerialPortHandler::onReadyRead()
{
    // 读取所有可用数据
    QByteArray data = m_serialPort->readAll();
    
    if (!data.isEmpty()) {
        m_receiveBuffer.append(data);
        
        // 发出接收到数据的信号
        emit dataReceived(data);
        
        qDebug() << "接收数据:" << data.toHex(' ') << "长度:" << data.size();
    }
}

void SerialPortHandler::onErrorOccurred(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError) {
        return;
    }
    
    if (error == QSerialPort::ResourceError) {
        // 串口被移除或断开
        closePort();
    }
    
    QString errorMsg = QString("串口错误: %1").arg(m_serialPort->errorString());
    qDebug() << errorMsg;
    emit errorOccurred(errorMsg);
}
