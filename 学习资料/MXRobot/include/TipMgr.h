#pragma once

#include "Singleton.h"
#include "MXOgreWrapper.h"
#include "XMLWrapperTip.h"
#include "TipInfo.h"
using namespace std;

typedef struct {
	wstring m_wstrDescription;
	Ogre::String m_strIconType;
	bool m_bCustomPos;
	int m_nCustomPosX;
	int m_nCustomPosY;
	long m_nLastSeconds;
} Tip;

class CTipMgr : public CSingleT<CTipMgr>
{
public:
	CTipMgr(void);
	~CTipMgr(void);
	void LoadTips(const vector<CXMLWrapperTip *> & vtTips);
	bool ShowTip(Ogre::String strTipName, ...);
	bool ShowTipAndTerminate(Ogre::String strTipName);
	/** 设置下一个将要显示提示，在waitTime时间后 */
	void SetNextTip(const Ogre::String& tipName,float waitTime);
	bool DeleteNextTip(const Ogre::String& tipName);
	void DeleteAllNextTip();
	//void Terminate() { m_bTerminate = true; }
	void OnTrainCreated();
	void OnTrainStart();
	void OnTrainEnd();
	void OnTrainDestroyed();

	void Update(float dt);
	void SetQueeu(bool isOn);

private:
	void LoadPresetTips();

	map<Ogre::String, Tip> m_mapTips;
	list</*Ogre::String*/TipInfo>	m_tipqueue;
	int		m_FrameCount;
	bool m_bTerminate;
	bool m_bQueueInited;
	Ogre::String		m_lastTip;
// 	Ogre::String		m_nextTipName;
// 	float				m_waitTime;
	std::map<Ogre::String, float>	 m_nextTip;

	//float m_LastTipKeepTime;
	float m_LastTipDuration;
};
