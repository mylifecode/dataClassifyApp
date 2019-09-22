#include "..\include\SYScoreTable.h"
#include "SYUserData.h"
#include "SYDBMgr.h"
#include <QMessageBox>
#include <QDebug>
#include "SYScoreTableManager.h"

const QString SYScoreTable::ProductCode = "01";

const QString SYScoreTable::VersionCode = "01";

const int SYScoreTable::ScoreTableCodeLength = 8;

const int SYScoreTable::ScoreCodeLength = 10;

const int SYScoreTable::TrainCodeLength = 3;

const int SYScoreTable::StepCodeLength = 2;

const int SYScoreTable::ScoreItemCodeLength = 3;

const int SYScoreTable::ScorePointDetailCodeLength = 1;

const QString SYScoreTable::TrainAbilityNames[TA_NumOfAbility] = {
	QString::fromLocal8Bit("动作稳定性"),		//0
	QString::fromLocal8Bit("动作精确性"),		//1
	QString::fromLocal8Bit("镜下方向感"),		//2
	QString::fromLocal8Bit("器械使用"),			//3
	QString::fromLocal8Bit("双手协调配合"),		//4
	QString::fromLocal8Bit("手眼协调"),			//5
	QString::fromLocal8Bit("临床思维"),			//6
	QString::fromLocal8Bit("切割技巧"),			//7
	QString::fromLocal8Bit("分离技巧"),			//8
	QString::fromLocal8Bit("精细操作"),			//9
	QString::fromLocal8Bit("夹持稳定性"),		//10
	QString::fromLocal8Bit("止血技术"),			//11
	QString::fromLocal8Bit("结扎技术"),			//12
	QString::fromLocal8Bit("缝合技巧")
};


SYScoreTable::SYScoreTable()
	:m_topLevelUserData(nullptr),
	m_trainData(nullptr),
	m_emptyString("")
{
	memset(m_abilityCounters, 0, sizeof(int) * TA_NumOfAbility);
}

SYScoreTable::~SYScoreTable()
{
	//release memory
	//if(m_userData)
	//	delete m_userData;

	//if(m_trainData)
	//	delete m_trainData;

	for(auto stepData : m_stepDatas)
		delete stepData;

	for(auto scoreItemData : m_scoreItemDatas)
		delete scoreItemData;

	for(auto scorePointDetailData : m_scorePointDetailDatas)
		delete scorePointDetailData;
}

QString SYScoreTable::GetCode() const
{
	QString code;

	//训练类型编码存在且长度为1
	if(m_topLevelUserData == nullptr || m_topLevelUserData->Code().size() != 1)
		goto EXIT;

	//训练编码存在且长度为3
	if(m_trainData == nullptr || m_trainData->Code().size() != 3)
		goto EXIT;

	code += SYScoreTable::ProductCode;		//2 bit
	code += m_topLevelUserData->Code();		//1 bit
	code += m_trainData->Code();			//3 bit
	code += SYScoreTable::VersionCode;		//2 bit

EXIT:
	return code;
}

void SYScoreTable::SetTopLevelUserData(const std::shared_ptr<SYUserData>& userData)
{
	if(userData == nullptr)
		return;

	m_topLevelUserData = userData;
	if(m_trainData)
		m_trainData->SetParent(m_topLevelUserData.get());
}

// void SYScoreTable::SetTopLevelUserData(const QString& code)
// {
// 	m_userData = std::shared_ptr<SYUserData>(new SYUserData(code));
// 	if(m_trainData)
// 		m_trainData->SetParent(m_userData.get());
// }

void SYScoreTable::SetTrainData(const std::shared_ptr<SYTrainData>& trainData)
{
	//1 add train data
	if(trainData == nullptr)
		return;

	m_trainData = trainData;

	//2 add top level data
// 	SYUserData* userData = trainData->GetParent();
// 	if(userData == nullptr)
// 		throw "user data is null";
// 
// 	if(userData->DataType() != UDT_UserData)
// 		throw "data type is error";
// 
// 	SetTopLevelUserData(std::shared_ptr<SYUserData>(userData));

	m_trainData->SetParent(m_topLevelUserData.get());

	//changed step's parent
	auto ptr = m_trainData.get();
	for(auto stepData : m_stepDatas)
		stepData->SetParent(ptr);
}

