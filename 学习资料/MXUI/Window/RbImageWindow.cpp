#include "RbImageWindow.h"
#include <QImage>
#include <QPainter>
#include <QScrollBar>
#include <QResizeEvent>
#include <QScrollArea>
#include <QLabel>
#include <QPixmap>

RbImageWindow::RbImageWindow(QWidget * parent)
:QFrame(parent),
m_pHScrollBar(NULL),
m_pVScrollBar(NULL),
m_offsetX(0),
m_offsetY(0),
m_contentWidth(0),
m_contentHeight(0)
{
	resize(1270,605);
	m_pHScrollBar = new QScrollBar(Qt::Horizontal,this);
	m_pHScrollBar->setMinimum(0);
	m_pHScrollBar->setMaximum(0);
	m_pHScrollBar->hide();

	m_pVScrollBar = new QScrollBar(Qt::Vertical,this);
	m_pVScrollBar->setMinimum(0);
	m_pVScrollBar->setMaximum(0);
	m_pVScrollBar->hide();

	connect(m_pHScrollBar,SIGNAL(valueChanged(int)),this,SLOT(onHSliderValueChanged(int)));
	connect(m_pVScrollBar,SIGNAL(valueChanged(int)),this,SLOT(onVSliderValueChanged(int)));
}

RbImageWindow::~RbImageWindow(void)
{
	for(int i = 0;i<m_vecImage.size();++i)
		delete m_vecImage[i];
	m_vecImage.clear();
}

bool RbImageWindow::event(QEvent* pEvent)
{
	switch(pEvent->type())
	{
	case QEvent::Resize:
		{
			QResizeEvent * pResizeEvent = dynamic_cast<QResizeEvent*>(pEvent);
			if(pResizeEvent)
			{
				QSize sizeParent = pResizeEvent->size();
				m_pHScrollBar->resize(sizeParent.width(),20);
				m_pHScrollBar->move(0,sizeParent.height() - m_pHScrollBar->height() - 20);
				m_pHScrollBar->setMaximum(m_contentWidth - sizeParent.width());

				m_pVScrollBar->resize(20,sizeParent.height());
				m_pVScrollBar->move(sizeParent.width()- m_pVScrollBar->width(),0);
				m_pVScrollBar->setMaximum(m_contentHeight - sizeParent.height());
			}
			
		}
		break;
	}
	return __super::event(pEvent);
}



void RbImageWindow::scrollaction(int steps)
{
	int barpos = m_pVScrollBar->value();
	barpos += steps;
	m_pVScrollBar->setValue(barpos);
}



bool RbImageWindow::addImage(const QString& fileName)
{
	//QImage * pImage = new QImage(fileName);
	QPixmap * pImage = new QPixmap(fileName);
	if(pImage->isNull())
		return false;
	m_vecImage.push_back(pImage);
 	if(m_contentWidth < pImage->width())
 		m_contentWidth = pImage->width();

	m_pHScrollBar->setMaximum(m_contentWidth - this->width());

	m_contentHeight += pImage->height();
	m_pVScrollBar->setMaximum(m_contentHeight - this->height());
	if(m_contentHeight > this->height())
		m_pVScrollBar->show();
	update();
	return true;
}

void RbImageWindow::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	int width = 0;
	int height = 0;
	for(int i =0;i<m_vecImage.size();++i)
	{
		//painter.drawImage(m_offsetX,m_offsetY + height,*m_vecImage[i]);
		//painter.drawPixmap(m_offsetX,m_offsetY + height,this->width(),m_vecImage[i]->height(),*m_vecImage[i]);
		painter.drawPixmap(m_offsetX - 80,m_offsetY + height,*m_vecImage[i]);
		height += m_vecImage[i]->height();
	}
}


void RbImageWindow::onHSliderValueChanged(int value)
{
	m_offsetX = -value;
	update();
}

void RbImageWindow::onVSliderValueChanged(int value)
{
	m_offsetY = -value;
	update();
}
