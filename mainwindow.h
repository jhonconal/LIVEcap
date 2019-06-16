#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QDebug>
#include <QDate>
#include <QFileDialog>
#include <QScreen>
#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QDesktopWidget>

#include "gifcreatorthread.h"
#include "qmovablewidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMovableWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void InitWnd();
    void GetSystemScreenInfo();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:

    void on_tryFrame();

    void on_startButton_clicked();

    void on_stopButton_clicked();

    void on_widthSpinBox_valueChanged(int arg1);

    void on_heightSpinBox_valueChanged(int arg1);

    void on_frameSpinBox_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;

    GifCreatorThread *mGifMaker;

    QRect mRecordRect;
    bool mStartRecoard;
    QTimer mTimer;
    bool      mStartResize;
    QRect  mStartGeometry;
    QString mGifFileName;

};

#endif // MAINWINDOW_H
