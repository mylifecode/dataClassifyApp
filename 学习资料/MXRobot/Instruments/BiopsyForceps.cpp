#include "BiopsyForceps.h"
#include "Topology/GoPhysSoftBodyRestShapeModify.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Math/GoPhysTransformUtil.h"

#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "BasicTraining.h"
#include "MisRobotInput.h"
#include "MXDebugInf.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "EffectManager.h"
#include "XMLWrapperPart.h"
#include "XMLWrapperPursue.h"
#include "InputSystem.h"
#include "SmokeManager.h"
#include "XMLWrapperTraining.h"
//#include "../NewTrain/CustomConstraint.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"
#include "MisCTool_PluginClamp.h"



CBiopsyForceps::CBiopsyForceps() : CForceps()
{
	m_fClampedTime = 0;
	//for debug
	m_pManualObjectForDebugLeft = NULL;
	m_pSceneNodeForDebugLeft = NULL;
}

CBiopsyForceps::CBiopsyForceps(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig)
{
	m_fClampedTime = 0;
	//for debug
	m_pManualObjectForDebugLeft = NULL;
	m_pSceneNodeForDebugLeft = NULL;
}

CBiopsyForceps::~CBiopsyForceps()
{

}

std::string CBiopsyForceps::GetCollisionConfigEntryName()
{
	return TT_BIOPSYFORCEPS;
}
bool CBiopsyForceps::Initialize(CXMLWrapperTraining * pTraining)
{
	bool succed = CTool::Initialize(pTraining);

	if(pTraining->m_flag_GraspMode)
		m_GraspMode = pTraining->m_GraspMode;

	m_ToolForceBackRate = 0.6f;

	if(m_GraspMode == 1)
	{
		m_pluginclamp = new MisCTool_PluginClamp(this, 8.0f);
		m_pluginclamp->SetClampRegion(m_righttoolpartconvex,//.m_rigidbody,
			GFPhysVector3(0 , -0.19f , 0.48),
			GFPhysVector3(1 , 0 , 0),
			GFPhysVector3(0 , 0 , 1),
			0.15f,
			0.26f,
			MisCTool_PluginClamp::ClampReg_Right,
			1
			);

		m_pluginclamp->SetClampRegion(m_lefttoolpartconvex,//.m_rigidbody,
			GFPhysVector3(0 , 0.12f , 0.48),
			GFPhysVector3(1 , 0 , 0),
			GFPhysVector3(0 , 0 , 1),
			0.15f,
			0.26f,
			MisCTool_PluginClamp::ClampReg_Left,
			-1
			);

		m_ToolPlugins.push_back(m_pluginclamp);
	}

	m_leftShaftAsideScale = 0;

	//for debug
	Ogre::SceneManager * pSMG =  MXOgreWrapper::Get()->GetDefaultSceneManger();
	m_pManualObjectForDebugLeft = pSMG->createManualObject();
	m_pSceneNodeForDebugLeft = pSMG->getRootSceneNode()->createChildSceneNode();
	m_pSceneNodeForDebugLeft->attachObject(m_pManualObjectForDebugLeft);
	
	return succed;
}


