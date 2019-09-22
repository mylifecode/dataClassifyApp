#include "ACTitaniumClipTraining.h"
#include "MisMedicOrganOrdinary.h"
#include "Instruments/Tool.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "ACTubeShapeObject.h"
#include "InputSystem.h"
#include "WaterPool.h"
#include "MxToolEvent.h"
#include "SYScoreTableManager.h"
#include "Instruments/DissectingForcepsSize10.h"
#include "Instruments/SuctionAndlrrigationTube.h"
#include "Instruments/MisCTool_PluginSuction.h"

#define XUEGUAN_ID1 101
#define XUEGUAN_ID2 102
#define XUEGUAN_ID3 103

#define CUTPOINT0 62
#define CUTPOINT1 68

#define BLOOD_RADIUS	0.001
#define BLOOD_FLOW		0.01*0.01*0.01*1.25

//=============================================================================================
ACTitaniumClipTraining::ACTitaniumClipTraining(float limitTime)
	:m_limitTime(limitTime),
	m_hasLimitTime(limitTime > 0.f),
	m_curWaterPool(nullptr),
	m_hasOperation(false),
	m_hasSuction(false),
	m_releaseClipTool(nullptr),
	m_toolMinShaft(0.f),
	m_hasReleaseClip(false),
	m_needCheckKeepClip(false),
	m_hasCheckedKeepClip(false),
	m_isKeepClip(true),
	m_curKeepClipTime(0.f),
	m_nUnFullClip(0),
	m_needCleanBleed(false),
	m_terminalWaitTime(10.f)
{
	
}
//=============================================================================================
ACTitaniumClipTraining::~ACTitaniumClipTraining(void)
{
	
}
//======================================================================================================================
bool ACTitaniumClipTraining::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	//
	MisMedicOrgan_Ordinary * Dome_Active = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[100]);
	Ogre::Vector3 P0 = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetPivotPosition();
	Ogre::Vector3 P1 = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetPivotPosition();
	std::vector<GFPhysSoftBodyFace*> aroundfaces;
	Dome_Active->DisRigidCollAroundPoint(aroundfaces, OgreToGPVec3(P0), 2.0f, false);
	Dome_Active->DisRigidCollAroundPoint(aroundfaces, OgreToGPVec3(P1), 2.0f, false);

	
	m_markedTexture = Ogre::TextureManager::getSingletonPtr()->load("xueguan_mark.dds", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	m_blankTexture = Ogre::TextureManager::getSingletonPtr()->getByName("MixDefaultTex.dds");

	const int nVein = 3;
	m_veinInfos.resize(nVein);

	m_veinInfos[0].m_vein = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[XUEGUAN_ID1]);
	m_veinInfos[0].m_TearNode0 = 57;
	m_veinInfos[0].m_TearNode1 = 51;

	m_veinInfos[1].m_vein = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[XUEGUAN_ID2]);
	m_veinInfos[1].m_TearNode0 = 57;
	m_veinInfos[1].m_TearNode1 = 51;

	m_veinInfos[2].m_vein = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[XUEGUAN_ID3]);
	m_veinInfos[2].m_TearNode0 = 51;
	m_veinInfos[2].m_TearNode1 = 45;

	for(auto& veinInfo : m_veinInfos){
		veinInfo.m_materialPtr = veinInfo.m_vein->GetOwnerMaterialPtr();
	}

	return result; 
}

