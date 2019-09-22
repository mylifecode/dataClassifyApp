#pragma once
#include "ui_RbMonitorTipsWindow.h"
#include "RbShieldLayer.h"

//个人中心提示窗口
class  RbMonitorTipsWindow: public RbShieldLayer
{
	Q_OBJECT
public:
	RbMonitorTipsWindow(QWidget *parent);
	~RbMonitorTipsWindow();
signals: 
	protected slots:
		void onBtnOK();												//我知道了按钮槽函数
private:
	Ui::RbMonitorTipsWindow ui;

};