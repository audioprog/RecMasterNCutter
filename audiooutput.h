#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QFile>
#include <QAudioOutput>
#include <QTimer>
#include <QMutex>
#include "marks.h"

class RingBuffer
{
 public:
    RingBuffer();
    virtual ~RingBuffer();

 public:
    virtual void   Initialize(int inBufferByteSize);
    virtual void   Uninitialize();

    virtual int GetBufferByteSize() const;
    virtual int GetDataAvailable() const;
    virtual int GetSpaceAvailable() const;

    int   In(const void* data, int ioBytes);
    int   Out(void* data, int ioBytes);

 protected:
    qint8 *  mBuffer;
    QMutex mutex;

    int  mBStart;
    int  mBEnd;

    int  mBSize;
    bool mBWrapped;
};

class WaveOutIODevice : public QIODevice
{
    Q_OBJECT
public:
    WaveOutIODevice(QFile *file, QObject *parent);
    ~WaveOutIODevice();

    void start(const qint64 newPos) { setPos(newPos); start(); }
    void start();
    void stop();

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);
    qint64 bytesAvailable() const;
    void setPos(const qint64 newPos);
    qint64 len() { return ofile->size() - m_headersize; }

public slots:
    void VolChange(int newVol) { vol = newVol; }

private slots:
    void readFile();

private:
    QFile *ofile;
    qint64 m_headersize;
    qint64 m_pos;
    qint64 c_pos;
    QByteArray m_buffer;
    int m_sizebuffer;
    int vol;

    RingBuffer ring;

    QTimer readTimer;
};

class AudioOutput : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutput(QObject *parent = 0);
    qint64 Pos() const;
    bool isPlaying() { return firstrun ? false : audio->state() == QAudio::ActiveState; }
    void setMarks(Marks* newMarks) { marks = newMarks; }
    QStringList DeviceList();

signals:
    void PosChanged(qint64 pos);
    void VolChanged(int newVol);
    void Debug( QString text );

public slots:
    void startPlaying(qint64 newpos);
    void finishedPlaying(QAudio::State state);
    //void setFile(QString Filename) { inputFile.setFileName(Filename); inputFile.open(QIODevice::ReadOnly); }
    //void setFilePos(qint64 pos) { if (startpos == -1) { inputFile.seek(pos); startpos = pos; } }

    void setFile(QString Filename) { inputFile.setFileName(Filename); }
    void setFilePos(qint64 newpos);
    void stop() { audio->stop(); }
    void VolChange(int newVol) { emit VolChanged(newVol); }
    void setHardware( int newHardware ) { oldhardware = hardware; hardware = newHardware; }

private slots:
    void notify();

private:
    Marks *marks;
    qint64 startpos;
    QFile inputFile;
    QAudioOutput* audio;
    bool convert;
    WaveOutIODevice *out;
    QFile *outfile;
    bool firstrun;
    int hardware, oldhardware;
};

#endif // AUDIOOUTPUT_H