void SYScoreTable::SetTrainData(const QString& trainName, const QString& code)
{
	m_trainData = std::shared_ptr<SYTrainData>(new SYTrainData(code));
	m_trainData->SetName(trainName);
	m_trainData->SetParent(m_topLevelUserData.get());
	m_trainData->ResetDataState();

	//changed step's parent
	auto ptr = m_trainData.get();
	for(auto stepData : m_stepDatas)
		stepData->SetParent(ptr);
}

void SYScoreTable::AddStepData(SYStepData* stepData)
{
	//1 add step data
	if(stepData == nullptr)
		return;

	m_stepDatas.insert(stepData);

	//2 add train data
// 	SYUserData* parentData = stepData->GetParent();
// 	if(parentData == nullptr)
// 		throw "parent is null";
// 
// 	if(parentData->DataType() != UDT_Train)
// 		throw "parent data type is error";
// 
// 	SetTrainData(std::shared_ptr<SYTrainData>(static_cast<SYTrainData*>(parentData)));
}

SYStepData* SYScoreTable::AddStepData(const QString& stepName, const QString& stepCode, const QString& stepDetailedDescription, int isKeyStep)
{
	for(auto step : m_stepDatas){
		if(step->Code() == stepCode)
			return step;
	}

	SYStepData* stepData = new SYStepData(stepCode);
	stepData->SetStepDescription(stepName);
	stepData->SetStepDetailDescription(stepDetailedDescription);
	stepData->SetKeyStep(isKeyStep);
	stepData->SetParent(m_trainData.get());
	stepData->ResetDataState();

	m_stepDatas.insert(stepData);
	return stepData;
}

void SYScoreTable::AddScoreItemData(SYScoreItemData* scoreItemData)
{
	//1 add score item data
	if(scoreItemData == nullptr)
		return;

	m_scoreItemDatas.insert(scoreItemData);

	//2 step data
// 	SYUserData* parentData = scoreItemData->GetParent();
// 	if(parentData == nullptr)
// 		throw "parent is null";
// 
// 	if(parentData->DataType() != UDT_Step)
// 		throw "parent data type is error";
// 
// 	AddStepData(static_cast<SYStepData*>(parentData));
}

SYScoreItemData* SYScoreTable::AddScoreItemData(SYStepData* stepData, const QString& scoreItemContent, const QString& scoreItemCode, int scoreValue)
{
	if(stepData == nullptr)
		return nullptr;

	if(m_stepDatas.find(stepData) == m_stepDatas.end())
		return nullptr;

	for(auto scoreItem : m_scoreItemDatas){
		if(scoreItem->GetParent() == stepData && scoreItem->Code() == scoreItemCode)
			return scoreItem;
	}

	SYScoreItemData* scoreItemData = new SYScoreItemData(scoreItemCode);
	scoreItemData->SetScoreContent(scoreItemContent);
	scoreItemData->SetScoreValue(scoreValue);
	scoreItemData->SetParent(stepData);
	scoreItemData->ResetDataState();

	m_scoreItemDatas.insert(scoreItemData);
	return scoreItemData;
}

void SYScoreTable::AddScorePointDetailData(SYScorePointDetailData* scorePointDetailData)
{
	//1 add score point detail data
	if(scorePointDetailData == nullptr)
		return;

	m_scorePointDetailDatas.insert(scorePointDetailData);

	//2 add score item data
// 	SYUserData* parentData = scorePointDetailData->GetParent();
// 	if(parentData == nullptr)
// 		throw "parent is null";
// 
// 	if(parentData->DataType() != UDT_ScoreItem)
// 		throw "parent data type is error";
// 
// 	AddScoreItemData(static_cast<SYScoreItemData*>(parentData));
}

