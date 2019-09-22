#include "Instruments/SuctionAndlrrigationTube.h"
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
#include "MisCTool_PluginSuction.h"
#include "WaterManager.h"
#include "WaterPool.h"

#define SUCTION_HEAD_RADIUS 0.15


SuctionAndIrrigationTube::SuctionAndIrrigationTube()
{
	//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
}

SuctionAndIrrigationTube::SuctionAndIrrigationTube(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig)
{
	//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
}

SuctionAndIrrigationTube::~SuctionAndIrrigationTube()
{
	//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}

std::string SuctionAndIrrigationTube::GetCollisionConfigEntryName()
{
	//Åö×²Ìå
	return "Suction And Irrigation Tube";

}
bool SuctionAndIrrigationTube::Initialize(CXMLWrapperTraining * pTraining)
{
	__super::Initialize(pTraining);

	m_pPluginSuction = NULL;

	m_LastLeftElecBtn = false;

	m_LastRightElecBtn = false;

	m_IsWaterColumnExistent = false;

	m_WaterColumnID = -1;

	if(m_pOwnerTraining->m_IsNewTrainMode)
	{
		Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();
		
		MisCTool_PluginSuction * pPluginSuction = new MisCTool_PluginSuction(this);

        GFPhysVector3 regionCenter = OgreToGPVec3(m_centertoolpartconvex.m_CollideShapesData[0].m_boxcenter);

        if (m_centertoolpartconvex.m_CollideShapesData[0].m_ShapeType == 0)
        {
            pPluginSuction->SetSuctionRegion(m_centertoolpartconvex.m_rigidbody, -regionCenter,
                GFPhysVector3(1, 0, 0),
                GFPhysVector3(0, 1, 0),
                GFPhysVector3(0, 0, 1),
                SUCTION_HEAD_RADIUS,
                0.1);
        }
        else if (m_centertoolpartconvex.m_CollideShapesData[0].m_ShapeType == 2)
        {
            pPluginSuction->SetSuctionRegion(m_centertoolpartconvex.m_rigidbody, regionCenter,
                GFPhysVector3(1, 0, 0),
                GFPhysVector3(0, 1, 0),
                GFPhysVector3(0, 0, -1),
                SUCTION_HEAD_RADIUS,
                0.1);
        }
        
		m_ToolPlugins.push_back(pPluginSuction);

		m_pPluginSuction = pPluginSuction;
	}

	return true;
}
//========================================================================================================
void SuctionAndIrrigationTube::onFrameUpdateStarted(float timeelapsed)
{
	CTool::onFrameUpdateStarted(timeelapsed);
}
//========================================================================================================
void SuctionAndIrrigationTube::onFrameUpdateEnded()
{
	CTool::onFrameUpdateEnded();
}
//========================================================================================================
bool SuctionAndIrrigationTube::Update(float dt)
{
	__super::Update(dt);

	if(m_bElectricRightPad != m_LastRightElecBtn)
	{
		if(m_bElectricRightPad)
			m_pPluginSuction->SetCanSuck(true);
		else
			m_pPluginSuction->SetCanSuck(false);

		m_LastRightElecBtn = m_bElectricRightPad;
	}
	
	if(m_bElectricLeftPad)
	{
		WaterManager * pWaterMgr = EffectManager::Instance()->GetWaterManager();
		if(!m_IsWaterColumnExistent && pWaterMgr)
		{
            m_WaterColumnID = pWaterMgr->addWaterColumn(m_curKernelPos, GPVec3ToOgre(m_pPluginSuction->GetSuctionInvDir()), 0.15f);
			m_IsWaterColumnExistent = true;
		}
		else
            pWaterMgr->setWaterColumn(m_WaterColumnID, m_curKernelPos, GPVec3ToOgre(m_pPluginSuction->GetSuctionInvDir()), 0.15f);
		
		//AddWaterToWaterPool(dt);
	}
	else
	{
		RemoveWaterColumn();
	}

	m_LastLeftElecBtn = m_bElectricLeftPad;

	return true;
}
//============================================================================================================
void SuctionAndIrrigationTube::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationStart( currStep ,  TotalStep ,  dt);


}
//============================================================================================================
bool SuctionAndIrrigationTube::RemoveWaterColumn(void)
{
	if(m_IsWaterColumnExistent)
	{
		WaterManager * pWaterMgr = EffectManager::Instance()->GetWaterManager();
		if(pWaterMgr==NULL)
			return false;

		pWaterMgr->delWaterColumn(m_WaterColumnID);
		m_IsWaterColumnExistent = false;
		m_WaterColumnID = -1;
	}
	return true;
}
//============================================================================================================
void SuctionAndIrrigationTube::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{
	CTool::InternalSimulationEnd( currStep ,  TotalStep ,  dt);
}
//============================================================================================================
GFPhysVector3 SuctionAndIrrigationTube::CalculateToolCustomForceFeedBack()
{
	//Total Drag force
	GFPhysVector3 TotalDragForce(0,0,0);

	for(size_t p = 0 ; p < m_ToolPlugins.size() ; p++)
	{
		MisCTool_PluginSuction * sunctPlugin = dynamic_cast<MisCTool_PluginSuction*>(m_ToolPlugins[p]);
		if(sunctPlugin)
		{
			TotalDragForce += sunctPlugin->GetPluginForceFeedBack()*0.1f;
		}
	}

	return TotalDragForce;
}
//============================================================================================================
void SuctionAndIrrigationTube::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{	

}
//============================================================================================================
void SuctionAndIrrigationTube::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	
}
//============================================================================================================
GFPhysVector3 SuctionAndIrrigationTube::GetSuctionCenter() { return m_pPluginSuction->GetSuctionCenter(); }
//===========================================================================================================
GFPhysVector3 SuctionAndIrrigationTube::GetSuctionInvDir() { return m_pPluginSuction->GetSuctionInvDir(); }
//===========================================================================================================
void SuctionAndIrrigationTube::AddWaterToWaterPool(float dt)
{
	ITraining * train =  GetOwnerTraining();
	if(train)
	{
		MisNewTraining * newTraining = dynamic_cast<MisNewTraining*>(train);
		if(newTraining)
		{
			int num = newTraining->GetNumOfWaterPools();
			for(size_t p = 0 ; p < num ; p++)
			{
				WaterPool * pPool = newTraining->GetWaterPools(p);
				pPool->AddWater(dt * 20.0f);
			}
		}
	}
}