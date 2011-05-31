#include <QtGui/QApplication>
#include <QSettings>
#include <QXmlStreamReader>
#include <QDir>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("BGak");
    QCoreApplication::setApplicationName("remoteRecMaster");

#ifdef Q_OS_WIN32
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QDir::homePath());
#else
    QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, QDir::homePath());
#endif

    MainWindow w;
    w.show();

    return a.exec();
}
