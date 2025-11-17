#include "MainWindow.h"
#include "SerialConnection.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("TWhostcomputer"));
    QApplication::setOrganizationName(QStringLiteral("TWhostcomputer"));
    QApplication::setOrganizationDomain(QStringLiteral("example.local"));

    qRegisterMetaType<SerialSettings>("SerialSettings");

    MainWindow window;
    window.show();

    return QApplication::exec();
}
