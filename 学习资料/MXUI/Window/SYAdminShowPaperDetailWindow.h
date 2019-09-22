#pragma once
#include "ui_SYAdminShowPaperDetailWindow.h"
#include <QDialog>
#include<qvector.h>
#include<qsqlrecord.h>
#include"RbShieldLayer.h"

class SYAdminShowPaperDetailWindow : public RbShieldLayer
{
    Q_OBJECT

public:
     SYAdminShowPaperDetailWindow(QString PaperType, QWidget *parent = 0);
    ~SYAdminShowPaperDetailWindow();
	void  Initialize();  //�����ݿ��ж�ȡ����


private:
    Ui::SYAdminShowPaperDetailWindow ui;
	QString cur_PaperName;
	QVector<QSqlRecord> results;
signals:
	//void sendHideBackgroundWin();  //�����黯����
private slots:

	void on_closeBtn_clicked();
};


