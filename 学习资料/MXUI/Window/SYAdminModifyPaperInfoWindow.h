#pragma once 
#include "ui_SYAdminModifyPaperInfoWindow.h"
#include"SYAdminPaperContentFrameWindow.h"
#include"RbShieldLayer.h"
#include <QWidget>


class SYAdminModifyPaperInfoWindow : public RbShieldLayer
{
    Q_OBJECT

public:
    explicit SYAdminModifyPaperInfoWindow(QString paperName,QWidget *parent = 0);
    ~SYAdminModifyPaperInfoWindow();
	void Initialize();
	void RefreshPage();
	void SaveModifyPaperInfo();
	void RefreshTable();
	int GetItemChosedNumber();
private:
    Ui::SYAdminModifyPaperInfoWindow ui;
	QString m_curPaperName;

private slots:
	void on_delete_Btn_Clicked();
	void on_return_Btn_Clicked();

};

