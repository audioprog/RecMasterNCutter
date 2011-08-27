#include "audiooutput.h"

//#include <QtDebug>
#include <QtEndian>
#include "wavfile.h"


RingBuffer::RingBuffer() :
mBuffer(NULL),
mBStart(0),
mBEnd(0),
mBSize(0),
mBWrapped(false)
{
}

RingBuffer::~RingBuffer() {
    delete[] mBuffer;
}


void RingBuffer::Initialize(int inBufferByteSize) {
    mBSize = inBufferByteSize;

    if (mBuffer)
        delete[] mBuffer;

    mBuffer = new qint8[mBSize];

    mBStart = 0;
    mBEnd = 0;
    mBWrapped = false;
}

void RingBuffer::Uninitialize() {
    mBSize = 0;

    if (mBuffer) {
        delete[] mBuffer;
        mBuffer = NULL;
    }

}

int RingBuffer::GetBufferByteSize() const {
    return mBSize;
}

int RingBuffer::GetDataAvailable() const {
    int ret = 0;

    if (mBStart < mBEnd)
        ret = mBEnd - mBStart;
    else if (mBEnd < mBStart)
        ret = mBSize + mBEnd - mBStart;
    else if (mBWrapped)
        ret = mBSize;

    return ret;
}

int RingBuffer::GetSpaceAvailable() const {
    int ret = mBSize;

    if (mBStart > mBEnd)
        ret =  mBStart - mBEnd;
    else if (mBEnd > mBStart)
        ret = mBSize - mBEnd + mBStart;
    else if (mBWrapped)
        ret = 0;

    return ret;

}


int RingBuffer::In(const void* data, int ioBytes) {
    mutex.lock();
    int copiedBytes = GetSpaceAvailable();
    if (copiedBytes > ioBytes)
        copiedBytes = ioBytes;
   // else if (copiedBytes != ioBytes)
   //     qDebug() << copiedBytes << ioBytes;

    if (mBEnd == mBSize) {
        memcpy(mBuffer, data, copiedBytes);
        mBEnd = copiedBytes;
    } else if (mBEnd + copiedBytes <= mBSize) {
        memcpy(mBuffer + mBEnd, data, copiedBytes);
        mBEnd += copiedBytes;
    } else {
        int wrappedBytes = mBSize - mBEnd;
        const qint8* dataSplit = static_cast<const qint8*>(data) + wrappedBytes;
        memcpy(mBuffer + mBEnd, data, wrappedBytes);

        mBEnd = copiedBytes - wrappedBytes;
        memcpy(mBuffer, dataSplit, mBEnd);
    }
    if (mBEnd == mBStart)
        mBWrapped = true;
    else
        mBWrapped = false;

    mutex.unlock();
    return copiedBytes;
}

int RingBuffer::Out(void *data, int ioBytes)
{
    mutex.lock();
    int copiedBytes = GetDataAvailable();
    if (copiedBytes > ioBytes)
        copiedBytes = ioBytes;

    if (mBStart + copiedBytes <= mBSize) {
        memcpy(data, mBuffer + mBStart, copiedBytes);
        mBStart += copiedBytes;
    } else {
        int wrappedBytes = mBSize - mBStart;
        qint8* dataSplit = static_cast<qint8*>(data) + wrappedBytes;
        memcpy(data, mBuffer + mBStart, wrappedBytes);
        mBStart = copiedBytes - wrappedBytes;
        memcpy(dataSplit, mBuffer, mBStart);
    }
    if (copiedBytes == mBSize)
        mBWrapped = false;

    mutex.unlock();
    return copiedBytes;
}

WaveOutIODevice::WaveOutIODevice(QFile *file, QObject *parent)
    :   QIODevice(parent)
    ,   m_pos(0)
{
    ofile = new QFile(file->fileName());

    m_headersize = file->fileName().section('.', -1, -1).toLower() == "wav" ? WavFile(file->fileName()).headerLength() : 0;

    m_sizebuffer = 10 * 44100 * 2 * 2;

    ring.Initialize(m_sizebuffer);

    m_buffer.resize(m_sizebuffer);

    QObject::connect(&readTimer, SIGNAL(timeout()), this, SLOT(readFile()));
    readTimer.setInterval(100);
}

WaveOutIODevice::~WaveOutIODevice()
{
}

void WaveOutIODevice::start()
{
    ofile->open(QFile::ReadOnly);
    open(QIODevice::ReadOnly);
    ofile->seek(m_headersize + m_pos);
    readFile();
}

void WaveOutIODevice::stop()
{
    m_pos = 0;
    close();
}

qint64 WaveOutIODevice::readData(char *data, qint64 len)
{
    if (ring.GetDataAvailable() > 0) {
        int chunk = (int)qMin(len, (qint64)ring.GetDataAvailable());
        return ring.Out(data, chunk);
    }
    return 0;
}

qint64 WaveOutIODevice::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

qint64 WaveOutIODevice::bytesAvailable() const
{
    return ring.GetDataAvailable();
}

void WaveOutIODevice::setPos(const qint64 newPos)
{
    //qDebug() << "wionewpos" << newPos;
    //ofile->seek(newPos);
    if (newPos > -1)
        m_pos = newPos;
    else
        m_pos = m_pos;
}

void WaveOutIODevice::readFile()
{
    readTimer.stop();
    //qDebug() << "readfile" << m_pos;
    //ofile->seek(m_pos);
    //qDebug() << "seek" << m_pos;
    int len = ring.GetSpaceAvailable();
    if (len > 0) {
        //qint64 anfang = m_pos;
        qint64 filebytesavail = (ofile->size() - ofile->pos()) / 3 * 2;
        if (len > filebytesavail)
            len = (int)filebytesavail;

        QByteArray bf = ofile->read(len / 2 * 3);
        len = bf.count() / 3 * 2;
        for (int i = 0, j = 1; j < bf.count(); i+=2, j+=3) {
            m_buffer[i] = bf.at(j);
            m_buffer[i+1] = bf.at(j+1);
        }
        m_pos += ring.In(m_buffer, len) / 2 * 3;
        //qDebug() << "readFile" << m_pos << ofile->pos() << m_pos - anfang - bf.count();
    }
    readTimer.start();
}

AudioOutput::AudioOutput(QObject *parent) :
    QObject(parent)
{
    convert = false;
    firstrun = true;
}

void AudioOutput::startPlaying(qint64 newpos)
{
    if (!firstrun) {
        if (audio->state() != QAudio::StoppedState)
            stop();
    }
    else {
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
        }
        else {
            audio = new QAudioOutput(format, this);
        }
        connect(audio,SIGNAL(stateChanged(QAudio::State)),SLOT(finishedPlaying(QAudio::State)));
        connect(audio, SIGNAL(notify()), this, SLOT(notify()));
        firstrun = false;
    }
    //delete out;
    out = 0;
    out = new WaveOutIODevice(&inputFile, this);
    startpos = newpos;
    out->setPos(startpos);
    out->start();

    audio->setNotifyInterval(100);
    audio->start(out);
}

void AudioOutput::setFilePos(qint64 newpos)
{
    //qDebug() << newpos;
    if (newpos > -1)
        startpos = newpos;
    else
        startpos = startpos;
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
    //qDebug() << "state" << state;
    if (state == QAudio::StoppedState) {
        emit PosChanged(-1);
        startpos = -1;
    }
}

void AudioOutput::notify()
{
    emit PosChanged(Pos());
}
