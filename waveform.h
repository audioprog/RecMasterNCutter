#ifndef WAVEFORM_H
#define WAVEFORM_H

#include <QWidget>
#include <QBrush>
#include <QPen>
#include <QFile>
#include <QTimer>
#include <QList>
#include "wavedata.h"
#include "marks.h"

class WaveForm : public QWidget
{
    Q_OBJECT

public:
    explicit WaveForm(QWidget *parent = 0);
    ~WaveForm();
    void SetFile(QFile *file);
    void SetFollowEnd(bool newValue) { followend = newValue; }
    bool followEnd() { return followend; }
    void setMarks(Marks *newFlags) { marks = newFlags; }
    Marks* getMarks() { return marks; }
    void AddInsertPos( float newpos ) { if (newpos >= 0.0 && newpos <= 1.0) { insPosList.append(newpos); } }
    int InsertPosCount() { return insPosList.count(); }
    void AddMarkAtInsertPos(int InsPosItem, Marks::MarkTypes typ);
    void SetRulerHeight( int newheight ) { rulerHeight = newheight; }
    void SetDotWidthSecs(int sec) { onesecund = sec; data.SetDotWidth(44100 * sec); }
    void SetDotWidth( int newwidth ) { data.SetDotWidth(newwidth); onesecund = 44100 / newwidth; }
    int DotWidth() { return data.DotWidth(); }
    int Selected() { return selected; }
    void SetDebugNr(int nr) { data.setDebugNr(nr); }

signals:
    void LenghtChanged(int newLen, int windowlength);
    void PosChanged(int diff, bool absolut);
    void ContextMenuWanted(QPoint pos, int x, int Mark);
    void OverviewMarkPosChanged(int x);
    void Play(qint64 pos);

public slots:
    void NewPos(int newPos);
    void actualize();
    void SaveMarks();
    void AddMark(int x, Marks::MarkTypes Mark);
    void SelectNext() { if (selected < marks->Count() - 1) { selected++; MoveToSelection(); } }
    void SelectPrevious() { if (selected > 0) { selected--; MoveToSelection(); } }
    void Select(int Nr) { if (Nr > -1 && Nr < marks->Count()) { selected = Nr; MoveToSelection(); } }
    void MoveToSelection() { if (selected > -1 && selected < marks->Count()) { int np = marks->Pos(selected) / data.DotWidth() - width() / 2; emit PosChanged(np, true); } }
    void OverviewMarkChanged(int count, int pos) { mrkMovePos = pos; mrkMoveSlide = count; emit PosChanged(pos - width() / 2, true); actualize(); update(); }
    void Play() { emit Play(data.Pos() * data.DotWidth()); }
    void Clear() { data.Clear(); update(); }
    void setExpand( bool newVal ) { expand = newVal; update(); }

protected:
    void paintEvent(QPaintEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void leaveEvent(QEvent *);
    void contextMenuEvent(QContextMenuEvent *event);

private:
    WaveData data;
    QTimer *timer;
    Marks *marks;
    int hoverMark;
    int lastLen;
    int lastwidth;
    int onesecund;
    bool followend;
    int selected;
    bool expand;

    Marks::MarkTypes mrkMoveType;
    int mrkMoveNr;
    int mrkMovePos;
    int mrkMoveDiff;
    int mrkMoveSlide;

    QList<float> insPosList;

    int rulerHeight;
};

#endif // WAVEFORM_H