SYScorePointDetailData* SYScoreTable::AddScorePointDetailData(SYScoreItemData* scoreItemData, const QString& scorePointDetailName, const QString& scorePointDetailCode, int scoreValue, int abilitys)
{
	if(m_scoreItemDatas.find(scoreItemData) == m_scoreItemDatas.end())
		return nullptr;

	for(auto spdd : m_scorePointDetailDatas){
		if(spdd->GetParent() == scoreItemData && spdd->Code() == scorePointDetailCode)
			return spdd;
	}

	SYScorePointDetailData* spdd = new SYScorePointDetailData(scorePointDetailCode);
	spdd->SetDescription(scorePointDetailName);
	spdd->SetScoreValue(scoreValue);
	spdd->SetAbilitys(abilitys);
	spdd->SetParent(scoreItemData);
	spdd->ResetDataState();

	m_scorePointDetailDatas.insert(spdd);

	//能力计数
	if(abilitys != 0){
		for(int i = 0; i < TA_NumOfAbility; ++i){
			if(abilitys & 0x01)
				m_abilityCounters[i] += 1;

			abilitys >>= 1;
		}
	}
	
	return spdd;
}

QString SYScoreTable::GetScoreTableCode()
{
	m_tempScoreTableCode.clear();

	if(m_topLevelUserData && m_trainData){
		//评分表编码，8位：2位产品编码-1位训练类型-3位训练编码-2位版本序列
		m_tempScoreTableCode.append(SYScoreTable::ProductCode);
		m_tempScoreTableCode.append(m_topLevelUserData->Code());
		m_tempScoreTableCode.append(m_trainData->Code());
		m_tempScoreTableCode.append(SYScoreTable::VersionCode);
	}

	return m_tempScoreTableCode;
}

QString SYScoreTable::GetScoreCode(SYStepData* stepData, SYScoreItemData* scoreItemData, SYScorePointDetailData* spdd)
{
	m_tempScoreCode.clear();

	if(m_topLevelUserData && m_trainData && stepData && scoreItemData && spdd){
		//评分编码,10位
		m_tempScoreCode.append(m_trainData->Code());	//3 bit
		m_tempScoreCode.append(stepData->Code());		//2 bit
		m_tempScoreCode.append(scoreItemData->Code());	//3 bit
		if(stepData->IsKeyStep())						//1 bit
			m_tempScoreCode.append("1");
		else
			m_tempScoreCode.append("0");
		m_tempScoreCode.append(spdd->Code());			//1 bit
	}

	return m_tempScoreCode;
}

QString SYScoreTable::GetScoreCode(SYScorePointDetailData* spdd)
{
	m_tempScoreCode.clear();

	if(m_topLevelUserData && m_trainData){
		if(spdd && (m_scorePointDetailDatas.find(spdd) != m_scorePointDetailDatas.end())){
			SYStepData* stepData = nullptr;
			SYScoreItemData* scoreItemData = static_cast<SYScoreItemData*>(spdd->GetParent());
			if(scoreItemData){
				stepData = static_cast<SYStepData*>(scoreItemData->GetParent());
				if(stepData){
					//评分编码,10位
					m_tempScoreCode.append(m_trainData->Code());	//3 bit
					m_tempScoreCode.append(stepData->Code());		//2 bit
					m_tempScoreCode.append(scoreItemData->Code());	//3 bit
					if(stepData->IsKeyStep())						//1 bit
						m_tempScoreCode.append("1");
					else
						m_tempScoreCode.append("0");
					m_tempScoreCode.append(spdd->Code());			//1 bit
				}
			}
		}
	}

	return m_tempScoreCode;
}

