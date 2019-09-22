#pragma once
#include"RbShieldLayer.h"
#include <QDialog>
#include "ui_SYAddModuleWindow.h"
#include<qicon.h>
class SYAddModuleWindow : public RbShieldLayer
{
    Q_OBJECT

public:
    explicit SYAddModuleWindow(QWidget *parent = 0);
    ~SYAddModuleWindow();
	void choosedModueIcon();
signals:
	void on_confirm_Btn_Clicked(QString iconPath, QString &moduleName);

private:
    Ui::SYAddModuleWindow ui;
	QString iconPath;
};


