#include "audiooutput.h"

#include <QtDebug>

AudioOutput::AudioOutput(QObject *parent) :
    QObject(parent)
{
    convert = false;
}

void AudioOutput::startPlaying()
{
    QAudioFormat format;
    // Set up the format, eg.
    format.setFrequency(44100);
    format.setChannels(2);
    format.setSampleSize(24);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(format)) {
        convert = true;
        format.setSampleSize(16);
        audio = new QAudioOutput(format, this);
        connect(audio,SIGNAL(stateChanged(QAudio::State)),SLOT(finishedPlaying(QAudio::State)));
        out = audio->start();
        finishedPlaying(QAudio::ActiveState);
        qWarning()<<"raw audio format not supported by backend, cannot play audio.";
        return;
    }
    else {
        audio = new QAudioOutput(format, this);
        connect(audio,SIGNAL(stateChanged(QAudio::State)),SLOT(finishedPlaying(QAudio::State)));
        audio->start(&inputFile);
    }
}

/*void AudioOutput::setFile(QString Filename)
{
    inputFile.setFileName(Filename);
    inputFile.open(QIODevice::ReadOnly);
}

void AudioOutput::setFilePos(qint64 pos)
{
    inputFile.seek(pos);
}*/

void AudioOutput::finishedPlaying(QAudio::State state)
{
    if (state == QAudio::ActiveState) {
        QByteArray ba = inputFile.read(1 * 44100 * 2 * 3);
        QByteArray ba2(1 * 44100 * 2 * 2, 0);
        for (int i = 0; i < 1 * 44100 * 2; i++) {
            ba2[i * 2] = ba.at(i * 3 + 1);
            ba2[i * 2 + 1] = ba.at(i * 3 + 2);
        }
        out->write(ba);
    }
    if(state == QAudio::IdleState) {
        if (convert) {
            QByteArray ba = inputFile.read(1 * 44100 * 2 * 3);
            QByteArray ba2(1 * 44100 * 2 * 2, 0);
            for (int i = 0; i < 1 * 44100 * 2; i++) {
                ba2[i * 2] = ba.at(i * 3);
                ba2[i * 2 + 1] = ba.at(i * 3 + 1);
            }
            out->write(ba2);
        }
        else {
            audio->stop();
            //inputFile.close();
            delete audio;
        }
    }
}

void AudioOutput::notify()
{
    emit PosChanged((qint64)((double)audio->processedUSecs() * ((double)audio->format().frequency() / 1000000.0)));
}
