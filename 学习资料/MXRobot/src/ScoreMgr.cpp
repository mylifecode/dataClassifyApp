#include "ScoreMgr.h"
#include "Inception.h"
#include "EffectManager.h"
#include "Helper.h"
#include "ScreenEffect.h"
#include "XMLWrapperScore.h"

CScoreMgr::CScoreMgr(void)
: m_fScore(0)
, m_fTimeScore(0)
, m_bTerminate(false)
{
}

CScoreMgr::~CScoreMgr(void)
{
	m_mapScores.clear();
}

void CScoreMgr::SetTrainingTime(long lTrainingTime)
{
	m_lTrainingTime = lTrainingTime;
}

void CScoreMgr::LoadScores(const vector<CXMLWrapperScore *> & vtScores)
{
	m_mapScores.clear();

	for (vector<CXMLWrapperScore *>::const_iterator it = vtScores.begin(); it != vtScores.end(); it++)
	{
		ScoreItemDefine & score = m_mapScores[(*it)->m_Name];

		score.m_bResult = false;
		score.m_fScore = (*it)->m_Score;
		score.m_fTimeScore = (*it)->m_TimeScore;
		score.m_fTimeLimit = (*it)->m_TimeLimit;
		score.m_description = (*it)->m_Description;
		std::wstring description = QApplication::translate("ScoreMgr", (*it)->m_Description.c_str()).toStdWString();
		score.m_strCondition = (*it)->m_Condition;
		score.m_nLastSeconds=(*it)->m_LastSeconds;

		if (score.m_nLastSeconds > 0)
		{
			int yyy = 0;
			int gg = yyy + 1;
		}
		if ((*it)->m_flag_CustomPosX && (*it)->m_flag_CustomPosY)
		{
			score.m_bCustomPos = true;
			score.m_nCustomPosX = (*it)->m_CustomPosX;
			score.m_nCustomPosY = (*it)->m_CustomPosY;
		}
		else
		{
			score.m_bCustomPos = false;
			score.m_nCustomPosX = 0;
			score.m_nCustomPosY = 0;
		}

		if ((*it)->m_flag_ValidTimes)
		{
			score.m_nValidTimes = (*it)->m_ValidTimes;
		}
		else
		{
			score.m_nValidTimes = 1;
		}

		if ((*it)->m_flag_ExtendTimeLimit)
		{
			score.m_fExtendTimeLimit = max((*it)->m_ExtendTimeLimit, (*it)->m_TimeLimit);
		}
		else
		{
			score.m_fExtendTimeLimit = (*it)->m_TimeLimit;
		}

		if ((*it)->m_flag_ExtendTimeScore)
		{
			score.m_fExtendTimeScore = min((*it)->m_ExtendTimeScore, (*it)->m_TimeScore);
		}
		else
		{
			score.m_fExtendTimeScore = (*it)->m_TimeScore;
		}

		if((*it)->m_flag_ValidTimeInterval)
		{
			score.m_validTimeInterval = (*it)->m_ValidTimeInterval;
		}
		else
			score.m_validTimeInterval = 1.f;
	}
}

