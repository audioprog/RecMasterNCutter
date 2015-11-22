#include "icondelegate.h"

IconDelegate::IconDelegate(QObject *parent) :
    QAbstractItemDelegate(parent)
{
}

void IconDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->drawPixmap(option.rect.left(), option.rect.top(), images.pix(index.data().value<int>()));
}
