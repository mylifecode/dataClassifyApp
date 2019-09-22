#include "..\include\SYScoreTableManager.h"
#include <xstddef>
#include "SYScoreTable.h"
#include "SYDBMgr.h"
#include <QVector>
#include <QSqlRecord>
#include <QMap>
#include "SYUserData.h"



SYScoreTableManager::SYScoreTableManager()
{
	SYDBMgr::Instance()->Instance()->Open();
}

SYScoreTableManager::~SYScoreTableManager()
{
	for(auto scoreTable : m_scoreTables)
		delete scoreTable;
}

SYScoreTableManager* SYScoreTableManager::GetInstance()
{
	static SYScoreTableManager object;

	return std::addressof(object);
}

void SYScoreTableManager::Initialize()
{
	QVector<QSqlRecord> records;
	SYDBMgr::Instance()->QueryAllScoreTableItem(records);

	QString lastScoreTableCode,curScoreTableCode;
	SYScoreTable* scoreTable = nullptr;
	QMap<QString, SYScoreTable*> scoreTableMap;
	//init top level user data
	QVector<std::shared_ptr<SYUserData>> topLevelUserDatas;
	topLevelUserDatas.push_back(std::shared_ptr<SYUserData>(new SYUserData("0")));	//ʰ��ѵ��
	topLevelUserDatas.push_back(std::shared_ptr<SYUserData>(new SYUserData("1")));	//���⼼��ѵ��
	topLevelUserDatas.push_back(std::shared_ptr<SYUserData>(new SYUserData("2")));	//����ѵ��

	for(auto record : records){
		curScoreTableCode = record.value("scoreTableCode").toString();

		if(curScoreTableCode != lastScoreTableCode){
			auto itr = scoreTableMap.find(curScoreTableCode);
			if(itr == scoreTableMap.end()){
				scoreTable = CreateTable();

				//1 set top level user data . 
				//scoreTable->SetTopLevelUserData(record.value("trainTypeCode").toString());	//�������ûᵼ���ж��top level���ݶ��󣬶���������
				int index = record.value("trainTypeCode").toInt(); 
				if(index < 0 || index >= topLevelUserDatas.size())
					throw "error train type code";
				scoreTable->SetTopLevelUserData(topLevelUserDatas[index]);
				//2 set train data
				scoreTable->SetTrainData(record.value("trainName").toString(), record.value("trainCode").toString());

				// save score table object to the template map
				scoreTableMap.insert(curScoreTableCode, scoreTable);
			}
			else
				scoreTable = itr.value();

			lastScoreTableCode = curScoreTableCode;
		}
		
		//3 add step data
		SYStepData* stepData = scoreTable->AddStepData(record.value("stepName").toString(),
													   record.value("stepCode").toString(),
													   record.value("stepDetailedDescritpion").toString(),
													   record.value("isKeyStep").toInt());
		Q_ASSERT(stepData && "add step data fail");

		//4 add score item data
		SYScoreItemData* scoreItemData = scoreTable->AddScoreItemData(stepData,
																	  record.value("scoreItemContent").toString(),
																	  record.value("scoreItemCode").toString(),
																	  record.value("scoreItemValue").toInt());
		Q_ASSERT(scoreItemData && "add score item data fail");

		//5 add score point detail data
		SYScorePointDetailData* spdd = scoreTable->AddScorePointDetailData(scoreItemData,
																		   record.value("scorePointDetailName").toString(),
																		   record.value("scorePointDetailCode").toString(),
																		   record.value("scorePointDetailValue").toInt(),
																		   record.value("abilitys").toInt());
		Q_ASSERT(spdd && "add spdd data fail");
		spdd->SetFlag(1);	//���������Ǵ����ݿ��е��룬����ͨ���û�����
	}

	//update ability counter
	for(auto itr : scoreTableMap){
		itr->UpdateAbilityCounters();
		//m_scoreTables.push_back(itr);
	}
}

SYScoreTable* SYScoreTableManager::CreateTable()
{
	SYScoreTable* table = new SYScoreTable();
	m_scoreTables.push_back(table);

	return table;
}

SYScoreTable* SYScoreTableManager::GetTable(const QString& tableCode)
{
	for(auto scoreTable : m_scoreTables){
		if(scoreTable->GetCode() == tableCode)
			return scoreTable;
	}

	return nullptr;
}

void SYScoreTableManager::DeleteTable(SYScoreTable* table)
{
	bool ok = m_scoreTables.removeOne(table);
	if(ok)
		delete table;
}