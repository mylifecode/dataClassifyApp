#pragma once
#include "ui_SYAdminSkillTrainAnalysisWindow.h"
#include <SYScoreTable.h>

enum TrainType;

class SYAdminTrainAnalysisWindow;

struct TrainAbilityCounter
{
public:
	TrainAbilityCounter()
	{
		for(int i = 0; i < TA_NumOfAbility; ++i){
			m_normalAbilityCounter[i] = 0;
			m_curAbilityCounter[i] = 0;
		}
	}

	void AddAbilityCounter(int abilitys)
	{
		if(abilitys == 0)
			return;

		for(int i = 0; i < TA_NumOfAbility; ++i){
			if(abilitys & 0x01)
				m_curAbilityCounter[i] += 1;

			abilitys >>= 1;
		}
	}

 	void IncreaseSpddCounter(const SYScorePointDetailData* spdd)
 	{
 		auto itr = m_spddCounterMap.find(spdd);
 		if(itr == m_spddCounterMap.end()){
 			m_spddCounterMap.insert(spdd, 1);
 		}
 		else{
 			++itr.value();
 		}
 	}

	void Clear()
	{
		m_scoreTableCode.clear();
		m_trainCode.clear();
		m_trainName.clear();
		m_scoreIds.clear();
		for(int i = 0; i < TA_NumOfAbility; ++i){
			m_normalAbilityCounter[i] = 0;
			m_curAbilityCounter[i] = 0;
		}

		m_spddCounterMap.clear();
	}

public:
	QString m_scoreTableCode;
	QString m_trainCode;
	QString m_trainName;
	/// 用于标识训练次数
	QSet<int> m_scoreIds;
	int m_normalAbilityCounter[TA_NumOfAbility];
	int m_curAbilityCounter[TA_NumOfAbility];
	QMap<const SYScorePointDetailData*, int> m_spddCounterMap;
};


class SYAdminSkillTrainAnalysisWindow : public QWidget
{
public:
	SYAdminSkillTrainAnalysisWindow(QWidget* parent = nullptr);

	virtual ~SYAdminSkillTrainAnalysisWindow();

	virtual void LoadTrainStatisticData();

	virtual void LoadAbilityCounterData();

private:
	void AddTrainAnalysisWindow();

protected:
	Ui::SYAdminSkillTrainAnalysisWindow ui;

	TrainType m_trainType;

	QMap<QString, TrainAbilityCounter> m_trainAbilityCounterMap;
};

