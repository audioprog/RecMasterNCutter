#ifndef ICONDELEGATE_H
#define ICONDELEGATE_H

#include <QPainter>
#include <QAbstractItemDelegate>
#include "markimages.h"

class IconDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit IconDelegate(QObject *parent = 0);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const { return QSize(16, 16); }

private:
    MarkImages images;

};

#endif // ICONDELEGATE_H
