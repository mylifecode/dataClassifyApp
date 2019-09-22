#include "RbProgressBar.h"
#include <QResizeEvent>
#include <Qpainter>

RbProgressBar::RbProgressBar(const QString& strBackBarPictureName,
                             const QString& strFrontBarPictureName,
                             int progressNum /*= 10*/,
							 CALLBACKFUNCTION pCallBack,
                             QWidget *parent) :
    QWidget(parent),
    m_numCurProgress(0),
    m_numTotalProgress(progressNum),
    m_pPixmapBackBar(NULL),
    m_pPixmapFrontBar(NULL),
    m_pixmapWidth(0),
    m_pixmapHeight(0),
	m_pCallBack(pCallBack),
    m_vecBDrawBackBar(progressNum)
{
    m_pPixmapBackBar = new QPixmap(strBackBarPictureName);
    m_pPixmapFrontBar = new QPixmap(strFrontBarPictureName);

    for(int i = 0;i<m_vecBDrawBackBar.size(); ++i)
        m_vecBDrawBackBar[i] = true;
}


bool RbProgressBar::event(QEvent * pEvent)
{
    switch(pEvent->type())
	{
	case QEvent::Resize:
		{
			QResizeEvent * pResizeEvent = static_cast<QResizeEvent*>(pEvent);
			QSize sizeWidget = pResizeEvent->size();
			m_pixmapWidth = sizeWidget.width() / m_numTotalProgress;
			m_pixmapHeight = sizeWidget.height();
		}
		break;
	case QEvent::MouseButtonPress:		//test
		//addProgress();
		break;
	}
    return __super::event(pEvent);
}

void RbProgressBar::paintEvent(QPaintEvent * pEvent)
{
    QPainter painter(this);
    int barSpacing = 5;
    int x = 0;
    int y = 0;
    for(int i = 0;i<m_vecBDrawBackBar.size();++i)
    {
        if(m_vecBDrawBackBar[i])
            painter.drawPixmap(x,y,m_pixmapWidth - barSpacing,m_pixmapHeight,*m_pPixmapBackBar);

        else
            painter.drawPixmap(x,y,m_pixmapWidth - barSpacing,m_pixmapHeight,*m_pPixmapFrontBar);
        x += m_pixmapWidth;
    }
}

bool RbProgressBar::addProgress(unsigned int numStep /* = 1 */)
{
	if(m_numCurProgress == m_numTotalProgress)
		return false;
	for(int i = 0;i<numStep;++i)
	{
		m_vecBDrawBackBar[m_numCurProgress++] = false;
		if(m_numCurProgress == m_numTotalProgress)
		{
			if(m_pCallBack)
				m_pCallBack();
			break;
		}
	}
	update();
	return true;
}