#include "marks.h"
#include <QTextStream>

Marks::Marks()
{
    _pos = QList<qint64>();
    _marks = QList<MarkTypes>();
    _samplesize = 3;
    _startnr = 1;

    timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &Marks::tmpSave);
}

void Marks::Save(QFile *file, QStringList label)
{
    try {
    validate();
    file->open(QFile::WriteOnly);
    QTextStream ts(file);
    ts.setCodec("UTF-8");
    ts << (_samplesize * 8) << ";" << _startnr << ";" << label.join("\\") << "\n";
    for (int i = 0; i < _pos.count(); i++) {
        if (_strings.at(i) != "")
            ts << _pos.at(i) << "," << _marks.at(i) << "," << _strings.at(i) << "\n";
        else
            ts << _pos.at(i) << "," << _marks.at(i) << "\n";
    }
    } catch (...) {
        emit Debug("Save " + file->fileName() + " " + label.join(";"));
    }
    tmpfilename = file->fileName() + ".save";
    _label = label;

    if (!timer->isActive())
        timer->start(10000);
}

QStringList Marks::Read(QFile *file)
{
    _pos.clear();
    _marks.clear();
    _startnr = 1;
    QStringList ret;
    if (file->exists()) {
        file->open(QFile::ReadOnly);
        if (!file->atEnd()) {
            QString line = QString::fromUtf8(file->readLine());
            bool ok;
            _samplesize = line.section(';', 0, 0).toInt(&ok);
            if (!ok) {
                _samplesize = 3;
                emit Debug("Mark read first line Error:" + line);
            }
            else {
                _samplesize /= 8;
                if (line.contains(';')) {
                    _startnr = line.section(';', 1, 1).toInt(&ok);
                    if (!ok) {
                        _startnr = 1;
                        emit Debug("Mark read first line Error:" + line);
                    }
                    else {
                        ret = line.section(';', 2, -1).split('\\');
                    }
                }
            }
        }
        while (!file->atEnd()) {
            QString line = file->readLine();
            if (line != "") {
                qint64 ipos = line.section(',',0,0).toLongLong();
                int typ = line.section(',', 1, 1).toInt();
                if (ipos > -1) {
                    if (typ > -1 && typ < noFlag) {
                        if (line.count(',') > 1)
                            Add(ipos, static_cast<MarkTypes>(typ), line.section(',', 2, -1).simplified());
                        else
                            Add(ipos, static_cast<MarkTypes>(typ));
                    }
                }
            }
        }
    }
    validate();

    tmpfilename = file->fileName() + ".save";
    _label = ret;

    if (!timer->isActive())
        timer->start(10000);

    return ret;
}

QStringList Marks::ReadAutoSaved(QString filename)
{
    QStringList ret = Read(new QFile(filename + ".save"));
    tmpfilename = filename;
    return ret;
}

void Marks::tmpSave()
{
    try {
    validate();
    QFile *tmpfile = new QFile(tmpfilename);
    tmpfile->open(QFile::WriteOnly);
    QTextStream ts(tmpfile);
    ts << (_samplesize * 8) << ";" << _startnr << ";" << _label.join("\\") << "\n";
    for (int i = 0; i < _pos.count(); i++) {
        if (_strings.at(i) != "")
            ts << _pos.at(i) << "," << _marks.at(i) << "," << _strings.at(i) << "\n";
        else
            ts << _pos.at(i) << "," << _marks.at(i) << "\n";
    }
    } catch (...) {
        emit Debug("tmpSave " + tmpfilename + " " + _label.join(";"));
    }
}

void Marks::validate()
{
    int len = _pos.length();
    if (_marks.length() < len)
        len = _marks.length();
    if (_strings.length() < len)
        len = _strings.length();
    for (int i = 0; i < len; i++) {
        if (_strings.at(i) != "") {
            if (_marks.at(i) != StartTrack) {
                QString txt = _strings.at(i);
                QString old = "";
                _strings[i] = "";
                for (int j = i - 1; j >= 0 && txt != ""; j--) {
                    if (_marks.at(j) == StartTrack) {
                        if (_strings.at(j) == txt) {
                            txt = "";
                        }
                        else {
                            old = _strings.at(j);
                            _strings[j] = txt;
                            txt = old;
                        }
                    }
                }
            }
        }
    }
}
