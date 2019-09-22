#include "SYAdminGroupMemberWindow.h"
#include <QPushButton>
#include <QSound>
#include <QDesktopWidget>
#include <QMouseEvent>
#include <qlabel.h>
#include "MxDefine.h"
#include "MxGlobalConfig.h"
#include "SYDBMgr.h"
#include "SYUserInfo.h"
#include "SYStringTable.h"
#include <QMessageBox>

SYGroupMemberItem::SYGroupMemberItem(QWidget * parent, int itemindex) : QFrame(parent)
{
	m_itemIndex = itemindex;
	m_lb_name0 = m_lb_class0 = 0;
	m_lb_name1 = m_lb_class1 = 0;
}

SYGroupMemberItem::~SYGroupMemberItem()
{

}

void SYGroupMemberItem::Create(const QString & userName0,const QString & className0,
	                           const QString & userName1, const QString & className1,
							   int ItemFixHeight,
							   const QString & skinDir)
{
	setFixedHeight(ItemFixHeight);
	m_lb_name0  = new QLabel(this);
	m_lb_class0 = new QLabel(this);
	m_lb_name1 = new QLabel(this);
	m_lb_class1 = new QLabel(this);

	int offset = 0;

	//user name
	m_lb_name0->move(offset, 0);
	m_lb_name0->setFixedHeight(ItemFixHeight);
	m_lb_name0->setFixedWidth(110);
	m_lb_name0->setText(userName0);
	m_lb_name0->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_lb_name0->setStyleSheet(QString("font-size:16px; color:#b7c5d8;").arg(skinDir));
	offset += 110;

	//user class
	m_lb_class0->move(offset, 0);
	m_lb_class0->setFixedHeight(ItemFixHeight);
	m_lb_class0->setFixedWidth(120);
	m_lb_class0->setText(className0);
	m_lb_class0->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_lb_class0->setStyleSheet(QString("font-size:16px; color:#b7c5d8;").arg(skinDir));
	offset += 120;
	
	offset += 70;
	m_lb_name1->move(offset, 0);
	m_lb_name1->setFixedHeight(ItemFixHeight);
	m_lb_name1->setFixedWidth(110);
	m_lb_name1->setText(userName1);
	m_lb_name1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_lb_name1->setStyleSheet(QString("font-size:16px; color:#b7c5d8;").arg(skinDir));
	offset += 110;

	//user class
	m_lb_class1->move(offset, 0);
	m_lb_class1->setFixedHeight(ItemFixHeight);
	m_lb_class1->setFixedWidth(120);
	m_lb_class1->setText(className1);
	m_lb_class1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_lb_class1->setStyleSheet(QString("font-size:16px; color:#b7c5d8;").arg(skinDir));
}


SYAdminGroupMemberWindow::SYAdminGroupMemberWindow(QWidget *parent, int currGroupID, const QString & groupName)
:RbShieldLayer(parent)
{
	ui.setupUi(this);
	hideCloseButton();
	hideOkButton();

	setAttribute(Qt::WA_DeleteOnClose);
	Mx::setWidgetStyle(this,"qss:SYAdminGroupMemberWindow.qss");

	
	m_CurrentGroupID = currGroupID;
	m_GroupName = groupName;

	PullAllMembers(m_ThisGroup, m_CurrentGroupID);

	RefreshUILists();
	
	QString strNumMember = QString("(%1)").arg(m_ThisGroup.size());

	ui.lb_title->setText(groupName);

	ui.lb_memernum->setText(strNumMember);
	//connect(ui.list_other, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeOtherListItem(QListWidgetItem*)));
	connect(ui.list_this,  SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeThisListItem(QListWidgetItem*)));

}

SYAdminGroupMemberWindow::~SYAdminGroupMemberWindow()
{

}

void SYAdminGroupMemberWindow::PullAllMembers(std::vector<GroupMember> & memberList, int GroupID)
{
	std::vector<QSqlRecord> userRecords;

    SYDBMgr::Instance()->QueryUserInGroup(GroupID, userRecords);

	memberList.clear();

	for (int c = 0; c < (int)userRecords.size(); c++)
	{
		QString userName  = userRecords[c].value("userName").toString();
		QString groupName = userRecords[c].value("groupname").toString();
		QString realName  = userRecords[c].value("realName").toString();

		int userId = userRecords[c].value("id").toInt();
		int groupId = userRecords[c].value("groupId").toInt();
		QString classname = SYStringTable::GetString(SYStringTable::STR_DEFAULTCLASSNAME);

		GroupMember person(realName, classname, userId);

		memberList.push_back(person);
	}
}

void SYAdminGroupMemberWindow::RefreshUILists()
{
	QString skinDir = MxGlobalConfig::Instance()->GetSkinDir();
	
	int ItemFixHeight = 40;

	ui.list_this->clear();

	for (int c = 0; c < (int)m_ThisGroup.size(); c += 2)
	{
		QString name0  = m_ThisGroup[c].m_PersonName;
		QString class0 = m_ThisGroup[c].m_ClassName;

		QString name1  = SYStringTable::GetString(SYStringTable::STR_EMPTY);
		QString class1 = name1;

		if ((c + 1) < (int)m_ThisGroup.size())
		{
			name1  = m_ThisGroup[c + 1].m_PersonName;
			class1 = m_ThisGroup[c + 1].m_ClassName;
		}

		SYGroupMemberItem * itemFrame = new SYGroupMemberItem(ui.list_this, c);
		itemFrame->Create(name0, class0, name1 , class1 , ItemFixHeight, skinDir);
		
		QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_this);
		listWidgetItem->setSizeHint(QSize(100, ItemFixHeight));

		ui.list_this->setItemWidget(listWidgetItem, itemFrame);
	}
}


void SYAdminGroupMemberWindow::on_bt_conform_clicked()
{
	done(2);
}

void SYAdminGroupMemberWindow::on_bt_cancel_clicked()
{
	done(1);
}