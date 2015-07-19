#include <QtGui>
#include "waveform.h"

//#include <QtDebug>

WaveForm::WaveForm(QWidget *parent) :
    QWidget(parent)
{
    timer = new QTimer();
    lastLen = 0;
    onesecund = 0;
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(actualize()));
    followend = false;
    marks = new Marks();
    hoverMark = -1;
    setMouseTracking(true);
    rulerHeight = 14;
    mrkMoveNr = -1;
    mrkMovePos = -1;
    mrkMoveDiff = 0;
    mrkMoveSlide = 0;
    setContextMenuPolicy(Qt::DefaultContextMenu);
    selected = -1;
    expand = true;
    actPlayPos = -1;
    QObject::connect(&data, SIGNAL(CanReadNow()), this, SLOT(actualize()));
    QObject::connect(&data, SIGNAL(CanReadNow()), this, SLOT(update()));
}

WaveForm::~WaveForm()
{
}

void WaveForm::SetFile(QFile *file)
{
    data.FileOpen(file);
    //marks->Read(new QFile(file->fileName() + ".rmrk"));
    //marks->Add(200000, Marks::StartSilence);
    timer->stop();
    timer->start(300);
    this->update();
}

void WaveForm::AddMarkAtInsertPos(int InsPosItem, Marks::MarkTypes typ)
{
    qint64 npos = (data.Pos() + (qint64)((float)width() * insPosList.at(InsPosItem))) * data.DotWidth();
    marks->Add(npos, typ);
}

void WaveForm::actualize()
{
    int newlen = data.Count();
    if (lastLen != newlen || lastwidth != width()) {
        lastLen = newlen;
        lastwidth = width();
        emit LenghtChanged(newlen, width());
        if (followend) {
            NewPos(newlen - width());
            emit PosChanged(data.Pos(), true);
        }
    }
    if (mrkMoveSlide != 0 && rulerHeight > 0) {
        emit PosChanged(mrkMoveSlide, false);
    }
    QTimer::singleShot(100, this, SLOT(update()));
}

void WaveForm::SaveMarks(int startNr, QStringList label)
{
    try {
    if (data.FileName().count() > 0) {
        marks->setStartNr(startNr);
        marks->Save(new QFile(data.FileName() + ".rmrk"), label);
    }
    } catch (...) {
        emit debug("SaveMarks()");
    }
}

int WaveForm::AddMark(int x, Marks::MarkTypes Mark)
{
    int pos = marks->Add((data.Pos() + x) * data.DotWidth(), Mark);
    update();
    return pos;
}

void WaveForm::PlayPos(qint64 pPos)
{
    actPlayPos = pPos;
    update();
}

void WaveForm::NewPos(int newPos)
{
    if (newPos < 0)
        newPos = 0;
    data.SetPos(newPos);
    this->update();
}

