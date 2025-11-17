#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    // 添加调试输出
    qDebug() << "===========================================";
    qDebug() << "电机力矩标定系统启动...";
    qDebug() << "Qt版本:" << qVersion();
    qDebug() << "===========================================";
    
    QApplication app(argc, argv);
    qDebug() << "✓ QApplication 创建成功";
    
    // 设置应用信息
    app.setApplicationName("电机力矩标定系统");
    app.setOrganizationName("MotorCalibration");
    app.setApplicationVersion("1.0");
    qDebug() << "✓ 应用信息设置完成";
    
    // 加载样式表（可选，如果文件不存在不影响运行）
    QFile styleFile(":/qss/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
        styleFile.close();
        qDebug() << "✓ 样式表加载成功";
    } else {
        qDebug() << "⚠ 样式表未找到（不影响运行）";
    }
    
    // 使用try-catch捕获异常
    try {
        qDebug() << "开始创建主窗口...";
        MainWindow window;
        qDebug() << "✓ 主窗口创建成功";
        
        window.setWindowTitle("电机力矩标定系统 v1.0");
        window.resize(1400, 900);
        qDebug() << "✓ 窗口属性设置完成";
        
        window.show();
        qDebug() << "✓ 窗口显示成功";
        qDebug() << "===========================================";
        qDebug() << "程序启动完成，进入事件循环";
        qDebug() << "===========================================";
        
        return app.exec();
        
    } catch (const std::exception &e) {
        QString errorMsg = QString("程序异常: %1").arg(e.what());
        qCritical() << errorMsg;
        QMessageBox::critical(nullptr, "严重错误", errorMsg);
        return -1;
    } catch (...) {
        QString errorMsg = "程序发生未知异常";
        qCritical() << errorMsg;
        QMessageBox::critical(nullptr, "严重错误", errorMsg);
        return -1;
    }
}