void ACTitaniumClipTraining::OnTrainingIlluminated()
{
	MisNewTraining::OnTrainingIlluminated();

	float weights[3] = {0.33333f, 0.33333f, 0.33333f};
	m_veinInfos[0].m_vein->ApplyEffect_VolumeBlood(169, weights);
	m_veinInfos[0].m_vein->SetVolumeBloodParameter(BLOOD_RADIUS,BLOOD_FLOW);

	m_veinInfos[1].m_vein->ApplyEffect_VolumeBlood(169, weights);
	m_veinInfos[1].m_vein->SetVolumeBloodParameter(BLOOD_RADIUS, BLOOD_FLOW);

	m_veinInfos[2].m_vein->ApplyEffect_VolumeBlood(169, weights);
	m_veinInfos[2].m_vein->SetVolumeBloodParameter(BLOOD_RADIUS, BLOOD_FLOW);

	if(m_WaterPools.size()){
		MisMedicOrgan_Ordinary * Dome_Active = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[100]);

		m_curWaterPool = m_WaterPools[0];
		m_curWaterPool->OnOrganBleeding(XUEGUAN_ID1, 1);
		m_curWaterPool->OnOrganBleeding(XUEGUAN_ID2, 1);
		m_curWaterPool->OnOrganBleeding(XUEGUAN_ID3, 1);
		m_curWaterPool->SetHeight(0.5f);
		//GFPhysSoftBodyNode * refNode = Dome_Active->m_physbody->GetNode(169);
		//m_curWaterPool->SetReferencNode(refNode);
	}
	CTipMgr::Instance()->ShowTip("GraspTheVessel");
}

//======================================================================================================================
void ACTitaniumClipTraining::InternalSimulateStart(int currStep, int TotalStep, Real dt)
{
	MisNewTraining::InternalSimulateStart(currStep, TotalStep, dt);
}
//======================================================================================================================
void ACTitaniumClipTraining::InternalSimulateEnd(int currStep, int TotalStep, Real dt)
{
	MisNewTraining::InternalSimulateEnd(currStep, TotalStep, dt);
}
//=============================================================================================
bool ACTitaniumClipTraining::BeginRendOneFrame(float timeelpsed)
{
	MisNewTraining::BeginRendOneFrame(timeelpsed);

	return true;
}
//===================================================================