void WaveForm::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    //painter.setPen(palette().dark().brush());
    //painter.setBrush(brush);
    //qDebug() << onesecund << data.DotWidth();
    if (onesecund == 0) {
        onesecund = painter.device()->logicalDpiX() * 1.2;
        data.setSamplesize(marks->SampleSize());
        data.SetDotWidth(44100 / onesecund);
    }

    if (data.Count()) {
        int datacount = data.Count() > width() ? width() : data.Count();
        QVector<int> vectors = data.Data(datacount);
        //qDebug() << datacount << vectors.count() << vectors.count() / 4;
        int channel = data.Channel();
        int channel_x_2 = channel * 2;
        datacount = vectors.count() / channel_x_2;
        int rulerHalfHeight = rulerHeight / 2;
        int restheight = height() - rulerHeight;
        painter.setPen(Qt::white);
        painter.setBrush(QBrush(Qt::white));
        painter.drawRect(0, rulerHeight, width(), restheight);
        int hmax = 0x7fffff;
        float max = hmax;
        float mheight = restheight / channel_x_2;
        Marks::MarkTypes lastFlags = Marks::noFlag;
        if (rulerHeight > 0) {
            painter.setPen(Qt::black);
            painter.drawLine(0, rulerHeight, width(), rulerHeight);
            int counter = onesecund > 80 ? onesecund : 80 - ( 80 % onesecund );
            for (int i = counter - (data.Pos() % counter); i < width(); i += counter) {
                QTime time = QTime(0,0,0);
                time = time.addSecs((data.Pos() + i) / onesecund);
                QString text = "";
                if (time.hour() > 0)
                    text += time.toString("h") + " St ";
                if (time.minute() > 0)
                    text += time.toString("m") + " m ";
                if (time.second() > 0)
                    text += time.toString("s") + " s";
                painter.drawText(i + 2, -1, counter - 4, rulerHeight - 1, Qt::AlignVCenter | Qt::AlignLeft, text);
                painter.drawLine(i, 0, i, rulerHeight);
            }

            QList<int> lst = marks->Range((data.Pos() - rulerHalfHeight) * data.DotWidth(), (width() + rulerHeight) * data.DotWidth());

            painter.setOpacity(0.4);
            bool onemarked = false;
            foreach(int itm, lst) {
                int x = marks->Pos(itm) / data.DotWidth() - data.Pos();
                int mtrack1 = x - 1, mtrack2 = x + 1;
                if (lastFlags != marks->Type(itm)) {
                    lastFlags = marks->Type(itm);
                    switch (marks->Type(itm)) {
                    case Marks::Standard:
                        painter.setBrush(QBrush(selected == itm ? Qt::black : Qt::white));
                        break;
                    case Marks::StartTrack:
                    case Marks::EndTrack:
                        painter.setBrush(QBrush(Qt::blue));
                        break;
                    case Marks::StartSilence:
                    case Marks::EndSilence:
                        painter.setBrush(QBrush(Qt::green));
                        break;
                    case Marks::FadeIn:
                    case Marks::FadeOut:
                        painter.setBrush(QBrush(QColor(255, 96, 00)));
                        break;
                    default:
                        painter.setBrush(QBrush(Qt::black));
                        break;
                    }
                }
                QPoint points[3];
                if (lastFlags == Marks::Standard) {
                    points[0] = QPoint(x - rulerHalfHeight, 0);
                    points[1] = QPoint(x + rulerHalfHeight, 0);
                    points[2] = QPoint(x, rulerHeight);
                    mtrack1 -= 5;
                    mtrack2 += 5;
                }
                else if (lastFlags == Marks::StartTrack || lastFlags == Marks::StartSilence) {
                    points[0] = QPoint(x, 0);
                    points[1] = QPoint(x + rulerHalfHeight, rulerHalfHeight);
                    points[2] = QPoint(x, rulerHeight);
                    mtrack2 += 5;
                }
                else if (lastFlags == Marks::FadeIn) {
                    points[0] = QPoint(x - 10, rulerHeight);
                    points[1] = QPoint(x, 1);
                    points[2] = QPoint(x, rulerHeight);
                    mtrack1 -= 10;
                }
                else if (lastFlags == Marks::FadeOut) {
                    points[0] = QPoint(x, 1);
                    points[1] = QPoint(x + 10, rulerHeight);
                    points[2] = QPoint(x, rulerHeight);
                    mtrack2 += 10;
                }
                else {
                    points[0] = QPoint(x, 0);
                    points[1] = QPoint(x - rulerHalfHeight, rulerHalfHeight);
                    points[2] = QPoint(x, rulerHeight);
                    mtrack1 -= 5;
                }
                if (hoverMark > mtrack1 && hoverMark < mtrack2 && !onemarked)
                    painter.setOpacity(0.8);
                if (selected == itm)
                    painter.setOpacity(1.0);
                painter.drawPolygon(points, 3, Qt::OddEvenFill);
                if (hoverMark > mtrack1 && hoverMark < mtrack2 && !onemarked) {
                    painter.setOpacity(0.3);
                    onemarked = true;
                }
                else if (selected == itm) {
                    painter.setOpacity(0.3);
                }
                painter.drawLine(x, rulerHeight, x, height());
            }
            //painter.setBrush(QBrush(Qt::black));
            painter.setOpacity(1.0);
        }

        if (rulerHeight == 0 && mrkMovePos > -1) {
            painter.setBrush(QBrush(Qt::cyan));
            painter.setPen(Qt::blue);
            int x = mrkMovePos - data.Pos();
            painter.drawRect(x, 0, mrkMoveSlide, height() - 1);
        }

        if (expand) {
            int dmax = 1;
            for (int i = 0; i < vectors.count() - 1; i += channel) {
                if (dmax < vectors.at(i))
                    dmax = vectors.at(i);
                if (dmax < -vectors.at(i + 1))
                    dmax = -vectors.at(i + 1);
            }
            max = dmax;
        }

        painter.setPen(Qt::black);
        int idxv = 0, midx = 0, vectorsidx = 0;
        for (int i = 0; i < datacount; ++i)
        {
            idxv = i * channel_x_2;
            for (int idxvc = 0; idxvc < channel; ++idxvc) {
                midx = rulerHeight + (mheight + mheight * idxvc * 2);
                vectorsidx = idxv + (idxvc * 2);
                painter.drawLine(i, midx - (int)((float)vectors.at(vectorsidx) / max * mheight), i, midx - (int)((float)vectors.at(vectorsidx + 1) / max * mheight));
            }
        }

        if (mrkMoveNr > -1) {
            QPoint points[3];
            int x = mrkMovePos;
            if (mrkMoveType == Marks::Standard) {
                points[0] = QPoint(x - rulerHalfHeight, 0);
                points[1] = QPoint(x + rulerHalfHeight, 0);
                points[2] = QPoint(x, rulerHeight);
            }
            else if (mrkMoveType == Marks::StartTrack || mrkMoveType == Marks::StartSilence) {
                points[0] = QPoint(x, 0);
                points[1] = QPoint(x + rulerHalfHeight, rulerHalfHeight);
                points[2] = QPoint(x, rulerHeight);
            }
            else if (mrkMoveType == Marks::FadeIn) {
                points[0] = QPoint(x - rulerHeight, rulerHeight);
                points[1] = QPoint(x, 1);
                points[2] = QPoint(x, rulerHeight);
            }
            else if (mrkMoveType == Marks::FadeOut) {
                points[0] = QPoint(x, 1);
                points[1] = QPoint(x + 10, rulerHeight);
                points[2] = QPoint(x, rulerHeight);
            }
            else {
                points[0] = QPoint(x, 0);
                points[1] = QPoint(x - rulerHalfHeight, rulerHalfHeight);
                points[2] = QPoint(x, rulerHeight);
            }
            if (lastFlags != mrkMoveType) {
                lastFlags = mrkMoveType;
                switch (lastFlags) {
                case Marks::Standard:
                    painter.setBrush(QBrush(Qt::white));
                    break;
                case Marks::StartTrack:
                case Marks::EndTrack:
                    painter.setBrush(QBrush(Qt::blue));
                    break;
                case Marks::StartSilence:
                case Marks::EndSilence:
                    painter.setBrush(QBrush(Qt::green));
                    break;
                case Marks::FadeIn:
                case Marks::FadeOut:
                    painter.setBrush(QBrush(QColor(255, 96, 00)));
                    break;
                default:
                    painter.setBrush(QBrush(Qt::black));
                    break;
                }
            }
            painter.drawPolygon(points, 3, Qt::OddEvenFill);
            painter.drawLine(x, rulerHeight, x, height());
        }

        if (insPosList.count() > 0) {
            painter.setBrush(QBrush(Qt::red));

            for(int i = 0; i < insPosList.count(); ++i) {
                float itm = insPosList.at(i);
                painter.drawLine(itm * width(), 0, itm * width(), height());
                painter.setOpacity(0.6);
                if (itm * width() + 2 + rulerHeight > width()) {
                    painter.drawRoundedRect(itm * width() - rulerHeight - 1, rulerHeight + 1, rulerHeight, rulerHeight, 5.0, 4.0);
                    painter.drawText(itm * width() - rulerHeight - 1, rulerHeight + 1, rulerHeight, rulerHeight, Qt::AlignCenter, QString::number(i + 1));
                }
                else {
                    painter.drawRoundedRect(itm * width() + 1, rulerHeight + 1, rulerHeight, rulerHeight, 5.0, 4.0);
                    painter.drawText(itm * width() + 1, rulerHeight + 1, rulerHeight, rulerHeight, Qt::AlignCenter, QString::number(i + 1));
                }
            }
        }
        else if (actPlayPos > -1) {
            qint64 np = actPlayPos / (marks->SampleSize() * 2) / data.DotWidth() - data.Pos();
            if (np <= width() && np > -1) {
                painter.setPen(Qt::blue);
                painter.drawLine((int)np, rulerHeight + 1, (int)np, height());
                if (np > width() / 5 * 4)
                    emit PosChanged(actPlayPos / (marks->SampleSize() * 2) / data.DotWidth(), true);
            }
            else {
                emit PosChanged(actPlayPos / (marks->SampleSize() * 2) / data.DotWidth(), true);
            }
        }

        if (lastwidth != width()) {
            lastwidth = width();
            emit LenghtChanged(data.Count(), width());
        }
    }
}

