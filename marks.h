#ifndef MARKS_H
#define MARKS_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QStringList>

class Marks : public QObject
{

    Q_OBJECT
    Q_ENUMS(MarkTypes)

public:
    enum MarkTypes {
        Standard = 0,
        StartTrack = 1,
        EndTrack = 2,
        StartSilence = 3,
        EndSilence = 4,
        FadeIn = 5,
        FadeOut = 6,
        noFlag = 7
    };

    Marks();
    qint64 Pos(int nr) const { return _pos.at(nr); }
    MarkTypes Type(int nr) const { return _marks.at(nr); }
    void setPos(int nr, int newPos) { if (newPos < 0) { newPos = 0; } if (nr < _pos.count() - 1 && newPos <= _pos.at(nr + 1)) { _pos[nr] = newPos; } { MarkTypes typ = _marks.takeAt(nr); _pos.removeAt(nr); Add(newPos, typ); } emit MarksChanged(); }
    void setMark(int nr, MarkTypes typ) { if (nr > -1 && nr < _pos.count()) { _marks[nr] = typ; emit MarksChanged(); } }
    void setMark(MarkTypes typ) { if (_marks.count() > 0) { _marks[_marks.count() - 1] = typ; emit MarksChanged(); } }
    MarkTypes FlagFromPos(qint64 pos) const { for (int i = _pos.count() - 1; i > -1; i--) { if (pos < _pos.at(i)) { return _marks.at(i); } } return noFlag; }
    QList<qint64> PosOfMarks(MarkTypes type) const { QList<qint64> ret; for (int i = 0; i < _marks.count(); i++) { if (_marks.at(i) == type) { ret << _pos.at(i); } } return ret; }
    QList<int> AllOfMarks(MarkTypes type) const { QList<int> ret; for (int i = 0; i < _marks.count(); ++i) { if (_marks.at(i) == type) { ret << i; } } return ret; }
    QList<int> Range(qint64 start, qint64 width) const { QList<int> ret; for (int i = 0; i < _pos.count(); i++) { if (_pos.at(i) >= start) { if (_pos.at(i) < start + width) { ret << i; } else { return ret; } } } return ret; }
    bool Add(qint64 pos, MarkTypes typ) { for (int i = 0; i < _pos.count(); i++) { if (pos < _pos.at(i)) { _pos.insert(i, pos); _marks.insert(i, typ); _strings.insert(i, ""); emit MarksChanged(); return true; } } if (pos < 0) { return false; } _pos.append(pos); _marks.append(typ); _strings.append(""); emit MarksChanged(); return true; }
    bool Add(qint64 pos, MarkTypes typ, QString text) { for (int i = 0; i < _pos.count(); i++) { if (pos < _pos.at(i)) { _pos.insert(i, pos); _marks.insert(i, typ); _strings.insert(i, text); emit MarksChanged(); return true; } } if (pos < 0) { return false; } _pos.append(pos); _marks.append(typ); _strings.append(text); emit MarksChanged(); return true; }
    void Remove(int Nr) { if (Nr > -1 && Nr < _pos.count()) { _pos.removeAt(Nr); _marks.removeAt(Nr); emit MarksChanged(); } }
    int Count() const { return _pos.count(); }
    int Count(MarkTypes type) const { int ret = 0; foreach (MarkTypes itm, _marks) { if (itm == type) { ++ret; } } return ret; }
    QString Text(int nr) { return _strings.at(nr); }
    void setText(int nr, QString newText) { _strings[nr] = newText; }
    void Save(QFile *file);
    void Read(QFile *file);

    void setSampleSize(int newSize) { _samplesize = newSize; }
    int SampleSize() { return _samplesize; }

signals:
    void MarksChanged();
    void Debug( QString text );

private:
    QList<qint64> _pos;
    QList<MarkTypes> _marks;
    QStringList _strings;
    int _samplesize;
};

#endif // MARKS_H
