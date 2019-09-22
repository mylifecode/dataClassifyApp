#include "SYMonitorSubWindow.h"
#include <QPixmap>
#include <QPainter>
#include <windows.h>
#include <QDebug>
#include <QByteArray>
#include <QPaintEvent>

SYMonitorSubWindow::SYMonitorSubWindow(QWidget * parent)
	:QWidget(parent),
	m_pBigRenderWindow(NULL),
	m_bDrawPixmapInBigRenderWindow(false),
	m_extraData(0)
{
	ui.setupUi(this);
	ui.videoWindow->installEventFilter(this);
	connect(ui.quitBtn,SIGNAL(clicked()),this,SLOT(onExitWatch()));
	
	m_pBigRenderWindow = new BigRenderWindow(this);
	m_pBigRenderWindow->hideOkButton();
	m_pBigRenderWindow->hideCloseButton();
	m_pBigRenderWindow->hide();
	connect(m_pBigRenderWindow,SIGNAL(clickedCloseButton()),this,SLOT(onCloseBigRenderWindow()));
	connect(this, SIGNAL(hideControlInternal()), this, SLOT(onHideControl()));
	connect(this, SIGNAL(updateContentInternal()), this, SLOT(onUpdateContent()));

	onHideControl();

	Mx::setWidgetStyle(this,"qss:SYMonitorSubWindow.qss");

}

SYMonitorSubWindow::~SYMonitorSubWindow(void)
{
	if(m_pBigRenderWindow)
	{
		delete m_pBigRenderWindow;
		m_pBigRenderWindow = NULL;
	}
}

void SYMonitorSubWindow::updateWindow(const QByteArray &pixmapData)
{
	m_propertyMutex.lock();
	if(m_bDrawPixmapInBigRenderWindow)
	{
		m_pBigRenderWindow->updateWindow(pixmapData);  //显示视频数据
	}
	else
	{
		m_pixmap.loadFromData((const uchar *)pixmapData.data(),pixmapData.size(),"jpg");
		ui.videoWindow->update();
	}
	m_propertyMutex.unlock();
}

void SYMonitorSubWindow::updateWindow(const MxVideoFrame& videoFrame)
{
	m_propertyMutex.lock();
	if(m_bDrawPixmapInBigRenderWindow)
	{
		m_pBigRenderWindow->updateWindow(videoFrame);
	}
	else
	{
		m_pixmap = QPixmap::fromImage(QImage((const uchar*)videoFrame.Data(), videoFrame.GetWidth(), videoFrame.GetHeight(), QImage::Format_RGB888));
		emit updateContentInternal();
	}
	m_propertyMutex.unlock();
}

bool SYMonitorSubWindow::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == ui.videoWindow && event->type() == QEvent::Paint)
	{
		QPainter painter(ui.videoWindow);
		m_propertyMutex.lock();
		painter.drawPixmap(0,
			0,
			ui.videoWindow->width(),
			ui.videoWindow->height(),
			m_pixmap,
			0,
			0,
			m_pixmap.width(),
			m_pixmap.height());
		m_propertyMutex.unlock();
		return true;
	}

	return __super::eventFilter(obj,event);
}

void SYMonitorSubWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
	QPoint origin = ui.videoWindow->mapToGlobal(QPoint(0, 0));
	QRect region(origin.x(), origin.y(),
				 ui.videoWindow->width(), ui.videoWindow->height());
	
	if(region.contains(event->screenPos().toPoint())&&m_extraData) //判断鼠标位置
	{
		m_propertyMutex.lock();
		m_pBigRenderWindow->showFullScreen();
		m_bDrawPixmapInBigRenderWindow = true;
		m_propertyMutex.unlock();
	}
}

void SYMonitorSubWindow::onCloseBigRenderWindow()
{
	m_propertyMutex.lock();
	m_bDrawPixmapInBigRenderWindow = false;
	m_propertyMutex.unlock();
}

void SYMonitorSubWindow::onExitWatch()
{
	hideControl();
	emit exitWatch();
}

void SYMonitorSubWindow::hideControl()
{
	emit hideControlInternal();
}

void SYMonitorSubWindow::onHideControl()
{
	m_propertyMutex.lock();
	m_pixmap = QPixmap();
	m_propertyMutex.unlock();
	ui.videoWindow->update();

	ui.labelName->hide();
	ui.quitBtn->hide();

	if(m_pBigRenderWindow)
		m_pBigRenderWindow->clearBackgroud();
}

void SYMonitorSubWindow::onUpdateContent()
{
	ui.videoWindow->repaint();
}

void SYMonitorSubWindow::showControl()
{
	ui.labelName->show();
	ui.quitBtn->show();
}

void SYMonitorSubWindow::SetExtraData(uint32_t data)
{
	m_propertyMutex.lock();
	m_extraData = data;
	m_propertyMutex.unlock();
}

uint32_t SYMonitorSubWindow::GetExtraData()
{
	uint32_t data;

	m_propertyMutex.lock();
	data = m_extraData;
	m_propertyMutex.unlock();

	return data;
}