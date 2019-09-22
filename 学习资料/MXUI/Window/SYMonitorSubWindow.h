#ifndef RBMONITORSUBWINDOW_H
#define RBMONITORSUBWINDOW_H
#include <QWidget>
#include "ui_SYMonitorSubWindow.h"
#include "ui_BigRenderWindow.h"
#include "RbShieldLayer.h"

#include <QWidget>
#include <QByteArray>
#include <QPixmap>
#include <QMutex>
#include <QPainter>
#include <QPixmap>
#include <MxVideoFrame.h>
#include <cstdint>
#include "MxDefine.h"

class BigRenderWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	BigRenderWindow(QWidget * parent)
		:RbShieldLayer(parent)
	{
		ui.setupUi(this);
		ui.renderWindow->installEventFilter(this);
		installEventFilter(this);

		connect(ui.closeBtn, SIGNAL(clicked()), this, SLOT(onCloseButton()));
		Mx::setWidgetStyle(this,"qss:BigRenderWindow.qss");
	}

	void updateWindow(const QByteArray &pixmapData)
	{
		m_mutexPixmap.lock();
		m_pixmap.loadFromData((const uchar *)pixmapData.data(),pixmapData.size(),"jpg");
		m_mutexPixmap.unlock();
		ui.renderWindow->update();
	}

	void updateWindow(const MxVideoFrame& videoFrame)
	{
		m_mutexPixmap.lock();
		m_pixmap = QPixmap::fromImage(QImage((const uchar*)videoFrame.Data(), videoFrame.GetWidth(), videoFrame.GetHeight(), QImage::Format_RGB888));
		m_mutexPixmap.unlock();
		ui.renderWindow->update();
	}
	
	void setName(const QString& strName)
	{
		ui.labelName->setText(strName);
	}

	void clearBackgroud()
	{
		m_mutexPixmap.lock();
		m_pixmap = QPixmap();
		m_mutexPixmap.unlock();
	}

protected:
	bool eventFilter(QObject *pObject,QEvent * pEvent)
	{
		if(pObject == ui.renderWindow && pEvent->type() == QEvent::Paint)
		{
			QPainter painter(ui.renderWindow);
			//painter.setRenderHint(QPainter::Antialiasing);
			painter.setRenderHint(QPainter::SmoothPixmapTransform);

			painter.fillRect(ui.renderWindow->rect(), QColor(255, 255, 255));
			m_mutexPixmap.lock();
			painter.drawPixmap(0,
				0,
				ui.renderWindow->width(),
				ui.renderWindow->height(),
				m_pixmap,
				0,
				0,
				m_pixmap.width(),
				m_pixmap.height());
			m_mutexPixmap.unlock();
			return true;
		}
		else
			if(pEvent->type() == QEvent::KeyPress)
			{
				QKeyEvent * pKeyEvent = static_cast<QKeyEvent*>(pEvent);
				if(pKeyEvent->key() == Qt::Key_Escape)
					return true;
			}
		return __super::eventFilter(pObject,pEvent);
	}
	
	void onCloseButton()
	{
		emit clickedCloseButton();
		__super::onCloseButton();
	}

signals:
	void clickedCloseButton();
private:
	Ui::BigRenderWindow ui;
	QPixmap m_pixmap;
	QMutex m_mutexPixmap;
};



class SYMonitorSubWindow : public QWidget
{
Q_OBJECT

	
public:
	explicit SYMonitorSubWindow(QWidget *parent = 0);
	~SYMonitorSubWindow();

	void updateWindow(const QByteArray& pixmapData);
	void updateWindow(const MxVideoFrame& videoFrame);
	void hideControl();
	void showControl();
	
	void setName(const QString& strName)
	{
		ui.labelName->setText(strName);
		if(m_pBigRenderWindow)
			m_pBigRenderWindow->setName(strName);
	}

	void SetExtraData(uint32_t data);

	uint32_t GetExtraData();

signals:
	void exitWatch();
	void hideControlInternal();
	void updateContentInternal();

private slots:
	void onExitWatch();
	void onCloseBigRenderWindow();
	void onHideControl();
	void onUpdateContent();

protected:
	bool eventFilter(QObject *, QEvent *);
	void mouseDoubleClickEvent(QMouseEvent *);
private:
	Ui::MonitorSubWindow ui;
	QPixmap m_pixmap;
	BigRenderWindow * m_pBigRenderWindow;
	bool m_bDrawPixmapInBigRenderWindow;
	uint32_t m_extraData;
	QMutex m_propertyMutex;
};

#endif // RBMONITORSUBWINDOW_H


