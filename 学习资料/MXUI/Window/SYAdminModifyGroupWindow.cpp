#include "SYAdminModifyGroupWindow.h"
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
SYGroupExchangeItem::SYGroupExchangeItem(QWidget * parent, int itemindex, bool isChecked) : QFrame(parent)
{
	m_bChecked  = isChecked;
	m_itemIndex = itemindex;
	m_lb_name = m_lb_arrow = m_lb_class = 0;
}

SYGroupExchangeItem::~SYGroupExchangeItem()
{

}

void SYGroupExchangeItem::Create(const QString & userName , const QString & groupName,int ItemFixHeight, const QString & skinDir)
{
	setFixedHeight(ItemFixHeight);
	m_lb_name  = new QLabel(this);
	m_lb_arrow = new QLabel(this);
	m_lb_class = new QLabel(this);

	//arrow picture
	m_lb_arrow->move(11, 14);
	m_lb_arrow->setFixedSize(16, (ItemFixHeight - 14) / 2);
	m_lb_arrow->setStyleSheet(QString("border-image:url(%1/SYAdmnPersonMgr/left_arrow.png);").arg(skinDir));
	m_lb_arrow->setVisible(m_bChecked);


	//user name
	m_lb_name->move(15 + 16, 0);
	m_lb_name->setFixedHeight(ItemFixHeight);
	m_lb_name->setFixedWidth(110);
	m_lb_name->setText(userName);
	m_lb_name->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_lb_name->setStyleSheet(QString("font-size:16px; color:#b7c5d8;").arg(skinDir));

	//user class
	m_lb_class->move(15 + 16 + 110, 0);
	m_lb_class->setFixedHeight(ItemFixHeight);
	m_lb_class->setFixedWidth(120);
	m_lb_class->setText(SYStringTable::GetString(SYStringTable::STR_NO));
	m_lb_class->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_lb_class->setStyleSheet(QString("font-size:16px; color:#b7c5d8;").arg(skinDir));

}

void SYGroupExchangeItem::switchCheckState()
{
	m_bChecked = !m_bChecked;
	if (m_bChecked)
		m_lb_arrow->setVisible(true);
	else
		m_lb_arrow->setVisible(false);
}

SYAdminModifyGroupWindow::SYAdminModifyGroupWindow(QWidget *parent, int currGroupID, const QString & groupName)
:RbShieldLayer(parent)
{
	ui.setupUi(this);
	hideCloseButton();
	hideOkButton();

	setAttribute(Qt::WA_DeleteOnClose);
	Mx::setWidgetStyle(this,"qss:SYAdminModifyGroupWindow.qss");

	
	PullStudents(m_UnGroupedStudnet, -1);

	if (currGroupID >= 0)
	{
		PullStudents(m_ThisGroup, currGroupID);
		ui.HeadFrame->hide();
		ui.verticalSpacer_6->changeSize(0,0);
	}
	m_CurrentGroupID = currGroupID;
	m_GroupName = groupName;

	RefreshUILists();
	
	connect(ui.list_other, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeOtherListItem(QListWidgetItem*)));
	connect(ui.list_this,  SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onClickeThisListItem(QListWidgetItem*)));

}

SYAdminModifyGroupWindow::~SYAdminModifyGroupWindow()
{

}

void SYAdminModifyGroupWindow::PullStudents(std::vector<PersonInList> & userVec, int GroupID)
{
	std::vector<QSqlRecord> userRecords;

	if (GroupID < 0)
	    SYDBMgr::Instance()->QueryNoGroupUserInfo(userRecords);
	else
		SYDBMgr::Instance()->QueryUserInGroup(GroupID, userRecords);

	userVec.clear();

	for (int c = 0; c < (int)userRecords.size(); c++)
	{
		QString userName  = userRecords[c].value("userName").toString();
		QString groupName = userRecords[c].value("groupname").toString();
		QString realName  = userRecords[c].value("realName").toString();

		int userId = userRecords[c].value("id").toInt();
		int groupId = userRecords[c].value("groupId").toInt();
		
		PersonInList person(realName, groupName, userId, groupId);

		userVec.push_back(person);
	}
}

