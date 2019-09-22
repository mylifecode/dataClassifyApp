#pragma once
#include "ui_SYAdminPaperRandomChooseWindow.h"
#include <QDialog>
#include<qsqlrecord.h>
#include"RbShieldLayer.h"
#include "SYMainWindow.h"

class SYAdminPaperRandomChooseWindow : public RbShieldLayer
{
    Q_OBJECT

public:
	explicit SYAdminPaperRandomChooseWindow(QVector<QVector<QSqlRecord>> all_records, QWidget *parent = 0);
    ~SYAdminPaperRandomChooseWindow();

	void Initialize();

private slots:
	void on_Btn_click();

signals:
	void ReturnWin();
private:
    Ui::SYAdminPaperRandomChooseWindow ui;
	QVector<QVector<QSqlRecord>> m_curRecords;
};


