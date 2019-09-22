/**Author:zx**/
#pragma once
#include "tool.h"

class CXMLWrapperTool;
class MisMedicBindedRope;
class MisMedicThreadRope;
class CKnotter;

class CKnotter : public CTool
{
public:
	enum ThreadState
	{
		TS_NONE,
		TS_LOOP,//init loop sate
		TS_BINDED,//bind in organ
		TS_CUTTED,//cut by scissor
	};
	enum ShaftForTightState
	{
		SFT_PREPARED = 0,
		SFT_TIGHTEN = 1,
		SFT_DICARD = 2,
	};
	class KnotterLoopControlRegion
	{
	public:
		virtual bool CanControlLoop(CKnotter * knotter) = 0;
	};
	CKnotter();
	CKnotter(CXMLWrapperTool * pToolConfig);
	virtual ~CKnotter(void);
	
	void OnOrganBeRemoved(MisMedicOrganInterface * organif);

	bool Update(float dt);
	
	bool Initialize(CXMLWrapperTraining * pTraining);
	std::string GetCollisionConfigEntryName();

	//loop mode this is the init state
	void ChangeToLoopMode();
	
	//bind mode  this iswhen bind to organ
	bool CreateBindThread(bool reachMinLen);

	virtual void  InternalSimulationStart(int currStep , int TotalStep , float dt);
	virtual void  InternalSimulationEnd(int currStep , int TotalStep , float dt);

	//void ChangeToKnotState();

	void KnotThreadInOrgan(MisMedicOrgan_Ordinary & organ);//fake method


	GFPhysVector3 CalculateToolCustomForceFeedBack();

	void SetLoopControlRegion(KnotterLoopControlRegion * controlReg);

	MisMedicThreadRope * GetCurrentThread();

	KnotterLoopControlRegion * m_LoopControlRegion;

	MisMedicThreadRope * m_Looper;
	
	MisMedicBindedRope * m_OrganBindRope;
	
	MisMedicOrgan_Ordinary * m_OrganBinded;

	float m_NodeStartBindLength;

	//float m_RecnetMaxShaft;

	//bool  m_PreparedForTighten;

	//bool  m_HasTightened;
	ShaftForTightState  m_ShaftStateForTighten;//0--prepared  1--has tighten 2 -- release tighten

	Ogre::Vector3 m_LoopPlaneNormal;
	Ogre::Vector3 m_LoopFixPtLocal;


	ThreadState m_ThreadState;

	Ogre::Vector3 m_KernalNodeInitPos;

	bool m_firstTimeSetPosFromInput;

};