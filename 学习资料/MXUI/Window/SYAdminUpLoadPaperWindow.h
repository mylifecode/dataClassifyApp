#pragma once
#include "ui_SYAdminUpLoadPaperWindow.h"
#include"RbShieldLayer.h"
#include <QDialog>


class SYAdminUpLoadPaperWindow : public RbShieldLayer
{
    Q_OBJECT

public:
    explicit SYAdminUpLoadPaperWindow(QWidget *parent = 0);
    ~SYAdminUpLoadPaperWindow();

private slots:
	void on_addFile_Btn_Clicked();
	void on_cancel_Btn_Clicked();
	void on_confirm_Btn_Clicked();

private:
    Ui::SYAdminUpLoadPaperWindow ui;
	QString m_fileName;
};