void SYScoreTable::Update()
{
	QString oldScoreTableCode, oldScoreCode;
	QString curScoreTableCode, curScoreCode;
	
	for(auto spdd : m_scorePointDetailDatas){
		auto scoreItemData = static_cast<SYScoreItemData*>(spdd->GetParent());
		auto stepData = static_cast<SYStepData*>(scoreItemData->GetParent());

		//1 评分表编码，8位：2位产品编码-1位训练类型-3位训练编码-2位版本序列
		curScoreTableCode.clear();
		curScoreTableCode.append(SYScoreTable::ProductCode);
		curScoreTableCode.append(m_topLevelUserData->Code());
		curScoreTableCode.append(m_trainData->Code());
		curScoreTableCode.append(SYScoreTable::VersionCode);

		//2 评分编码,10位
		curScoreCode.clear();
		curScoreCode.append(m_trainData->Code());	//3 bit
		curScoreCode.append(stepData->Code());		//2 bit
		curScoreCode.append(scoreItemData->Code());	//3 bit
		if(stepData->IsKeyStep())					//1 bit
			curScoreCode.append("1");
		else
			curScoreCode.append("0");
		curScoreCode.append(spdd->Code());			//1 bit


		//如果有标记，说明数据库中已经存储过，本次只要做相应修改即可,否则需要创建一条记录
		if(spdd->GetFlag()){
 			if(m_topLevelUserData->DataHasChanged()
 			   || m_trainData->DataHasChanged()
 			   || stepData->DataHasChanged()
 			   || scoreItemData->DataHasChanged()
 			   || spdd->DataHasChanged()){

				QString userDataCode, trainDataCode, stepDataCode, scoreItemDataCode, scorePointDetailDataCode;
				//code 1
				if(m_topLevelUserData->CodeHasChanged())
					userDataCode = m_topLevelUserData->OldCode();
				else
					userDataCode = m_topLevelUserData->Code();
				//code 2
				if(m_trainData->CodeHasChanged())
					trainDataCode = m_trainData->OldCode();
				else
					trainDataCode = m_trainData->Code();
				//code 3
				if(stepData->CodeHasChanged())
					stepDataCode = stepData->OldCode();
				else
					stepDataCode = stepData->Code();
				//code 4
				if(scoreItemData->CodeHasChanged())
					scoreItemDataCode = scoreItemData->OldCode();
				else
					scoreItemDataCode = scoreItemData->Code();
				//code 5
				if(spdd->CodeHasChanged())
					scorePointDetailDataCode = spdd->OldCode();
				else
					scorePointDetailDataCode = spdd->Code();

				//计算旧的编码值

				//1 评分表编码，8位：2位产品编码-1位训练类型-3位训练编码-2位版本序列
				oldScoreTableCode.clear();
				oldScoreTableCode.append(SYScoreTable::ProductCode);
				oldScoreTableCode.append(userDataCode);
				oldScoreTableCode.append(trainDataCode);
				oldScoreTableCode.append(SYScoreTable::VersionCode);

				//2 评分编码,10位
				oldScoreCode.clear();
				oldScoreCode.append(trainDataCode);				//3 bit
				oldScoreCode.append(stepDataCode);				//2 bit
				oldScoreCode.append(scoreItemDataCode);			//3 bit
				//key step
				int keyStep;
				if(stepData->KeyStepChanged())
					keyStep = stepData->OldKeyStep();
				else
					keyStep = stepData->IsKeyStep();
				if(keyStep)										//1 bit
					oldScoreCode.append("1");
				else
					oldScoreCode.append("0");
				oldScoreCode.append(scorePointDetailDataCode);	//1 bit

				SYDBMgr::Instance()->UpdateScoreTableItem(oldScoreTableCode, curScoreTableCode,
														  oldScoreCode,curScoreCode,
														  m_trainData->GetName(), m_trainData->Code(), m_topLevelUserData->Code(),
														  stepData->GetStepDescription(), stepData->Code(), stepData->GetStepDetailedDescription(), stepData->IsKeyStep(),
														  scoreItemData->GetScoreContent(), scoreItemData->Code(), scoreItemData->GetScoreValue(),
														  spdd->GetDescription(), spdd->Code(), spdd->GetScoreValue(), spdd->GetAbilitys());
			}
		}
		else{
			//insert data
			int ret = SYDBMgr::Instance()->AddScoreTableItem(curScoreTableCode, curScoreCode,
															 m_trainData->GetName(), m_trainData->Code(), m_topLevelUserData->Code(),
															 stepData->GetStepDescription(), stepData->Code(), stepData->GetStepDetailedDescription(), stepData->IsKeyStep(),
															 scoreItemData->GetScoreContent(), scoreItemData->Code(), scoreItemData->GetScoreValue(),
															 spdd->GetDescription(), spdd->Code(), spdd->GetScoreValue(), spdd->GetAbilitys());
			if(ret < 0)
				QMessageBox::information(nullptr, "error", QString::fromLocal8Bit("操作失败"));
			else{
// 				m_userData->SetFlag(1);
// 				m_trainData->SetFlag(1);
// 				stepData->SetFlag(1);
// 				scoreItemData->SetFlag(1);
				spdd->SetFlag(1);
			}
		}

		spdd->ResetDataState();
	}

	m_topLevelUserData->ResetDataState();
	m_trainData->ResetDataState();
	for(auto stepData : m_stepDatas)
		stepData->ResetDataState();
	for(auto scoreItemData : m_scoreItemDatas)
		scoreItemData->ResetDataState();
}

