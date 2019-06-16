#include "qmovablewidget.h"

QMovableWidget::QMovableWidget(QWidget *parent) : QWidget(parent)
{
    mLeftMousePressed = false;
}

QMovableWidget::~QMovableWidget()
{

}

void QMovableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mLeftMousePressed = true;
        mLastPos = event->globalPos();
    }
}

void QMovableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mLeftMousePressed = false;
        mLastPos = event->globalPos();
    }
}

void QMovableWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mLeftMousePressed)
    {
        QPoint lastpos = pos();
        lastpos.setX( lastpos.x() + event->globalX() - mLastPos.x());
        lastpos.setY( lastpos.y() + event->globalY() - mLastPos.y());
        move(lastpos);
        mLastPos = event->globalPos();
    }
}
