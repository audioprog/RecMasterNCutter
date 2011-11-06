#include <QFile>
#include <QDir>
#include "ziprw.h"
#include "quazip.h"
#include "quazipfile.h"

ZipRW::ZipRW(QObject *parent) :
    QObject(parent)
{
}

QString ZipRW::FileText(QString ZipFile, QString File)
{
    QuaZip qz(ZipFile);
    qz.open(QuaZip::mdUnzip);
    qz.setCurrentFile(File, QuaZip::csInsensitive);
    QuaZipFile qf(&qz);
    qf.open(QFile::ReadOnly);
    QString txt = qf.readAll();
    qf.close();
    qz.close();
    return txt;
}

void ZipRW::SetFileText(QString ZipFile, QString File, QString Text)
{
    QFile(ZipFile).rename(ZipFile + ".old");

    QuaZip qz2(ZipFile);
    qz2.open(QuaZip::mdCreate);

    QuaZipFile qzf(&qz2);
    qzf.open(QFile::WriteOnly, QuaZipNewInfo(File));
    qzf.write(Text.toLatin1());
    qzf.close();

    QuaZip qz(ZipFile + ".old");
    qz.open(QuaZip::mdUnzip);
    QList<QuaZipFileInfo> lst = qz.getFileInfoList();
    foreach (QuaZipFileInfo itm, lst) {
        if (itm.name != File) {
            qz.setCurrentFile(itm.name, QuaZip::csInsensitive);
            QuaZipFile qf(&qz);
            qf.open(QFile::ReadOnly);
            QByteArray ba = qf.readAll();
            qf.close();

            QuaZipFile qzf2(&qz2);
            qzf2.open(QFile::WriteOnly, QuaZipNewInfo(itm.name));
            qzf2.write(ba);
            qzf2.close();
        }
    }

    qz.close();
    qz2.close();
    QFile(ZipFile + ".old").remove();
}

void ZipRW::CpFileText(QString source, QString dest, QString File, QString Text)
{
    QuaZip qz2(dest);
    qz2.open(QuaZip::mdCreate);

    QuaZipFile qzf(&qz2);
    qzf.open(QFile::WriteOnly, QuaZipNewInfo(File));
    qzf.write(Text.toLatin1());
    qzf.close();

    QuaZip qz(source);
    qz.open(QuaZip::mdUnzip);
    QList<QuaZipFileInfo> lst = qz.getFileInfoList();
    foreach (QuaZipFileInfo itm, lst) {
        if (itm.name != File) {
            qz.setCurrentFile(itm.name, QuaZip::csInsensitive);
            QuaZipFile qf(&qz);
            qf.open(QFile::ReadOnly);
            QByteArray ba = qf.readAll();
            qf.close();

            QuaZipFile qzf2(&qz2);
            qzf2.open(QFile::WriteOnly, QuaZipNewInfo(itm.name));
            qzf2.write(ba);
            qzf2.close();
        }
    }

    qz.close();
    qz2.close();
}
