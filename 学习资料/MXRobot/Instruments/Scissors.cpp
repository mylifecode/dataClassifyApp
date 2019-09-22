/**Author:zx**/
#include "Instruments/Scissors.h"
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


CStraightScissors::CStraightScissors()
{
	m_NewCanClampTube = true;
	m_timeSinceLastCut = 3000;
	m_shaftInLastCut = 0;
	m_CanPerformCut = true;
	m_PuncThresholdMultiply = 0.5f;
}

CStraightScissors::CStraightScissors(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig)
{
	m_NewCanClampTube = true;
	m_timeSinceLastCut = 3000;
	m_shaftInLastCut = 0;
	m_CanPerformCut = true;
	m_PuncThresholdMultiply = 0.5f;
}

CStraightScissors::~CStraightScissors()
{

}
std::string CStraightScissors::GetCollisionConfigEntryName()
{
	//Åö×²Ìå
	return "Straight Scissor";
}
bool CStraightScissors::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	
	//test
	if(m_pOwnerTraining->m_IsNewTrainMode)
	{
		Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();

		

		/*
		m_CutBladeRight.m_LinPoints[0] = GFPhysVector3(-0.02f, 0.02f, 1.1f);//positivie z means outward
		m_CutBladeRight.m_LinPoints[1] = GFPhysVector3(-0.02f, 0.08f, -0.5f);
		m_CutBladeRight.m_CuttDirection = GFPhysVector3(0, -1.0f, 0);


		m_CutBladeLeft.m_LinPoints[0] = GFPhysVector3(0, 0.0f, 1.3f);
		m_CutBladeLeft.m_LinPoints[1] = GFPhysVector3(0, -0.06, 0.0f);
		*/

		m_CutBladeRight.m_LinPoints[0] = GFPhysVector3(-0.02, 0.07, 1.35f);//positivie z means outward
		m_CutBladeRight.m_LinPoints[1] = GFPhysVector3(-0.02, 0.08, 0.0f);
		m_CutBladeRight.m_CuttDirection = GFPhysVector3(0, -1.0f, 0);


		m_CutBladeLeft.m_LinPoints[0] = GFPhysVector3(0, -0.07f, 1.35f);
		m_CutBladeLeft.m_LinPoints[1] = GFPhysVector3(0, -0.08, 0.0f);
		m_CutBladeLeft.m_CuttDirection = GFPhysVector3(0, 1.0f, 0);

		//transform back to convex coordinate
		/*
		NewTrainToolConvexData::CollideShapeData & rdata = m_righttoolpartconvex.m_CollideShapesData[0];
		NewTrainToolConvexData::CollideShapeData & ldata = m_lefttoolpartconvex.m_CollideShapesData[0];
	
		GFPhysQuaternion rightQuatInv = OgreToGPQuaternion(rdata.m_boxrotate).Inverse();
		GFPhysVector3    rightcenter  = OgreToGPVec3(rdata.m_boxcenter);

		GFPhysQuaternion leftQuatInv = OgreToGPQuaternion(ldata.m_boxrotate).Inverse();
		GFPhysVector3    leftcenter = OgreToGPVec3(ldata.m_boxcenter);

		for (int c = 0; c < 2; c++)
		{
			m_CutBladeRight.m_LinPoints[c] = QuatRotate(rightQuatInv, (m_CutBladeRight.m_LinPoints[c] - rightcenter));
			m_CutBladeLeft.m_LinPoints[c] = QuatRotate(leftQuatInv, (m_CutBladeLeft.m_LinPoints[c] - leftcenter));
		}
		//m_CutBladeLeft.m_LinPoints[0] = GFPhysVector3(0, 0.0f, 1.1f);
		//m_CutBladeLeft.m_LinPoints[1] = GFPhysVector3(0, -0.06f, -0.5f);
		m_CutBladeLeft.m_CuttDirection = GFPhysVector3(0, 1.0f, 0);
		*/

		m_CanPunctureOgran = true;

        m_plugincut = new MisCTool_PluginCut(this,4.0f);
        m_ToolPlugins.push_back(m_plugincut);
        //////////////////////////////////////////////////////////////////////////

        m_pluginclamp = new MisCTool_PluginClamp(this, 5.0f);

        Ogre::Vector2 tri_vertices_right[6] = {
            Ogre::Vector2(-0.0f, 1.4f),
            Ogre::Vector2(-0.1f, 0),
            Ogre::Vector2(0.1f, 0),
            Ogre::Vector2(-0.0f, 1.4f),
            Ogre::Vector2(0.1f, 0),
            Ogre::Vector2(0.0f, 1.4f)
        };
        Ogre::Vector2 tri_vertices_left[6] = {
            Ogre::Vector2(-0.0f, 1.4f),
            Ogre::Vector2(-0.1f, 0),
            Ogre::Vector2(0.1f, 0),
            Ogre::Vector2(-0.0f, 1.4f),
            Ogre::Vector2(0.1f, 0),
            Ogre::Vector2(0.0f, 1.4f)
        };

        m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
            GFPhysVector3(0, 0.0f, 0),
            GFPhysVector3(1, 0, 0),
            GFPhysVector3(0, 0, 1),
            tri_vertices_right,
            6,
            MisCTool_PluginClamp::ClampReg_Right,
            1.0f
            );

        m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
            GFPhysVector3(0, 0.0f, 0),
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
void CStraightScissors::onFrameUpdateStarted(float tiemelapsed)
{
	CTool::onFrameUpdateStarted(tiemelapsed);

	m_timeSinceLastCut += tiemelapsed;
	
	if(m_timeSinceLastCut > 1.5f && GetShaftAside() > m_shaftInLastCut)
	{
		m_CanPerformCut = true;
	}
}
//========================================================================================================
void CStraightScissors::onFrameUpdateEnded()
{
	CTool::onFrameUpdateEnded();
}
//========================================================================================================
bool CStraightScissors::Update(float dt)
{
	__super::Update(dt);

	m_plugincut->m_IsClampingOrgans = (m_pluginclamp->GetNumOrgansBeClamped() > 0 ? true : false);

	m_plugincut->m_IsClampingConnect = (m_pluginclamp->GetNumVeinConnectPairsBeClamped() > 0 ? true : false);

	return true;
}
//============================================================================================================
float CStraightScissors::GetCutWidth()
{
	return 0.12;
}
//============================================================================================================
void CStraightScissors::InternalSimulationStart(int currStep, int TotalStep, float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);
}
//============================================================================================================
void CStraightScissors::InternalSimulationEnd(int currStep, int TotalStep, float dt)
{
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);
}
void  CStraightScissors::onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs, int NumCollidePair)
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
