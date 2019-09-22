#pragma once
#include "ui_SYAdminGroupMgrWindow.h"
#include "ui_SYAdmin_GroupListItem.h"
#include "QListWidgetItem"
#include "MxDefine.h"

class SYMessageSendWindow;
class SYProcessingDlg;

class SYAdmin_GroupListItem : public QWidget
{
	Q_OBJECT

public:
	SYAdmin_GroupListItem(QWidget *parent = 0);

	~SYAdmin_GroupListItem();

	void SetContent(int grpID , const QString & grpName,
		            const QString & foundedTime,
		            const QString & OwnerName,
		            int   grpNumPerson,
		            int   grpNumMission,
		            int   grpNumCourse);

	QString m_GroupName;
	QString m_OwnerName;
	int     m_GroupID;
signals:
	void item_desc_clicked(SYAdmin_GroupListItem*);

	void item_add_clicked(SYAdmin_GroupListItem*);

	void item_edit_clicked(SYAdmin_GroupListItem*);

	void item_remove_clicked(SYAdmin_GroupListItem*);

	void item_send_clicked(SYAdmin_GroupListItem*);

private slots:

	void on_bt_desc_clicked();

	void on_bt_add_clicked();

	void on_bt_edit_clicked();

	void on_bt_remove_clicked();

	void on_bt_send_clicked();

private:
	Ui::SYAdmin_GroupListItem ui;
};

class SYAdminGroudpMgrWindow : public QWidget
{
	Q_OBJECT
public:
	SYAdminGroudpMgrWindow::SYAdminGroudpMgrWindow(QWidget *parent = nullptr);
	
	~SYAdminGroudpMgrWindow(void);
	
	void RefreshGroupList();

	void SetFilterText(const QString& text);

private:
	bool FilterRecord(const QString & grpName,
					  const QString & foundedTime,
					  const QString & OwnerName,
					  int   grpNumPerson,
					  int   grpNumMission,
					  int   grpNumCourse);

signals:
	void showNextWindow(WindowType type);

public slots:
    void item_desc_clicked(SYAdmin_GroupListItem*);

	void item_add_clicked(SYAdmin_GroupListItem*);

	void item_edit_clicked(SYAdmin_GroupListItem*);

	void item_remove_clicked(SYAdmin_GroupListItem*);

	void item_send_clicked(SYAdmin_GroupListItem*);


    void on_bt_creategroup_clicked();
    void on_bt_personmgr_clicked();
	void on_bt_studymgr_clicked();
	void on_bt_exammgr_clicked();
	void on_bt_knowledgemgr_clicked();
	void on_bt_datacenter_clicked();
	void on_bt_traincenter_clicked();
	void on_bt_personcenter_clicked();

private:
	Ui::SYAdminGroupMgrWindow ui;
	QString m_userName;
	QString m_realName;
	UserPermission m_permission;
	std::multimap<UserPermission, QString> m_mapLoginModule;
	QVector<QPushButton*> m_buttons;

	SYMessageSendWindow* m_messageSendWindow;
	SYProcessingDlg* m_processingDlg;
	
	QString m_filterText;
};
