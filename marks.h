#ifndef MARKS_H
#define MARKS_H

#include <QObject>
#include <QList>
#include <QFile>
#include <QStringList>
#include <QTimer>

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
    int NextTrackEnd(int start) { for (int i = start + 1; i < _marks.count(); i++) { if (_marks.at(i) == Marks::EndTrack) { return i; } } return -1; }
    int Add(qint64 pos, MarkTypes typ) { for (int i = 0; i < _pos.count(); i++) { if (pos < _pos.at(i)) { _pos.insert(i, pos); _marks.insert(i, typ); _strings.insert(i, ""); emit MarksChanged(); return i; } } if (pos < 0) { return -1; } _pos.append(pos); _marks.append(typ); _strings.append(""); emit MarksChanged(); return _marks.count() - 1; }
    int Add(qint64 pos, MarkTypes typ, QString text) { for (int i = 0; i < _pos.count(); i++) { if (pos < _pos.at(i)) { _pos.insert(i, pos); _marks.insert(i, typ); _strings.insert(i, text); emit MarksChanged(); return i; } } if (pos < 0) { return -1; } _pos.append(pos); _marks.append(typ); _strings.append(text); emit MarksChanged(); return _marks.count() - 1; }
    void Remove(int Nr) { if (Nr > -1 && Nr < _pos.count()) { _pos.removeAt(Nr); _marks.removeAt(Nr); _strings.removeAt(Nr); emit MarksChanged(); } }
    int Count() const { return _pos.count(); }
    int Count(MarkTypes type) const { int ret = 0; foreach (MarkTypes itm, _marks) { if (itm == type) { ++ret; } } return ret; }
    int Count(MarkTypes type, int startnr) const { int ret = 0; for (int i = startnr; i < _pos.count(); i++) { if (_marks.at(i) == type) { ++ret; } } return ret; }
    QString Text(int nr) { return _strings.at(nr); }
    void setText(int nr, QString newText) { _strings[nr] = newText; }
    void Save(QFile *file, QStringList label);
    QStringList Read(QFile *file);
    QStringList ReadAutoSaved(QString filename);

    void setSampleSize(int newSize) { _samplesize = newSize; }
    int SampleSize() { return _samplesize; }
    void setStartNr(int newNr) { _startnr = newNr; }
    int StartNr() { return _startnr; }

signals:
    void MarksChanged();
    void Debug( QString text );

private slots:
    void tmpSave();

private:
    void validate();

    QString tmpfilename;

    QTimer *timer;
    QList<qint64> _pos;
    QList<MarkTypes> _marks;
    QStringList _strings;
    QStringList _label;
    int _samplesize;
    int _startnr;
};

#endif // MARKS_H
