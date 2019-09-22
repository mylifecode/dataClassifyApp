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
		bool m_bResult;					//���m_nValidTimesʹ�ã�����õ÷����Ѿ��Ʒ���ָ��������������Ϊtrue���Ժ��ٶԸ÷�����Ʒ�
		Ogre::String m_description;
		Ogre::String m_strCondition;
		bool m_bCustomPos;
		int m_nCustomPosX;
		int m_nCustomPosY;
		int m_nValidTimes;				//�÷���Ч������default��1
		long m_nLastSeconds;
		float m_validTimeInterval;		//�÷�ʱ��������ʱ�����ڣ����ڶ�����ͬ�ĵ÷��ֻ����¼�����һ����default��1
	} ScoreItemDefine;

public:
	CScoreMgr(void);
	~CScoreMgr(void);
	void LoadScores(const vector<CXMLWrapperScore *> & vtScores);
	void SetTrainingTime(long lTrainingTime);
	// bAccumulate : �Ƿ��ۼƵ÷���
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
		���ݷ��������ɵ�ǰ��һ��������
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