void ACTitaniumClipTraining::CheckTearOff(int index)
{
	MisMedicOrgan_Ordinary* vein = m_veinInfos[index].m_vein;
	
	GFPhysSoftBodyNode * node0 = vein->m_physbody->GetNode(m_veinInfos[index].m_TearNode0);
	
	GFPhysSoftBodyNode * node1 = vein->m_physbody->GetNode(m_veinInfos[index].m_TearNode1);

	if (node0 && node1)
	{
		float currlength = (node0->m_CurrPosition - node1->m_CurrPosition).Length();
		float   restLenght = (node0->m_UnDeformedPos - node1->m_UnDeformedPos).Length();
		if (currlength > restLenght * 1.4f)
		{
			GFPhysVector3 cutcenter = (node0->m_UnDeformedPos + node1->m_UnDeformedPos) * 0.5f;

			GFPhysVector3 noramlDir = (node0->m_UnDeformedPos - node1->m_UnDeformedPos).Normalized();

			GFPhysVector3 dir0 = Perpendicular(noramlDir);

			GFPhysVector3 dir1 = noramlDir.Cross(dir0).Normalized();

			GFPhysVector3 cutQuads[4];
			cutQuads[0] = cutcenter - dir0 * 20 - dir1 * 20;
			cutQuads[1] = cutcenter + dir0 * 20 - dir1 * 20;
			cutQuads[2] = cutcenter - dir0 * 20 + dir1 * 20;
			cutQuads[3] = cutcenter + dir0 * 20 + dir1 * 20;

			vein->TearOrganBySemiInfinteQuad(cutQuads, false);

			m_veinInfos[index].m_HasBeTearedOff = true;
			m_veinInfos[index].m_vein->StopEffect_VolumeBlood();
			m_veinInfos[index].m_CanBlood = false;
		}
		
	}
}
//======================================================================================================================
bool ACTitaniumClipTraining::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);

	if (!m_bTrainingRunning)
		return result;

	if(m_needCleanBleed){
		//m_terminalWaitTime -= dt;
		//if(m_terminalWaitTime <= 0.f)
		if(m_curWaterPool->GetCurrHeight() <= 0)
			TrainingFinish();

		return result;
 	}

	int  numVesselBeCliped = 0;

	bool tipNotTearVessel = false;

	bool TipTubeBeErrorCliped = false;

	bool TipTubeBeCliped = false;

	CheckSuction();
	CheckKeepClip(dt);

	//check tool clamp organ
	for(std::size_t i = 0; i < m_veinInfos.size(); ++i){
		MisMedicOrgan_Ordinary* vein = m_veinInfos[i].m_vein;
		if(vein)
		{
			CDissectingForcepsSize10 * leftTool = dynamic_cast<CDissectingForcepsSize10*>(GetLeftTool());
			
			CDissectingForcepsSize10 * rightTool = dynamic_cast<CDissectingForcepsSize10*>(GetRightTool());

			bool isGraspedByTool = false;
			
			if (leftTool && vein->IsClampedByLTool())
			{
				isGraspedByTool = true;
			}
			if (rightTool && vein->IsClampedByRTool())
			{
				isGraspedByTool = true;
			}
			
			if (isGraspedByTool && (!m_veinInfos[i].m_HasBeTearedOff)){
				
				ApplyTextureToMaterial(m_veinInfos[i].m_materialPtr, m_markedTexture, "MixTextureMap");
				
				m_veinInfos[i].m_HasBeGrasped = true;
				
				m_veinInfos[i].m_IsBeGraspedNow = true;
				
				//检查抓取的血管是否被扯断
				CheckTearOff(i);
				
				if (m_veinInfos[i].m_HasBeTearedOff)
					tipNotTearVessel = true;

				if (m_veinInfos[i].m_ClipGoodCount == 0)
				{
					if (m_veinInfos[i].m_ClipErrorCount == 0)
					{
						TipTubeBeCliped = true;
					}
					else
					{
						TipTubeBeErrorCliped = true;
					}
				}
			}
			else{
				ApplyTextureToMaterial(m_veinInfos[i].m_materialPtr, m_blankTexture, "MixTextureMap");
				m_veinInfos[i].m_IsBeGraspedNow = false;
			}

			if(m_veinInfos[i].m_IsBeGraspedNow){
				if(m_veinInfos[i].m_CanBlood){
					m_veinInfos[i].m_vein->SetVolumeBloodParameter(0, 0);
					m_veinInfos[i].m_BloodIsPaused = true;
				}
			}
			else{
				if(m_veinInfos[i].m_CanBlood && m_veinInfos[i].m_BloodIsPaused){
					m_veinInfos[i].m_BloodIsPaused = false;
					m_veinInfos[i].m_vein->SetVolumeBloodParameter(BLOOD_RADIUS, BLOOD_FLOW);
				}
			}

			if (m_veinInfos[i].m_ClipGoodCount + m_veinInfos[i].m_ClipErrorCount > 0)
				numVesselBeCliped++;
		}
	}
	int numVesselCompleted = 0;
	int nTearVein = 0;

	for (std::size_t i = 0; i < m_veinInfos.size(); ++i)
	{

		if(m_veinInfos[i].m_HasBeTearedOff){
			nTearVein++;
			numVesselCompleted++;
		}
		else if(m_veinInfos[i].m_ClipGoodCount + m_veinInfos[i].m_ClipErrorCount > 0)
		{
			numVesselCompleted++;
		}
	}

	if(nTearVein == m_veinInfos.size()){
		TrainingFatalError();
	}
	else if(numVesselCompleted >= m_veinInfos.size())
	{
		//TrainingFinish();
		m_needCleanBleed = true;
	}

	if(numVesselCompleted > 0)
		m_hasOperation = true;

	if (m_bTrainingIlluminated)
	{
		if (tipNotTearVessel)
			CTipMgr::Instance()->ShowTip("DonnotTear");

		else if (TipTubeBeCliped)
			CTipMgr::Instance()->ShowTip("ClipInMarkPos");

		else if (TipTubeBeErrorCliped)
			CTipMgr::Instance()->ShowTip("ClipPosError");

		else if (numVesselBeCliped == 0)
			CTipMgr::Instance()->ShowTip("GraspTheVessel");
		else if(numVesselCompleted < m_veinInfos.size())
			CTipMgr::Instance()->ShowTip("GraspNextVessel");
		else
			CTipMgr::Instance()->ShowTip("CleanBleed");
	}
	
	
	return result;
}
//======================================================================================================================
void ACTitaniumClipTraining::AddDefaultScoreItemDetail()
{
	MisNewTraining::AddDefaultScoreItemDetail();

	//夹闭位置有3处错误
	RemoveScoreItemDetail("0170302219");
}
//======================================================================================================================
void ACTitaniumClipTraining::OnSaveTrainingReport()
{
	int numMarkBeExpolred = 0;

	int numVeinBeClipped = 0;

	int numVeinBeClippedGood = 0;//夹闭位置正确

	int numClipUsed = 0;

	bool isClipOnceSucceed = true;//是否一次夹闭成功

	bool AlwaysGraspWhenClip = true;

	int tearoffnum = 0;

	const int TimeUsed = GetElapsedTime();

	if(m_hasOperation){
		for(int c = 0; c < m_veinInfos.size(); c++)
		{
			VeinInfo & veinInfo = m_veinInfos[c];

			if(veinInfo.m_HasBeTearedOff)
				tearoffnum++;

			if(veinInfo.m_HasBeGrasped)
				numMarkBeExpolred++;

			if(veinInfo.m_ClipGoodCount > 0)
				numVeinBeClippedGood++;

			if(veinInfo.m_ClipGoodCount + veinInfo.m_ClipErrorCount > 0)
			{
				numVeinBeClipped++;
			}

			if(veinInfo.m_ClipErrorCount > 0)
			{
				isClipOnceSucceed = false;
			}

			if(veinInfo.m_AlwaysGraspWhenClip == false)
				AlwaysGraspWhenClip = false;

			numClipUsed += (veinInfo.m_ClipGoodCount + veinInfo.m_ClipErrorCount);

		}

		QString trianCodePrefix("017");

		//暴露标记位置个数
		if(numVeinBeClippedGood == 0)
			AddScoreItemDetail(trianCodePrefix + "0101809", 0);
		else if(numMarkBeExpolred == 3)
			AddScoreItemDetail(trianCodePrefix + "0101801", 0);
		else
			AddScoreItemDetail(trianCodePrefix + "0101800", 0);

		//成功夹闭个数
		if(numVeinBeClipped == 0)
			AddScoreItemDetail(trianCodePrefix + "0204419", 0);
		else if(numVeinBeClipped == 1)
			AddScoreItemDetail(trianCodePrefix + "0204412", 0);
		else if(numVeinBeClipped == 2)
			AddScoreItemDetail(trianCodePrefix + "0204411", 0);
		else
			AddScoreItemDetail(trianCodePrefix + "0204410", 0);

		//夹闭位置全部正确
		if(numVeinBeClipped > 0)
		{
			if(numVeinBeClippedGood == 0)
				AddScoreItemDetail(trianCodePrefix + "0302219", 0);
			else if(numVeinBeClippedGood == 1)
				AddScoreItemDetail(trianCodePrefix + "0302212", 0);
			else if(numVeinBeClippedGood == 2)
				AddScoreItemDetail(trianCodePrefix + "0302211", 0);
			else
				AddScoreItemDetail(trianCodePrefix + "0302210", 0);
		}

		//是否一次夹闭成功
		if(numVeinBeClipped)
		{
			if(isClipOnceSucceed)
				AddScoreItemDetail(trianCodePrefix + "0400110", 0);
			else
				AddScoreItemDetail(trianCodePrefix + "0400111", 0);
		}

		//夹闭时间
		if(m_hasReleaseClip){
			if(m_isKeepClip)
				AddScoreItemDetail(trianCodePrefix + "0505610", 0);
			else
				AddScoreItemDetail(trianCodePrefix + "0505611", 0);
		}

		//钛夹数量合理
		if(numClipUsed > 0){
			if(numClipUsed <= 4)
				AddScoreItemDetail(trianCodePrefix + "0604300", 0);
			else
				AddScoreItemDetail(trianCodePrefix + "0604301", 0);
		}

		//是否冲洗排除血水
		if(m_curWaterPool){
			if(m_hasSuction){
				if(m_curWaterPool->GetCurrHeight() <= 0)
					AddScoreItemDetail(trianCodePrefix + "0703000", 0);
				else
					AddScoreItemDetail(trianCodePrefix + "0703001", 0);
			}
			else
				AddScoreItemDetail(trianCodePrefix + "0703009", 0);
		}

		//操作得当，没有其他损伤
		if(tearoffnum == 0)
			AddScoreItemDetail(trianCodePrefix + "0801300", 0);
		else
			AddScoreItemDetail(trianCodePrefix + "0801308", 0);
	
		//施夹过程中始终暴露标记
		if(AlwaysGraspWhenClip)
			AddScoreItemDetail(trianCodePrefix + "0904510", 0);
		else
			AddScoreItemDetail(trianCodePrefix + "0904511", 0);

		//整体操作情况
		if(numVeinBeClippedGood == 3 && isClipOnceSucceed)
		{
			AddScoreItemDetail(trianCodePrefix + "1000300", 0);
		}
		else if(numVeinBeClipped == 3)
		{
			AddScoreItemDetail(trianCodePrefix + "1000301", 0);
		}

		float leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
		float rightToolSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
		if(leftToolSpeed > 10 || rightToolSpeed > 10)
		{
			AddScoreItemDetail(trianCodePrefix + "1100802", 0);//移动速度过快，有安全隐患
		}
		else if(leftToolSpeed < 5 && rightToolSpeed < 5)
		{
			AddScoreItemDetail(trianCodePrefix + "1100800", 0);//移动平稳流畅
		}
		else
		{
			AddScoreItemDetail(trianCodePrefix + "1100801", 0);//移动速度较快
		}

		//操作是否熟练
		if(TimeUsed < 60)
			AddScoreItemDetail(trianCodePrefix + "1200500", 0);//1分钟内完成所有操作
		else if(TimeUsed < 120)
			AddScoreItemDetail(trianCodePrefix + "1200501", 0);//在2分钟~3分钟内完成所有操作
		else
			AddScoreItemDetail(trianCodePrefix + "1200502", 0);//完成所有规定操作时超过了3分钟

		if(m_hasReleaseClip){
			if(m_nUnFullClip == 0)
				AddScoreItemDetail(trianCodePrefix + "1304210", 0);//所有钛夹均充分夹闭
			else if(m_nUnFullClip == 1)
				AddScoreItemDetail(trianCodePrefix + "1304211", 0);//有一颗钛夹未充分夹闭
			else
				AddScoreItemDetail(trianCodePrefix + "1304212", 0);//有两颗以上钛夹未充分夹闭
		}
		
	}

	__super::OnSaveTrainingReport();
}

