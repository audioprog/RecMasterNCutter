#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QFile>
#include <QAudioOutput>
#include <QTimer>
#include <QMutex>

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

private slots:
    void readFile();

private:
    QFile *ofile;
    qint64 m_headersize;
    qint64 m_pos;
    qint64 c_pos;
    QByteArray m_buffer;
    int m_sizebuffer;

    RingBuffer ring;

    QTimer readTimer;
};

class AudioOutput : public QObject
{
    Q_OBJECT
public:
    explicit AudioOutput(QObject *parent = 0);
    qint64 Pos() const { qint64 bytesInBuffer = audio->bufferSize() - audio->bytesFree(); qint64 byProcc = audio->processedUSecs() * 6 * 44100 / (qint64)(1000000);
                   qint64 byPlayed = startpos + byProcc - (bytesInBuffer / 2 * 3); return byPlayed; }

signals:
    void PosChanged(qint64 pos);

public slots:
    void startPlaying(qint64 newpos);
    void finishedPlaying(QAudio::State state);
    //void setFile(QString Filename) { inputFile.setFileName(Filename); inputFile.open(QIODevice::ReadOnly); }
    //void setFilePos(qint64 pos) { if (startpos == -1) { inputFile.seek(pos); startpos = pos; } }

    void setFile(QString Filename) { inputFile.setFileName(Filename); }
    void setFilePos(qint64 newpos);
    void stop() { audio->stop(); }

private slots:
    void notify();

private:
    qint64 startpos;
    QFile inputFile;
    QAudioOutput* audio;
    bool convert;
    WaveOutIODevice *out;
    bool firstrun;
};

#endif // AUDIOOUTPUT_H