void SYScoreTable::Delete(SYUserData* userData)
{
	if(userData == nullptr)
		return;

	if(userData == m_topLevelUserData.get())
		return;

	//case 1: delete train and this table
	if(userData == m_trainData.get()){
		//delete all record from database
		SYDBMgr::Instance()->DeleteScoreTable(GetScoreTableCode());

		//delete table
		SYScoreTableManager::GetInstance()->DeleteTable(this);
	}
	else{
		auto dataType = userData->DataType();
		const QString& curScoreTableCode = GetScoreTableCode();

		//case2: delete step
		if(dataType == UDT_Step){
			auto stepData = static_cast<SYStepData*>(userData);
			auto itr = m_stepDatas.find(stepData);
			if(itr != m_stepDatas.end()){
				for(auto itrScoreItem = m_scoreItemDatas.begin(); itrScoreItem != m_scoreItemDatas.end();){
					SYScoreItemData* scoreItemData = static_cast<SYScoreItemData*>(*itrScoreItem);
					if(scoreItemData->GetParent() == stepData){
						for(auto itrScorePointDetail = m_scorePointDetailDatas.begin(); itrScorePointDetail != m_scorePointDetailDatas.end();){
							SYScorePointDetailData* spdd = static_cast<SYScorePointDetailData*>(*itrScorePointDetail);
							if(spdd->GetParent() == scoreItemData){
								//delete recored from database
								SYDBMgr::Instance()->DeleteScoreTableItem(curScoreTableCode, GetScoreCode(stepData, scoreItemData, spdd));

								//delete spdd and remove it from set
								delete spdd;
								itrScorePointDetail = m_scorePointDetailDatas.erase(itrScorePointDetail);
							}
							else
								++itrScorePointDetail;
						}

						//delete score item and remove it from set
						delete scoreItemData;
						itrScoreItem = m_scoreItemDatas.erase(itrScoreItem);
					}
					else
						++itrScoreItem;
				}

				//delete step data
				delete stepData;
				m_stepDatas.erase(itr);
			}

		}//case3: delete score item
		else if(dataType == UDT_ScoreItem){
			auto scoreItemData = static_cast<SYScoreItemData*>(userData);
			auto stepData = static_cast<SYStepData*>(userData->GetParent());
			auto itrScoreItem = m_scoreItemDatas.find(scoreItemData);
			if(itrScoreItem != m_scoreItemDatas.end()){
				for(auto itrScorePointDetail = m_scorePointDetailDatas.begin(); itrScorePointDetail != m_scorePointDetailDatas.end();){
					SYScorePointDetailData* spdd = static_cast<SYScorePointDetailData*>(*itrScorePointDetail);
					if(spdd->GetParent() == scoreItemData){
						//delete data from database
						SYDBMgr::Instance()->DeleteScoreTableItem(curScoreTableCode, GetScoreCode(stepData, scoreItemData, spdd));

						//delete spdd
						delete spdd;
						itrScorePointDetail = m_scorePointDetailDatas.erase(itrScorePointDetail);
					}
					else
						++itrScorePointDetail;
				}

				//delete score item data
				delete scoreItemData;
				m_scoreItemDatas.erase(itrScoreItem);
			}
		}//case4: delete score point detail
		else if(dataType == UDT_ScorePointDetail){
			auto spdd = static_cast<SYScorePointDetailData*>(userData);
			auto itr = m_scorePointDetailDatas.find(spdd);

			if(itr != m_scorePointDetailDatas.end()){
				//
				auto scoreItemData = static_cast<SYScoreItemData*>(spdd->GetParent());
				auto stepData = static_cast<SYStepData*>(scoreItemData->GetParent());
				SYDBMgr::Instance()->DeleteScoreTableItem(GetScoreTableCode(), GetScoreCode(stepData, scoreItemData, spdd));
				//
				m_scorePointDetailDatas.erase(itr);
				delete spdd;
			}
		}
	}
}

