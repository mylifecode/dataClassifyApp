#pragma once
#include "ui_SYUserListWindow.h"
#include <RbShieldLayer.h>
#include <QMap>
#include <QSqlRecord>
#include <vector>

class SYUserListWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	SYUserListWindow(QWidget* parent = nullptr);

	~SYUserListWindow();

	void SetGroupFilter(int groupId);

	const QMap<int, QString>& GetSelectedUserInfos() const { return m_userNameMap; }

private slots:
	void on_cancelBtn_clicked();

	void on_okBtn_clicked();

	void on_searchBtn_clicked();

	void onCheckBoxStateChanged(int state);

	void on_lineEdit_textChanged(const QString& text);

private:
	void setTableWidgetAttribute();

	void setTableWidgetContents(const std::vector<int>& recordIndexs);

protected:
	void showEvent(QShowEvent* event);

private:
	Ui::UserListWindow ui;

	QMap<int, QString> m_userNameMap;

	int m_groupId;
	std::vector<QSqlRecord> m_userDataRecords;
	std::vector<int> m_recordIndexs;
};

