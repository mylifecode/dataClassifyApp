#include "SYAdminGroupMgrWindow.h"
#include <QMessageBox>
#include "SYUserInfo.h"
#include "SYMessageBox.h"
#include "MxGlobalConfig.h"
#include "MXApplication.h"
#include "SYDBMgr.h"
#include "SYStringTable.h"
#include "SYAdminModifyGroupWindow.h"
#include "SYAdminGroupMemberWindow.h"
#include <tchar.h>
#include "SYMainWindow.h"
#include "SYMessageSendWindow.h"
#include "SYProcessingDlg.h"

SYAdmin_GroupListItem::SYAdmin_GroupListItem(QWidget *parent) : QWidget(parent)
{
	ui.setupUi(this);
	Mx::setWidgetStyle(this, "qss:SYAdmin_GroupListItem.qss");
}

SYAdmin_GroupListItem::~SYAdmin_GroupListItem()
{

}

void SYAdmin_GroupListItem::SetContent(int grpID, const QString & grpName,
	const QString & foundedTime,
	const QString & OwnerName,
	int   grpNumPerson,
	int   grpNumMission,
	int   grpNumCourse)
{
	m_GroupID = grpID;
	m_GroupName = grpName;
	m_OwnerName = OwnerName;

	ui.lb_grpname->setText(grpName);
	ui.lb_date->setText(foundedTime);
	ui.lb_owner->setText(OwnerName);

	ui.lb_numperson->setText(QString::number(grpNumPerson));
	ui.lb_nummission->setText(QString::number(grpNumMission));
	ui.lb_numcoruse->setText(QString::number(grpNumCourse));
}
void SYAdmin_GroupListItem::on_bt_desc_clicked()
{
	emit item_desc_clicked(this);
}

void SYAdmin_GroupListItem::on_bt_add_clicked()
{
	emit item_add_clicked(this);
}

void SYAdmin_GroupListItem::on_bt_edit_clicked()
{
	emit item_edit_clicked(this);
}

void SYAdmin_GroupListItem::on_bt_remove_clicked()
{
	emit item_remove_clicked(this);
}

void SYAdmin_GroupListItem::on_bt_send_clicked()
{
	emit item_send_clicked(this);
}


SYAdminGroudpMgrWindow::SYAdminGroudpMgrWindow(QWidget *parent) : QWidget(parent),
m_userName(""),
m_realName(""),
m_messageSendWindow(nullptr),
m_processingDlg(nullptr)
{
	m_userName = SYUserInfo::Instance()->GetUserName();
	m_realName = SYUserInfo::Instance()->GetRealName();

	ui.setupUi(this);
	
	Mx::setWidgetStyle(this, "qss:SYAdminGroupMgrWindow.qss");

	RefreshGroupList();
}

SYAdminGroudpMgrWindow::~SYAdminGroudpMgrWindow(void)
{
	
}

void SYAdminGroudpMgrWindow::RefreshGroupList()
{
	QVector<QSqlRecord> grpResults;

	SYDBMgr::Instance()->QueryAllGroupInfo(grpResults);

	ui.list_group->clear();

	for (int c = 0; c < grpResults.size(); c++)
	{
		QString ownername = grpResults[c].value("ownername").toString();

		QString grpname = grpResults[c].value("name").toString();

		QDate dateT = grpResults[c].value("foundedTime").toDate();

		QString foundedTime = dateT.toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT));

		int ownerId = grpResults[c].value("ownerId").toInt();

		int groupId = grpResults[c].value("id").toInt();

		int personNum = grpResults[c].value("personnum").toInt();

		int missionNum = grpResults[c].value("missionnum").toInt();

		int courseNum = grpResults[c].value("coursenum").toInt();

		if(FilterRecord(grpname, foundedTime, ownername, personNum, missionNum, courseNum) == false)
			continue;

		QListWidgetItem * listWidgetItem = new QListWidgetItem(ui.list_group);//list clear 的时候会删除

		SYAdmin_GroupListItem * theWidgetItem = new SYAdmin_GroupListItem(ui.list_group);//list clear 的时候会删除

		theWidgetItem->SetContent(groupId , grpname, foundedTime, ownername, personNum, missionNum, courseNum);

		//Making sure that the listWidgetItem has the same size as the TheWidgetItem
		listWidgetItem->setSizeHint(QSize(370, 70));

		//Finally adding the itemWidget to the list
		ui.list_group->setItemWidget(listWidgetItem, theWidgetItem);

		//
		connect(theWidgetItem, SIGNAL(item_desc_clicked(SYAdmin_GroupListItem*)), this, SLOT(item_desc_clicked(SYAdmin_GroupListItem*)));
		connect(theWidgetItem, SIGNAL(item_add_clicked(SYAdmin_GroupListItem*)),  this, SLOT(item_add_clicked(SYAdmin_GroupListItem*)));
		connect(theWidgetItem, SIGNAL(item_edit_clicked(SYAdmin_GroupListItem*)), this, SLOT(item_edit_clicked(SYAdmin_GroupListItem*)));
		connect(theWidgetItem, SIGNAL(item_remove_clicked(SYAdmin_GroupListItem*)), this, SLOT(item_remove_clicked(SYAdmin_GroupListItem*)));
		connect(theWidgetItem, SIGNAL(item_send_clicked(SYAdmin_GroupListItem*)), this, SLOT(item_send_clicked(SYAdmin_GroupListItem*)));
	}
	//
}

