#pragma once
#include <QDialog>
#include "ui_SYAdminPaperInfoEditWindow.h"
#include"RbShieldLayer.h"
#include"SYDBMgr.h"

class SYAdminPaperInfoEditWindow : public RbShieldLayer
{
    Q_OBJECT

public:
	explicit SYAdminPaperInfoEditWindow(int chosedNum,QWidget *parent = 0, PaperInfoDetail* paperInfoDetail=0);
    ~SYAdminPaperInfoEditWindow();
	void SetPage();

signals:
	void createPaper(QString tableName, QString examTime, QString  examScore);

public slots:
	void on_Btn_clicked();
private:
    Ui::SYAdminPaperInfoEditWindow ui;
	PaperInfoDetail* m_paperInfoDetail;
};


