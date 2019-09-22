#include "SYElectricHookTrain.h"
#include "MisMedicOrganOrdinary.h"
#include "instruments\ElectricNeedle.h"
#include "VeinConnectObject.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "TextureBloodEffect.h"
#include "SYScoreTableManager.h"
#define VESSELID 28
//=============================================================================================
SYElectricHookTrain::SYElectricHookTrain(void)
{
	m_ConnectObject = 0;
	m_ClusterRemovedNum = 0;
	//m_VelsselBeBurnTime = 0;
	//m_VesselBeBurned = false;

	m_VesselBeElecCogTime = 0;
	m_VesselBeElecCutTime = 0;

	m_VesselBeElecCog = false;
	m_VesselBeElecCut = false;

	m_IsElecCogTimeExceed = false;
	m_IsElecCutTimeExceed = false;
}
//=============================================================================================
SYElectricHookTrain::~SYElectricHookTrain(void)
{
	

}
//======================================================================================================================
bool SYElectricHookTrain::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig, pToolConfig);

	m_ConnectObject = dynamic_cast<VeinConnectObject*>(m_DynObjMap[10]);

	CheckPairsInProcessRange(m_ConnectObject, GFPhysVector3(-0.007f, -7.435f, -10.081f), 2.0f);

	return result;
}
//======================================================================================================================
bool SYElectricHookTrain::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);

	std::map<OrganSurfaceBloodTextureTrack*, InjuryPoint>::iterator itor = m_bloodTrackTimeMap.begin();
	while (itor != m_bloodTrackTimeMap.end())
	{
		if (itor->second.m_IsStoped == false)
		{
			itor->second.m_BloodTotalTime += dt;
		}
		itor++;
	}

	if (m_VesselBeElecCog)
	{
		m_VesselBeElecCogTime += dt;
	}
	else
	{
		m_VesselBeElecCogTime = 0;
	}

	if (m_VesselBeElecCut)
	{
		m_VesselBeElecCutTime += dt;
	}
	else
	{
		m_VesselBeElecCutTime = 0;
	}

	if (m_VesselBeElecCogTime > 4.0f)
	{
		m_IsElecCogTimeExceed = true;
	}

	if (m_VesselBeElecCutTime > 2.0f)
	{
		m_IsElecCutTimeExceed = true;
	}
	return result;
}
//=====================================================================================================================
void SYElectricHookTrain::CheckPairsInProcessRange( VeinConnectObject * pVeinConnectObject, const GFPhysVector3 & center, float radius)
{
	for (std::size_t c = 0; c < pVeinConnectObject->m_clusters.size(); ++c)
	{
		VeinConnectCluster  & cluster = pVeinConnectObject->m_clusters[c];

		VeinConnectPair & pair0 = cluster.m_pair[0];
		
		GFPhysSoftBodyFace * faceA0 = pair0.m_faceA;
		GFPhysSoftBodyFace * faceB0 = pair0.m_faceB;

		GFPhysVector3 pA0 = faceA0->m_Nodes[0]->m_UnDeformedPos*pair0.m_weightsA[0]
			+ faceA0->m_Nodes[1]->m_UnDeformedPos*pair0.m_weightsA[1]
			+ faceA0->m_Nodes[2]->m_UnDeformedPos*pair0.m_weightsA[2];

		GFPhysVector3 pB0 = faceB0->m_Nodes[0]->m_UnDeformedPos*pair0.m_weightsB[0]
			+ faceB0->m_Nodes[1]->m_UnDeformedPos*pair0.m_weightsB[1]
			+ faceB0->m_Nodes[2]->m_UnDeformedPos*pair0.m_weightsB[2];

		GFPhysVector3 closetPt = CloasetPtToSegment(center, pA0, pB0);

		float dist = (center - closetPt).Length();

		if (dist < radius)
		{
			m_ConnectClusterNeedProcess.push_back(c);//add to process list

			//cluster.m_Color = pair0.m_PairColor = Ogre::ColourValue::Red;//temp
		}
		
	}
}
//===================================================================================================================
SYScoreTable* SYElectricHookTrain::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01100701");
}
//=====================================================================================================================
void SYElectricHookTrain::OnHandleEvent(MxEvent* pEvent)
{
	VeinConnectPair * pPair = NULL;
	MxToolEvent * pToolEvent = (MxToolEvent *)pEvent;
	MisMedicOrgan_Ordinary * pOrgan = NULL;
	
	switch (pEvent->m_enmEventType)
	{
	case MxEvent::MXET_VeinConnectBreak:
	case MxEvent::MXET_VeinConnectBurned:
		 pPair = static_cast<VeinConnectPair*>(pToolEvent->m_pUserPoint);
		 for (int c = 0; c < m_ConnectClusterNeedProcess.size(); c++)
		 {
			 if (pPair == &(m_ConnectObject->m_clusters[c].m_pair[0]))
			 {
				 m_ClusterRemovedNum++;
				 break;
			 }
		 }
		 //CTipMgr::Instance()->ShowTip("OperateRatio", ratios[0], ratios[1], ratios[2]);
		 break;
	
	case MxEvent::MXET_BleedStart:
		 {
			pToolEvent = static_cast<MxToolEvent*>(pEvent);
			TextureBloodTrackEffect * bloodEffect = static_cast<TextureBloodTrackEffect*>(pToolEvent->m_pUserPoint);
			OrganSurfaceBloodTextureTrack * pBloodTrack = bloodEffect->GetLatestTextureBloodTrack();
			m_bloodTrackTimeMap.insert(make_pair(pBloodTrack, pBloodTrack));
		 }
		 break;
	case MxEvent::MXET_BleedEnd:
	     {
		    pToolEvent = static_cast<MxToolEvent*>(pEvent);
		    TextureBloodTrackEffect * bloodEffect = static_cast<TextureBloodTrackEffect*>(pToolEvent->m_pUserPoint);
		    OrganSurfaceBloodTextureTrack * pBloodTrack = bloodEffect->GetLatestTextureBloodTrack();
		   
		    std::map<OrganSurfaceBloodTextureTrack*, InjuryPoint>::iterator itor = m_bloodTrackTimeMap.find(pBloodTrack);
		    if (itor != m_bloodTrackTimeMap.end())
		    {
			   itor->second.m_IsStoped = true;
		    }
	     }
	     break;

	case MxEvent::MXET_ElecCoagKeep:
		 pOrgan = dynamic_cast<MisMedicOrgan_Ordinary *>(pToolEvent->GetOrgan());
		 if (pOrgan->m_OrganID == VESSELID)//电到血管
		 {
			m_VesselBeElecCog = true;
		 }
		 break;

	case MxEvent::MXET_ElecCutKeep:
		 pOrgan = dynamic_cast<MisMedicOrgan_Ordinary *>(pToolEvent->GetOrgan());
		 if (pOrgan->m_OrganID == VESSELID)//电到血管
		 {
			m_VesselBeElecCut = true;
		 }
		 break;
	
	case MxEvent::MXET_ElecCoagEnd:
		 pOrgan = dynamic_cast<MisMedicOrgan_Ordinary *>(pToolEvent->GetOrgan());
		 if (pOrgan->m_OrganID == VESSELID)//电到血管
		 {
			m_VesselBeElecCog = false;
		 }
		 break;
	
	case MxEvent::MXET_ElecCutEnd:
		 pOrgan = dynamic_cast<MisMedicOrgan_Ordinary *>(pToolEvent->GetOrgan());
		 if (pOrgan->m_OrganID == VESSELID)//电到血管
		 {
			m_VesselBeElecCut = false;
		 }
		 break;
	

	}
}

