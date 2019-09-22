#include "SYConfirmSendPaperWindow.h"
#include "ui_SYConfirmSendPaperWindow.h"
#include"RbShieldLayer.h"
#include"MxDefine.h"
#include"SYDBMgr.h"


SYConfirmSendPaperWindow::SYConfirmSendPaperWindow(QVector<QString> senders,QVector<QString> receivers,QWidget *parent):
RbShieldLayer(parent)
   
{
    ui.setupUi(this);
	Mx::setWidgetStyle(this, QString("qss:SYConfirmSendPaperWindow.qss"));

	hideOkButton();
	hideCloseButton();

	SetContent(senders,receivers);

	connect(ui.returnBtn, &QPushButton::clicked, this, [=]()
	{

		done(0);
	});
	connect(ui.sendBtn, &QPushButton::clicked, this, [=](){
	
		done(1);
	});

}

SYConfirmSendPaperWindow::~SYConfirmSendPaperWindow()
{
}


void SYConfirmSendPaperWindow::SetContent(QVector<QString> senders, QVector<QString> receivers)
{

	QString sendInfo = "";
	for (auto &sender : senders)
	{
	sendInfo += sender + ",";
	}

	QString receivedInfo = "";

	for (auto &receiver : receivers)
	{
	receivedInfo += receiver + ",";
	}

	ui.courseName_1->setText(sendInfo);
	ui.receivedObject->setText(receivedInfo);
}