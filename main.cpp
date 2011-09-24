#include <QtGui/QApplication>
#include <QSettings>
#include <QXmlStreamReader>
#include <QDir>
#include <QLocale>
#include <QTranslator>
//#include <QtDebug>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString locale = QLocale::system().name().left(2);
    //qDebug() << locale;

    QTranslator translator;
    translator.load("remoteRecMaster_" + locale, ":/translations/");
    a.installTranslator(&translator);

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