void SYElectricHookTrain::OnSaveTrainingReport()
{
	int RemoveNum = 0;

	for (int c = 0; c < m_ConnectClusterNeedProcess.size(); c++)
	{
		if (m_ConnectObject->m_clusters[c].m_pair[0].m_Valid == false)
			RemoveNum++;
	}

	float connectRemovePct = (float)RemoveNum / (float)m_ConnectClusterNeedProcess.size();
	
	//分离程度
	if (connectRemovePct > 0.9f){
		QString fullcode = QString("0070100110");
		AddScoreItemDetail(fullcode, 0.0f);
	}
	else if (connectRemovePct > 0.8f){
		QString fullcode = QString("0070100111");
		AddScoreItemDetail(fullcode, 0.0f);
	}
	else if (connectRemovePct > 0.6f){
		QString fullcode = QString("0070100112");
		AddScoreItemDetail(fullcode, 0.0f);
	}
	else if(connectRemovePct > 0.1f){
		QString fullcode = QString("0070100113");
		AddScoreItemDetail(fullcode, 0.0f);
	}
	else{
		QString fullcode = QString("0070100119");
		AddScoreItemDetail(fullcode, 0.0f);
	}
	
	//确认组织结构
	AddScoreItemDetail(QString("0070201200"), 0.0f);//temp

	//是否伤害重要组织
	if (m_IsElecCogTimeExceed || m_IsElecCutTimeExceed)
	{
		AddScoreItemDetail(QString("0070301311"), 0.0f);
	}
	else
	{
		AddScoreItemDetail(QString("0070301310"), 0.0f);
	}

	//是否造成出血或其他损伤
	int numBlood = m_bloodTrackTimeMap.size();
	if (numBlood > 3)
		AddScoreItemDetail(QString("0070301412"), 0.0f);//temp
	else if (numBlood > 0)
		AddScoreItemDetail(QString("0070301411"), 0.0f);//temp
	else
		AddScoreItemDetail(QString("0070301410"), 0.0f);//temp

	//止血
	bool hasBloodTimeLargeThan10 = false;

	std::map<OrganSurfaceBloodTextureTrack*, InjuryPoint>::iterator itor = m_bloodTrackTimeMap.begin();
	
	while (itor != m_bloodTrackTimeMap.end())
	{
		if (itor->second.m_BloodTotalTime > 10)
		{
			hasBloodTimeLargeThan10 = true;
			break;
		}
		itor++;
	}

	if (numBlood == 0)
	{
		AddScoreItemDetail(QString("0070301519"), 0.0f);
	}
	else if (hasBloodTimeLargeThan10)
	{
		AddScoreItemDetail(QString("0070301511"), 0.0f);
	}
	else
	{
		AddScoreItemDetail(QString("0070301510"), 0.0f);
	}

	//持续通电时间
	AddScoreItemDetail(QString("0070401610"), 0.0f);

	//通电效率
	AddScoreItemDetail(QString("0070401710"), 0.0f);

	//

	MisNewTraining::OnSaveTrainingReport();
}