#ifndef MARKIMAGES_H
#define MARKIMAGES_H

#include <QObject>
#include <QList>
#include <QPixmap>
#include <QIcon>
#include "marks.h"

class MarkImages : public QObject
{
    Q_OBJECT

public:
    MarkImages();
    QIcon icon(int Nr) const { if (Nr > -1 && Nr < pixlist.count()) { return icolist.at(Nr); } return QIcon(); }
    QIcon icon(Marks::MarkTypes typ) const { return icon(static_cast<int>(typ)); }
    int count() { return pixlist.count(); }
    QIcon nricon(int Nr) const { if (Nr > -1 && Nr < nrPixs.count()) { return QIcon(nrPixs.at(Nr)); } return QIcon(); }
    int nrcount() const { return nrPixs.count(); }
    QPixmap pix(int Nr) const {if (Nr > -1 && Nr < pixlist.count()) { return pixlist.at(Nr); } return QPixmap(); }
    QPixmap pix(Marks::MarkTypes typ) const { int Nr = static_cast<int>(typ); if (Nr > -1 && Nr < pixlist.count()) { return pixlist.at(Nr); } return QPixmap(); }

private:
    QList<QPixmap> pixlist;
    QList<QIcon> icolist;
    QList<QPixmap> nrPixs;
};

#endif // MARKIMAGES_H