void SYAdminModifyGroupWindow::RefreshUILists()
{
	QString skinDir = MxGlobalConfig::Instance()->GetSkinDir();
	int ItemFixHeight = 40;

	ui.list_other->clear();
	ui.list_this->clear();

	for (int c = 0; c < (int)m_UnGroupedStudnet.size(); c++)
	{
		SYGroupExchangeItem * itemFrame = new SYGroupExchangeItem(ui.list_other, c, m_UnGroupedStudnet[c].m_IsSelected);
		itemFrame->Create(m_UnGroupedStudnet[c].m_PersonName, m_UnGroupedStudnet[c].m_BelongGroup, ItemFixHeight, skinDir);
		
		QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_other);
		listWidgetItem->setSizeHint(QSize(100, ItemFixHeight));

		ui.list_other->setItemWidget(listWidgetItem, itemFrame);
	}

	for (int c = 0; c < (int)m_ThisGroup.size(); c++)
	{
		SYGroupExchangeItem * itemFrame = new SYGroupExchangeItem(ui.list_this, c, m_ThisGroup[c].m_IsSelected);
		itemFrame->Create(m_ThisGroup[c].m_PersonName, m_ThisGroup[c].m_BelongGroup, ItemFixHeight, skinDir);

		QListWidgetItem  * listWidgetItem = new QListWidgetItem(ui.list_this);
		listWidgetItem->setSizeHint(QSize(100, ItemFixHeight));

		ui.list_this->setItemWidget(listWidgetItem, itemFrame);
	}
}

void SYAdminModifyGroupWindow::onClickeOtherListItem(QListWidgetItem *item)
{
	SYGroupExchangeItem * itemFrame = dynamic_cast<SYGroupExchangeItem*>(ui.list_other->itemWidget(item));
	if (itemFrame)
	{
		itemFrame->switchCheckState();
		m_UnGroupedStudnet[itemFrame->m_itemIndex].m_IsSelected = itemFrame->GetCheckState();
	}
}

void SYAdminModifyGroupWindow::onClickeThisListItem(QListWidgetItem *item)
{
	SYGroupExchangeItem * itemFrame = dynamic_cast<SYGroupExchangeItem*>(ui.list_this->itemWidget(item));
	if (itemFrame)
	{
		itemFrame->switchCheckState();
		m_ThisGroup[itemFrame->m_itemIndex].m_IsSelected = itemFrame->GetCheckState();
	}
}

