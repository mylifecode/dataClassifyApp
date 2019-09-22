#include "RbMonitorNextPageButton.h"
#include "MxDefine.h"

RbMonitorNextPageButton::RbMonitorNextPageButton(QWidget * parent) 
:QWidget(parent)
{
	ui.setupUi(this);
	connect(ui.nextPageBtn, SIGNAL(clicked()), this, SLOT(onClickedNextPage()));
	Mx::setWidgetStyle(this,"qss:RbMonitorNextPageButton.qss");
}

RbMonitorNextPageButton::~RbMonitorNextPageButton(void)
{

}

void RbMonitorNextPageButton::setWaitingInfo(const QString& userId, const int nWaitingCount)
{
	QString info = QString::fromLocal8Bit("%1 等%2名用户").arg(userId).arg(nWaitingCount);
	ui.label->setText(info);
}

void RbMonitorNextPageButton::onClickedNextPage()
{
	emit nextPage();
}


QString RbMonitorNextPageButton::getLastWaittingUserID()
{
	return m_strUserId;
}