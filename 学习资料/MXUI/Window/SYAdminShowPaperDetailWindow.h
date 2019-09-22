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
	void  Initialize();  //从数据库中读取数据


private:
    Ui::SYAdminShowPaperDetailWindow ui;
	QString cur_PaperName;
	QVector<QSqlRecord> results;
signals:
	//void sendHideBackgroundWin();  //隐藏虚化窗口
private slots:

	void on_closeBtn_clicked();
};


