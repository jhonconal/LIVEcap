#include "mainwindow.h"
#include "ui_mainwindow.h"

#define ShowWindowsHint 0

MainWindow::MainWindow(QWidget *parent) :
    QMovableWidget(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mGifMaker = new GifCreatorThread(this);
    InitWnd();
    GetSystemScreenInfo();
#if ShowWindowsHint
    setWindowFlags(Qt::WindowStaysOnTopHint);
#else
    setWindowFlags(Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitWnd()
{
    mStartRecoard = false;
    mStartResize = false;
    if(mStartRecoard)
        ui->stopButton->setEnabled(true);
    else
        ui->stopButton->setEnabled(false);
    setMouseTracking(true);
    ui->stopButton->setFocus();
    mTimer.setSingleShot(false);
    ui->operationWidget->adjustSize();
    setMinimumSize(480,320);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(on_tryFrame()));
    connect(mGifMaker, &GifCreatorThread::finished, this, [this](){
        QMessageBox::information(this, QStringLiteral("Info"), QStringLiteral("GIF动画捕捉完成.       "));//
    });
}

void MainWindow::GetSystemScreenInfo()
{
    QDesktopWidget* desktopWidget = QApplication::desktop();
    //获取可用桌面大小
    QRect deskRect = desktopWidget->availableGeometry();
    //获取设备屏幕大小
    QRect screenRect = desktopWidget->screenGeometry();
//    ui->widthSpinBox->setMaximum(screenRect.width()-6);
//    ui->heightSpinBox->setMaximum(screenRect.height()-23-40);
//    qDebug()<<"width_max(): "<<screenRect.width()-6<<"height_max(): "<<screenRect.height()-23-40;
}

#ifdef Q_OS_WIN32
#include <windows.h>
static QImage getScreenImage(int x, int y, int width, int height)
{
    HDC hScrDC = ::GetDC(nullptr);
    HDC hMemDC = nullptr;

    BYTE *lpBitmapBits = nullptr;

    int nWidth = width;
    int nHeight = height;

    hMemDC = ::CreateCompatibleDC(hScrDC);

    BITMAPINFO bi;
    ZeroMemory(&bi, sizeof(BITMAPINFO));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = nWidth;
    bi.bmiHeader.biHeight = nHeight;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 24;

    CURSORINFO hCur ;
    ZeroMemory(&hCur,sizeof(hCur));
    hCur.cbSize = sizeof(CURSORINFO);
    GetCursorInfo(&hCur);

    ICONINFO IconInfo = {};
    if(GetIconInfo(hCur.hCursor, &IconInfo))
    {
        hCur.ptScreenPos.x -= IconInfo.xHotspot;
        hCur.ptScreenPos.y -= IconInfo.yHotspot;
        if(nullptr != IconInfo.hbmMask)
            DeleteObject(IconInfo.hbmMask);
        if(nullptr != IconInfo.hbmColor)
            DeleteObject(IconInfo.hbmColor);
    }

    HBITMAP bitmap = ::CreateDIBSection(hMemDC, &bi, DIB_RGB_COLORS, (LPVOID*)&lpBitmapBits, nullptr, 0);
    HGDIOBJ oldbmp = ::SelectObject(hMemDC, bitmap);

    ::BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, x, y, SRCCOPY);
    DrawIconEx(hMemDC, hCur.ptScreenPos.x - x, hCur.ptScreenPos.y - y, hCur.hCursor, 0, 0, 0, nullptr, DI_NORMAL | DI_COMPAT);

    BITMAPFILEHEADER bh;
    ZeroMemory(&bh, sizeof(BITMAPFILEHEADER));
    bh.bfType = 0x4d42;  //bitmap
    bh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bh.bfSize = bh.bfOffBits + ((nWidth*nHeight)*3);

    int size = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 3 * nWidth * nHeight;
    uchar *bmp = new uchar[size];
    uint offset = 0;
    memcpy(bmp, (char *)&bh, sizeof(BITMAPFILEHEADER));
    offset = sizeof(BITMAPFILEHEADER);
    memcpy(bmp + offset, (char *)&(bi.bmiHeader), sizeof(BITMAPINFOHEADER));
    offset += sizeof(BITMAPINFOHEADER);
    memcpy(bmp + offset, (char *)lpBitmapBits, 3 * nWidth * nHeight);

    ::SelectObject(hMemDC, oldbmp);
    ::DeleteObject(bitmap);
    ::DeleteObject(hMemDC);
    ::ReleaseDC(nullptr, hScrDC);
    QImage image = QImage::fromData(bmp, size);
    delete[] bmp;
    return image;
}
#endif

void MainWindow::on_tryFrame()
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    auto screen = qApp->screenAt(pos());
#else
    QScreen *screen = nullptr;
    foreach (screen, qApp->screens())
    {
        if (screen->geometry().contains(pos()))
        {
            break;
        }
    }
