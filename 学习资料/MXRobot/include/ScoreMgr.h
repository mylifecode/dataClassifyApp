#pragma once

#include "Singleton.h"
#include "MXOgreWrapper.h"
#include <string>
#include "MxOperateItem.h"

using namespace std;

struct ScoreRecord;
struct TrainResultRecord;

class CXMLWrapperScore;


class CScoreMgr : public CSingleT<CScoreMgr>
{
	typedef struct {
		float m_fScore;
		float m_fTimeScore;
		float m_fTimeLimit;
		float m_fExtendTimeScore;
		float m_fExtendTimeLimit;
		bool m_bResult;					//配合m_nValidTimes使用，如果该得分项已经计分了指定次数，则设置为true，以后不再对该分数项计分
		Ogre::String m_description;
		Ogre::String m_strCondition;
		bool m_bCustomPos;
		int m_nCustomPosX;
		int m_nCustomPosY;
		int m_nValidTimes;				//得分有效次数，default：1
		long m_nLastSeconds;
		float m_validTimeInterval;		//得分时间间隔，该时间间隔内，对于多条相同的得分项，只被记录最早的一条。default：1
	} ScoreItemDefine;

public:
	CScoreMgr(void);
	~CScoreMgr(void);
	void LoadScores(const vector<CXMLWrapperScore *> & vtScores);
	void SetTrainingTime(long lTrainingTime);
	// bAccumulate : 是否累计得分项
	bool Grade(Ogre::String strScoreName,bool bAccumulate = false);
	bool Grade(Ogre::String strScoreName,int nTimes);
	void AddBleedingRecords(bool isStopped , bool isAutoStopped , float remainderTimeAtBleeding , float bleedingTime , const std::string & organName);
	float GetTotalOperationScore(void);
	float GetTotalTimeScore(void);
	float GetTotalScore();
	const vector<MxOperateItem::ScoreItem>& GetScoreItems();
	//void Terminate() { m_bTerminate = true; }
	void OnTrainCreated();
	void OnTrainStart();
	void OnTrainEnd();
	void OnTrainDestroyed();
	/**
		根据分数名生成当前的一个分数项
	*/
	bool GenScoreItem(Ogre::String strScoreName, MxOperateItem::ScoreItem& scoreItem);

private:
	bool _CheckCondition(Ogre::String strScoreName);

	map<Ogre::String, ScoreItemDefine> m_mapScores;
	long m_lTrainingTime;
	float m_fScore;
	float m_fTimeScore;
	vector<MxOperateItem::ScoreItem> m_scoreItems;
	bool m_bTerminate;
};
