#include "marks.h"
#include <QTextStream>

Marks::Marks()
{
    _pos = QList<qint64>();
    _marks = QList<MarkTypes>();
}

void Marks::Save(QFile *file)
{
    file->open(QFile::WriteOnly);
    QTextStream ts(file);
    ts << (s24 ? "24" : "16") << "\n";
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
                    if (line.startsWith("16"))
                        s24 = false;
                    else
                        s24 = true;
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
                            Add(ipos, static_cast<MarkTypes>(typ), line.section(',', 2, -1));
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