#endif
    if (screen != nullptr)
    {
#ifdef Q_OS_WIN32
        QImage img = getScreenImage(x() + mRecordRect.x(),
                                    y() + mRecordRect.y(),
                                    mRecordRect.width(),
                                    mRecordRect.height());
#else
        QImage img = screen->grabWindow(0,
                                        x() + mRecordRect.x(),
                                        y() + mRecordRect.y(),
                                        mRecordRect.width(),
                                        mRecordRect.height()).toImage();
#endif
        mGifMaker->frame(img, ui->frameSpinBox->value());
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(65,65,65));
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QRect rect = this->rect();
    rect.adjust(3, 3, -3, -(ui->operationWidget->height())-3);
    mRecordRect = rect;
    qDebug()<<"width(): "<<mRecordRect.width()<<"height(): "<<mRecordRect.height();

    ui->widthSpinBox->setValue(mRecordRect.width());
    ui->heightSpinBox->setValue(mRecordRect.height());
#if ShowWindowsHint
    QRegion region(0,-23,width(),height() +23);
#else
    QRegion region(this->rect());
#endif
    setMask(region.xored(mRecordRect));

    ui->operationWidget->move(width() - ui->operationWidget->width() , height() - ui->operationWidget->height() - 3);
    return QWidget::resizeEvent(event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    qApp->quit();
    QMovableWidget::mouseDoubleClickEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QRect rect(QPoint(width() - 3, height() - 3), QSize(3, 3));
    if (rect.contains(event->pos()) && !mTimer.isActive())
    {
        mStartResize = true;
        mStartGeometry = QRect(event->globalPos(), size());
    }
    else {
        QMovableWidget::mousePressEvent(event);
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    mStartResize = false;
    setCursor(Qt::ArrowCursor);
    QMovableWidget::mouseReleaseEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QRect rect(QPoint(width() - 3, height() - 3), QSize(3, 3));

    if (mStartResize)
    {
        QPoint ch = event->globalPos() - mStartGeometry.topLeft();
        resize(mStartGeometry.size() + QSize(ch.x(), ch.y()));
    }
    else if (rect.contains(event->pos()) && !mTimer.isActive())
    {
        setCursor(Qt::SizeFDiagCursor);
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        QMovableWidget::mouseMoveEvent(event);
    }
}

void MainWindow::on_startButton_clicked()
{
    if (!mTimer.isActive())
    {
        if (mGifMaker->isRunning())
        {
            qWarning()<<"GifMaker Thread Running.";
            QMessageBox::about(this, "Info", "GifMaker Thread Running.");
            return;
        }
        mGifFileName.clear();
        mGifFileName = QFileDialog::getSaveFileName(this, "", QString("snapshoot_%1.gif").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")));
        if(!mGifFileName.isEmpty())
        {
            qDebug()<<"Get Gif FileName: "<<mGifFileName;
            mGifMaker->startThread();

            mGifMaker->begin(mGifFileName.toUtf8().data(), ui->widthSpinBox->value(), ui->heightSpinBox->value(), 1);
            ui->startButton->setEnabled(false);
            ui->stopButton->setEnabled(true);

            on_tryFrame();
            mTimer.start(static_cast<int>(1000.0 / ui->frameSpinBox->value()));
            ui->widthSpinBox->setDisabled(true);
            ui->heightSpinBox->setDisabled(true);
            ui->frameSpinBox->setDisabled(true);
        }
    }
    else
    {
        mGifFileName.clear();
        mGifFileName = QFileDialog::getSaveFileName(this, "", QString("snapshoot_%1.gif").arg(QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss")));
        if(!mGifFileName.isEmpty())
        {
            qDebug()<<"Get Gif FileName: "<<mGifFileName;
            mGifMaker->startThread();

            mGifMaker->begin(mGifFileName.toUtf8().data(), ui->widthSpinBox->value(), ui->heightSpinBox->value(), 1);
            ui->startButton->setEnabled(false);
            ui->stopButton->setEnabled(true);

            on_tryFrame();
            mTimer.start(static_cast<int>(1000.0 / ui->frameSpinBox->value()));
            ui->widthSpinBox->setDisabled(true);
            ui->heightSpinBox->setDisabled(true);
            ui->frameSpinBox->setDisabled(true);
        }
    }
}

void MainWindow::on_stopButton_clicked()
{
    mTimer.stop();
    ui->stopButton->setEnabled(false);
    ui->startButton->setEnabled(true);
    mGifMaker->end();

    ui->widthSpinBox->setEnabled(true);
    ui->heightSpinBox->setEnabled(true);
    ui->frameSpinBox->setEnabled(true);
}

void MainWindow::on_widthSpinBox_valueChanged(int arg1)
{
    QRect rect(pos(), QSize(arg1, ui->heightSpinBox->value()));
    rect.adjust(-3, -3, 3, ui->operationWidget->height() + 3);
    resize(rect.size());
    qDebug()<<Q_FUNC_INFO<<"width: "<<arg1<<"height(): "<<ui->heightSpinBox->value();
}

void MainWindow::on_heightSpinBox_valueChanged(int arg1)
{
    QRect rect(pos(), QSize(ui->widthSpinBox->value(),arg1));
    rect.adjust(-3, -3, 3, ui->operationWidget->height() + 3);
    resize(rect.size());
    qDebug()<<Q_FUNC_INFO<<"width(): "<<ui->widthSpinBox->value()<<"height: "<<arg1;
}

void MainWindow::on_frameSpinBox_valueChanged(int arg1)
{
    qDebug()<<Q_FUNC_INFO<<"frame: "<<arg1;
}
