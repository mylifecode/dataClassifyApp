#pragma once
#include "ui_RbAllocCoursePlanWindow.h"
#include "RbShieldLayer.h"

#define CANCEL_ALLOC	0	
#define CONFIRM_ALLOC	1

//课程计划信息
struct COURSEPLANINFO
{
	QString strPlanID;															//课程ID号
	QString strPlanName;														//课程计划名
	QSet<QString> strUserID;													//userid集合
	QSet<QString> strTrainName;													//训练名集合
};

class RbAllocCoursePlanWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	RbAllocCoursePlanWindow(QWidget *parent, QVector<QString> vecPlanID, QList<COURSEPLANINFO> listCoursePlanInfo = QList<COURSEPLANINFO>());
	~RbAllocCoursePlanWindow(void);
signals:
	void allocPlanWindowClose(int,QString);										//关闭信号,0表示取消,1表示确定
protected slots:
	void onCancelBtn();
	void onOkButton();
private:
	Ui::RbAllocCoursePlanWindow ui;
	QVector<QString> m_vecPlanID;												//待分配计划的课程计划名

	QList<int> m_listGrayRowInStudentTableWidget;								//studentTableWdiget中灰色的行
	QList<int> m_listGrayRowInGroupTableWidget;									//groupTableWdiget中灰色的行

	QList<COURSEPLANINFO> m_listCoursePlanInfo;									//所有的课程计划信息

	QMap<QString, int> m_mapUserID2Count;										//用户与已分配数量的映射
	QMap<QString, QString> m_mapUserID2UserName;								//用户ID与用户名的映射
	QMultiMap<QString, QString> m_mapGroupID2UserID;							//小组ID与用户ID的映射

};

