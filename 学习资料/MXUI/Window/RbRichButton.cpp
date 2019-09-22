#include "RbRichButton.h"
#include "MxDefine.h"

RbRichButton::RbRichButton(QWidget * parent /*= NULL*/,
						   QString strCenterBtnName /*= "CenterBtn"*/,
						   QString strId /*= ""*/,
						   bool bShowRightBottomBtn /*= true*/,
						   bool bShowRightTopLabel /*= true*/,
						   bool bShowBottomLabel /*= true*/)
:QWidget(parent),
m_clickType(CLICK_NONE),
m_strId(strId),
m_pBottomLabel(NULL),
m_pRightTopLabel(NULL),
m_pCenterBtn(NULL),
m_pRightBottomBtn(NULL),
m_rightBottomBtnPos(CP_RightBottom)
{
	setObjectName("RichButton");
	m_pCenterBtn = new QPushButton(this);
	m_pCenterBtn->setObjectName(strCenterBtnName);
	m_pCenterBtn->setCheckable(true);
	m_pCenterBtn->show();

	m_pRightBottomBtn =  new QPushButton(m_pCenterBtn);
	m_pRightBottomBtn->setObjectName("RightBottomBtn");
	m_pRightBottomBtn->setCheckable(true);
	if(bShowRightBottomBtn)
		m_pRightBottomBtn->show();
	else
		m_pRightBottomBtn->hide();
	
	m_pRightTopLabel = new QLabel(this);
	m_pRightTopLabel->setAlignment(Qt::AlignCenter);
	m_pRightTopLabel->setObjectName("RightTopLabel");
	if(bShowRightTopLabel)
		m_pRightTopLabel->show();
	else
		m_pRightTopLabel->hide();

	m_pBottomLabel = new QLabel(this);
	m_pBottomLabel->setObjectName("BottomLabel");
	m_pBottomLabel->setAlignment(Qt::AlignHCenter);
	if(bShowBottomLabel)
		m_pBottomLabel->show();
	else
		m_pBottomLabel->hide();

	/*m_pCenterBtn->setText("C");
	m_pRightBottomBtn->setText("R");*/
	m_pRightTopLabel->setText("t");
	m_pBottomLabel->setText("bottom");

	//m_pRightBottomBtn->installEventFilter(this);

	connect(m_pRightBottomBtn,SIGNAL(clicked()),this,SLOT(onClickedRightBottmBtn()));
	connect(m_pCenterBtn,SIGNAL(clicked()),this,SLOT(onClickedCenterBtn()));
	connect(m_pCenterBtn, SIGNAL(pressed()),this,SLOT(onPressCenterBtn()));

	Mx::setWidgetStyle(this,"qss:RbRichButton.qss");
}

RbRichButton::~RbRichButton(void)
{

}

void RbRichButton::showRightBottomBtn()
{
	m_pRightBottomBtn->show();
}

void RbRichButton::hideRightBottomBtn()
{
	m_pRightBottomBtn->hide();
}

void RbRichButton::setBottomLabelText(QString strText)
{
	//if(m_pBottomLabel) m_pBottomLabel->setText(strText);
	if(m_pBottomLabel) 
	{
		int n = strText.size();
		for(int i = 0 ;i<n;++i)
		{
			if(strText[i] == '\\')
				strText[i] = '\n';
		}
		m_pBottomLabel->setText(strText);
	}
}

