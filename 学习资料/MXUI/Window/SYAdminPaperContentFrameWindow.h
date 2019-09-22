#pragma once

#include "ui_SYAdminPaperContentFrameWindow.h"
#include <QWidget>
#include<QSqlRecord>

class SYAdminPaperContentFrameWindow : public QWidget
{
    Q_OBJECT

public:
	SYAdminPaperContentFrameWindow(QSqlRecord record, int index, QWidget *parent = 0);
	~SYAdminPaperContentFrameWindow();
	void Initalize( );

private:
    Ui::SYAdminPaperContentFrameWindow ui;
	QSqlRecord cur_record;
	int cur_index;
	bool chosed;
};