void WaveForm::wheelEvent(QWheelEvent *event)
{
    emit PosChanged(-event->delta(), false);
}

void WaveForm::mouseMoveEvent(QMouseEvent *event)
{
    if (rulerHeight > 0) {
        if (event->buttons() == 0) {
            if (event->y() <= rulerHeight) {
                if (hoverMark != event->x()) {
                    hoverMark = event->x();
                    //qDebug() << hoverMark;
                    update();
                }
            }
            else {
                if (hoverMark != -1) {
                    hoverMark = -1;
                    update();
                }
            }
        }
        else if (event->buttons() == Qt::LeftButton) {
            int npos = event->x() - mrkMoveDiff;
            if (npos != mrkMovePos) {
                mrkMovePos = npos;
                if (npos < 0) {
                    emit PosChanged(npos, false);
                    mrkMoveSlide = npos;
                }
                else if (npos > width()) {
                    emit PosChanged(npos - width(), false);
                    mrkMoveSlide = npos - width();
                }
                else {
                    update();
                    mrkMoveSlide = 0;
                }
            }
        }
    }
    else if (event->buttons() == Qt::LeftButton && mrkMovePos > -1 && mrkMoveNr > 0) {
        int x = event->x() - mrkMoveDiff;
        x += data.Pos();
        emit OverviewMarkPosChanged(x);
    }
}

