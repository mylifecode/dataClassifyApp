#include "RbEditUserInfoWindow.h"
#include "MxDefine.h"
#include "SYDBMgr.h"
#include "SYUserInfo.h"

RbEditUserInfoWindow::RbEditUserInfoWindow(int adminId,const QString& userName,QWidget *pParent)
:RbShieldLayer(pParent),
m_userId(-1),
m_canUpdate(false),
m_isResetPassword(false)
{
	ui.setupUi(this);

	hideOkButton();
	hideCloseButton();
	
	bool bRet;
	QSqlRecord record;

	bRet = SYDBMgr::Instance()->QueryUserInfo(userName,record);
	bool userIsAdmin = false;
	if(bRet)
	{
		m_userId = record.value("id").toInt();
		m_originUserName = userName;
		m_originRealName = record.value("realName").toString();
		m_originGroupId = record.value("groupId").toInt();
		m_initPassword = record.value("initPassword").toString();
		m_curPassword = record.value("password").toString();
		userIsAdmin = SYUserInfo::IsAdminPermission(record.value("permission").toInt());

		bool isSuperAdmin = false;
		bool isAdmin = false;
		if(SYDBMgr::Instance()->QueryUserInfo(adminId,record))
		{
			int permissionValue = record.value("permission").toInt();
			switch(SYUserInfo::ConvertValueToPermission(permissionValue))
			{
			case UP_Teacher:
				isAdmin = true;
				break;
			case UP_SuperManager:
				isSuperAdmin = true;
				break;
			}
		}

		QVector<QSqlRecord> results;
		if(isSuperAdmin)
		{
			if(SYDBMgr::Instance()->QueryAllGroupInfo(results))
			{
				for(int i = 0 ;i < results.size();++i)
					m_adminGroupMap.insert(results[i].value("id").toInt(),results[i].value("Name").toString());
			}
			m_canUpdate = true;
		}
		else if(isAdmin)
		{
			if(m_originGroupId == 0)
				m_canUpdate = true;
			if(SYDBMgr::Instance()->QueryGroupInfo(adminId,results))
			{
				for(int i = 0;i < results.size();++i)
					m_adminGroupMap.insert(results[i].value("id").toInt(),results[i].value("Name").toString());
				if(m_originGroupId != 0 && m_adminGroupMap.contains(m_originGroupId) == false)
				{
					m_adminGroupMap.clear();
					Message(CHS("错误"),CHS("无操作权限"));
				}
				else
				{
					m_canUpdate = true;
				}
			}
		}
		else
		{
			Message(CHS("错误"),CHS("无操作权限"));
		}
	}
	else
		Message(CHS("错误"),CHS("无法查找到该用户"));

	ui.userNameLineEdit->setText(m_originUserName);
	ui.realNameLineEdit->setText(m_originRealName);
	//组号为0，表示无任何分组
	m_adminGroupMap.insert(0,CHS("未分组"));
	int i = 0;
	for(QMap<int,QString>::const_iterator itr = m_adminGroupMap.begin();itr != m_adminGroupMap.end();++itr)
	{
		ui.groupComboBox->addItem(itr.value(),itr.key());
		if(itr.key() == m_originGroupId)
			ui.groupComboBox->setCurrentIndex(i);
		++i;
	}

	//ui.userNameLineEdit->setEnabled(m_canUpdate);
	ui.userNameLineEdit->setEnabled(false);
	ui.realNameLineEdit->setEnabled(m_canUpdate);

	if(userIsAdmin)
	{
		ui.groupComboBox->setItemText(0,CHS("管理员"));
		ui.groupComboBox->setCurrentIndex(0);
		ui.groupComboBox->setEnabled(false);
	}
	
	Mx::setWidgetStyle(this,"qss:RbEditUserInfoWindow.qss");
}

RbEditUserInfoWindow::~RbEditUserInfoWindow(void)
{

}

void RbEditUserInfoWindow::on_cancelBtn_clicked()
{
	done(0);
}

void RbEditUserInfoWindow::on_okBtn_clicked()
{
	if(m_canUpdate)
	{
		bool updated = false;
		//1 update user name
		if(m_originUserName != ui.userNameLineEdit->text())
		{
			if(SYDBMgr::Instance()->UpdateUserName(m_userId,ui.userNameLineEdit->text()))
			{
				m_originUserName = ui.userNameLineEdit->text();
				updated = true;
			}
			else
			{
				Message(CHS("提示"),CHS("用户名已存在，请重新输入"));
				return;
			}
		}
	
		//2 update realName
		if(m_originRealName != ui.realNameLineEdit->text())
		{
			if(SYDBMgr::Instance()->UpdateUserRealName(m_userId,ui.realNameLineEdit->text()))
			{
				m_originRealName = ui.realNameLineEdit->text();
				updated = true;
			}
		}

		//3 update group id
		const int newGroupId = ui.groupComboBox->itemData(ui.groupComboBox->currentIndex()).toInt();
		if(newGroupId != m_originGroupId)
		{
			if(SYDBMgr::Instance()->UpdateUserGroupId(m_userId,newGroupId))
			{
				m_originGroupId = newGroupId;
				updated = true;
			}
		}

		//4 reset password
		if(ui.radioButton->isChecked() && m_curPassword != m_initPassword)
		{
			if(SYDBMgr::Instance()->UpdateUserPassword(m_userId,m_initPassword))
			{
				m_curPassword = m_initPassword;
				updated = true;
			}
		}

		if(updated)
			done(1);
		else
			done(0);
	}
	else
		done(0);
}