bool RbRichButton::event(QEvent * e)
{
	switch(e->type())
	{
	case QEvent::Resize:
		QResizeEvent * resizeEvent = static_cast<QResizeEvent*>(e);
		QSize sizeWidget = resizeEvent->size();
		QSize sizeCenterBtn = m_pCenterBtn->size();
		QSize sizeRightTopLabel = m_pRightTopLabel->size();
		QSize sizeBottomLabel = m_pBottomLabel->size();
		int deltaWidth = sizeWidget.width() - sizeCenterBtn.width();

		m_pCenterBtn->move(/*deltaWidth / 2*/0,sizeRightTopLabel.height() / 2);
		//m_pRightTopLabel->move(deltaWidth / 2 + sizeCenterBtn.width(),0);
		
		QSize sizeRightBottomBtn = m_pRightBottomBtn->size();
		if(m_rightBottomBtnPos == CP_Center)
			m_pRightBottomBtn->move(sizeCenterBtn.width()/2 - sizeRightBottomBtn.width()/2,
									sizeCenterBtn.height()/2 - sizeRightBottomBtn.height()/2);
		else if(m_rightBottomBtnPos == CP_RightTop)
			m_pRightBottomBtn->move(sizeCenterBtn.width() - sizeRightBottomBtn.width(),
									0);
		else
			m_pRightBottomBtn->move(sizeCenterBtn.width() - sizeRightBottomBtn.width(),
			sizeCenterBtn.height() - sizeRightBottomBtn.height());

		QPoint ptCenterBtn = m_pCenterBtn->pos();
		m_pBottomLabel->move(0,ptCenterBtn.y() + sizeCenterBtn.height());
		//---
		if(sizeBottomLabel.width() > sizeCenterBtn.width())
		{
			m_pCenterBtn->move((sizeBottomLabel.width() - sizeCenterBtn.width()) / 2,ptCenterBtn.y());
		}
		ptCenterBtn = m_pCenterBtn->pos();
		m_pRightTopLabel->move(ptCenterBtn.x() + sizeCenterBtn.width() - sizeRightTopLabel.width() *2/3,
			ptCenterBtn.y() - sizeRightTopLabel.height() /3);
	}
	return __super::event(e);
}

bool RbRichButton::eventFilter(QObject * watched,QEvent * e)
{
	if(watched == m_pRightBottomBtn )
	{
		switch(e->type())
		{
		case QEvent::Enter:
			return true;
		case QEvent::MouseButtonRelease:
			onClickedRightBottmBtn();
			break;
		}
	}
	return __super::eventFilter(watched,e);
}

void RbRichButton::onClickedRightBottmBtn()
{
	m_clickType = CLICK_CHILD;
	emit clickedBtn();
}

void RbRichButton::onClickedCenterBtn()
{
	m_clickType = CLICK_PARENT;
	emit clickedBtn();
	emit releaseBtn();
}

RbRichButton::CLICKCONTROLTYPE RbRichButton::getClickType()
{
	CLICKCONTROLTYPE clickType = m_clickType;
	m_clickType = CLICK_NONE;
	return clickType;
}

void RbRichButton::setRightBottomBtnPosition(ControlPosition cp)
{
	m_rightBottomBtnPos = cp;
	//resize(this->size());
}

void RbRichButton::setCenterBtnSize(int width,int height)
{
	if(m_pCenterBtn)
		m_pCenterBtn->setStyleSheet(QString("min-width: %1px;\
												   min-height: %2px;\
												   max-width: %1px;\
												   max-height: %2px;").arg(width).arg(height));
}

void RbRichButton::setRightBottomBtnSize(int width,int height)
{
	if(m_pRightBottomBtn)
		m_pRightBottomBtn->setStyleSheet(QString("min-width: %1px;\
											min-height: %2px;\
											max-width: %1px;\
											max-height: %2px;").arg(width).arg(height));
}

void RbRichButton::setRightTopLabelSize(int width,int height)
{
	if(m_pRightTopLabel)
		m_pRightTopLabel->setStyleSheet(QString("min-width: %1px;\
												min-height: %2px;\
												max-width: %1px;\
												max-height: %2px;").arg(width).arg(height));
}

void RbRichButton::resize(int width,int height)
{
	setStyleSheet(QString("min-width: %1px;\
						  min-height: %2px;\
						  max-width: %1px;\
						  max-height: %2px;").arg(width).arg(height));
	__super::resize(width,height);
}

void RbRichButton::setCenterBtnStyleSheet(const QString& strStyleSheet)
{
	m_pCenterBtn->setStyleSheet(strStyleSheet);
}

void RbRichButton::onPressCenterBtn()
{
	m_clickType = CLICK_PARENT;
	emit pressBtn();
}