#include "RingForceps.h"
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
CRingForceps::CRingForceps() : m_GraspMode(0)
{
	m_hasBitted = false;
}
//========================================================================================================
CRingForceps::CRingForceps(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig) , m_GraspMode(0)
{
	m_hasBitted = false;
}
//========================================================================================================
CRingForceps::~CRingForceps()
{
	
}
//========================================================================================================
std::string CRingForceps::GetCollisionConfigEntryName()
{
	//碰撞体
	return "Ring Forceps";
}
//========================================================================================================
bool CRingForceps::Initialize(CXMLWrapperTraining * pTraining)
{
	bool succed = CForceps::Initialize(pTraining);
	
	SetLeftShaftAsideScale(0.6);

	SetRightShaftAsideScale(0.6);

	//三角形顶点必须按逆时针顺序排列
	Ogre::Vector2 clampTriangleVerts[10];

	clampTriangleVerts[0] = Ogre::Vector2(-0.21f , 0.2f);
	clampTriangleVerts[1] = Ogre::Vector2(0.21f , 0.2f);

	clampTriangleVerts[2] = Ogre::Vector2(-0.08f , 5.8f);
	clampTriangleVerts[3] = Ogre::Vector2(0.18f , 5.6f);

	clampTriangleVerts[4] = Ogre::Vector2(0.47f , 6.9);
	clampTriangleVerts[5] = Ogre::Vector2(0.67f , 6.8f);

	clampTriangleVerts[6] = Ogre::Vector2(0.8f ,7.7f);
	clampTriangleVerts[7] = Ogre::Vector2(1.15f,7.15f);

	clampTriangleVerts[8] = Ogre::Vector2(1.12f, 7.85f);
	clampTriangleVerts[9] = Ogre::Vector2(1.35f, 7.55f);

	Ogre::Vector2 tirVertices[24] = { 
		                              clampTriangleVerts[0] , clampTriangleVerts[1] , clampTriangleVerts[2],
		                              clampTriangleVerts[1] , clampTriangleVerts[3], clampTriangleVerts[2],

		                              clampTriangleVerts[2] , clampTriangleVerts[3] , clampTriangleVerts[4],
		                              clampTriangleVerts[3] , clampTriangleVerts[5], clampTriangleVerts[4],

		                              clampTriangleVerts[4] , clampTriangleVerts[5] , clampTriangleVerts[6],
		                              clampTriangleVerts[5] , clampTriangleVerts[7], clampTriangleVerts[6],

		                              clampTriangleVerts[6] , clampTriangleVerts[7] , clampTriangleVerts[8] ,
		                              clampTriangleVerts[7] , clampTriangleVerts[9] , clampTriangleVerts[8]

		                             
	};
	m_pluginclamp = new MisCTool_PluginClamp(this, 3.0f);
	m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0 , 0.0f , 0),
		GFPhysVector3(1 , 0 , 0),
		GFPhysVector3(0 , 0 , 1),
		tirVertices,
		24,
		MisCTool_PluginClamp::ClampReg_Right,
		1.0f
		);
	m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
		GFPhysVector3(0 , 0.0f , 0),
		GFPhysVector3(1 , 0 , 0),
		GFPhysVector3(0 , 0 , 1),
		tirVertices,
		24,
		MisCTool_PluginClamp::ClampReg_Left,
		-1.0f
		); 
	m_pluginclamp->m_ShowClampRegion = false;
	//m_pluginClamp->SetMaxReleasingTime(0.5f);
	m_pluginclamp->m_CanClampConnect = false;//need do optimize for clamp connect performance is not good for large clamp region
	m_pluginclamp->m_CanClampThread = false;

	m_ToolPlugins.push_back(m_pluginclamp);

	return succed;
}
//===================================================================================================
bool CRingForceps::Update(float dt)
{
	__super::Update(dt);

	return true;
}
float CRingForceps::GetMaxShaftSpeed()
{
	return 200.0f;
}
//==============================================================================================================
void CRingForceps::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
}
void CRingForceps::onFrameUpdateEnded()
{
	CTool::onFrameUpdateEnded();
}
//==============================================================================================================
void CRingForceps::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
}
//==============================================================================================================
void CRingForceps::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);
}
//==============================================================================================================
void CRingForceps::onRSContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	CForceps::onRSContactsSolveEnded(RSContactConstraints);
}
//=======================================================================================================
