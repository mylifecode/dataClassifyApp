#pragma once
#include <QWidget>
#include <RbShieldLayer.h>
#include "ui_SYAddUserWindow.h"

enum UserPermission;

class SYAddUserWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	SYAddUserWindow(QWidget* parent = nullptr);

	~SYAddUserWindow();


	/** 只能设置为UP_Student or UP_Teacher */
	void setUserPermission(UserPermission permission);

protected:
	void showEvent(QShowEvent* event);

private slots:
	void on_cancelBtn_clicked();

	void on_okBtn_clicked();

private:
	Ui::SYAddUserWindow ui;

	UserPermission m_permission;
};

