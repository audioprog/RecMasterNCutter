#ifndef BUTTONSTATEITEMDELEGATE_H
#define BUTTONSTATEITEMDELEGATE_H

#include <QStyledItemDelegate>

class ButtonStateItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ButtonStateItemDelegate(QWidget *parent = 0) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

signals:
    void fieldClicked(int row, int column);

private slots:
    void commitAndCloseEditor();
};

#endif // BUTTONSTATEITEMDELEGATE_H