void SYScoreTable::GetData(const QString& scoreCode, SYStepData** ppStepData, SYScoreItemData** ppScoreItemData, SYScorePointDetailData** ppSpdd)
{
	do{
		if(scoreCode.length() != SYScoreTable::ScoreCodeLength){
			qDebug() << "score code length error: " << scoreCode;
			break;
		}

		//1 train code
		if(m_trainData.get() == nullptr){
			qDebug() << "no train data";
			break;
		}

		QString code = scoreCode.mid(0, 3);		//3 bit:0-2
		if(code != m_trainData->Code()){
			qDebug() << "train code error";
			break;
		}

		//2 step code
		code = scoreCode.mid(3, 2);				//2 bit:3-4
		SYStepData* stepData = nullptr;
		for(const auto& e : m_stepDatas){
			if(e->Code() == code){
				stepData = e;
				break;
			}
		}

		if(stepData == nullptr){
			qDebug() << "not found step";
			break;
		}

		if(ppStepData)	*ppStepData = stepData;

		//3 score item code
		code = scoreCode.mid(5, 3);				//3 bit:5-7
		SYScoreItemData* scoreItemData = nullptr;
		for(const auto& e : m_scoreItemDatas){
			if(e->Code() == code && e->GetParent() == stepData){
				scoreItemData = e;
				break;
			}
		}

		if(scoreItemData == nullptr){
			qDebug() << "not found score item";
			break;
		}

		if(ppScoreItemData) *ppScoreItemData = scoreItemData;

		//4 score point detail code
		code = scoreCode.right(1);				//1 bit:9
		for(const auto& spdd : m_scorePointDetailDatas){
			if(spdd->Code() == code && spdd->GetParent() == scoreItemData){
				if(ppSpdd) *ppSpdd = spdd;
				return;
			}
		}

		qDebug() << "not found score point detail data";
	} while(0);
	
	if(ppStepData)		ppStepData = nullptr;
	if(ppScoreItemData) ppScoreItemData = nullptr;
	if(ppSpdd)			ppSpdd = nullptr;
}

int SYScoreTable::GetScoreValue(const QString& scoreCode) const
{
	if(scoreCode.length() != SYScoreTable::ScoreCodeLength){
		qDebug() << "score code length error: " << scoreCode;
		return 0;
	}

	//1 train code
	if(m_trainData.get() == nullptr){
		qDebug() << "no train data";
		return 0;
	}
	
	QString code = scoreCode.mid(0, 3);		//3 bit:0-2
	if(code != m_trainData->Code()){
		qDebug() << "train code error";
		return 0;
	}

	//2 step code
	code = scoreCode.mid(3, 2);				//2 bit:3-4
	SYStepData* stepData = nullptr;
	for(auto itr = m_stepDatas.begin(); itr != m_stepDatas.end();++itr){
		SYStepData* tempStepData = *itr;
		if(tempStepData->Code() == code && tempStepData->GetParent() == m_trainData.get()){
			stepData = tempStepData;
			break;
		}
	}

	if(stepData == nullptr){
		qDebug() << "not found step";
		return 0;
	}

	//3 score item code
	code = scoreCode.mid(5, 3);				//3 bit:5-7
	SYScoreItemData* scoreItemData = nullptr;
	for(auto itr = m_scoreItemDatas.begin(); itr != m_scoreItemDatas.end();++itr){
		SYScoreItemData* tempScoreItemData = *itr;
		if(tempScoreItemData->GetParent() == stepData && tempScoreItemData->Code() == code){
			scoreItemData = tempScoreItemData;
			break;
		}
	}

	if(scoreItemData == nullptr){
		qDebug() << "not found score item";
		return 0;
	}

	//4 score point detail code
	code = scoreCode.right(1);				//1 bit:9
	for(const auto& spdd : m_scorePointDetailDatas){
		if(spdd->GetParent() == scoreItemData && spdd->Code() == code){
			return spdd->GetScoreValue();
		}
	}

	qDebug() << "not found score point detail data";
	return 0;
}