bool ACTitaniumClipTraining::OrganShouldBleed(MisMedicOrgan_Ordinary* organ, int faceID, float* pFaceWeight)
{
	if(organ->m_OrganID >= XUEGUAN_ID1 && organ->m_OrganID <= XUEGUAN_ID3)
		return true;
	else
		return false;
}

void ACTitaniumClipTraining::OnTimerTimeout(int id, float dt, void* userData)
{
	
}

bool ACTitaniumClipTraining::IsClipInOrganValidError(MisMedicTitaniumClampV2 * clip, MisMedicOrgan_Ordinary * organ , float minv , float maxv)
{
	for (int c = 0; c < organ->m_physbody->GetNumFace(); c++)
	{
		 GFPhysSoftBodyFace * face = organ->m_physbody->GetFaceAtIndex(c);
		 Ogre::Vector3 triVerts[3];
		 triVerts[0] = GPVec3ToOgre(face->m_Nodes[0]->m_UnDeformedPos);
		 triVerts[1] = GPVec3ToOgre(face->m_Nodes[1]->m_UnDeformedPos);
		 triVerts[2] = GPVec3ToOgre(face->m_Nodes[2]->m_UnDeformedPos);

		 Ogre::Vector3 intersecPt[3];
		 int numPt = clip->GetTriEdgesIntersectClip(triVerts, intersecPt);
		 for (int c = 0; c < numPt; c++)
		 {
			 float weights[3];
			 CalcBaryCentric(OgreToGPVec3(triVerts[0]), OgreToGPVec3(triVerts[1]), OgreToGPVec3(triVerts[2]),
				             OgreToGPVec3(intersecPt[c]), weights[0], weights[1], weights[2]);

			 Ogre::Vector2 texCoord = organ->GetTextureCoord(face, weights);

			 if (texCoord.y > minv && texCoord.y < maxv)
			 {
				 return true;
			 }
		 }
	}
	return false;
}

