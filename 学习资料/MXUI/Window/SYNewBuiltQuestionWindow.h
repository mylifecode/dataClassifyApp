#pragma once
#include "ui_SYNewBuiltQuestionWindow.h"
#include"RbShieldLayer.h"
#include <QDialog>
#include<qsqlrecord.h>

class SYNewBuiltQuestionWindow : public RbShieldLayer
{
    Q_OBJECT

public:
    explicit SYNewBuiltQuestionWindow(int type,QSqlRecord* record=0, QWidget *parent = 0);
    ~SYNewBuiltQuestionWindow();
	void InitPage();
	bool VerifyOption(QString questionTitle, QString optA, QString optB, QString optC, QString optD, QString optE, QString answer);

private slots:
	void do_btn_Clicked();

private:
    Ui::SYNewBuiltQuestionWindow ui;
	int m_curType;
	QSqlRecord* m_record;
};

