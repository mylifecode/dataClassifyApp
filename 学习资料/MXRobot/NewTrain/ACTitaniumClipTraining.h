#ifndef _BASIC_TitaniumClipTraining
#define _BASIC_TitaniumClipTraining
#include "MisNewTraining.h"
#include <OgreMaterial.h>

class MisMedicOrgan_Ordinary;
class WaterPool;

class ACTitaniumClipTraining :public MisNewTraining
{
	struct VeinInfo
	{
		VeinInfo()
		{
			m_HasBeTearedOff = false;
			m_IsBeGraspedNow = false;
			m_HasBeGrasped = false;
			m_AlwaysGraspWhenClip = true;
			m_CanBlood = true;
			m_BloodIsPaused = false;

			m_ClipErrorCount = 0;
			m_ClipGoodCount = 0;
		}
		MisMedicOrgan_Ordinary* m_vein;
		Ogre::MaterialPtr m_materialPtr;

		//logic data
		bool m_HasBeTearedOff;
		bool m_IsBeGraspedNow;
		bool m_HasBeGrasped;
		bool m_CanBlood;
		bool m_BloodIsPaused;

		int  m_ClipErrorCount;
		int  m_ClipGoodCount;

		bool m_AlwaysGraspWhenClip;
		int m_TearNode0;
		int m_TearNode1;

	};

public:
	ACTitaniumClipTraining(float limitTime = -1.f);

	virtual ~ACTitaniumClipTraining(void);

public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual bool Update(float dt);

	void CheckTearOff(int vein);

	virtual void InternalSimulateStart(int currStep , int TotalStep , Real dt);
	virtual void InternalSimulateEnd(int currStep , int TotalStep , Real dt);
	 
	bool BeginRendOneFrame(float timeelpsed);

	virtual void AddDefaultScoreItemDetail();

	void OnSaveTrainingReport();

private:

	bool OrganShouldBleed(MisMedicOrgan_Ordinary* organ, int faceID, float* pFaceWeight);

	void OnTrainingIlluminated();

	void OnTimerTimeout(int id,float dt ,void* userData);
		
	void OnHandleEvent(MxEvent* pEvent);

	void CheckSuction();

	void CheckKeepClip(float dt);

private:

	SYScoreTable * GetScoreTable();

	bool IsClipInOrganValidError(MisMedicTitaniumClampV2 * clip, MisMedicOrgan_Ordinary * organ, float minv, float maxv);

	VeinInfo * FindVienInfo(MisMedicOrgan_Ordinary * vein)
	{
		for (int c = 0; c < m_veinInfos.size(); c++)
		{
			if (m_veinInfos[c].m_vein == vein)
				return &m_veinInfos[c];
		}
		return 0;
	}

	/// 每次投放的时间限制
	const float m_limitTime;
	const bool m_hasLimitTime;

	WaterPool* m_curWaterPool;
	std::vector<VeinInfo> m_veinInfos;

	Ogre::TexturePtr m_blankTexture;
	Ogre::TexturePtr m_markedTexture;

	bool m_hasOperation;
	bool m_hasSuction;

	ITool* m_releaseClipTool;
	float m_toolMinShaft;
	bool m_hasReleaseClip;
	bool m_needCheckKeepClip;
	bool m_hasCheckedKeepClip;
	bool m_isKeepClip;
	float m_curKeepClipTime;

	/// 未充分夹闭的数量
	int m_nUnFullClip;

	bool m_needCleanBleed;
	float m_terminalWaitTime;
};

#endif