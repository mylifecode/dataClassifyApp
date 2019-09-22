#include "SYTrainTaskStruct.h"
#include "SYDBMgr.h"
#include "SYStringTable.h"
#include "XMLWrapperTraining.h"
SYTaskTrainDataMgr SYTaskTrainDataMgr::s_Instance;
SYTaskWork::SYTaskWork(int ID,
	                   const QString & name,
					   const QString & tag,
	                   const QString & assignor, 
					   const QString & receiver, 
					   int assignorid,
					   int receiverid,
					   int TaskTimeDay,
					   QDate & sendData,
					   int state)
{
	m_ID = ID;
	m_TaskName = name;
	m_TaskTag  = tag;
	m_Assignor = assignor;
	m_Receiver = receiver;
	m_SendDate = sendData;
	m_IsTemplate = false;
	m_IsFinished = false;

	m_Assignorid = assignorid;
	m_Receiverid = receiverid;
	m_TaskTimeDay = TaskTimeDay;
	m_State = state;

	m_StateStr = SYStringTable::GetString(SYStringTable::STR_NOTBEGIN);

	if (m_State == 1)
	{
		m_StateStr = SYStringTable::GetString(SYStringTable::STR_INPROCESS);
	}
	else if (m_State == 2)
	{
		m_StateStr = SYStringTable::GetString(SYStringTable::STR_FINISH);
	}
}

int SYTaskWork::AddTrainUnit(int id, const QString & scorecode, int PassNeedScore, int NeedPassTimes)
{
	TaskTrainUnit trainunit(id, scorecode, m_TrainUnits.size());
	trainunit.m_NeedPassTimes = NeedPassTimes;
	trainunit.m_PassNeedScore = PassNeedScore;
	m_TrainUnits.push_back(trainunit);

	return (int)m_TrainUnits.size() - 1;
}

SYTaskTrainDataMgr::SYTaskTrainDataMgr()
{
	m_GlobalTraining = 0;
	m_SelectedTrainID = -1;
}

SYTaskTrainDataMgr::~SYTaskTrainDataMgr()
{
	if (m_GlobalTraining)
	{
		delete m_GlobalTraining;
		m_GlobalTraining = 0;
	}
}

void SYTaskTrainDataMgr::Initialize()
{
	if (m_GlobalTraining == 0)
	    m_GlobalTraining = dynamic_cast<CXMLWrapperGlobalTraining *>(m_XMLContentManager.Load(MULTIPORT_GLOBAL_CONFIG_FILE));

	PullAllTrainsCanBeAssignToTask();
}

void SYTaskTrainDataMgr::PullAllTrainsCanBeAssignToTask()
{
	std::vector<QSqlRecord> records;
	SYDBMgr::Instance()->QueryAllTrainsCanBeAssignedToTask(records);

	for (int c = 0; c < (int)records.size(); c++)
	{
		QSqlRecord & record = records[c];

		QString sheetCode = record.value("SheetCode").toString();
	
		for (int c = 0; c < m_GlobalTraining->m_Trainings.size(); c++)
		{
			CXMLWrapperTraining * trainConfig = m_GlobalTraining->m_Trainings[c];
			
			QString trainSheetcode = QString::fromUtf8(trainConfig->m_SheetCode.c_str());

			if (trainSheetcode == sheetCode)
			{
				QString trainShowName = QString::fromUtf8(trainConfig->m_ShowName.c_str());
				QString trainRunName = QString::fromUtf8(trainConfig->m_Name.c_str());
				QString trainMainCatogery = SYStringTable::GetString(SYStringTable::STR_TRAINCATEGORYVIRTUALSKIILL);////QString::fromUtf8(trainConfig->m_MainCatogery.c_str());
				
				QChar trainType = sheetCode[2];
				
				if (trainType == QChar('0'))
					trainMainCatogery = SYStringTable::GetString(SYStringTable::STR_TRAINCATEGORYREALOBJECTTRAIN);

				else if (trainType == QChar('1'))
					trainMainCatogery = SYStringTable::GetString(SYStringTable::STR_TRAINCATEGORYVIRTUALSKIILL);
				
				else
					trainMainCatogery = SYStringTable::GetString(SYStringTable::STR_TRAINCATEGORYVIRTUALSURGEY);

				QString trainSubCatogery = QString::fromUtf8(trainConfig->m_SubCatogery.c_str());

				TrainCanBeAssign train(trainSheetcode,trainRunName,
					                   trainShowName,trainMainCatogery,
					                   trainSubCatogery);

				m_TransCanBeAssign.insert(std::make_pair(sheetCode, train));

			}
		}
	}
}
void SYTaskTrainDataMgr::PullAllTasksBelongsToReceiver(int receiverID, std::vector<SYTaskWork> & allWorks)
{
	std::vector<QSqlRecord> records;
	SYDBMgr::Instance()->QueryAllReceiverTasks( receiverID, records);

	for (int c = 0; c < (int)records.size(); c++)
	{
		QSqlRecord & record = records[c];
		
		int taskid = record.value("id").toInt();
		int ReceiverID   = record.value("ReceiverID").toInt();
		int AsssignorID  = record.value("AssignorID").toInt();
		
		int taskTimeDay = record.value("TaskTime").toInt();

		QString taskName = record.value("TaskName").toString();
		QString taskTag = record.value("taskTag").toString();
		QString assignorName = record.value("AssignorName").toString();
		QString receiverName = record.value("ReceiverName").toString();
		QDate   date     = record.value("SenderData").toDate();
		int State = record.value("State").toInt();
		
		SYTaskWork task(taskid, taskName, taskTag ,assignorName, receiverName, AsssignorID, receiverID, taskTimeDay, date, State);
		allWorks.push_back(task);
	}
}

