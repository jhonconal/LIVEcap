#ifndef QMOVABLEWIDGET_H
#define QMOVABLEWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>

class QMovableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QMovableWidget(QWidget *parent = nullptr);
    ~QMovableWidget();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    bool            mLeftMousePressed;
    QPoint        mLastPos;
};

#endif // QMOVABLEWIDGET_H
