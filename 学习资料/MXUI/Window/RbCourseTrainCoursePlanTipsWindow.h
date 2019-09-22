#pragma once
#include "ui_RbCourseTrainCoursePlanTipsWindow.h"
#include "RbShieldLayer.h"

//个人中心提示窗口
class  RbCourseTrainCoursePlanTipsWindow: public RbShieldLayer
{
	Q_OBJECT
public:
	RbCourseTrainCoursePlanTipsWindow(QWidget *parent, QPoint posBtnEdit, QPoint posBtnAdd,QPoint posBtnBatchEdit);
	~RbCourseTrainCoursePlanTipsWindow();
signals: 
	void tipsWindowClose();											//提示窗口关闭的信号
protected slots:
	void onBtnOK();													//我知道了按钮槽函数
private:
	Ui::RbCourseTrainCoursePlanTipsWindow ui;
};