void WaveForm::mousePressEvent(QMouseEvent *event)
{
    if (event->y() <= rulerHeight) {
        if (data.Count()) {
            qint64 start = data.Pos() < rulerHeight ? 0 : (data.Pos() - rulerHeight) * data.DotWidth();
            QList<int> lst = marks->Range(start, (width() + rulerHeight) * data.DotWidth());
            int hoverMark = event->x();
            foreach (int itm, lst) {
                int x = marks->Pos(itm) / data.DotWidth() - data.Pos();
                int mtrack1 = x - 1, mtrack2 = x + 1;
                Marks::MarkTypes lastFlags = marks->Type(itm);
                if (lastFlags == Marks::Standard) {
                    mtrack1 -= 5;
                    mtrack2 += 5;
                }
                else if (lastFlags == Marks::StartTrack || lastFlags == Marks::StartSilence) {
                    mtrack2 += 5;
                }
                else if (lastFlags == Marks::FadeIn) {
                    mtrack1 -= 10;
                }
                else if (lastFlags == Marks::FadeOut) {
                    mtrack2 += 10;
                }
                else {
                    mtrack1 -= 5;
                }
                if (hoverMark > mtrack1 && hoverMark < mtrack2) {
                    mrkMoveType = lastFlags;
                    mrkMoveNr = itm;
                    mrkMovePos = x;
                    mrkMoveDiff = event->x() - x;
                    update();
                }
            }
        }
    }
    else if (rulerHeight == 0 && event->buttons() == Qt::LeftButton && mrkMovePos > -1) {
        int x = data.Pos() + event->x();
        if (x >= mrkMovePos && x < mrkMovePos + mrkMoveSlide) {
            mrkMoveDiff = x - mrkMovePos - 1;
            mrkMoveNr = 1;
        }
        else {
            mrkMoveNr = 0;
        }
    }
    else if (event->buttons() == Qt::LeftButton && rulerHeight > 0 && followend) {
        int pos = AddMark(event->x(), Marks::Standard);
        emit OverviewMarkPosChanged(pos);
    }
}

