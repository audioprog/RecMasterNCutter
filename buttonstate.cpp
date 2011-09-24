#include "buttonstate.h"
#include <QtGui>
//#include <QtDebug>

ButtonState::ButtonState():
    icomode(QIcon::Normal), oldmode(QIcon::Normal)
{
    ButtonState("Save");
}

ButtonState::ButtonState(QString iconName):
    icomode(QIcon::Normal), oldmode(QIcon::Normal)
{
    QString normal = ":/buttonstateicons/" + iconName, active = ":/buttonstateicons/" + iconName + "Gear", disabled = ":/buttonstateicons/" + iconName + "Shaddow", selected = ":/buttonstateicons/" + iconName + "Tick";
    QSize siz(24, 24);
    ico.addFile(normal, siz, QIcon::Normal);
    ico.addFile(active, siz, QIcon::Active);
    ico.addFile(disabled, siz, QIcon::Disabled);
    ico.addFile(selected, siz, QIcon::Selected);
}

ButtonState::ButtonState(QString iconName, QString icondisabled):
    icomode(QIcon::Normal), oldmode(QIcon::Normal)
{
    QString normal = ":/buttonstateicons/" + iconName, active = ":/buttonstateicons/" + iconName, disabled = ":/buttonstateicons/" + icondisabled, selected = ":/buttonstateicons/" + iconName;
    QSize siz(24, 24);
    ico.addFile(normal, siz, QIcon::Normal);
    ico.addFile(active, siz, QIcon::Active);
    ico.addFile(disabled, siz, QIcon::Disabled);
    ico.addFile(selected, siz, QIcon::Selected);
}

ButtonState::ButtonState(QString iconnormal, QString iconactive, QString icondisabled, QString iconselected):
    icomode(QIcon::Normal), oldmode(QIcon::Normal)
{
    QString normal = ":/buttonstateicons/" + iconnormal, active = ":/buttonstateicons/" + iconactive, disabled = ":/buttonstateicons/" + icondisabled, selected = ":/buttonstateicons/" + iconselected;
    QSize siz(24, 24);
    ico.addFile(normal, siz, QIcon::Normal);
    ico.addFile(active, siz, QIcon::Active);
    ico.addFile(disabled, siz, QIcon::Disabled);
    ico.addFile(selected, siz, QIcon::Selected);
}

void ButtonState::paint(QPainter *painter, const QRect &rect,
                       const QPalette &palette, EditMode mode) const
{
    painter->save();

    painter->setRenderHint(QPainter::Antialiasing, true);
    ico.paint(painter, rect, Qt::AlignCenter, icomode);
    //qDebug() << icomode;

    painter->restore();
}
