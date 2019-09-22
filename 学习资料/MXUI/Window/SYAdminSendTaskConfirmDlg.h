#ifndef _SYAdminSendTaskConfirmDlg_H
#define _SYAdminSendTaskConfirmDlg_H
#include "ui_SYAdminSendTaskConfirmDlg.h"
#include "RbShieldLayer.h"
#include <QVector>
#include <QPushButton>
#include "SYTrainTaskStruct.h"

class  SYAdminSendTaskConfirmDlg : public RbShieldLayer
{
	Q_OBJECT

public:

	SYAdminSendTaskConfirmDlg(QWidget *parent , QStringList & tasknamelist , QStringList & receivernamelist);
	
	~SYAdminSendTaskConfirmDlg();


private slots:

    void on_bt_cancel_clicked();

	void on_bt_conform_clicked();

private:



	Ui::SYAdminSendTaskConfirmDlg  ui;

	int m_CurrentGroupID;
	QString m_GroupName;

	int m_TrainChoosed;
};

#endif