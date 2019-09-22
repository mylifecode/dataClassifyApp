#include "GraspingForceps.h"
#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "BasicTraining.h"
#include "EffectManager.h"
#include "GoPhysGlobalConfig.h"
#include "XMLWrapperToolPlace.h"
#include "XMLWrapperTraining.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"
#include "../NewTrain/CustomConstraint.h"
#include "MisCTool_PluginClamp.h"
#include "MisMedicOrganShapeModify.h"
//========================================================================================================
CGraspingForceps::CGraspingForceps() : m_GraspMode(0)
{
	m_hasBitted = false;
}
//========================================================================================================
CGraspingForceps::CGraspingForceps(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig), m_GraspMode(0)
{
	m_hasBitted = false;
}
//========================================================================================================
CGraspingForceps::~CGraspingForceps()
{
	
}
std::string CGraspingForceps::GetCollisionConfigEntryName()
{
	//创建抓钳的碰撞体
	return "Grasper Forceps";
}
//========================================================================================================
bool CGraspingForceps::Initialize(CXMLWrapperTraining * pTraining)
{
	bool succed = CForceps::Initialize(pTraining);
	
	
	if(pTraining->m_flag_GraspMode)
	   m_GraspMode = pTraining->m_GraspMode;

	m_ToolForceBackRate = 1.0f;

	if(m_GraspMode == 1)
	{
		m_pluginclamp = new MisCTool_PluginClamp(this, 3.0f);
		m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
									GFPhysVector3(0 , -0.1f , 0),
									GFPhysVector3(1 , 0 , 0),
									GFPhysVector3(0 , 0 , 1),
									0.21f,
									0.76f,
									MisCTool_PluginClamp::ClampReg_Right,
									1
									);

		m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
									GFPhysVector3(0 , 0.1f , 0),
									GFPhysVector3(1 , 0 , 0),
									GFPhysVector3(0 , 0 , 1),
									0.21f,
									0.76f,
									MisCTool_PluginClamp::ClampReg_Left,
									-1
									);
		m_pluginclamp->m_minShaftRangeUpperValue = 4.0f;
		m_pluginclamp->m_ShowClampRegion = false;
		m_ToolPlugins.push_back(m_pluginclamp);
	}
	return succed;
}
//===================================================================================================
bool CGraspingForceps::Update(float dt)
{
	__super::Update(dt);

	//BiteFrom();
	return true;
}
//=============================================================================================================
MisCTool_PluginClamp * CGraspingForceps::GetClampPlugin()
{
	return m_pluginclamp;
}
//==============================================================================================================
void CGraspingForceps::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
}
void CGraspingForceps::onFrameUpdateEnded()
{
	CTool::onFrameUpdateEnded();
}
//==============================================================================================================
void CGraspingForceps::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
}
//==============================================================================================================
void CGraspingForceps::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);
}
//==============================================================================================================
void CGraspingForceps::onRSContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	CForceps::onRSContactsSolveEnded(RSContactConstraints);
	
	//m_NodesImpluse.clear();
	/*
	for(size_t c = 0 ; c < RSContactConstraints.size() ; c++)
	{
		const GFPhysSoftRigidContact & rsContact = RSContactConstraints[c];

		GFPhysVector3 contactImpluse = (-rsContact.m_NormalOnRigid*rsContact.m_AccumulateImpluse)*0.20f;

		GFPhysSoftBodyNode * contactNode = rsContact.m_SoftBodyNode;

		std::map<GFPhysSoftBodyNode* , GFPhysVector3>::iterator itor = m_NodesImpluse.find(contactNode);
		if(itor == m_NodesImpluse.end())
		   m_NodesImpluse.insert(std::make_pair(contactNode , contactImpluse));
		else
		   itor->second += contactImpluse;
	}
	*/
}
//=======================================================================================================
bool CGraspingForceps::HasGraspSomeThing()
{
	GFPhysVectorObj<GFPhysSoftBodyFace*> resultFaces;
	for (int i = 0; i != m_ToolPlugins.size(); ++i)
	{
		MisCTool_PluginClamp* p = dynamic_cast<MisCTool_PluginClamp*>(m_ToolPlugins[i]);
		if (p)
		{
			std::vector<MisMedicOrgan_Ordinary *> organsclamped;
			p->GetOrgansBeClamped(organsclamped);
			if (organsclamped.size() > 0)
			{
				return true;
			}
		}
	}
	return false;
}