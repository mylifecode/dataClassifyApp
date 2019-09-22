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
	void messageBoxLikeTipWindowClose(int);								//�ر��ź�,0��ʾȡ��,1��ʾȷ��
protected slots:
	void onCancelBtn();													//ȡ��
	void onOKBtn();														//ȷ��
private:
	Ui::RbMessageBoxLikeTipWindow ui;
};