//======================================================================================================================
SYScoreTable* ACTitaniumClipTraining::GetScoreTable()
{
	return SYScoreTableManager::GetInstance()->GetTable("01101701");
}

//======================================================================================================================
void ACTitaniumClipTraining::OnHandleEvent(MxEvent* pEvent)
{
	MxToolEvent * pToolEvent = NULL;
	
	switch (pEvent->m_enmEventType)
	{
	case MxEvent::MXET_AddHemoClip:
		 pToolEvent = (MxToolEvent*)(pEvent);
		 
		 MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary *>(pToolEvent->GetOrgan());
		 
		 MisMedicTitaniumClampV2 * titanClip = (MisMedicTitaniumClampV2 *)(pToolEvent->m_pUserPoint);
		 
		 VeinInfo * veininfo = FindVienInfo(organ);
		 
		 if (pToolEvent && veininfo)
		 {
			 bool isvalid = IsClipInOrganValidError(titanClip, organ, 0.406f, 0.549f);

			 if (isvalid)
			 {
				 veininfo->m_ClipGoodCount++;
				 
			 }
			 else
			 {
				 veininfo->m_ClipErrorCount++;
			 }

			 if (veininfo->m_IsBeGraspedNow == false)
			 {
				 veininfo->m_AlwaysGraspWhenClip = false;
			 }

			 organ->StopEffect_VolumeBlood();
			 veininfo->m_CanBlood = false;

			 m_curWaterPool->OnOrganStopBleeding(organ->m_OrganID, 1);
		 }

		 //只有当夹住器官时，才部位空，释放空夹子，指针为空
		 if(titanClip){
			 m_hasReleaseClip = true;
			 m_needCheckKeepClip = true;
			 m_curKeepClipTime = 0.f;

			 m_releaseClipTool = pToolEvent->m_pTool;
			 m_toolMinShaft = m_releaseClipTool->GetShaftAside();
			 m_needCheckKeepClip = true;

			 //判断夹闭是否充分
			 float percent = titanClip->GetValidClipPercent();
			 if(percent < 0.65f)
				 ++m_nUnFullClip;
		 }

		 break;
	}
}

