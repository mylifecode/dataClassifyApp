#pragma once
#include "ui_RbMessageBoxLikeTipWindow.h"
#include "RbShieldLayer.h"

#define CANCEL_PRESSED	0
#define CONFIRM_PRESSED	1



class RbMessageBoxLikeTipWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	RbMessageBoxLikeTipWindow(QWidget *parent);
	~RbMessageBoxLikeTipWindow(void);
	void setTitle(const QString& strTitle);
signals:
	void messageBoxLikeTipWindowClose(int);								//关闭信号,0表示取消,1表示确定
protected slots:
	void onCancelBtn();													//取消
	void onOKBtn();														//确定
private:
	Ui::RbMessageBoxLikeTipWindow ui;
};

