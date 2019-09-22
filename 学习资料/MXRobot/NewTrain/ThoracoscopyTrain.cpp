#include "ThoracoscopyTrain.h"
#include "MisMedicOrganOrdinary.h"
#include "TextureBloodEffect.h"
#include "TextureWaterEffect.h"
#include "MisMedicEffectRender.h"
#include "EffectManager.h"
#include "time.h"
#include "MisMedicOrganAttachment.h"
#include "../Instruments/GraspingForceps.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "XMLWrapperTraining.h"
#include "ShadowMap.h"
#include "ScreenEffect.h"
#include "DeferredRendFrameWork.h"
#define THORATRAINGROUNDRIGID (OPC_USERCUSTOMSTART << 1)
//=============================================================================================
CThoracoscopyTrain::CThoracoscopyTrain(void)
{
	srand((unsigned)time(NULL));
	m_GroundSphere[0] = m_GroundSphere[1] = 0;
}
//=============================================================================================
CThoracoscopyTrain::~CThoracoscopyTrain(void)
{
	if (m_GroundSphere[0])
	{
		PhysicsWrapper::GetSingleTon().DestoryRigidBody(m_GroundSphere[0]);
	}

	if (m_GroundSphere[1])
	{
		PhysicsWrapper::GetSingleTon().DestoryRigidBody(m_GroundSphere[1]);
	}
}
//=============================================================================================
void CThoracoscopyTrain::SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ)
{
	if (organ->m_OrganID == EDOT_LUNG_FASICA)
	    organ->GetCreateInfo().m_distributemass = false;
}
//=============================================================================================
bool CThoracoscopyTrain::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);
	return result;
}
//=============================================================================================
void CThoracoscopyTrain::CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig)
{
	MisNewTraining::CreateTrainingScene(pTrainingConfig);
}
//======================================================================================================================
bool CThoracoscopyTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);
	
    PhysicsWrapper::GetSingleTon().m_dynamicsWorld->SoftCollisionHashGridSize(1.0f);

	m_TrainName = pTrainingConfig->m_Name;

	DynObjMap::iterator itor = m_DynObjMap.begin();
	
	while(itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * oif = itor->second;
		
		MisMedicOrgan_Ordinary * organmesh = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);
		
		if(organmesh)
		{
			MisMedicDynObjConstructInfo & cs = organmesh->GetCreateInfo();

			if (cs.m_OrganType != EDOT_LOWERLOBE_LEFT)
			    organmesh->m_physbody->SetCollisionMask(organmesh->m_physbody->m_MaskBits & (~THORATRAINGROUNDRIGID));

			if(cs.m_OrganType == EDOT_LUNG_FASICA)//only for test
			{
				//organmesh->m_EleCatogeryCanCut = EMMT_BeMesentary;//only can cut menstaryEMMT_BeTissue | 
				organmesh->m_ElecCutRadius = 0.3f;
				organmesh->m_MaxCutCount = 3;
				organmesh->SetTetraCollapseParam(0.3f, 1);
				organmesh->SetBleedRadius(0.015f);
				//organmesh->GetCreateInfo().m_BloodRadius = 0.002f;
			}
			if (cs.m_OrganType == EDOT_LUNG_YEJIANLIE)
			{
				organmesh->m_MaxCutCount = 2;
				organmesh->SetTetraCollapseParam(0.3f, 1); //(bad tetra ration , start collapse time)
			}
			if(cs.m_OrganType == EDOT_UPPERLOBE_LEFT || cs.m_OrganType == EDOT_LOWERLOBE_LEFT)//only for test
			{
				organmesh->SetMinPunctureDist(1000.0f);
			}

			if (cs.m_OrganType == EDOT_BRONCHIAL || cs.m_OrganType == EDOT_LUNGVEIN || cs.m_OrganType == EDOT_OULMONARYARTERY)//only for test
			{
				organmesh->m_MaxCutCount = 1;
				organmesh->SetTimeNeedToEletricCut(1000);
				organmesh->SetTetraCollapseParam(0.3f, 1);
			}
		}
		itor++;
	}

    //DeferredRendFrameWork::Get()->SetBloomBrightPassThreshold(1.0f);
	//
	m_DetectCameraIntersect = true;
	return result;
}
//==============================================================================================================
void CThoracoscopyTrain::OnOrganCutByTool(MisMedicOrganInterface * organ , bool iselectriccut)
{
	
}
//========================================================================================================
void CThoracoscopyTrain::onDebugMessage(const std::string& value)
{
	
}