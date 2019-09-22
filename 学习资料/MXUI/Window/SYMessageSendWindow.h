#pragma once
#include <RbShieldLayer.h>
#include "ui_SYMessageSendWindow.h"
#include <vector>

class SYUserListWindow;

class SYMessageSendWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	SYMessageSendWindow(QWidget* parent = nullptr);

	~SYMessageSendWindow();

	void SetReceiveGroup(int groupId);

private slots:
	void on_userListBtn_clicked();

	void on_cancelBtn_clicked();

	void on_okBtn_clicked();

protected:
	void showEvent(QShowEvent* event);

private:
	Ui::MessageSendWindow ui;

	int m_groupId;
	SYUserListWindow* m_userListWindow;
	std::vector<int> m_receiverIds;
};

