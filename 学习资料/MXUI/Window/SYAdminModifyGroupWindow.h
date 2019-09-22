#ifndef SYAdminModifyGroup_H
#define SYAdminModifyGroup_H
#include "ui_SYAdminModifyGroupWindow.h"
#include "RbShieldLayer.h"
#include <QVector>
#include <QPushButton>


class SYGroupExchangeItem : public QFrame
{
public:
	SYGroupExchangeItem(QWidget * parent , int itemIndex , bool isChecked);

	virtual ~SYGroupExchangeItem();

	void Create(const QString & userName, const QString & groupName, int ItemFixHeight, const QString & skinDir);

	void switchCheckState();

	bool GetCheckState()
	{
		return m_bChecked;
	}

	int  m_itemIndex;
protected:
	
	bool m_bChecked;

	
	QLabel * m_lb_class;
	QLabel * m_lb_name;
	QLabel * m_lb_arrow;
};

class  SYAdminModifyGroupWindow : public RbShieldLayer
{
	Q_OBJECT

public:

	class PersonInList
	{
	public:
		PersonInList()
		{
			m_GroupID = -1;
			m_IsSelected = false;
		}
		PersonInList(const QString & personName, const QString & group, int userID , int groupID)
		{
			m_PersonName = personName;
			m_BelongGroup = group;
			m_GroupID = groupID;
			m_OldGroupID = groupID;
			m_userID = userID;
			m_IsSelected = false;
		}

		QString m_PersonName;
		
		QString m_BelongGroup;
		
		int m_GroupID;

		int m_OldGroupID;

		int m_userID;

		bool m_IsSelected;
	};

	SYAdminModifyGroupWindow(QWidget *parent, int currGroupID, const QString & groupName);
	
	~SYAdminModifyGroupWindow();


private slots:

	void onClickeOtherListItem(QListWidgetItem *item);

	void onClickeThisListItem(QListWidgetItem *item);

	void on_bt_exchange_clicked();

	void on_bt_conform_clicked();

	void on_bt_cancel_clicked();

private:

	void PullStudents(std::vector<PersonInList> & userVec, int GroupID);

	void RefreshUILists();

	std::vector<PersonInList> m_ThisGroup;
	std::vector<PersonInList> m_UnGroupedStudnet;
	Ui::SYAdminModifyGroupWindow  ui;

	int m_CurrentGroupID;
	QString m_GroupName;
};

#endif