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

private slots:
    void readFile();

private:
    QFile *ofile;
    qint64 m_pos;
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

signals:
    void PosChanged(qint64 pos);

public slots:
    void startPlaying();
    void finishedPlaying(QAudio::State state);
    void setFile(QString Filename) { inputFile.setFileName(Filename); inputFile.open(QIODevice::ReadOnly); }
    void setFilePos(qint64 pos) { inputFile.seek(pos); }
    void stop() { audio->stop(); }

private slots:
    void notify();

private:
    QFile inputFile;
    QAudioOutput* audio;
    bool convert;
    WaveOutIODevice *out;
};

#endif // AUDIOOUTPUT_H
