#include "audiooutput.h"

#include <QtDebug>
#include <QtEndian>

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

    if (mBEnd == mBSize) {
        memcpy(mBuffer, data, copiedBytes);
        mBEnd = copiedBytes;
        mBWrapped = false;
    } else if (mBEnd + copiedBytes <= mBSize || mBEnd == mBSize) {
        memcpy(mBuffer + mBEnd, data, copiedBytes);
        //BlockMoveData(data, mBuffer + mBEnd, copiedBytes);
        mBEnd += copiedBytes;
        if (mBEnd == mBStart)
            mBWrapped = true;
        else
            mBWrapped = false;
    } else {
        int wrappedBytes = mBSize - mBEnd;
        const qint8* dataSplit = static_cast<const qint8*>(data) + wrappedBytes;
        memcpy(mBuffer + mBEnd, data, wrappedBytes);
        //BlockMoveData(data, mBuffer + mBEnd, wrappedBytes);

        mBEnd = copiedBytes - wrappedBytes;
        memcpy(mBuffer, dataSplit, mBEnd);
        if (copiedBytes == mBSize)
            mBWrapped = true;
        else
            mBWrapped = false;
        //BlockMoveData(dataSplit, mBuffer, mBEnd);
    }

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
   mBWrapped = false;

   mutex.unlock();
   return copiedBytes;
}

WaveOutIODevice::WaveOutIODevice(QFile *file, QObject *parent)
    :   QIODevice(parent)
    ,   m_pos(0)
{
    ofile = new QFile(file->fileName());
    ring.Initialize(10 * 44100 * 2 * 2);

    m_sizebuffer = 10 * 44100 * 2 * 2;
    m_buffer.resize(m_sizebuffer);
    QObject::connect(&readTimer, SIGNAL(timeout()), this, SLOT(readFile()));
    readTimer.setInterval(50);
}

WaveOutIODevice::~WaveOutIODevice()
{
}

void WaveOutIODevice::start()
{
    ofile->open(QFile::ReadOnly);
    readFile();
    open(QIODevice::ReadOnly);
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
    qDebug() << "wionewpos" << newPos;
    m_pos = newPos;
}

void WaveOutIODevice::readFile()
{
    readTimer.stop();
    //qDebug() << "readfile" << m_pos;
    ofile->seek(m_pos);
    //qDebug() << "seek" << m_pos;
    int len = ring.GetSpaceAvailable();
    if (len > 0) {
        qint64 filebytesavail = (ofile->size() - ofile->pos()) / 3 * 2;
        if (len > filebytesavail)
            len = (int)filebytesavail;

        QByteArray bf = ofile->read(len / 2 * 3);
        //uchar *sptr = reinterpret_cast<uchar*>(bf.data());
        //uchar *dptr = reinterpret_cast<uchar*>(m_buffer.data());
        //sptr++;
        len = bf.count() / 3 * 2;
        for (int i = 0, j = 1; i < len; i+=2, j+=3) {
            //qint16 value = *sptr;
            //qToLittleEndian<qint16>(value, dptr);
            m_buffer[i] = bf.at(j);
            m_buffer[i+1] = bf.at(j+1);
            /**dptr = *sptr;
            dptr++;
            sptr++;
            *dptr = *sptr;
            dptr++;
            sptr+=2;*/
            //dptr+=2;
            //sptr+=3;
        }
        m_pos += ring.In(m_buffer, len) / 2 * 3;
    }
    readTimer.start();
}

AudioOutput::AudioOutput(QObject *parent) :
    QObject(parent)
{
    convert = false;
}

void AudioOutput::startPlaying()
{
    //delete out;
    out = 0;
    out = new WaveOutIODevice(&inputFile, this);
    out->setPos(inputFile.pos());
    out->start();

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
        audio->start(out);
        qDebug() << audio->error();
        //finishedPlaying(QAudio::ActiveState);
        qWarning()<<"raw audio format not supported by backend, cannot play audio.";
        return;
    }
    else {
        audio = new QAudioOutput(format, this);
        connect(audio,SIGNAL(stateChanged(QAudio::State)),SLOT(finishedPlaying(QAudio::State)));
        audio->start(out);
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
    qDebug() << "state" << state;
}

void AudioOutput::notify()
{
    emit PosChanged((qint64)((double)audio->processedUSecs() * ((double)audio->format().frequency() / 1000000.0)));
}
