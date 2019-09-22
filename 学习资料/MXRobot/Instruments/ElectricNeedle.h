#pragma once
#include "Tool.h"
#include "ElectricTool.h"
#include "OgreParticleSystem.h"
class CXMLWrapperTool;
class VeinConnectObject;

class CElectricNeedle : public CElectricTool
{
public:
	class VeinHookShapeData
	{
	public:
		GFPhysVector3 m_HookLinePoints[2];

		GFPhysVector3 m_HookSupportOffsetVec;

		GFPhysVector3 m_HookProbDir;

		float m_HookLineRadius;
	};

	CElectricNeedle();
	CElectricNeedle(CXMLWrapperTool * pToolConfig);
	virtual ~CElectricNeedle(void);

public:
	bool Initialize(CXMLWrapperTraining * pTraining);
	
	std::string GetCollisionConfigEntryName();
	
	virtual void NewToolModeUpdate();
	
	virtual void InternalSimulationStart(int currStep , int TotalStep , float dt);

	virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt);

	virtual void onFrameUpdateStarted(float timeelpased);

	virtual bool Update(float dt);

	//for new train
	GFPhysVector3 m_NeedleRoot_WCS;
	GFPhysVector3 m_NeedleHead_WCS;
private:
	
	float GetFaceToElecBladeDist(GFPhysVector3 triVerts[3]);
	
	bool  IsRightPartConductElectric();

	bool  ElectricCutOrgan(MisMedicOrgan_Ordinary * organ, GFPhysSoftBodyFace * face, float weights[3]);

	GFPhysVector3 m_NeedleRoot;
	GFPhysVector3 m_NeedleHead;

	
};