void SYTaskTrainDataMgr::PullAllTaskTemplatesBelongToAssignor(int assignorID, std::vector<SYTaskWork> & allTasks)
{
	std::vector<QSqlRecord> records;

	SYDBMgr::Instance()->QueryAllAssignorTaskTemplate(-1, records);

	for (std::size_t i = 0; i < records.size(); ++i){

		QSqlRecord& record = records[i];

		int id = record.value("id").toInt();

		QString taskName = record.value("TaskName").toString();

		QString taskTag = record.value("TaskTag").toString();

		QDate   taskDate = record.value("SenderData").toDate();// .toString(SYStringTable::GetString(SYStringTable::STR_DATEFORMAT));

		QString assignor = record.value("AssignorName").toString();

		int assignorid = record.value("AssignorID").toInt();

		allTasks.push_back(SYTaskWork(id, taskName, taskTag, assignor, "", assignorid, -1, 0, taskDate, 0));
	}
}
void SYTaskTrainDataMgr::PullAllTrainsBelongsToTask(SYTaskWork & task)
{
	if (task.m_TrainUnits.size() == 0)//already pulled
	{
		std::vector<QSqlRecord> records;

		SYDBMgr::Instance()->QueryAllTrainsInOneTaks(task.m_ID, records);

		for (int c = 0; c < (int)records.size(); c++)
		{
			QSqlRecord & record = records[c];

			int id = record.value("id").toInt();
			//QString trainName = record.value("trainname").toString();
			QString scoreCode = record.value("traingradecode").toString();
			int isfinished    = record.value("isfinished").toInt();
			int PassNeedScore     = record.value("passneedscore").toInt();
			int NeedPassTimes = record.value("passneedtimes").toInt();
			
			int CurrPasstimes = record.value("currpasstimes").toInt();
			int TrainTimeUsed = record.value("traintimeused").toInt();
			
			int index = task.AddTrainUnit(id, scoreCode, PassNeedScore, NeedPassTimes);
			
			task.m_TrainUnits[index].m_CurrPassedTimes  = CurrPasstimes;
			task.m_TrainUnits[index].m_TotalTrainedTime = TrainTimeUsed;
		}
	}
}

bool SYTaskTrainDataMgr::GetTrainInforBySheetCode(const QString & sheetCode , TrainCanBeAssign & trainInfo)
{
	std::map<QString, TrainCanBeAssign>::iterator itor = m_TransCanBeAssign.find(sheetCode);
	if (itor != m_TransCanBeAssign.end())
	{
		trainInfo = itor->second;
		return true;
	}
	else
	{
		return false;
	}
}