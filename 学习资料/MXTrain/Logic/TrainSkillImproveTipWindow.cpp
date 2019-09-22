#include "TrainSkillImproveTipWindow.h"
#include "ui_TrainSkillImproveTipWindow.h"
#include<QPair>
#include<QVector>
#include"MxDefine.h"


TrainSkillImproveTipWindow::TrainSkillImproveTipWindow(QWidget *parent) :
RbShieldLayer(parent),
    ui(new Ui::TrainSkillImproveTipWindow)
{
    ui->setupUi(this);
	
	hideOkButton();
	hideCloseButton();


	ui->frame_1->setVisible(false);
	 ui->frame_1->setProperty("level",1);
	 ui->frame_2->setVisible(false);
	 ui->frame_2->setProperty("level",2);
	 ui->frame_3->setVisible(false);
	 ui->frame_3->setProperty("level", 3);

	Mx::setWidgetStyle(this,"qss:TrainSkillImproveTipWindow.qss");
}

void TrainSkillImproveTipWindow::setBtnVisble(QVector<QPair<QString,QString>> &trainBtnInfos)
{

    for(auto&trainBtnInfo:trainBtnInfos)
    {
        int level=(trainBtnInfo.second).toInt();
        if(level==1)
        {
			ui->frame_1->setVisible(true);

        }
        if(level==2)
        {
			ui->frame_2->setVisible(true);

        }
        if(level==3)
        {
			ui->frame_3->setVisible(true);

        }
    }
}

void TrainSkillImproveTipWindow::setTrainLevelState(QVector<QPair<QString,bool>> trainLevelInfos)
{

    for( auto& trainLevelInfo:trainLevelInfos)
    {
        int level=(trainLevelInfo.first).toInt();
		bool ret = trainLevelInfo.second;
        if(level==1)
        {

			if (ret)

				 ui->level_1_Btn->setStyleSheet(QString("border-image:url(icons:/TrainSkillImproveTipWindow/level1.png)"));
			else
				ui->level_1_Btn->setStyleSheet(QString("border-image:url(icons:/TrainSkillImproveTipWindow/level1disabled.png)"));
			ui->frame_1->setStyleSheet(QString("background-color:yellow;border:none;"));
        }
        if(level==2)
        {
			if (ret)
			  ui->level_2_Btn->setStyleSheet(QString("border-image:url(icons:/TrainSkillImproveTipWindow/level2.png)"));
			else
				ui->level_2_Btn->setStyleSheet(QString("border-image:url(icons:/TrainSkillImproveTipWindow/level2disabled.png)"));
			ui->frame_2->setStyleSheet(QString("background-color:yellow;border:none;"));
        }
        if(level==3)
        {
			if (ret)
				 ui->level_3_Btn->setStyleSheet(QString("border-image:url(icons:/TrainSkillImproveTipWindow/level3.png)"));
			else
				ui->level_3_Btn->setStyleSheet(QString("border-image:url(icons:/TrainSkillImproveTipWindow/level3disabled.png)"));
			ui->frame_3->setStyleSheet(QString("background-color:yellow;border:none;"));
        }
    }

}


TrainSkillImproveTipWindow::~TrainSkillImproveTipWindow()
{
    delete ui;
}


void TrainSkillImproveTipWindow::timeOut()
{

	done(RC_Ok);
}

