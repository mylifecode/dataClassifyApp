#include "RbMessageBoxLikeTipWindow.h"
#include "MxDefine.h"

RbMessageBoxLikeTipWindow::RbMessageBoxLikeTipWindow(QWidget *parent):RbShieldLayer(parent)
{
	ui.setupUi(this);
	hideOkButton();
	hideCloseButton();

	connect(ui.cancelBtn, SIGNAL(clicked()), this, SLOT(onCancelBtn()));
	connect(ui.okBtn, SIGNAL(clicked()), this, SLOT(onOKBtn()));

	Mx::setWidgetStyle(this,"qss:RbMessageBoxLikeTipWindow.qss");
}

RbMessageBoxLikeTipWindow::~RbMessageBoxLikeTipWindow()
{
}

void RbMessageBoxLikeTipWindow::onCancelBtn()
{
	this->done(1);
	emit messageBoxLikeTipWindowClose(CANCEL_PRESSED);
}

void RbMessageBoxLikeTipWindow::onOKBtn()
{
	this->done(1);
	emit messageBoxLikeTipWindowClose(CONFIRM_PRESSED);
}


void RbMessageBoxLikeTipWindow::setTitle(const QString& strTitle)
{
	ui.label->setText(strTitle);
}