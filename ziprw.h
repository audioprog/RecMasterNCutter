#ifndef ZIPRW_H
#define ZIPRW_H

#include <QObject>

class ZipRW : public QObject
{
    Q_OBJECT
public:
    explicit ZipRW(QObject *parent = 0);
    static QString FileText(QString ZipFile, QString File);
    static void SetFileText(QString ZipFile, QString File, QString Text);
    static void CpFileText(QString source, QString dest, QString File, QString Text);
};

#endif // ZIPRW_H
