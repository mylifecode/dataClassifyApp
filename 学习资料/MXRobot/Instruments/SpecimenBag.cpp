#include "Instruments/SpecimenBag.h"
#include "XMLWrapperTool.h"
#include "IObjDefine.h"
#include "ScreenEffect.h"
#include "EffectManager.h"
#include "BasicTraining.h"
#include "TrainingMgr.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"
#include "../NewTrain/CustomConstraint.h"
#include "../NewTrain/MisNewTraining.h"
#include "MisCTool_PluginContainer.h"
#include "MisMedicRigidPrimtive.h"




SpecimenBag::SpecimenBag()
{
	
}

SpecimenBag::SpecimenBag(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig)
{
	
}

SpecimenBag::~SpecimenBag()
{
	
}

std::string SpecimenBag::GetCollisionConfigEntryName()
{
	//Åö×²Ìå
	return "SpecimenBag";
}

bool SpecimenBag::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	m_leftShaftAsideScale = 0.0f;
	
	m_rightShaftAsdieScale = 0.0f;

	if(m_pOwnerTraining->m_IsNewTrainMode)
	{
	
		MisCTool_PluginContainer * pp = new MisCTool_PluginContainer(this);
		pp->SetContainerRegion(this->m_lefttoolpartconvex.m_rigidbody , 
			GFPhysVector3(0.0, 0, -1.55),
			GFPhysVector3(1,0,0),
			GFPhysVector3(0,0,1),
			GFPhysVector3(0,-1,0),
			GFPhysVector3(-1.0, -1.0, 0),
			GFPhysVector3(1.0 , 1.0 , 2.8));
		m_ToolPlugins.push_back(pp);
		
	}

	return true;
}
//========================================================================================================
void SpecimenBag::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
}
//========================================================================================================
void SpecimenBag::onFrameUpdateEnded()
{
	CTool::onFrameUpdateEnded();
}
//========================================================================================================
bool SpecimenBag::Update(float dt)
{
	__super::Update(dt);
	
	return true;
}
//============================================================================================================
void SpecimenBag::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
}
//============================================================================================================
void SpecimenBag::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);
}
//============================================================================================================
GFPhysVector3 SpecimenBag::CalculateToolCustomForceFeedBack()
{
	return GFPhysVector3(0,0,0);
}
//============================================================================================================
void SpecimenBag::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{	

}
//============================================================================================================
void SpecimenBag::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	
}
//============================================================================================================
