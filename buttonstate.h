#ifndef BUTTONSTATE_H
#define BUTTONSTATE_H

//#include <QObject>
#include <QMetaType>
#include <QPainter>
#include <QIcon>

class ButtonState //: public QObject
{
    //Q_OBJECT

public:
    enum EditMode { Editable, ReadOnly };

    ButtonState();
    ButtonState(QString iconName);
    ButtonState(QString iconName, QString icondisabled);
    ButtonState(QString iconnormal, QString iconactive, QString icondisabled, QString iconselected);

    void paint(QPainter *painter, const QRect &rect,
               const QPalette &palette, EditMode mode) const;

    QIcon::Mode IconMode() const { return icomode; }
    void setIconMode( QIcon::Mode newMode ) { if (newMode == QIcon::Disabled) { oldmode = icomode; } icomode = newMode; }
    void restore() { icomode = oldmode; }

    QSize sizeHint() const { return QSize(16, 16); }

private:
    QIcon ico;

    QIcon::Mode icomode;
    QIcon::Mode oldmode;
};

Q_DECLARE_METATYPE(ButtonState)

#endif // BUTTONSTATE_H
