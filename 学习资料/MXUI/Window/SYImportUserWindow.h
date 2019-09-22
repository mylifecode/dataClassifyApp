#pragma once
#include <RbShieldLayer.h>
#include "ui_SYImportUserWindow.h"

enum UserPermission;

class SYImportUserWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	SYImportUserWindow(QWidget* parent = nullptr);

	~SYImportUserWindow();

	void setUserPermission(UserPermission permission);

protected:
	void showEvent(QShowEvent* event);

private slots:
	void on_importUserBtn_clicked();

	void on_okBtn_clicked();

	void on_cancelBtn_clicked();

private:
	Ui::importUserWindow ui;

	UserPermission m_permission;

	QString m_fileName;
};