void WaveForm::mouseReleaseEvent(QMouseEvent *event)
{
    if (rulerHeight > 0 && mrkMoveNr != -1) {
        qint64 x = (qint64)(mrkMovePos + data.Pos()) * data.DotWidth();
        if (x < 0)
            x = 0;
        marks->setPos(mrkMoveNr, x);
        mrkMoveNr = -1;
        mrkMoveSlide = 0;
        update();
    }
    else if (rulerHeight == 0) {
        if (mrkMoveNr == 0) {
            int x = data.Pos() + event->x() - mrkMoveSlide / 2 + 1;
            if (x < 0)
                x = 0;
            if (x > data.Count() - mrkMoveSlide)
                x = data.Count() - mrkMoveSlide;
            emit OverviewMarkPosChanged(x);
        }
        mrkMoveNr = 0;
    }

    /*if (event->button() == Qt::MidButton)
    {
        //qDebug() << data.Pos() << event->x() << data.DotWidth();
        emit Play((data.Pos() + event->x()) * (qint64)data.DotWidth() * 6);
    }*/
}

void WaveForm::mouseDoubleClickEvent(QMouseEvent *event)
{
    int samplesize = marks->SampleSize() * 2;
    emit Play((data.Pos() + event->x()) * (qint64)data.DotWidth() * samplesize);
}

void WaveForm::leaveEvent(QEvent *)
{
    if (hoverMark != -1) {
        hoverMark = -1;
        update();
    }
}

void WaveForm::contextMenuEvent(QContextMenuEvent *event)
{
    if (data.Count()) {
        if (event->y() < rulerHeight) {
            QList<int> lst = marks->Range(data.Pos() * data.DotWidth(), width() * data.DotWidth());
            int hoverMark = event->x();
            foreach (int itm, lst) {
                int x = marks->Pos(itm) / data.DotWidth() - data.Pos();
                int mtrack1 = x - 1, mtrack2 = x + 1;
                Marks::MarkTypes lastFlags = marks->Type(itm);
                if (lastFlags == Marks::Standard) {
                    mtrack1 -= 5;
                    mtrack2 += 5;
                }
                else if (lastFlags == Marks::StartTrack || lastFlags == Marks::StartSilence) {
                    mtrack2 += 5;
                }
                else {
                    mtrack1 -= 5;
                }
                if (hoverMark > mtrack1 && hoverMark < mtrack2) {
                    emit ContextMenuWanted(event->globalPos(), event->x(), itm);
                    return;
                }
            }
            emit ContextMenuWanted(event->globalPos(), event->x(), -1);
        }
        else
            emit ContextMenuWanted(event->globalPos(), event->x(), -1);
    }
}
