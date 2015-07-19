#ifndef BUTTONSTATEWIDGET_H
#define BUTTONSTATEWIDGET_H

#include <QWidget>
#include "buttonstate.h"

class ButtonStateWidget : public QWidget
{
    Q_OBJECT

public:
    ButtonStateWidget(QWidget *parent = 0);
    ~ButtonStateWidget() { myButtonState.setIconMode(org); }

    QSize sizeHint() const;
    void setButtonState(const ButtonState &buttonState) {
        myButtonState = buttonState;
        org = myButtonState.IconMode();
    }
    ButtonState buttonState() const { return myButtonState; }
    void restore() { myButtonState.setIconMode(org); }
    void setPos(const int newRow, const int newColumn) { _row = newRow; _col = newColumn; }
    int Row() const { return _row; }
    int Column() const { return _col; }

signals:
    void editingFinished();

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    ButtonState myButtonState;
    QIcon::Mode org;
    int _row, _col;
};

#endif // BUTTONSTATEWIDGET_H