void SYAdminGroudpMgrWindow::SetFilterText(const QString& text)
{
	m_filterText = text;
}

bool SYAdminGroudpMgrWindow::FilterRecord(const QString & grpName,
										  const QString & foundedTime,
										  const QString & OwnerName,
										  int   grpNumPerson,
										  int   grpNumMission,
										  int   grpNumCourse)
{
	if(m_filterText.size() == 0)
		return true;

	if(grpName.contains(m_filterText, Qt::CaseInsensitive))
		return true;

	if(foundedTime.contains(m_filterText, Qt::CaseInsensitive))
		return true;

	if(OwnerName.contains(m_filterText, Qt::CaseInsensitive))
		return true;

	bool ok = false;
	int n = m_filterText.toInt(&ok);
	if(ok){
		if(n == grpNumPerson || n == grpNumMission || n == grpNumCourse)
			return true;
	}

	return false;
}

void SYAdminGroudpMgrWindow::item_desc_clicked(SYAdmin_GroupListItem * item)
{
	QString grpName = item->m_GroupName;
	int     grpID = item->m_GroupID;
	SYAdminGroupMemberWindow * group = new SYAdminGroupMemberWindow(this, grpID, grpName);

	group->showFullScreen();

	int button = group->exec();

}

void SYAdminGroudpMgrWindow::item_add_clicked(SYAdmin_GroupListItem * item)
{
	QString grpName = item->m_GroupName;
	int     grpID = item->m_GroupID;

	SYAdminModifyGroupWindow * group = new SYAdminModifyGroupWindow(this, grpID, grpName);

	group->showFullScreen();

	int button = group->exec();

	if (button == 2)
		RefreshGroupList();
}

void SYAdminGroudpMgrWindow::item_edit_clicked(SYAdmin_GroupListItem * item)
{
	QString grpName = item->m_GroupName;
	int     grpID = item->m_GroupID;
	
	SYAdminModifyGroupWindow * group = new SYAdminModifyGroupWindow(this, grpID, grpName);
	
	group->showFullScreen();
	
	int button = group->exec();

	if (button == 2)
		RefreshGroupList();
}

void SYAdminGroudpMgrWindow::item_remove_clicked(SYAdmin_GroupListItem * item)
{
	int     grpID = item->m_GroupID;
	
	QString grpName = item->m_GroupName;

	QString title = SYStringTable::GetString(SYStringTable::STR_CONFIRMREMOVEGROUP);

	SYMessageBox * group = new SYMessageBox(this, grpName, title, 2);

	group->showFullScreen();

	int button = group->exec();

	if (button == 2)
	{
		SYDBMgr::Instance()->DeleteGroup(grpID);
	}
	RefreshGroupList();
}

void SYAdminGroudpMgrWindow::item_send_clicked(SYAdmin_GroupListItem* item)
{
	QString grpName = item->m_GroupName;
	int     grpID = item->m_GroupID;

	if(m_messageSendWindow == nullptr)
		m_messageSendWindow = new SYMessageSendWindow(this);

	m_messageSendWindow->SetReceiveGroup(grpID);
	m_messageSendWindow->showFullScreen();

	if(m_messageSendWindow->exec() == RbShieldLayer::RC_Ok){
		if(m_processingDlg == nullptr){
			m_processingDlg = new SYProcessingDlg(SYMainWindow::GetInstance());
			m_processingDlg->setAttribute(Qt::WA_DeleteOnClose, false);
		}

		m_processingDlg->SetAutoProcess(2000);
		m_processingDlg->showFullScreen();
	}
}

void SYAdminGroudpMgrWindow::on_bt_creategroup_clicked()
{
	SYAdminModifyGroupWindow * group = new SYAdminModifyGroupWindow(this , -1 ,"");
	group->showFullScreen();
	int button = group->exec();

	if (button == 2)
	    RefreshGroupList();
}

void SYAdminGroudpMgrWindow::on_bt_personmgr_clicked()
{
	emit showNextWindow(WT_AdminGroupMgrWindow);
}

void SYAdminGroudpMgrWindow::on_bt_studymgr_clicked()
{

}

void SYAdminGroudpMgrWindow::on_bt_exammgr_clicked()
{

}

void SYAdminGroudpMgrWindow::on_bt_knowledgemgr_clicked()
{

}
void SYAdminGroudpMgrWindow::on_bt_datacenter_clicked()
{

}
void SYAdminGroudpMgrWindow::on_bt_traincenter_clicked()
{

}
void SYAdminGroudpMgrWindow::on_bt_personcenter_clicked()
{

}
