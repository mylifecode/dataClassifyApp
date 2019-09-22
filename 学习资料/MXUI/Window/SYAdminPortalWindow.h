#pragma once
#include "ui_SYAdminPortalWindow.h"
#include "MxDefine.h"

class SYAdminPortalWindow : public QWidget
{
	Q_OBJECT
public:
	SYAdminPortalWindow::SYAdminPortalWindow(QWidget *parent = nullptr);
	
	~SYAdminPortalWindow(void);
	
	void hidepBtn();

	//void OnReceiveMessage(MxProcessCommunicator::MessageType type, const QString& message);

	//void OnReceiveStop();
signals:
	void showNextWindow(WindowType type);

public slots:
    void on_bt_personmgr_clicked();
	void on_bt_studymgr_clicked();
	//void on_bt_exammgr_clicked();
	void on_bt_knowledgemgr_clicked();
	void on_bt_datacenter_clicked();
	void on_bt_traincenter_clicked();
	void on_bt_personcenter_clicked();
	void on_bt_networkCenter_clicked();
	void on_bt_theoryTestmgr_clicked();
	
private:
	Ui::SYAdminPortalWindow ui;
	QString m_userName;
	QString m_realName;
	UserPermission m_permission;
	std::multimap<UserPermission, QString> m_mapLoginModule;
	QVector<QPushButton*> m_buttons;
};