const QString& SYScoreTable::ConvertTrainTypeCodeToTrainTypeName(const QString& typeCode)
{
	static const QString name0 = QString::fromLocal8Bit("拾物训练");
	static const QString name1 = QString::fromLocal8Bit("虚拟技能训练");
	static const QString name2 = QString::fromLocal8Bit("手术训练");
	static const QString errorName = QString::fromLocal8Bit("error");

	if(typeCode == "0")
		return name0;
	else if(typeCode == "1")
		return name1;
	else if(typeCode == "2")
		return name2;
	else
		return errorName;
}

const QString& SYScoreTable::GetTrainTypeName() const
{
	if(m_topLevelUserData){
		const QString& typeCode = m_topLevelUserData->Code();
		return ConvertTrainTypeCodeToTrainTypeName(typeCode);
	}
	else
		return m_emptyString;
}

const QString& SYScoreTable::GetTrainTypeCode() const
{
	if(m_topLevelUserData)
		return m_topLevelUserData->Code();
	else
		return m_emptyString;
}

const QString& SYScoreTable::GetTrainName() const
{
	if(m_trainData)
		return m_trainData->GetName();
	else
		return m_emptyString;
}

const QString& SYScoreTable::GetTrainCode() const
{
	if(m_trainData)
		return m_trainData->Code();
	else
		return m_emptyString;
}

void SYScoreTable::UpdateAbilityCounters()
{
	//reset
	memset(m_abilityCounters, 0, sizeof(int) * TA_NumOfAbility);

	for(const auto& spdd : m_scorePointDetailDatas){
		int abilitys = spdd->GetAbilitys();

		if(abilitys == 0)
			continue;

		for(int i = 0; i < TA_NumOfAbility; ++i){
			if(abilitys & 0x01)
				m_abilityCounters[i] += 1;

			abilitys >>= 1;
		}
	}
}

int SYScoreTable::GetCounterOfAbility(SYTrainAbility ability)
{
	if(ability < 0 || ability >= TA_NumOfAbility)
		return 0;
	else
		return m_abilityCounters[ability];
}

void SYScoreTable::GetAbilityCounters(int* counters, int n) const
{
	if(n == 0 || counters == nullptr)
		return;

	if(n < 0 || n > TA_NumOfAbility)
		n = TA_NumOfAbility;

	for(int i = 0; i < n; ++i){
		counters[i] = m_abilityCounters[i];
	}
}

const QString& SYScoreTable::GetAbilityName(SYTrainAbility abilityValue)
{
	static const QString error = QString("error ability value");
	if(abilityValue < 0 || abilityValue >= TA_NumOfAbility)
		return error;
	else
		return TrainAbilityNames[abilityValue];
}


bool SYScoreTable::HasAbility(int abilitys, SYTrainAbility a)
{
	if(abilitys == 0)
		return false;
	else
		return (abilitys & (0x01 << a));
}

QString SYScoreTable::GetScoreTableCode(const QString& trainTypeCode, const QString& trainCode)
{
	QString code;

	code += SYScoreTable::ProductCode;
	code += trainTypeCode;
	code += trainCode;
	code += SYScoreTable::VersionCode;

	if(code.size() != SYScoreTable::ScoreTableCodeLength)
		code.clear();

	return code;
}