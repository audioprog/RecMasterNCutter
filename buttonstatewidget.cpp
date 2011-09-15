#include <QtGui>
#include "buttonstatewidget.h"

ButtonStateWidget::ButtonStateWidget(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setAutoFillBackground(true);
}

QSize ButtonStateWidget::sizeHint() const
{
    return myButtonState.sizeHint();
}

void ButtonStateWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    myButtonState.paint(&painter, rect(), this->palette(),
                       ButtonState::Editable);
}

void ButtonStateWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (QIcon::Disabled != myButtonState.IconMode()) {
        //myButtonState.setIconMode(QIcon::Disabled);
        update();
    }
}

void ButtonStateWidget::mouseReleaseEvent(QMouseEvent * /* event */)
{
    myButtonState.setIconMode(org);
    emit editingFinished();
}
