#ifndef SAVETRACKS_H
#define SAVETRACKS_H

#include <QProcess>
#include <QList>

#include "marks.h"

class SaveTracks : public QObject
{
    Q_OBJECT
public:
    explicit SaveTracks(QObject *parent = 0);

signals:
    void Finished(int TrackNr);

public slots:
    void SaveTrack(int TrackNr);

private slots:
    void canread();
    void finished();

private:
    void Start();

    QProcess proc;

    Marks *marks;

    QList<int> list;
};

#endif // SAVETRACKS_H
