#ifndef _SYELECTRICHOOKTRAIN_
#define _SYELECTRICHOOKTRAIN_

#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicRigidPrimtive.h"
class MisMedicOrgan_Ordinary;

class SYElectricHookTrain :public MisNewTraining
{
public:

	class InjuryPoint
	{
	public:
		InjuryPoint(OrganSurfaceBloodTextureTrack* btrack)
		{
			m_BloodTotalTime = 0;
			m_continuElecTime = 0;
			m_BloodTrack = btrack;
			m_IsStoped = false;
		}
		float m_BloodTotalTime;
		float m_continuElecTime;
		OrganSurfaceBloodTextureTrack * m_BloodTrack;
		bool  m_IsStoped;
	};
	SYElectricHookTrain(void);
	
	virtual ~SYElectricHookTrain(void);

public:
	
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	
	virtual bool Update(float dt);

private:
	
	void OnHandleEvent(MxEvent* pEvent);
	
	void CheckPairsInProcessRange( VeinConnectObject * pVeinConnectObject , const GFPhysVector3 & center, float radius);
	
	SYScoreTable * GetScoreTable();

	void OnSaveTrainingReport();

	std::vector<int> m_ConnectClusterNeedProcess;

	int  m_ClusterRemovedNum;

	VeinConnectObject * m_ConnectObject;

	std::map<OrganSurfaceBloodTextureTrack*, InjuryPoint> m_bloodTrackTimeMap;	//血流已被电凝的时间

	float m_VesselBeElecCogTime;
	float m_VesselBeElecCutTime;

	bool  m_VesselBeElecCog;
	bool  m_VesselBeElecCut;

	bool  m_IsElecCogTimeExceed;
	bool  m_IsElecCutTimeExceed;
};

#endif