#include "SYAdminSendTaskConfirmDlg.h"
#include <QPushButton>
#include  <QSound>
#include <QDesktopWidget>
#include <QMouseEvent>
#include "MxDefine.h"
#include "MxGlobalConfig.h"
#include "SYDBMgr.h"
#include "SYUserInfo.h"
#include "SYStringTable.h"
#include <QMessageBox>

SYAdminSendTaskConfirmDlg::SYAdminSendTaskConfirmDlg(QWidget *parent, QStringList & tasknamelist, QStringList & receivernamelist) :RbShieldLayer(parent)
{
	ui.setupUi(this);
	hideCloseButton();
	hideOkButton();

	setAttribute(Qt::WA_DeleteOnClose);
	Mx::setWidgetStyle(this,"qss:SYAdminSendTaskConfirmDlg.qss");

	
	//connect(ui.list_other, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeOtherListItem(QListWidgetItem*)));
	//connect(ui.list_this,  SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeThisListItem(QListWidgetItem*)));
	//connect(ui.trainlist, &QTreeWidget::itemClicked, this, &SYModifyTrainTaskWindow::onTreeWidgetItemClicked);
	QString missions;
	for (int c = 0; c < tasknamelist.size(); c++)
	{
		missions += tasknamelist[c];
		if (c < tasknamelist.size() - 1)
			missions += " , ";
	}

	QString students;
	for (int c = 0; c < receivernamelist.size(); c++)
	{
		students += receivernamelist[c];
		if (c < receivernamelist.size() - 1)
			students += " , ";
	}

	ui.lb_missionname->setText(missions);
	ui.lb_receiver->setText(students);
}

SYAdminSendTaskConfirmDlg::~SYAdminSendTaskConfirmDlg()
{

}

void SYAdminSendTaskConfirmDlg::on_bt_cancel_clicked()
{
	done(1);
}

void SYAdminSendTaskConfirmDlg::on_bt_conform_clicked()
{
	done(2);
}