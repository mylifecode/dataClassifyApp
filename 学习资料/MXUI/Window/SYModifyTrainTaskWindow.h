#ifndef _SYModifyTrainTaskWindow_H
#define _SYModifyTrainTaskWindow_H
#include "ui_SYModifyTrainTaskWindow.h"
#include "RbShieldLayer.h"
#include <QVector>
#include <QPushButton>
#include "SYTrainTaskStruct.h"

class  SYModifyTrainTaskWindow : public RbShieldLayer
{
	Q_OBJECT

public:

	class TrainCanBeAssignInstance : public TrainCanBeAssign
	{
	public:
		TrainCanBeAssignInstance(const TrainCanBeAssign & train , int myindex) : TrainCanBeAssign(train)
		{
			m_NeedPassTimes = 1;
			m_PassNeedScore = 60;
			m_MyIndex = myindex;
		}
		int m_NeedPassTimes;
		int m_PassNeedScore;
		int m_MyIndex;
	};

	class SubTypeTrains
	{
	public:
		SubTypeTrains(const TrainCanBeAssignInstance & unit)
		{
			m_SubType = unit.m_SubCategory;
			m_Trains.push_back(unit);
		}
		std::vector<TrainCanBeAssignInstance> m_Trains;
		QString m_SubType;
	};

	
	SYModifyTrainTaskWindow(QWidget *parent);
	
	~SYModifyTrainTaskWindow();


private slots:

    void onTreeWidgetItemClicked(QTreeWidgetItem* item, int column);

	void on_bt_add_clicked();

	void on_bt_conform_clicked();

	void on_bt_cancel_clicked();

private:

	void PullAllTrains();

	void RefreshFilteredTrainList(const std::vector<SubTypeTrains> & subTypeTrains);

	void FilterTrainsInType(std::vector<SubTypeTrains> & result , const QString & trainType);

	void RefreshSelectedTrainList();

	std::vector<TrainCanBeAssignInstance> m_AllTrains;//pull from data base

	std::vector<TrainCanBeAssignInstance> m_TrainSelected;

	Ui::SYModifyTrainTaskWindow  ui;

	int m_CurrentGroupID;
	QString m_GroupName;

	int m_TrainChoosed;
};

#endif