bool CScoreMgr::Grade(Ogre::String strScoreName,bool bAccumulate)
{
	if ( m_bTerminate )
	{
		return false;
	}
	//时间到后不再计分
	if(Inception::Instance()->m_remainTime <= 0)
	{
		m_bTerminate = true;
		return false;
	}

	if (m_mapScores.count(strScoreName) == 0)
	{
		return false;
	}

	if (!_CheckCondition(strScoreName))
	{
		return false;
	}

	ScoreItemDefine & score = m_mapScores[strScoreName];		//前面已经确保有该key对应的值

	if (score.m_bResult)		//default : false,
	{
		return false;
	}

	float fScore = 0;
	float fTimeScore = 0;

	if (score.m_nValidTimes == 1)
		score.m_bResult = true;

	score.m_nValidTimes = score.m_nValidTimes - 1;
	
	fScore = score.m_fScore; 

	float fTime = m_lTrainingTime - Inception::Instance()->m_remainTime;		//已经流逝的时间（s）

	int recordIndex = -1;
	for(int r = m_scoreItems.size() - 1;r >= 0;--r)
	{
		if(m_scoreItems[r].name == strScoreName)
		{
			recordIndex = r;
			break;
		}
	}

	if(recordIndex != -1 && fTime - m_scoreItems[recordIndex].time <= score.m_validTimeInterval)	//考虑有效时间间隔
		return false;

	//calculate time
	if (fTime <= score.m_fTimeLimit)
	{
		fTimeScore = score.m_fTimeScore;
	}
	else if (fTime > score.m_fExtendTimeLimit)
	{
		fTimeScore = 0;
	}
	else
	{
		fTimeScore = score.m_fTimeScore + (fTime - score.m_fTimeLimit) * (score.m_fExtendTimeScore - score.m_fTimeScore) / (score.m_fExtendTimeLimit - score.m_fTimeLimit);
	}


	fScore = (float)qRound(fScore * 10) / 10;
	fTimeScore = (float)qRound(fTimeScore * 10) / 10;

	m_fScore += fScore;
	m_fTimeScore += fTimeScore;


	if(recordIndex != -1 && bAccumulate)
	{
		m_scoreItems[recordIndex].score += fScore;
		m_scoreItems[recordIndex].timeScore += fTimeScore;
		m_scoreItems[recordIndex].times += 1;				//得分次数加1
	}
	else
	{
		MxOperateItem::ScoreItem recordItem;
		recordItem.name = strScoreName;
		recordItem.description = score.m_description;
		recordItem.score = fScore;
		recordItem.timeScore = fTimeScore;
		recordItem.time = fTime;
		recordItem.times = 1;
		m_scoreItems.push_back(recordItem);
	}
	return true;
}


bool CScoreMgr::Grade(Ogre::String strScoreName,int nTimes)
{
	if ( m_bTerminate )
	{
		return false;
	}
	//时间到后不再计分
	if(Inception::Instance()->m_remainTime <= 0)
	{
		m_bTerminate = true;
		return false;
	}

	if (m_mapScores.count(strScoreName) == 0)
	{
		return false;
	}

	if (!_CheckCondition(strScoreName))
	{
		return false;
	}

	ScoreItemDefine & score = m_mapScores[strScoreName];

	if (score.m_bResult)
	{
		return false;
	}

	float fScore = 0;
	float fTimeScore = 0;

	if (score.m_nValidTimes == 1)
	{
		m_mapScores[strScoreName].m_bResult = true;
	}

	if ( score.m_nValidTimes - nTimes < 0)
	{
		int nValidTime = score.m_nValidTimes;
		score.m_nValidTimes = 0;
		fScore = nValidTime*score.m_fScore; 
	}
	else
	{
		score.m_nValidTimes = score.m_nValidTimes - nTimes;
		fScore = nTimes*score.m_fScore; 
	}

	float fTime = m_lTrainingTime - Inception::Instance()->m_remainTime;
	if (fTime <= score.m_fTimeLimit)
	{
		fTimeScore = score.m_fTimeScore;
	}
	else if (fTime > score.m_fExtendTimeLimit)
	{
		fTimeScore = 0;
	}
	else
	{
		fTimeScore = score.m_fTimeScore + (fTime - score.m_fTimeLimit) * (score.m_fExtendTimeScore - score.m_fTimeScore) / (score.m_fExtendTimeLimit - score.m_fTimeLimit);
	}

	fScore = (float)qRound(fScore * 10) / 10;
	fTimeScore = (float)qRound(fTimeScore * 10) / 10;

	m_fScore += fScore;
	m_fTimeScore += fTimeScore;

	TipInfo::TipIconType eIconType;
	if (fScore == score.m_fScore && fTimeScore == score.m_fTimeScore)
	{
		eIconType = TipInfo::TIT_SCORE_INFO;
	}
	else
	{
		eIconType = TipInfo::TIT_SCORE_WARNING;
	}

	MxOperateItem::ScoreItem recordItem;
	recordItem.name = strScoreName;
	recordItem.description = score.m_description;
	recordItem.score = fScore;
	recordItem.timeScore = fTimeScore;
	recordItem.time = fTime;
	recordItem.times = 1;
	m_scoreItems.push_back(recordItem);

	return true;
}

