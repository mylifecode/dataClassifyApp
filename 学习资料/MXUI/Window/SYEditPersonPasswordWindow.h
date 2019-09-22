#pragma once
#include "ui_SYEditPersonPasswordWindow.h"
#include "RbShieldLayer.h"

class  SYEditPersonPasswordWindow: public RbShieldLayer
{
	Q_OBJECT
public:
	SYEditPersonPasswordWindow(QWidget *parent);

	~SYEditPersonPasswordWindow();
protected:
	void showEvent(QShowEvent* event);

public slots:
	void on_maleBtn_clicked();
	void on_femaleBtn_clicked();
	void onClickedOK();												//确定按钮槽函数
	void onClickedCancel();											//取消按钮槽函数
private:
	Ui::EditPersonPasswordWindow ui;
	QIcon m_emptyIcon;
	QIcon m_selectedIcon;
	bool m_selectedMaleBtn;
};