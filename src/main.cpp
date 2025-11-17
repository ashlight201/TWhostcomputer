#include "mainwindow.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用信息
    app.setApplicationName("电机力矩标定系统");
    app.setOrganizationName("MotorCalibration");
    app.setApplicationVersion("1.0");
    
    // 加载样式表
    QFile styleFile(":/qss/style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        app.setStyleSheet(style);
        styleFile.close();
    }
    
    MainWindow window;
    window.setWindowTitle("电机力矩标定系统 v1.0");
    window.resize(1400, 900);
    window.show();
    
    return app.exec();
}
