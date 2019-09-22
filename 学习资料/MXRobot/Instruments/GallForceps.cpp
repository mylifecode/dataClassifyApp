#include "GallForceps.h"
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
CGallForceps::CGallForceps() : m_GraspMode(0)
{
	m_hasBitted = false;
}
//========================================================================================================
CGallForceps::CGallForceps(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig), m_GraspMode(0)
{
	m_hasBitted = false;
}
//========================================================================================================
CGallForceps::~CGallForceps()
{
	
}
std::string CGallForceps::GetCollisionConfigEntryName()
{
	//创建抓钳的碰撞体
	return "Gall Forceps";
}
//========================================================================================================
bool CGallForceps::Initialize(CXMLWrapperTraining * pTraining)
{
	bool succed = CForceps::Initialize(pTraining);
	Ogre::Vector2 clampFaceVertex[8] = {
		Ogre::Vector2(0.148f, 2.16f),
		Ogre::Vector2(-0.161f, 2.16f),
		Ogre::Vector2(0.095f, 1.56f),
		Ogre::Vector2(-0.095f, 1.56f),
		Ogre::Vector2(0.111f, 1.15f),
		Ogre::Vector2(-0.111f, 1.15f)
	};

	Ogre::Vector2 ovRightPart[12] = {
		clampFaceVertex[0], clampFaceVertex[1], clampFaceVertex[2],
		clampFaceVertex[1], clampFaceVertex[3], clampFaceVertex[2],
		clampFaceVertex[2], clampFaceVertex[3], clampFaceVertex[4],
		clampFaceVertex[3], clampFaceVertex[5], clampFaceVertex[4]
	};

	Ogre::Vector2 ovLeftPart[12] = {
		clampFaceVertex[0], clampFaceVertex[2], clampFaceVertex[1],
		clampFaceVertex[1], clampFaceVertex[2], clampFaceVertex[3],
		clampFaceVertex[2], clampFaceVertex[4], clampFaceVertex[3],
		clampFaceVertex[3], clampFaceVertex[4], clampFaceVertex[5]
	};

	m_pluginclamp = new MisCTool_PluginClamp(this, 3.0f);
	m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,
			                      GFPhysVector3(0, -0.04, 0.0f),
			                      GFPhysVector3(1, 0, 0),
			                      GFPhysVector3(0, 0, 1),
			                      ovLeftPart, 12, MisCTool_PluginClamp::ClampReg_Left,
			                      -1.0f);

	m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
			                      GFPhysVector3(0, 0.04f, 0.0),
			                      GFPhysVector3(1, 0, 0),
			                      GFPhysVector3(0, 0, 1),
			                      ovRightPart, 12, MisCTool_PluginClamp::ClampReg_Right,
			                      1.0f);

	m_pluginclamp->m_minShaftRangeUpperValue = 4.0f;
	m_pluginclamp->m_ShowClampRegion = false;
	m_ToolPlugins.push_back(m_pluginclamp);
	
	return succed;
}
//===================================================================================================
bool CGallForceps::Update(float dt)
{
	__super::Update(dt);

	//BiteFrom();
	return true;
}
//=============================================================================================================
MisCTool_PluginClamp * CGallForceps::GetClampPlugin()
{
	return m_pluginclamp;
}
//==============================================================================================================
void CGallForceps::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
}
void CGallForceps::onFrameUpdateEnded()
{
	CTool::onFrameUpdateEnded();
}
//==============================================================================================================
void CGallForceps::InternalSimulationStart(int currStep, int TotalStep, float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
}
//==============================================================================================================
void CGallForceps::InternalSimulationEnd(int currStep, int TotalStep, float dt)
{
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);
}
//==============================================================================================================
void CGallForceps::onRSContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
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
bool CGallForceps::HasGraspSomeThing()
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