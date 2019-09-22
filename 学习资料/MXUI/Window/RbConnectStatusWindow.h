#pragma once
#include <QWidget>
#include "ui_RbConnectStatusWindow.h"
#include <QMap>
#include <stdint.h>

class RbConnectStatusWindow : public QWidget
{
	Q_OBJECT
public:
	RbConnectStatusWindow(QWidget* parent = nullptr);
	~RbConnectStatusWindow();

	void AddUser(uint32_t id);

	void RemoveUser(uint32_t id);

	void UpdateUserInfo(uint32_t id, const QString& userName, const QString& realName);

private slots:
	void on_statusBtn_clicked();
	void on_closeBtn_clicked();

private:
	Ui::ConnectStatusWindow m_ui;

	struct UserInfo
	{
		uint32_t id;
		uint32_t showIndex;
		QString info;
	};

	QMap<uint32_t, UserInfo> m_userInfoMap;
	uint32_t m_nextShowIndex;
	const uint32_t m_nCol;
};

