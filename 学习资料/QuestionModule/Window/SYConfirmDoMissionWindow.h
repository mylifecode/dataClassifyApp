#pragma once
#include"RbShieldLayer.h"
#include <QDialog>
#include "ui_SYConfirmDoMissionWindow.h"
namespace Ui {
class SYConfirmDoMissionWindow;
}

class SYConfirmDoMissionWindow : public RbShieldLayer
{
    Q_OBJECT

public:
    explicit SYConfirmDoMissionWindow(int num,int examTime,QWidget *parent = 0);
    ~SYConfirmDoMissionWindow();

private slots:
	void on_btn_clicked();

private:
    Ui::SYConfirmDoMissionWindow ui;
};


