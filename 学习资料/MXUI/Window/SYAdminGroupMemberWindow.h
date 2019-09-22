#ifndef _SYAdminGroupMemberWindow_H
#define _SYAdminGroupMemberWindow_H
#include "ui_SYAdminGroupMemberWindow.h"
#include "RbShieldLayer.h"
#include <QVector>
#include <QPushButton>


class SYGroupMemberItem : public QFrame
{
public:
	SYGroupMemberItem(QWidget * parent, int itemIndex);

	virtual ~SYGroupMemberItem();

	void Create(const QString & userName0, const QString & className0, const QString & userName1, const QString & className1, int ItemFixHeight, const QString & skinDir);

	int  m_itemIndex;
protected:
	

	
	QLabel * m_lb_class0;
	QLabel * m_lb_name0;

	QLabel * m_lb_class1;
	QLabel * m_lb_name1;
};

class  SYAdminGroupMemberWindow : public RbShieldLayer
{
	Q_OBJECT

public:

	class GroupMember
	{
	public:
		GroupMember()
		{
			m_GroupID = -1;
		}
		GroupMember(const QString & personName, const QString & className , int userID)
		{
			m_PersonName = personName;
			m_ClassName = className;
			m_userID = userID;
		}

		QString m_PersonName;
		
		QString m_ClassName;
		
		int m_GroupID;

		int m_userID;
	};

	SYAdminGroupMemberWindow(QWidget *parent, int currGroupID, const QString & groupName);
	
	~SYAdminGroupMemberWindow();


private slots:

    void on_bt_conform_clicked();

    void on_bt_cancel_clicked();
private:

	void PullAllMembers(std::vector<GroupMember> & memberList , int GroupID);

	void RefreshUILists();

	std::vector<GroupMember> m_ThisGroup;

	Ui::SYAdminGroupMemberWindow  ui;

	int m_CurrentGroupID;
	QString m_GroupName;
};

#endif