#ifndef _SYTRAINTASKSTRUCT_H
#define _SYTRAINTASKSTRUCT_H
#include "ui_SYModifyTrainTaskWindow.h"
#include "RbShieldLayer.h"
#include <QVector>
#include <QPushButton>
#include <QString>
#include <QDate>
#include "XMLSerialize.h"
#include "XMLWrapperGlobalTraining.h"

class TrainCanBeAssign
{
public:
	TrainCanBeAssign()
	{

	}
	TrainCanBeAssign(const QString & sheetCoded, const QString & runName, const QString &showName, const QString & mainCat, const QString & subCat)
	{
		m_SheetCode = sheetCoded;
		m_RunName = runName;
		m_ShowName = showName;
		m_MainCategory = mainCat;
		m_SubCategory = subCat;
	}
	QString m_SheetCode;
	QString m_RunName;
	QString m_ShowName;
	QString m_MainCategory;
	QString m_SubCategory;
};

class TaskTrainUnit
{
public:
	TaskTrainUnit(int DataBaseID , const QString & scorecode, int index)
	{
		m_DataBaseID = DataBaseID;
		m_ScoreCode = scorecode;
		m_IndexInAllTrain = index;
	}

	void SetData(int PassNeedScore, int NeedPassTimes, int TotalTrainedTime, int CurrPassedTime, int State)
	{
		m_PassNeedScore = PassNeedScore;
		m_NeedPassTimes = NeedPassTimes;
		m_TotalTrainedTime = TotalTrainedTime;
		m_CurrPassedTimes = CurrPassedTime;
		m_State = State;
	}

	int m_DataBaseID;
	QString m_ScoreCode;
	int m_IndexInAllTrain;

	//
	int m_PassNeedScore;//训练及格需要分数
	int m_NeedPassTimes;//训练需要及格次数
	
	int m_TotalTrainedTime;//hours

	int m_CurrPassedTimes;
	int m_State;
};


class  SYTaskWork
{
public:
	SYTaskWork(int ID,
		       const QString & name, 
			   const QString & tag,
		       const QString & assignor,
			   const QString & receiver, 
			   int assignorid,
			   int receiverid,
			   int TaskTimeDay,
			   QDate & sendData,
			   int state);

	int AddTrainUnit(int id, const QString & scorecode, int PassScore, int PassTimes);

	std::vector<TaskTrainUnit> m_TrainUnits;

	int m_ID;
	QString m_TaskName;
	QString m_TaskTag;

	QString m_Assignor;
	QString m_Receiver;

	int m_Assignorid;
	int m_Receiverid;
	bool    m_IsTemplate;
	QDate   m_SendDate;
	bool    m_IsFinished;
	int     m_State;//0--未开始 1--进行中 2 已完成
	int     m_TaskTimeDay;
	QString m_StateStr;
};

class SYTaskTrainDataMgr
{
public:
	static SYTaskTrainDataMgr & GetInstance()
	{
		return s_Instance;
	}

	SYTaskTrainDataMgr();

	~SYTaskTrainDataMgr();

	void Initialize();

	void PullAllTrainsCanBeAssignToTask();

	void PullAllTasksBelongsToReceiver(int receiverID, std::vector<SYTaskWork> & allWorks);

	void PullAllTaskTemplatesBelongToAssignor(int assignorID, std::vector<SYTaskWork> & allTasks);

	void PullAllTrainsBelongsToTask(SYTaskWork & task);

	bool GetTrainInforBySheetCode(const QString & sheetCode , TrainCanBeAssign & trainInfo);

	std::map<QString, TrainCanBeAssign> m_TransCanBeAssign;

	void SetSelectedTaskID(int id)
	{
		m_SelectedTrainID = id;
	}

	int GetSelectedTaskID()
	{
		return m_SelectedTrainID;
	}
protected:
	
	TrainCanBeAssign m_ErrorTrain;

	static SYTaskTrainDataMgr s_Instance;

	CXMLContentManager  m_XMLContentManager;

	CXMLWrapperGlobalTraining * m_GlobalTraining;

	int m_SelectedTrainID;//TEMP FOR USE between page
};
#endif