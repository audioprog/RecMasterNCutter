#include "buttonstateitemdelegate.h"
#include "buttonstate.h"
#include "buttonstatewidget.h"

#include <QtGui>

void ButtonStateItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    if (qVariantCanConvert<ButtonState>(index.data())) {
        ButtonState buttonState = qVariantValue<ButtonState>(index.data());

        if (option.state & QStyle::State_Selected)
            painter->fillRect(option.rect, option.palette.highlight());
        else if (buttonState.IconMode() == QIcon::Disabled)
            buttonState.restore();

        buttonState.paint(painter, option.rect, option.palette,
                         ButtonState::ReadOnly);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
//! [0]
}

//! [1]
QSize ButtonStateItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (qVariantCanConvert<ButtonState>(index.data())) {
        ButtonState buttonState = qVariantValue<ButtonState>(index.data());
        return buttonState.sizeHint();
    } else {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}
//! [1]

//! [2]
QWidget *ButtonStateItemDelegate::createEditor(QWidget *parent,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const

{
    if (qVariantCanConvert<ButtonState>(index.data())) {
        ButtonStateWidget *editor = new ButtonStateWidget(parent);
        editor->setButtonState(qVariantValue<ButtonState>(index.data()));
        editor->setPos(index.row(), index.column());
        connect(editor, SIGNAL(editingFinished()),
                this, SLOT(commitAndCloseEditor()));
        return editor;
    } else {
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
}
//! [2]

//! [3]
void ButtonStateItemDelegate::setEditorData(QWidget *editor,
                                 const QModelIndex &index) const
{
    if (qVariantCanConvert<ButtonState>(index.data())) {
        ButtonState buttonState = qVariantValue<ButtonState>(index.data());
        ButtonStateWidget *buttonStateWidget = qobject_cast<ButtonStateWidget *>(editor);
        buttonStateWidget->setButtonState(buttonState);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}
//! [3]

//! [4]
void ButtonStateItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                const QModelIndex &index) const
{
    if (qVariantCanConvert<ButtonState>(index.data())) {
        ButtonStateWidget *buttonStateWidget = qobject_cast<ButtonStateWidget *>(editor);
        model->setData(index, qVariantFromValue(buttonStateWidget->buttonState()));
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}
//! [4]

//! [5]
void ButtonStateItemDelegate::commitAndCloseEditor()
{
    ButtonStateWidget *editor = qobject_cast<ButtonStateWidget *>(sender());
    editor->restore();
    emit commitData(editor);
    emit fieldClicked(editor->Row(), editor->Column());
    emit closeEditor(editor);
}
