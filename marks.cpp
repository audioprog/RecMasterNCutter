#include "marks.h"
#include <QTextStream>

Marks::Marks()
{
    _pos = QList<qint64>();
    _marks = QList<MarkTypes>();
    _samplesize = 3;
}

void Marks::Save(QFile *file)
{
    file->open(QFile::WriteOnly);
    QTextStream ts(file);
    ts << (_samplesize * 8) << ";" << _startnr << "\n";
    for (int i = 0; i < _pos.count(); i++) {
        if (_strings.at(i) != "")
            ts << _pos.at(i) << "," << _marks.at(i) << "," << _strings.at(i) << "\n";
        else
            ts << _pos.at(i) << "," << _marks.at(i) << "\n";
    }
}

void Marks::Read(QFile *file)
{
    _pos.clear();
    _marks.clear();
    if (file->exists()) {
        file->open(QFile::ReadOnly);
        bool firstline = true;
        while (!file->atEnd()) {
            QString line = file->readLine();
            if (firstline) {
                if (!line.contains(',')) {
                    bool ok;
                    _samplesize = line.section(';', 0, 0).toInt(&ok);
                    if (!ok) {
                        _samplesize = 3;
                        emit Debug("Mark read first line Error:" + line);
                    }
                    else {
                        _samplesize /= 8;
                        if (line.contains(';')) {
                            _startnr = line.section(';', -1, -1).toInt(&ok);
                            if (!ok) {
                                _startnr = 1;
                                emit Debug("Mark read first line Error:" + line);
                            }
                        }
                    }
                }
                else
                    firstline = false;
            }
            if (!firstline) {
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
            else
                firstline = false;
        }
    }
}
