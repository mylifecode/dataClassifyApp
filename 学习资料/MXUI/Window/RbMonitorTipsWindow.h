#pragma once
#include "ui_RbMonitorTipsWindow.h"
#include "RbShieldLayer.h"

//����������ʾ����
class  RbMonitorTipsWindow: public RbShieldLayer
{
	Q_OBJECT
public:
	RbMonitorTipsWindow(QWidget *parent);
	~RbMonitorTipsWindow();
signals: 
	protected slots:
		void onBtnOK();												//��֪���˰�ť�ۺ���
private:
	Ui::RbMonitorTipsWindow ui;

};