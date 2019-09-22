#include "RbMessageViewWindow.h"
#include "MxDefine.h"

RbMessageViewWindow::RbMessageViewWindow(QWidget *parent,QString subject,QString content)
:QWidget(parent),
m_subject(subject),
m_content(content)
{
	ui.setupUi(this);
	ui.subjectLabel->setText(m_subject);
	ui.contentText->setText(m_content);

	connect(ui.cancelBtn,SIGNAL(clicked()),this,SLOT(onClickedcloseBtn()));
	Mx::setWidgetStyle(this,"qss:RbMessageViewWindow.qss");

}

RbMessageViewWindow::~RbMessageViewWindow()
{

}

void RbMessageViewWindow::closeEvent(QCloseEvent * event)
{
	emit messageClose();
}

void RbMessageViewWindow::onClickedcloseBtn()
{
	this->close();
}