void CScoreMgr::AddBleedingRecords(bool isStopped , bool isAutoStopped , float remainderTimeAtBleeding , float bleedingTime , const std::string & organName)
{
	static const std::string ScoreRecordName = "出血记录";
	static const std::string unhandledStr = "出血--未进行止血";
	static const std::string autoStopStr = "出血--伤口自动凝固";
	static const std::string ManuallyStopStr = "秒后进行止血";

	MxOperateItem::ScoreItem recordItem;
	recordItem.name = ScoreRecordName;
	recordItem.time = m_lTrainingTime - remainderTimeAtBleeding;
	recordItem.score = 0;
	recordItem.timeScore = 0;
	recordItem.times = 1;

	if(!isStopped) {
		recordItem.description = organName + unhandledStr;
	} else {
		if(isAutoStopped) {
			recordItem.description = organName + autoStopStr;
		} else {
			char temp[100] = {0};
			sprintf(temp,"出血--%.2f",bleedingTime);
			recordItem.description = organName +  temp + ManuallyStopStr;
		}
	}
	recordItem.description = QString::fromLocal8Bit(recordItem.description.c_str()).toUtf8().constData();
	m_scoreItems.push_back(recordItem);
}

bool CScoreMgr::_CheckCondition(Ogre::String strScoreName)
{
	if (m_mapScores[strScoreName].m_strCondition == "")
	{
		return true;
	}

	if (m_mapScores.count(m_mapScores[strScoreName].m_strCondition))
	{
		return m_mapScores[m_mapScores[strScoreName].m_strCondition].m_bResult;
	}

	return false;
}

float CScoreMgr::GetTotalOperationScore()
{
	return m_fScore;
}

float CScoreMgr::GetTotalTimeScore(void)
{
	return m_fTimeScore;
}

float CScoreMgr::GetTotalScore()
{
	return std::max(m_fScore + m_fTimeScore, 0.f);
}

const vector<MxOperateItem::ScoreItem>& CScoreMgr::GetScoreItems(void)
{
	return m_scoreItems;
}

void CScoreMgr::OnTrainStart()
{
	m_lTrainingTime = Inception::Instance()->m_remainTime;
	m_fScore = 0;
	m_fTimeScore = 0;
	m_scoreItems.clear();
	m_bTerminate = false;

}
void CScoreMgr::OnTrainEnd()
{
	
	m_bTerminate = true;
}
void CScoreMgr::OnTrainCreated()
{

}

void CScoreMgr::OnTrainDestroyed()
{
	m_mapScores.clear();
}

bool CScoreMgr::GenScoreItem(Ogre::String strScoreName, MxOperateItem::ScoreItem& scoreItem)
{
	if ( m_bTerminate )
	{
		return false;
	}
	//时间到后不再计分
	if(Inception::Instance()->m_remainTime <= 0)
	{
		m_bTerminate = true;
		return false;
	}

	if (m_mapScores.count(strScoreName) == 0)
	{
		return false;
	}

	if (!_CheckCondition(strScoreName))
	{
		return false;
	}

	ScoreItemDefine & score = m_mapScores[strScoreName];

	if (score.m_bResult)
	{
		return false;
	}

	float fScore = 0;
	float fTimeScore = 0;

	if (score.m_nValidTimes == 1)
	{
		m_mapScores[strScoreName].m_bResult = true;
	}

	if ( score.m_nValidTimes - 1 < 0)
	{
		int nValidTime = score.m_nValidTimes;
		score.m_nValidTimes = 0;
		fScore = nValidTime*score.m_fScore; 
	}
	else
	{
		score.m_nValidTimes = score.m_nValidTimes - 1;
		fScore = score.m_fScore; 
	}

	float fTime = m_lTrainingTime - Inception::Instance()->m_remainTime;
	if (fTime <= score.m_fTimeLimit)
	{
		fTimeScore = score.m_fTimeScore;
	}
	else if (fTime > score.m_fExtendTimeLimit)
	{
		fTimeScore = 0;
	}
	else
	{
		fTimeScore = score.m_fTimeScore + (fTime - score.m_fTimeLimit) * (score.m_fExtendTimeScore - score.m_fTimeScore) / (score.m_fExtendTimeLimit - score.m_fTimeLimit);
	}

	m_fScore += fScore;
	m_fTimeScore += fTimeScore;


	scoreItem.name = strScoreName;
	scoreItem.description = score.m_description;
	scoreItem.score = fScore;
	scoreItem.timeScore = fTimeScore;
	scoreItem.time = fTime;
	scoreItem.times = 1;

	return true;
}