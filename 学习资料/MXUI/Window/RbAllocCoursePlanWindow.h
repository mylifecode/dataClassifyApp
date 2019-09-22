#pragma once
#include "ui_RbAllocCoursePlanWindow.h"
#include "RbShieldLayer.h"

#define CANCEL_ALLOC	0	
#define CONFIRM_ALLOC	1

//�γ̼ƻ���Ϣ
struct COURSEPLANINFO
{
	QString strPlanID;															//�γ�ID��
	QString strPlanName;														//�γ̼ƻ���
	QSet<QString> strUserID;													//userid����
	QSet<QString> strTrainName;													//ѵ��������
};

class RbAllocCoursePlanWindow : public RbShieldLayer
{
	Q_OBJECT
public:
	RbAllocCoursePlanWindow(QWidget *parent, QVector<QString> vecPlanID, QList<COURSEPLANINFO> listCoursePlanInfo = QList<COURSEPLANINFO>());
	~RbAllocCoursePlanWindow(void);
signals:
	void allocPlanWindowClose(int,QString);										//�ر��ź�,0��ʾȡ��,1��ʾȷ��
protected slots:
	void onCancelBtn();
	void onOkButton();
private:
	Ui::RbAllocCoursePlanWindow ui;
	QVector<QString> m_vecPlanID;												//������ƻ��Ŀγ̼ƻ���

	QList<int> m_listGrayRowInStudentTableWidget;								//studentTableWdiget�л�ɫ����
	QList<int> m_listGrayRowInGroupTableWidget;									//groupTableWdiget�л�ɫ����

	QList<COURSEPLANINFO> m_listCoursePlanInfo;									//���еĿγ̼ƻ���Ϣ

	QMap<QString, int> m_mapUserID2Count;										//�û����ѷ���������ӳ��
	QMap<QString, QString> m_mapUserID2UserName;								//�û�ID���û�����ӳ��
	QMultiMap<QString, QString> m_mapGroupID2UserID;							//С��ID���û�ID��ӳ��

};

