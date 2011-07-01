#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include <QObject>
#include <QFile>
#include <QAudioOutput>

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
    QIODevice *out;
};

#endif // AUDIOOUTPUT_H
