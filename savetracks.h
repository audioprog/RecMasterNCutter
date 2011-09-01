#ifndef SAVETRACKS_H
#define SAVETRACKS_H

#include <QProcess>
#include <QList>
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

signals:
    void Finished(int TrackNr);
    void Debug(QString text);

public slots:
    void SaveTrack(int TrackNr);

private slots:
    void canread();
    void finished();

private:
    void Start();

    void Save(int startmark, int endmark);
    void SaveMerged(int startmark, int endmark);

    QProcess proc;

    Marks *marks;
    QFile *file;
    QString path;
    QString soxpath;
    bool isworking;

    QList<int> list;
};

#endif // SAVETRACKS_H
