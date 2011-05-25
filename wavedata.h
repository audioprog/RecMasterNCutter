#ifndef WAVEDATA_H
#define WAVEDATA_H

#include <QThread>
#include <QFile>
#include <QVector>
#include <QReadWriteLock>

class WaveData : public QThread
{
    Q_OBJECT

public:
    WaveData();
    ~WaveData();

    void FileOpen(QFile *newFile);
    QString FileName() { return fileloaded ? file->fileName() : ""; }
    qint64 Length();
    int Count();
    int Channel() { return channel; }
    QVector<int> Data(uint count, qint64 newpos = -1);

    qint64 Pos() { return _pos; }
    void SetPos( qint64 newpos ) { _pos = newpos; }
    uint DotWidth () { return dotwidth; }
    void SetDotWidth ( uint newwidth ) { dotwidth = newwidth; }
    int MaxClip() { return maxclip; }
    void setDebugNr(int nr) { debugnr = nr; }

    void run();

signals:
    void CanReadNow();

private:
    void read(qlonglong pos, int count);

    QReadWriteLock lock;
    bool isworking;
    bool fileloaded;
    QFile *file;
    qint64 _pos;
    QVector<int> lastdata;
    qint64 lastpos;
    uint lastcount;
    uint lastwidth;
    uint channel, samplesize, dotwidth;
    int maxclip;
    qlonglong readpos;
    int readcount;
    int debugnr;
};

#endif // WAVEDATA_H
