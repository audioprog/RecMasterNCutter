#include "markimages.h"
#include <QPainter>
#include <QBitmap>
#include <QImage>
//#include <QtDebug>

MarkImages::MarkImages()
{
    QSize dsiz(16, 16);
    QSize psiz(16, 16);
    QPixmap *pix1 = new QPixmap(psiz);
    QPainter *painter1 = new QPainter(pix1);
    QColor color(Qt::white);
    painter1->fillRect(0, 0, psiz.width(), psiz.height(), color);
    painter1->setBrush(Qt::black);
    QPoint Standard[] = {QPoint(dsiz.width() / 4,0), QPoint(dsiz.width() / 4 * 3, 0), QPoint(dsiz.width() / 2, dsiz.height())};
    painter1->drawPolygon(Standard, 3, Qt::OddEvenFill);
    painter1->end();
    pix1->setMask(pix1->createMaskFromColor(color.rgb(), Qt::MaskInColor));
    pixlist.append(*pix1);
    //pix1->save("Standard.png", "PNG");
    icolist.append(QIcon(*pix1));
    QPixmap *pix2 =new QPixmap(psiz);
    QPainter *painter2 = new QPainter(pix2);
    painter2->fillRect(0, 0, psiz.width(), psiz.height(), color);
    painter2->setBrush(Qt::blue);
    QPoint Start[] = {QPoint(dsiz.width() / 4,0), QPoint(dsiz.width() / 4 * 3, dsiz.height() / 2), QPoint(dsiz.width() / 4, dsiz.height())};
    painter2->drawPolygon(Start, 3, Qt::OddEvenFill);
    painter2->end();
    pix2->setMask(pix2->createMaskFromColor(color.rgb(), Qt::MaskInColor));
    //pix2->save("Start_Track.png", "PNG");
    icolist.append(QIcon(*pix2));
    pixlist.append(*pix2);
    QPixmap *pix3 =new QPixmap(psiz);
    QPainter *painter3 = new QPainter(pix3);
    painter3->setBrush(Qt::blue);
    painter3->fillRect(0, 0, psiz.width(), psiz.height(), color);
    QPoint end[] = {QPoint(dsiz.width() / 4 * 3,0), QPoint(dsiz.width() / 4, dsiz.height() / 2), QPoint(dsiz.width() / 4 * 3, dsiz.height())};
    painter3->drawPolygon(end, 3, Qt::OddEvenFill);
    painter3->end();
    pix3->setMask(pix3->createMaskFromColor(color.rgb(), Qt::MaskInColor));
    //pix3->save("End_Track.png", "PNG");
    pixlist.append(*pix3);
    icolist.append(QIcon(*pix3));
    QPixmap *pix4 =new QPixmap(psiz);
    QPainter *painter4 = new QPainter(pix4);
    painter4->fillRect(0, 0, psiz.width(), psiz.height(), color);
    painter4->setBrush(Qt::green);
    painter4->drawPolygon(Start, 3, Qt::OddEvenFill);
    painter4->end();
    pix4->setMask(pix4->createMaskFromColor(color.rgb(), Qt::MaskInColor));
    //pix4->save("Start_Silence.png", "PNG");
    pixlist.append(*pix4);
    icolist.append(QIcon(*pix4));
    QPixmap *pix5 =new QPixmap(psiz);
    QPainter *painter5 = new QPainter(pix5);
    painter5->fillRect(0, 0, psiz.width(), psiz.height(), color);
    painter5->setBrush(Qt::green);
    painter5->drawPolygon(end, 3, Qt::OddEvenFill);
    painter5->end();
    pix5->setMask(pix5->createMaskFromColor(color.rgb(), Qt::MaskInColor));
    //pix5->save("End_Silence.png", "PNG");
    pixlist.append(*pix5);
    icolist.append(QIcon(*pix5));
    for (int i = 1; i < 5; ++i) {
        pix1 = new QPixmap(psiz);
        painter1 = new QPainter(pix1);
        painter1->fillRect(0,0,psiz.width(), psiz.height(), color);
        painter1->setBrush(Qt::red);
        painter1->setOpacity(0.7);
        painter1->drawRoundedRect(0,0,psiz.width() - 1, psiz.height() - 1, 5.0, 4.0);
        painter1->drawText(0,0,psiz.width() - 1, psiz.height() - 1, Qt::AlignCenter, QString::number(i));
        painter1->end();
        pix1->setMask(pix1->createMaskFromColor(color.rgb(), Qt::MaskInColor));
        //pix1->save("Add_" + QString::number(i) + ".png", "PNG");
        nrPixs.append(*pix1);
    }
    /*pix1 = new QPixmap(psiz);
    painter1 = new QPainter(pix1);
    painter1->fillRect(0,0,psiz.width(), psiz.height(), color);
    painter1->setBrush(Qt::red);
    painter1->setOpacity(0.7);
    painter1->setFont(QFont(painter1->font().family(), painter1->font().pointSize() - 2));
    painter1->drawRoundedRect(0,0,psiz.width() - 1, psiz.height() - 1, 5.0, 4.0);
    painter1->drawText(0,1,psiz.width(), psiz.height() - 1, Qt::AlignCenter, "Act");
    painter1->end();
    pix1->setMask(pix1->createMaskFromColor(color.rgb(), Qt::MaskInColor));
    //pix1->save("Add_Actual.png", "PNG");
    nrPixs.append(*pix1);*/

    //QColor color(Qt::white);
    QPoint FaddIn[] = {QPoint(0, dsiz.height()-1), QPoint(dsiz.width()-1, 0), QPoint(dsiz.width() / 2, dsiz.height()-1)};
    QPixmap *pix6 =new QPixmap(psiz);
    QPainter *painter6 = new QPainter(pix6);
    painter6->fillRect(0, 0, psiz.width(), psiz.height(), color);
    painter6->setBrush(QColor(255, 96, 0));
    painter6->drawPolygon(FaddIn, 3, Qt::OddEvenFill);
    painter6->end();
    pix6->setMask(pix6->createMaskFromColor(color.rgb(), Qt::MaskInColor));
    pix6->save("F:/ngit/remoteRecMaster/icons/faddIn.png");
    pixlist.append(*pix6);
    icolist.append(QIcon(*pix6));

    QPoint FaddOut[] = {QPoint(1, 1), QPoint(dsiz.width()-1, dsiz.height()-1), QPoint(dsiz.width() / 2, dsiz.height()-1)};
    QPixmap *pix7 =new QPixmap(psiz);
    QPainter *painter7 = new QPainter(pix7);
    painter7->fillRect(0, 0, psiz.width(), psiz.height(), color);
    painter7->setBrush(QColor(255, 96, 0));
    painter7->drawPolygon(FaddOut, 3, Qt::OddEvenFill);
    painter7->end();
    pix7->setMask(pix7->createMaskFromColor(color.rgb(), Qt::MaskInColor));
    pix7->save("F:/ngit/remoteRecMaster/icons/faddOut.png");
    pixlist.append(*pix7);
    icolist.append(QIcon(*pix7));
}
