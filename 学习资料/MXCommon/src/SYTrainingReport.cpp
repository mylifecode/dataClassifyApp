#include "SYTrainingReport.h"

SYTrainingReport::SYTrainingReport(void)
{
	Reset();
}

SYTrainingReport::~SYTrainingReport(void)
{

}

SYTrainingReport* SYTrainingReport::Instance()
{
	static SYTrainingReport report;
	return &report;
}


void SYTrainingReport::Reset()
{
	m_totalTime = 0;
	m_remainTime = 0;
	m_electricTime = 0.f;
	m_validElectricTime = 0.f;
	m_maxKeeppingElectricBeginTime = 0.f;
	m_maxKeeppingElectricTime = 0.f;
	m_maxBleedTime = 0.f;
	m_nReleasedTitanicClip = 0;
	m_leftToolClosedTimes = 0;
	m_rightToolClosedTimes = 0;
	m_leftToolMovedDistance = 0.f;
	m_rightToolMovedDistance = 0.f;
	m_leftToolMovedSpeed = 0.f;
	m_rightToolMovedSpeed = 0.f;
	m_electricAffectTimeForHemoclip = 0.f;
	m_electricAffectTimeForOrdinaryOrgan = 0.f;

	m_operateItems.clear();
	m_scoreTable = nullptr;
	m_scoreItemDetails.clear();
}



void SYTrainingReport::AddOperateItems(const std::vector<MxOperateItem> & items)
{
	for(std::size_t i = 0;i < items.size();++i)
		m_operateItems.push_back(items[i]);
}

void SYTrainingReport::GetOperateItems(std::vector<MxOperateItem> & operateItems)
{
	//1 TODO add basic operate items

	//2 add training operate items
	for(std::size_t i = 0;i < m_operateItems.size();++i)
	{
		operateItems.push_back(m_operateItems[i]);
	}
}

void SYTrainingReport::GetScoreItemDetails(std::vector<SYScoreItemDetail>& scoreItemDetails) const
{
	scoreItemDetails.insert(scoreItemDetails.end(), m_scoreItemDetails.begin(), m_scoreItemDetails.end());
}

void SYTrainingReport::SetScoreItemDetail(SYScoreTable* scoreTable,const std::vector<SYScoreItemDetail>& scoreItemDetails)
{
	m_scoreTable = scoreTable;
	m_scoreItemDetails.insert(m_scoreItemDetails.end(), scoreItemDetails.begin(), scoreItemDetails.end());
}