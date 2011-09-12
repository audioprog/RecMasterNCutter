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
                firstline = false;
                if (!line.contains(',')) {
                    if (line == "16")
                        s24 = false;
                    else
                        s24 = true;
                }
            }
            qint64 ipos = line.section(',',0,0).toLongLong();
            int typ = line.section(',', -1, -1).toInt();
            if (ipos > -1) {
                if (typ > -1 && typ < noFlag) {
                    Add(ipos, static_cast<MarkTypes>(typ));
                }
            }
        }
    }
}
