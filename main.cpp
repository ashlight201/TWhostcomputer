#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("关节阻抗控制上位机");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("RoboticsLab");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
