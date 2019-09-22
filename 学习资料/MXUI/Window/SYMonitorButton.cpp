#include "SYMonitorButton.h"
#include <QMouseEvent>
#include "MxDefine.h"

SYMonitorButton::SYMonitorButton(QWidget * parent) 
:QWidget(parent),
m_bMoved(false),
m_extraData(0)
{
	ui.setupUi(this);
	installEventFilter(this);
	ui.watchBtn->installEventFilter(this);
	connect(ui.watchBtn,SIGNAL(clicked()),this,SLOT(onClickedWatchButton()));

	Mx::setWidgetStyle(this,"qss:SYMonitorButton.qss");
}

SYMonitorButton::~SYMonitorButton(void)
{

}

bool SYMonitorButton::eventFilter(QObject *pObject, QEvent *pEvent)
{
	if(pObject == ui.watchBtn)
	{
		
	}
	else if(pObject == this)
	{
		QMouseEvent * pMouseEvent;
		switch(pEvent->type())
		{
			case QEvent::MouseButtonPress:
				m_bMoved = false;
				pMouseEvent = static_cast<QMouseEvent*>(pEvent);
				m_ptStart = pMouseEvent->pos();
				break;
			case QEvent::MouseMove:
				m_bMoved = true;
				break;
			case QEvent::MouseButtonRelease:
				pMouseEvent = static_cast<QMouseEvent*>(pEvent);
				m_ptEnd = pMouseEvent->pos();
				int distance = m_ptEnd.x() - m_ptStart.x();
				if(m_bMoved && distance > 20)
				{
					emit stop();
					return true;
				}
		}
	}
	return __super::eventFilter(pObject,pEvent);
}

void SYMonitorButton::onClickedWatchButton()
{
	emit watch();	
}

void SYMonitorButton::SetExtraData(uint32_t data)
{
	m_propertyMutex.lock();
	m_extraData = data;
	m_propertyMutex.unlock();
}

uint32_t SYMonitorButton::GetExtraData()
{
	uint32_t data;

	m_propertyMutex.lock();
	data = m_extraData;
	m_propertyMutex.unlock();

	return data;
}