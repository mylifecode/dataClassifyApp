#pragma once
#include "ui_RbCourseTrainCoursePlanTipsWindow.h"
#include "RbShieldLayer.h"

//����������ʾ����
class  RbCourseTrainCoursePlanTipsWindow: public RbShieldLayer
{
	Q_OBJECT
public:
	RbCourseTrainCoursePlanTipsWindow(QWidget *parent, QPoint posBtnEdit, QPoint posBtnAdd,QPoint posBtnBatchEdit);
	~RbCourseTrainCoursePlanTipsWindow();
signals: 
	void tipsWindowClose();											//��ʾ���ڹرյ��ź�
protected slots:
	void onBtnOK();													//��֪���˰�ť�ۺ���
private:
	Ui::RbCourseTrainCoursePlanTipsWindow ui;
};