/**Author:zx**/
#include "CurvedScissors.h"
#include "XMLWrapperTool.h"
#include "IObjDefine.h"
#include "ScreenEffect.h"
#include "EffectManager.h"
#include "BasicTraining.h"
#include "TrainingMgr.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/VeinConnectObject.h"
#include "../NewTrain/CustomConstraint.h"
#include "Collision/CollisionDispatch/GoPhysSoftRigidCollision.h"

/*
CStraightScissors::CStraightScissors()
{
	m_NewCanClampTube = true;
	m_timeSinceLastCut = 3000;
	m_shaftInLastCut = 0;
	m_CanPerformCut = true;
	m_PuncThresholdMultiply = 0.5f;
}
*/
CCurvedScissors::CCurvedScissors(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig)
{
	m_NewCanClampTube = true;
	m_timeSinceLastCut = 3000;
	m_shaftInLastCut = 0;
	m_CanPerformCut = true;
	m_PuncThresholdMultiply = 0.5f;
}

CCurvedScissors::~CCurvedScissors()
{

}
std::string CCurvedScissors::GetCollisionConfigEntryName()
{
	//Åö×²Ìå
	return "Curved Scissor";
}

bool CCurvedScissors::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	//test
	if(m_pOwnerTraining->m_IsNewTrainMode)
	{
		Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();

		m_CutBladeRight.m_LinPoints[0] = GFPhysVector3(-0.17, 0.07, 1.35f);//positivie z means outward
		m_CutBladeRight.m_LinPoints[1] = GFPhysVector3(0.056 , 0.08, 0.135);
		m_CutBladeRight.m_CuttDirection = GFPhysVector3(0, -1.0f, 0);


		m_CutBladeLeft.m_LinPoints[0] = GFPhysVector3(-0.17, -0.07f, 1.35f);
		m_CutBladeLeft.m_LinPoints[1] = GFPhysVector3(0.056, -0.08, 0.135f);
		m_CutBladeLeft.m_CuttDirection = GFPhysVector3(0, 1.0f, 0);


		m_CanPunctureOgran = true;

        m_plugincut = new MisCTool_PluginCut(this,4.0f);
        m_ToolPlugins.push_back(m_plugincut);
        //////////////////////////////////////////////////////////////////////////

        m_pluginclamp = new MisCTool_PluginClamp(this, 5.0f);

        Ogre::Vector2 tri_vertices_right[6] = {
			Ogre::Vector2(-0.125f, 1.377f),
			Ogre::Vector2(0.079f, 0.33f),
			Ogre::Vector2(-0.024f, 0.314),
			Ogre::Vector2(-0.125f, 1.377f),
			Ogre::Vector2(-0.024f, 0.314f),
			Ogre::Vector2(-0.219f, 1.352f)
        };
        Ogre::Vector2 tri_vertices_left[6] = {
			Ogre::Vector2(-0.125f, 1.377f),
			Ogre::Vector2(0.079f, 0.33f),
			Ogre::Vector2(-0.024f, 0.314),
			Ogre::Vector2(-0.125f, 1.377f),
			Ogre::Vector2(-0.024f, 0.314f),
			Ogre::Vector2(-0.219f, 1.352f)
        };

        m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
            GFPhysVector3(0, 0, 0),
            GFPhysVector3(1, 0, 0),
            GFPhysVector3(0, 0, 1),
            tri_vertices_right,
            6,
            MisCTool_PluginClamp::ClampReg_Right,
            1.0f
            );

        m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
            GFPhysVector3(0, 0, 0),
            GFPhysVector3(1, 0, 0),
            GFPhysVector3(0, 0, 1),
            tri_vertices_left,
            6,
            MisCTool_PluginClamp::ClampReg_Left,
            -1.0f
            );
        m_pluginclamp->m_CanClampLargeFace = false;
		m_pluginclamp->m_ShowClampRegion = false;
		m_pluginclamp->m_NormalSolveSoftDist = 0;
		m_ToolPlugins.push_back(m_pluginclamp);


		//m_pluginpricker = new MisCTool_PluginPricker(this, 3.0);
		//m_ToolPlugins.push_back(m_pluginpricker);

		//m_pluginbluntdissection = new MisCTool_PluginBluntDissection(this);
		//m_ToolPlugins.push_back(m_pluginbluntdissection);


		//////////////////////////////////////////////////////////////////////////

		return true;
	}

	return true;
}
//========================================================================================================
void CCurvedScissors::onFrameUpdateStarted(float tiemelapsed)
{
	CTool::onFrameUpdateStarted(tiemelapsed);

	m_timeSinceLastCut += tiemelapsed;
	
	if(m_timeSinceLastCut > 1.5f && GetShaftAside() > m_shaftInLastCut)
	{
		m_CanPerformCut = true;
	}
}
//========================================================================================================
void CCurvedScissors::onFrameUpdateEnded()
{
	CTool::onFrameUpdateEnded();
}
//========================================================================================================
bool CCurvedScissors::Update(float dt)
{
	__super::Update(dt);

    m_plugincut->m_IsClampingOrgans  = (m_pluginclamp->GetNumOrgansBeClamped() > 0 ? true : false);
	
	m_plugincut->m_IsClampingConnect = (m_pluginclamp->GetNumVeinConnectPairsBeClamped() > 0 ? true : false);

	return true;
}
//============================================================================================================
float CCurvedScissors::GetCutWidth()
{
	return 0.12;
}
//============================================================================================================
void CCurvedScissors::InternalSimulationStart(int currStep, int TotalStep, float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
}
//============================================================================================================
void CCurvedScissors::InternalSimulationEnd(int currStep, int TotalStep, float dt)
{
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);
}
//============================================================================================================
void  CCurvedScissors::onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs, int NumCollidePair)
{
	CTool::onRSContactsBuildBegin(collidePairs ,  NumCollidePair);

	for(int c = 0 ; c < NumCollidePair ; c++)
	{
		ConvexSoftFaceCollidePair & collideFace = collidePairs[c];

		if((collideFace.m_stateTag & ConvexSoftFaceCollidePair::CFP_ISCOLLIDED) == 0)
			continue;

		if(collideFace.m_ConvexObj == m_lefttoolpartconvex.m_rigidbody)
		{
		   m_LCollideFaces.insert(std::make_pair(collideFace.m_SoftFace , c));
		}
		else if(collideFace.m_ConvexObj == m_righttoolpartconvex.m_rigidbody)
		{
		   m_RCollideFaces.insert(std::make_pair(collideFace.m_SoftFace , c));
		}
	}

	
	m_LCollideFaces.clear();
	m_RCollideFaces.clear();
}
//============================================================================================================