bool CBiopsyForceps::Update(float dt)
{
	__super::Update(dt);

	////for debug
	//if (m_pluginClamp)
	//{
	//	MisCTool_PluginClamp::ToolClampRegion clampReg[2];
	//	clampReg[0] = m_pluginClamp->GetClampRegion(0);
	//	clampReg[1] = m_pluginClamp->GetClampRegion(1);

	//	if (m_pManualObjectForDebugLeft && clampReg[0].m_WorldTriVerts.size() > 0 && clampReg[1].m_WorldTriVerts.size() > 0)
	//	{
	//		m_pManualObjectForDebugLeft->clear();
	//		m_pManualObjectForDebugLeft->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
	//		m_pManualObjectForDebugLeft->colour(0.0f,1.0f,0.0f);
	//		m_pManualObjectForDebugLeft->position(GPVec3ToOgre(clampReg[0].m_WorldTriVerts[0]));
	//		m_pManualObjectForDebugLeft->position(GPVec3ToOgre(clampReg[1].m_WorldTriVerts[0]));
	//		m_pManualObjectForDebugLeft->colour(0.0f,1.0f,1.0f);
	//		m_pManualObjectForDebugLeft->position(GPVec3ToOgre(clampReg[0].m_WorldTriVerts[1]));
	//		m_pManualObjectForDebugLeft->position(GPVec3ToOgre(clampReg[1].m_WorldTriVerts[1]));
	//		m_pManualObjectForDebugLeft->colour(1.0f,1.0f,0.0f);
	//		m_pManualObjectForDebugLeft->position(GPVec3ToOgre(clampReg[0].m_WorldTriVerts[2]));
	//		m_pManualObjectForDebugLeft->position(GPVec3ToOgre(clampReg[1].m_WorldTriVerts[2]));
	//		m_pManualObjectForDebugLeft->colour(0.0f,0.0f,1.0f);
	//		m_pManualObjectForDebugLeft->position(GPVec3ToOgre(clampReg[0].m_WorldTriVerts[5]));
	//		m_pManualObjectForDebugLeft->position(GPVec3ToOgre(clampReg[1].m_WorldTriVerts[5]));
	//		m_pManualObjectForDebugLeft->end();
	//	}
	//}
	////

	if (m_pluginclamp && m_pluginclamp->isInClampState())
	{
		m_fClampedTime += dt;

	}
	else
	{
		m_fClampedTime = 0;
	}


	//if(0 && m_bElectricButton)
	//{
	//	if(m_pluginClamp && m_pluginClamp->isInClampState())
	//	{
	//		m_isClampAndBurn = true;
	//		MisMedicOrgan_Ordinary * organInClamp = m_pluginClamp->GetOrganBeClamped();
	//		ElectricClampedFaces(dt);
	//		std::vector<GFPhysSoftBodyTetrahedron*> tretraBeingClamp;
	//		m_pluginClamp->CollectTetrasInClampRegion(tretraBeingClamp);
	//		/*for(size_t t = 0; t < tretraBeingClamp.size(); t++)
	//		{
	//			GFPhysSoftBodyTetrahedron * tetra = tretraBeingClamp[t];
	//			for(int n = 0; n < 4; n++)
	//			{
	//				organInClamp->GetPhysNodeData(tetra->m_TetraNodes[n]).m_burnValue += 0.5f;
	//				//tetra->m_TetraNodes[n]->m_UserDefValue0.m_x += 0.5f;
	//			}
	//		}*/
	//		organInClamp->HeatTetrahedrons(tretraBeingClamp , 1.0f*dt);

	//		GoPhysSoftBodyRestShapeModify restShapeModify;
	//		restShapeModify.ShrinkTetrahedrons( PhysicsWrapper::GetSingleTon().m_dynamicsWorld,
	//											organInClamp->m_physbody,
	//											tretraBeingClamp,
	//											0.98);
	//		//smoke
	//		std::set<GFPhysSoftBodyFace*> faces = GetFaceInClamp(); 
	//		if (!faces.empty())
	//		{
	//			SmokeManager * smokemgr = EffectManager::Instance()->GetSmokeManager();
	//			GFPhysVector3 pos = (*(faces.begin()))->m_Nodes[0]->m_CurrPosition;
	//			Ogre::Vector3 emitPt = GPVec3ToOgre(pos);
	//			smokemgr->addSmoke(emitPt, 0.15, 0.1, 5);

	//		}

	//		organInClamp->CreateAdditionalBendingForce();

	//		if(m_canCut)
	//		{
	//			organInClamp->CutByTool(this);
	//			ReleaseClampedOrgans();
	//		}

	//	}
	//	else
	//	{
	//		m_isClampAndBurn = false;
	//	}
	//}
	//else
	//{
	//	m_isClampAndBurn = false;
	//}


	return true;
}

void CBiopsyForceps::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
	//if(m_isClampAndBurn)
	//	m_burnTime += timeelapsed;
	//else
	//{
	//	m_burnTime = 0;
	//}
	//m_time_elapse = timeelapsed;
}


std::set<GFPhysSoftBodyFace*> CBiopsyForceps::GetFaceInClamp()
{
	std::set<GFPhysSoftBodyFace*> result;
	
	return result;
}

//================================================================================================================


bool CBiopsyForceps::HasGraspSomeThing()
{
	/*GFPhysVectorObj<GFPhysSoftBodyFace*> resultFaces;
	for (int i = 0; i != m_ToolPlugins.size(); ++i)
	{
		MisCTool_PluginClamp* p = dynamic_cast<MisCTool_PluginClamp*>(m_ToolPlugins[i]);
		if (p)
		{
			p->GetFacesBeClamped(resultFaces);
			if (resultFaces.size() == 0)
			{
				return false;
			}
		}
	}*/
	return false;
}