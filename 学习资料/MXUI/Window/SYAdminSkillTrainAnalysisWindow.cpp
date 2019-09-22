#include "SYAdminSkillTrainAnalysisWindow.h"
#include "MxDefine.h"
#include <SYScoreTableManager.h>
#include <SYStringTable.h>
#include <SYUserData.h>
#include "SYDBMgr.h"
#include "SYPieChartWindow.h"
#include "SYAdminTrainAnalysisWindow.h"

SYAdminSkillTrainAnalysisWindow::SYAdminSkillTrainAnalysisWindow(QWidget* parent)
	:QWidget(parent),
	m_trainType(TT_SkillTrain)
{
	ui.setupUi(this);


	Mx::setWidgetStyle(this, "qss:SYAdminSkillTrainAnalysisWindow.qss");
}

SYAdminSkillTrainAnalysisWindow::~SYAdminSkillTrainAnalysisWindow()
{

}

void SYAdminSkillTrainAnalysisWindow::LoadTrainStatisticData()
{
	ui.trainAnalysisWindow->SetTrainType(m_trainType);
	ui.trainAnalysisWindow->SetChartTitle(SYStringTable::GetString(SYStringTable::STR_MODULETRAINTIMES),
										   SYStringTable::GetString(SYStringTable::STR_MODULETRAINTIME));
	ui.trainAnalysisWindow->LoadData();
}

void SYAdminSkillTrainAnalysisWindow::LoadAbilityCounterData()
{
	std::vector<QSqlRecord> records;
	SYDBMgr::Instance()->QueryScoreDetailByTrainType(m_trainType,records);

	int scoreId;
	QString scoreTableCode;
	QString scoreCode;
	TrainAbilityCounter tempCounter;
	SYScoreTable* scoreTable;
	int abilitys;

	for(const auto& record : records){
		scoreId = record.value("scoreId").toInt();
		scoreTableCode = record.value("scoreTableCode").toString();
		scoreCode = record.value("scoreCode").toString();

		scoreTable = SYScoreTableManager::GetInstance()->GetTable(scoreTableCode);
		if(scoreTable == nullptr)
			continue;

		//get abilitys
		SYStepData* stepData = nullptr;
		SYScoreItemData* scoreItemData = nullptr;
		SYScorePointDetailData* spdd = nullptr;
		scoreTable->GetData(scoreCode, &stepData, &scoreItemData, &spdd);

		if(spdd == nullptr)
			continue;

		abilitys = spdd->GetAbilitys();

		auto itr = m_trainAbilityCounterMap.find(scoreTableCode);
		if(itr == m_trainAbilityCounterMap.end()){
			tempCounter.Clear();
			tempCounter.m_scoreTableCode = scoreTableCode;
			tempCounter.m_trainCode = record.value("trainCode").toString();
			tempCounter.m_trainName = record.value("trainName").toString();
			tempCounter.m_scoreIds.insert(scoreId);
			tempCounter.AddAbilityCounter(abilitys);
			tempCounter.IncreaseSpddCounter(spdd);

			scoreTable->GetAbilityCounters(tempCounter.m_normalAbilityCounter, TA_NumOfAbility);

			m_trainAbilityCounterMap.insert(scoreTableCode, tempCounter);
		}
		else{
			itr->m_scoreIds.insert(scoreId);
			itr->AddAbilityCounter(abilitys);
			itr->IncreaseSpddCounter(spdd);
		}
	}

	//abilitys percentage
	int numertors[TA_NumOfAbility] = {0};
	int denominators[TA_NumOfAbility] = {0};
	int percentages[TA_NumOfAbility] = {0};
	std::vector<int> indices1, indices2;

	for(const auto& trainAbilityCounter : m_trainAbilityCounterMap){
		int trainTimes = trainAbilityCounter.m_scoreIds.size();
		for(int i = 0; i < TA_NumOfAbility; ++i){
			numertors[i] = trainAbilityCounter.m_curAbilityCounter[i];
			denominators[i] = trainAbilityCounter.m_normalAbilityCounter[i] * trainTimes;
		}
	}

	for(int counter = 0; counter < TA_NumOfAbility; ++counter){
		if(denominators[counter] == 0)
			continue;

		int p = numertors[counter] * 100 / denominators[counter];
		percentages[counter] = p;

		if(p >= 60)
			indices1.push_back(counter);
		else if(p <= 50)
			indices2.push_back(counter);
	}

	//draw pie chart
	const int nMaxWindow = 5;
	int counter = 0;
	for(int index : indices1){
		SYPieChartWindow* pieChartWindow = new SYPieChartWindow();
		pieChartWindow->setObjectName("pieChartWindow");
		int p = percentages[index];
		pieChartWindow->addPieSlice(p, QColor(255, 255, 255));
		pieChartWindow->addPieSlice(100 - p, QColor(0x82, 0xe0, 0xb1));
		pieChartWindow->setCenterText(QString("%1%\n").arg(p) + SYStringTable::GetString(SYStringTable::STR_AchievementRate));
		pieChartWindow->setBottomText(SYScoreTable::GetAbilityName(static_cast<SYTrainAbility>(index)));
		pieChartWindow->setBackgroundColor(QColor(5, 193, 98));
		pieChartWindow->setCenterPoint(0.5f, 0.4f);
		pieChartWindow->setHoleSize(0.756f);

		ui.abilityLayout1->addWidget(pieChartWindow);

		if(++counter == nMaxWindow)
			break;
	}

	counter = 0;
	for(int index : indices2){
		SYPieChartWindow* pieChartWindow = new SYPieChartWindow();
		pieChartWindow->setObjectName("pieChartWindow");
		int p = percentages[index];
		pieChartWindow->addPieSlice(p, QColor(255, 255, 255));
		pieChartWindow->addPieSlice(100 - p, QColor(0xff, 0xe0, 0x94));
		pieChartWindow->setCenterText(QString("%1%\n").arg(p) + SYStringTable::GetString(SYStringTable::STR_AchievementRate));
		pieChartWindow->setBottomText(SYScoreTable::GetAbilityName(static_cast<SYTrainAbility>(index)));
		pieChartWindow->setBackgroundColor(QColor(255, 192, 40));
		pieChartWindow->setCenterPoint(0.5f, 0.4f);
		pieChartWindow->setHoleSize(0.756f);

		ui.abilityLayout2->addWidget(pieChartWindow);

		if(++counter == nMaxWindow)
			break;
	}

	//add train analysis window
	AddTrainAnalysisWindow();
}

void SYAdminSkillTrainAnalysisWindow::AddTrainAnalysisWindow()
{
	for(auto& trainAbilityCounter : m_trainAbilityCounterMap){
		auto trainAnalysisWindow = new SYAdminTrainAnalysisWindow;
		trainAnalysisWindow->SetTrainAbilityCounter(m_trainType,&trainAbilityCounter);
		trainAnalysisWindow->UpdateContent();
		ui.tabWidget->addTab(trainAnalysisWindow, trainAbilityCounter.m_trainName);
	}
}