void ACTitaniumClipTraining::CheckSuction()
{
	if(m_hasSuction == false){
		auto* leftTool = dynamic_cast<SuctionAndIrrigationTube*>(GetLeftTool());

		if(leftTool){
			MisCTool_PluginSuction* plugin = leftTool->GetPluginSuction();
			if(plugin){
				if(plugin->IsCanSucked())
					m_hasSuction = true;
			}
		}

		if(m_hasSuction == false){
			auto* rightTool = dynamic_cast<SuctionAndIrrigationTube*>(GetRightTool());

			if(rightTool){
				MisCTool_PluginSuction* plugin = rightTool->GetPluginSuction();
				if(plugin){
					if(plugin->IsCanSucked())
						m_hasSuction = true;
				}
			}
		}
	}
}

void ACTitaniumClipTraining::CheckKeepClip(float dt)
{
	if(m_needCheckKeepClip && m_hasCheckedKeepClip == false){

		ITool* leftTool = GetLeftTool();
		ITool* rightTool = GetRightTool();
		bool canStopCheck = false;

		m_curKeepClipTime += dt;

		if(leftTool == m_releaseClipTool || rightTool == m_releaseClipTool){
			if(m_curKeepClipTime < 2.f){
				float d = m_toolMinShaft - m_releaseClipTool->GetShaftAside();
				if(d > FLT_EPSILON || d < -FLT_EPSILON){
					m_hasCheckedKeepClip = true;
					m_isKeepClip = false;
				}
			}
			else{
				m_needCheckKeepClip = false;
			}
		}
		else{
			if(m_curKeepClipTime < 2.f){
				m_hasCheckedKeepClip = true;
				m_isKeepClip = false;
			}
			m_needCheckKeepClip = false;
		}
	}
}