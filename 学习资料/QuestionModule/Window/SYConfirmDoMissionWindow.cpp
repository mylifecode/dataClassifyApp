#include "SYConfirmDoMissionWindow.h"
#include"RbShieldLayer.h"

#include"MxDefine.h"


SYConfirmDoMissionWindow::SYConfirmDoMissionWindow(int num,int examTime,QWidget *parent):
RbShieldLayer(parent)
   
{
    ui.setupUi(this);
	Mx::setWidgetStyle(this, "qss:SYConfirmDoMissionWindow.qss");

	hideCloseButton();
	hideOkButton();

	ui.queNum->setText(QString::fromLocal8Bit("%1 Ã‚").arg(num));
	ui.examTime->setText(QString::fromLocal8Bit("%1 ∑÷÷”").arg(examTime));
	connect(ui.confirmBtn, &QPushButton::clicked, this, &SYConfirmDoMissionWindow::on_btn_clicked);
}

SYConfirmDoMissionWindow::~SYConfirmDoMissionWindow()
{
 
}

void SYConfirmDoMissionWindow::on_btn_clicked()
{
	QPushButton* btn = static_cast<QPushButton*>(sender());

	if (btn==ui.confirmBtn)
	{

		done(RC_Ok);

	}
	else
	{

		done(RC_Cancel);
	}


}