void SYAdminModifyGroupWindow::on_bt_exchange_clicked()
{
	std::vector<PersonInList> otherGroupRemoved;
	
	std::vector<PersonInList> thisGroupRemoved;

	{
		int headIndex = 0;

		int tailIndex = 0;

		while (tailIndex < (int)m_UnGroupedStudnet.size())
		{
			if (m_UnGroupedStudnet[tailIndex].m_IsSelected == false)
			{
				if (headIndex != tailIndex)
					m_UnGroupedStudnet[headIndex] = m_UnGroupedStudnet[tailIndex];
				headIndex++;
			}
			else
			{
				otherGroupRemoved.push_back(m_UnGroupedStudnet[tailIndex]);
			}
			tailIndex++;
		}
		m_UnGroupedStudnet.resize(headIndex);
	}

	{
		int headIndex = 0;

		int tailIndex = 0;

		while (tailIndex < (int)m_ThisGroup.size())
		{
			if (m_ThisGroup[tailIndex].m_IsSelected == false)
			{
				if (headIndex != tailIndex)
					m_ThisGroup[headIndex] = m_ThisGroup[tailIndex];
				headIndex++;
			}
			else
			{
				thisGroupRemoved.push_back(m_ThisGroup[tailIndex]);
			}
			tailIndex++;
		}
		m_ThisGroup.resize(headIndex);
	}


	//exchange removed item
	if (otherGroupRemoved.size() > 0)
	{
		int oldNum = m_ThisGroup.size();

		m_ThisGroup.resize(oldNum + otherGroupRemoved.size());
		
		for (int c = oldNum - 1; c >= 0; c--)
		{
			m_ThisGroup[c + otherGroupRemoved.size()] = m_ThisGroup[c];
		}
		for (int c = 0; c < (int)otherGroupRemoved.size(); c++)
		{
			m_ThisGroup[c] = otherGroupRemoved[c];
			m_ThisGroup[c].m_IsSelected = false;//unmark this to unselect all when change finish
			m_ThisGroup[c].m_GroupID = m_CurrentGroupID;
		}
	}


	if (thisGroupRemoved.size() > 0)
	{
		int oldNum = m_UnGroupedStudnet.size();
		
		m_UnGroupedStudnet.resize(oldNum + thisGroupRemoved.size());

		for (int c = oldNum - 1; c >= 0; c--)
		{
			m_UnGroupedStudnet[c + thisGroupRemoved.size()] = m_UnGroupedStudnet[c];
	    }
		for (int c = 0; c < (int)thisGroupRemoved.size(); c++)
		{
			m_UnGroupedStudnet[c] = thisGroupRemoved[c];
			m_UnGroupedStudnet[c].m_IsSelected = false;//unmark this to unselect all when change finish
			m_UnGroupedStudnet[c].m_GroupID = -1;//mark as ungrouped
		}
	}

	//
	RefreshUILists();
}


void SYAdminModifyGroupWindow::on_bt_conform_clicked()
{
	QString ownerName = SYUserInfo::Instance()->GetUserName();

	int ownerId = SYUserInfo::Instance()->GetUserId();

	//create new group mode
	if (m_CurrentGroupID < 0)
	{
		m_GroupName = ui.edit_groupname->text();

		if (m_GroupName == "")
		{
			QMessageBox::information(this, SYStringTable::GetString(SYStringTable::STR_GROUPNAMENOTALLOWEMPTY), SYStringTable::GetString(SYStringTable::STR_EMPTY), QMessageBox::Ok);// ;
			return;
		}
		//add to database and update student information
		m_CurrentGroupID = SYDBMgr::Instance()->AddGroup(m_GroupName, ownerName, ownerId);
		for (int c = 0; c < m_ThisGroup.size(); c++)
		     m_ThisGroup[c].m_GroupID = m_CurrentGroupID;
	}

	if (m_CurrentGroupID >= 0)
	{
		//update grouped student
		std::vector<int> userIDS;
		for (int c = 0; c < m_ThisGroup.size(); c++)
		{
			if (m_ThisGroup[c].m_GroupID != m_ThisGroup[c].m_OldGroupID)
				userIDS.push_back(m_ThisGroup[c].m_userID);
		}
		if (userIDS.size() > 0)
		    SYDBMgr::Instance()->UpdateUsersGroup(userIDS, m_CurrentGroupID, m_GroupName);

		//update ungrouped
		userIDS.clear();
		for (int c = 0; c < m_UnGroupedStudnet.size(); c++)
		{
			if (m_UnGroupedStudnet[c].m_GroupID != m_UnGroupedStudnet[c].m_OldGroupID)
				userIDS.push_back(m_UnGroupedStudnet[c].m_userID);
		}
		if (userIDS.size() > 0)
		    SYDBMgr::Instance()->UpdateUsersGroup(userIDS, -1, "");
		done(2);
	}
	else
	{
		QMessageBox::information(this, SYStringTable::GetString(SYStringTable::STR_GROUPNAMEISEXITS), SYStringTable::GetString(SYStringTable::STR_EMPTY), QMessageBox::Ok);// ;
	}
}

void SYAdminModifyGroupWindow::on_bt_cancel_clicked()
{
	done(1);
}