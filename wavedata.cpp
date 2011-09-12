#include "wavedata.h"
#include <QFileInfo>
#include "wavfile.h"

//#include <QtDebug>

WaveData::WaveData()
{
    channel = 2;
    samplesize = 3;
    dotwidth = 20;
    lastpos = -1;
    maxclip = 0x7fffff;
    fileloaded = false;
    _pos = 0;
    isworking = false;
    debugnr = -1;
    _headersize = 0;
}

WaveData::~WaveData()
{
    if (fileloaded)
        if (file->isOpen()) {
            file->close();
        }
}

void WaveData::FileOpen(QFile *newFile)
{
    if (fileloaded) {
        file->close();
        lastpos = -1;
    }
    file = new QFile(newFile->fileName());
    _headersize = newFile->fileName().section('.', -1, -1).toLower() == "wav" ? WavFile(newFile->fileName()).headerLength() : 0;
    if (!file->isOpen())
        file->open(QFile::ReadOnly);
    fileloaded = true;
}

qlonglong WaveData::Length()
{
    if (fileloaded)
    {
        return (file->size() - _headersize) / channel / samplesize;
    }
    return 0;
}

int WaveData::Count()
{
    if (fileloaded)
    {
        //qDebug() << "Count" << file->size() << DotWidth() << channel << samplesize;
        int len = (int)((file->size() - _headersize) / dotwidth / channel / samplesize);
        if (len == 0) {
            //qdir
            QFileInfo fi(file->fileName());
            fi.setCaching(false);
            //file->open(QFile::ReadOnly);
            len = (int)((fi.size() - _headersize) / dotwidth / channel / samplesize);
            return len;
        }
        else
            return len;
//        return (int)(file->size() / dotwidth / channel / samplesize);
    }
    return 0;
}

void WaveData::run()
{
    qlonglong npos = _headersize + readpos * dotwidth * channel * samplesize;
    if (npos > file->size()) {
        return;
    }
    file->seek(npos);

    QByteArray ba = file->read((qint64)readcount * dotwidth * channel * samplesize);
    if (ba.count() > 0) {
        int count = (int)(ba.count() / dotwidth / samplesize / channel);
        QVector<int> ret(count * channel * 2);

        uint cnt = 0;
        int retnr = 0;
        int max1[channel], max2[channel];
        int act;
        for(int i = 0; retnr < ret.count() && (i + channel) * samplesize < ba.count() && !breakWork; i+=channel) {
            for (uint j = 0; j < channel; j++) {
                int iba = (i + j) * samplesize;
                if (samplesize == 3)  {
                    if ((qint8)ba.at(iba + 2) < 0) {
                        act = 0xff000000 | ((quint8)ba.at(iba + 2)) << 16 | ((quint8)ba.at(iba + 1)) << 8 | (quint8)ba.at(iba);
                    }
                    else {
                        act = ((quint8)ba.at(iba + 2)) << 16 | ((quint8)ba.at(iba + 1)) << 8 | (quint8)ba.at(iba);
                    }
                }
                else {
                    if ((qint8)ba.at(iba + 1) < 0) {
                        act = 0xffff0000 | ((quint8)ba.at(iba + 1)) << 8 | (quint8)ba.at(iba);
                    }
                    else {
                        act = ((quint8)ba.at(iba + 1)) << 8 | (quint8)ba.at(iba);
                    }
                }
                if (cnt == 0) {
                    max1[j] = act;
                    max2[j] = act;
                }
                else if (act < max1[j])
                    max1[j] = act;
                else if (act > max2[j])
                    max2[j] = act;
            }
            cnt++;
            if (cnt >= dotwidth) {
                for (uint j = 0; j < channel; j++) {
                    ret[retnr++] = max2[j];
                    ret[retnr++] = max1[j];
                }
                cnt = 0;
            }
        }
        if (breakWork) {
            breakWork = false;
            lastdata.clear();
            lastpos = -1;
            emit CanReadNow();
        }
        else {
            if (retnr < ret.count() && retnr < ret.count() + samplesize + channel) {
                for (uint j = 0; j < channel; j++) {
                    ret[retnr++] = max2[j];
                    ret[retnr++] = max1[j];
                }
            }
            if (lastpos == -1 || lastwidth != dotwidth) {
                lock.lockForWrite();
                lastdata = ret;
                lastpos = readpos;
                lastcount = count;
                lastwidth = dotwidth;
                lock.unlock();
            }
            else if (readpos < lastpos) {
                if (readpos + count == lastpos) {
                    lock.lockForWrite();
                    ret << lastdata;
                    lastdata = ret;
                    lastpos = readpos;
                    lastcount = lastdata.count() / channel / 2;
                    lock.unlock();
                }
                else {
                    lock.lockForWrite();
                    lastdata = ret;
                    lastpos = readpos;
                    lastcount = count;
                    lock.unlock();
                }
            }
            else if (readpos > lastpos + lastcount + 1) {
                lock.lockForWrite();
                lastdata = ret;
                lastpos = readpos;
                lastcount = count;
                lock.unlock();
            }
            else if (readpos == lastpos + lastcount) {
                lock.lockForWrite();
                lastdata << ret;
                lastcount = lastdata.count() / channel / 2;
                lock.unlock();
            }

            emit CanReadNow();
        }
    }
}

QVector<int> WaveData::Data(uint count, qint64 newpos)
{
    //mutex.lock();
    if (isworking)
        return QVector<int>(count * 2 * channel, 0);
    if (!lock.tryLockForRead())
        return QVector<int>(count * 2 * channel, 0);
    isworking = true;
    if (newpos >= 0)
        _pos = newpos;
    QVector<int> ret;
    if (lastpos > -1) {
        if (lastwidth == dotwidth) {
            if (lastpos == _pos) {
                if (lastcount == count) {
                    ret = lastdata;
                }
                else if (lastcount > count) {
                    ret = lastdata.mid(0, count * channel * 2);
                }
            }
            if (lastpos < _pos && lastcount >= _pos - lastpos + count) {
                ret = lastdata.mid((_pos - lastpos) * channel * 2, count * channel * 2);
            }
            else if (lastpos <= _pos && Count() <= _pos + count )
                ret = lastdata.mid((_pos - lastpos) * channel * 2);
        }
        else
            lastpos = -1;
    }
    if (lastpos > -1) {
        if (lastwidth == dotwidth) {
            if (lastpos > _pos + count) {
                lastpos = -1;
            }
            else if (lastpos + lastcount < _pos) {
                lastpos = -1;
            }
            else if (lastpos > _pos) {
                int difference = lastpos - _pos;
                ret = QVector<int>(difference * channel * 2, 0);
                read(_pos, difference);
                ret << lastdata.mid(0, (count - difference) * channel * 2);
            }
            else if ((lastpos + lastcount) < (_pos + count)) {
                ret = lastdata.mid((_pos - lastpos) * channel * 2);
                ret << QVector<int>(count - (_pos - lastpos - lastcount), 0);
                read(lastpos + lastcount, count - (_pos - lastpos - lastcount));
            }
        }
        else
            lastpos = -1;
    }
    lock.unlock();
    isworking = false;
    if (lastpos > -1) {
        return ret;
    }

    read(_pos, count);

    return QVector<int>(count * 2 * channel, 0);
    return ret;
}

void WaveData::read(qlonglong pos, int count)
{
    readpos = pos;
    readcount = count;
    this->start();
}
