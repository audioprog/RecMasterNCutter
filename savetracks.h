#ifndef SAVETRACKS_H
#define SAVETRACKS_H

#include <QProcess>
#include <QList>
#include <QFile>

#include "marks.h"

class SaveTracks : public QObject
{
    Q_OBJECT
public:
    explicit SaveTracks(QObject *parent = 0);

    void SetFile(QFile *newfile) { file = newfile; }
    void SetPath(QString newPath) { path = newPath; }
    void SetSoxPath(QString newPath) { soxpath = newPath; }

signals:
    void Finished(int TrackNr);

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
