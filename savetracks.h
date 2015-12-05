#ifndef SAVETRACKS_H
#define SAVETRACKS_H

#include <QProcess>
#include <QList>
#include <QHash>
#include <QFile>
#include <QDir>

#include "marks.h"

class SaveTracks : public QObject
{
    Q_OBJECT
public:
    explicit SaveTracks(QObject *parent = 0);

    void SetFile(QFile *newfile) { file = newfile; }
    void SetPath(QString newPath) { path = newPath; QDir(path).mkpath(path); }
    void SetSoxPath(QString newPath) { soxpath = newPath; }
    void SetMarks(Marks *newMarks) { marks = newMarks; }
    QList<int> allMarks() { return proclist.keys(); }

signals:
    void Finished(int TrackNr);
    void Debug(QString text);

public slots:
    void SaveTrack(int TrackNr, const QString &fileTitle);
    void ReadSettings();

private slots:
    void canread();
    void finished();

private:
    void Start();

    void Save(int startmark, int fadein, int fadeout, int endmark, const QString &fileTitle);
    void SaveMerged(int startmark, int fadein, int fadeout, int endmark, const QString &fileTitle);

    QProcess proc;

    Marks *marks;
    QFile *file;
    QString path;
    QString soxpath;
    bool isworking;

    //QList<int> list;

    QHash<int, QStringList> proclist;
    QHash<int, QString> fileTitles;
};

#endif // SAVETRACKS_H
