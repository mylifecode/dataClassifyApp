#include "AcessoriesCutTrain.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicRigidPrimtive.h"
#include "Instruments/Tool.h"
#include "VeinConnectObject.h"
#include "TextureBloodEffect.h"
#include "MisMedicEffectRender.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "Dynamic/Constraint/GoPhysSoftFixpointConstraint.h"
#include "InputSystem.h"
#include "MXOgreGraphic.h"
#include "MXEvent.h"
#include "WaterPool.h"
#include "DeferredRendFrameWork.h"
// enum	AreaType
// {
// 	AT_Uterus,//	×Ó¹¬Ìå
// 	AT_Fallopian_Tube_Left,//	×óÊäÂÑ¹Ü
// 	AT_Fallopian_Tube_Right,//	ÓÒÊäÂÑ¹Ü
// 	AT_Ovary_Left,//	×óÂÑ³²
// 	AT_Ovary_Right,//	ÓÒÂÑ³²
// 	AT_Ligament_Left,//	×óÐüÈÍ´ø¹ÌÓÐÈÍ´ø
// 	AT_Ligament_Right,//	ÓÒÐüÈÍ´ø¹ÌÓÐÈÍ´ø
// 	AT_Determine_Area_Burn_1,
// 	AT_Determine_Area_Burn_2,
// 	AT_Focal_Zone_1,
// 	AT_Focal_Zone_2,
// 	AT_Focal_Zone_3,
// 	AT_TOTALNUM
// };

UINT32 AreaMark[] = 
{
	0x00001,		//	×Ó¹¬Ìå
	0x00004,		//	×óÊäÂÑ¹Ü
	0x00008,		//	ÓÒÊäÂÑ¹Ü
	0x00010,		//	×óÂÑ³²
	0x00020,		//	ÓÒÂÑ³²
	0x00040,		//	×óÐüÈÍ´ø¹ÌÓÐÈÍ´ø
	0x00080,		//	ÓÒÐüÈÍ´ø¹ÌÓÐÈÍ´ø
	0x00100,		//	¿ÉÉÕ°×ÇøÓò1
	0x00200,		//	¿ÉÉÕ°×ÇøÓò2
	0x01000,		//	ÅÐ¶¨ÇøÓò1
	0x02000,		//	ÅÐ¶¨ÇøÓò2
	0x10000,		//	²¡ÔîÇø3
	0x20000,		//	²¡ÔîÇø4
	0x40000,		//	²¡ÔîÇø5
};

#define ColFloorCat (MMRC_UserStart << 1)

void AccesoriesTrainingHandleEvent(MxEvent * pEvent, ITraining * pTraining)
{
	if (!pEvent || !pTraining)
		return;

	CAcessoriesCutTraining * pAcessoriesCutTraining = dynamic_cast<CAcessoriesCutTraining *> (pTraining);

	if(pAcessoriesCutTraining)
	{
		TrainScoreSystem * scoreSys = pAcessoriesCutTraining->GetScoreSystem();
		if(scoreSys)
		{
			scoreSys->ProcessTrainEvent(pEvent);
		}

		if(pEvent->m_enmEventType == MxEvent::MXET_ElecCutStart)
		{
			pAcessoriesCutTraining->ElecCutStart();
		}
		else if(pEvent->m_enmEventType == MxEvent::MXET_ElecCutEnd)
		{
			pAcessoriesCutTraining->ElecCutEnd();
		}
		else if(pEvent->m_enmEventType == MxEvent::MXET_TakeOutOrganWithSpecimenBag)
		{
			MxToolEvent * pToolEvent =  (MxToolEvent *)(pEvent);
			int organRatio = pToolEvent->m_UserData;		//Æ÷¹Ù±ÈÀý0-100%
			if(organRatio >= 90)
				pAcessoriesCutTraining->OnTakeOutSomething((int)pToolEvent->m_pUserPoint);
		}
		else if(pEvent->m_enmEventType == MxEvent::MXET_BurnCutFaceEnd)
		{
			MxToolEvent * pToolEvent =  (MxToolEvent *)(pEvent);
			int burnTime = pToolEvent->m_UserData;
			if(burnTime > 2000)
				pAcessoriesCutTraining->AddOperateItem("Over_Elec_Coagulation",true);
		}
	}
	

}
//=============================================================================================
CAcessoriesCutTraining::CAcessoriesCutTraining(const Ogre::String & strName)
:	m_bNeedCheck(false)
,	m_bToDebugDisplayMode(false)
,	m_mistakeCounterMax(0)
,	m_bFinished(false)
,	m_pEnvelopEmbryoAndUterus(NULL)
,	m_NumOfUterusBeClamped(0)
,	m_NumOfFallopianTubeBeClamped(0)
,	m_NumOfOvaryBeClamped(0) 
,   m_NumOfUterusBeDamaged(0)
,   m_NumOfFallopianTubeBeDamaged(0)
,   m_NumOfOvaryBeDamaged(0)

{
	m_trainName = strName;
	m_materialName = "";
	m_AreaMarkTextureName = "";
	memset(m_ScoreTimes, 0, sizeof(m_ScoreTimes));
//	m_BurnMarkArea = NULL;
// 	memset(m_BurnMarkArea, 0, sizeof(m_BurnMarkArea));
}
//=============================================================================================
CAcessoriesCutTraining::~CAcessoriesCutTraining(void)
{
// 	if (m_BurnMarkArea != NULL)
// 	{
// 		delete m_BurnMarkArea;
// 	}

	m_Area.setNull();
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetShowModel_Plus(false);   
    if(m_pDest)
    {
        delete []m_pDest;
        m_pDest = 0;
    }
}
bool CAcessoriesCutTraining::OrganShouldBleed(MisMedicOrgan_Ordinary* organ, int faceID, float* pFaceWeight)
{
	return true;
}
bool CAcessoriesCutTraining::BeginRendOneFrame(float timeelapsed)
{
	bool result  = MisNewTraining::BeginRendOneFrame(timeelapsed);

	//if(m_StaticPartMeshPtr.isNull())
	//   return true;

	////attach
	//m_StaticDynamicUnion.UpdateStaticVertexByDynamic(m_StaticPartMeshPtr);

	return result;
}
void CAcessoriesCutTraining::SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ)
{
	//if(organ->m_OrganID == EODT_UTERUS || organ->m_OrganID == EODT_PEITAI)
	//{
		//organ->GetCreateInfo().m_distributemass = true;
	//}
}
void CAcessoriesCutTraining::ReadCustomDataFile(const Ogre::String & customDataFile)
{
	std::ifstream stream;

	stream.open(customDataFile.c_str());

	if(stream.is_open())
	{
		char buffer[1000];
		while(!stream.eof())//while(stream.getline(buffer,99))
		{
			stream.getline(buffer,999);
			if(strcmp(buffer , "") != 0)
			{
				int faceIndex = atof(buffer);
				m_HiddedFaceIndex.push_back(faceIndex);
			}
		}
		
	}

}
//======================================================================================================================
bool CAcessoriesCutTraining::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{      
	bool result = MisNewTraining::Initialize(pTrainingConfig , pToolConfig);

	DynObjMap::iterator itor = m_DynObjMap.begin();

	while (itor != m_DynObjMap.end())
	{
		MisMedicOrganInterface * oif = itor->second;

		MisMedicOrgan_Ordinary * organmesh = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);

		MisMedicRigidPrimtive  * collidePlane = dynamic_cast<MisMedicRigidPrimtive*>(oif);

		if (collidePlane)
		{
			collidePlane->m_body->SetCollisionCategory(ColFloorCat);
		}
		else if (organmesh)
		{
			if (organmesh->GetOrganType() == EODT_DOME_ACTIVE)
			{
				organmesh->m_physbody->SetCollisionMask(organmesh->m_physbody->m_MaskBits & (~ColFloorCat));
			}
			else if (organmesh->GetOrganType() == EODT_UTERUS)
			{
				int numFace = organmesh->m_physbody->GetNumFace();
				for (int c = 0; c < numFace; c++)
				{
					GFPhysSoftBodyFace * face = organmesh->m_physbody->GetFaceAtIndex(c);
					face->m_RSCollisionMask &= (~ColFloorCat);
				}
			}
		}
		itor++;
	}
	//
	MisMedicOrgan_Ordinary * Dome_Active = dynamic_cast<MisMedicOrgan_Ordinary*>(m_DynObjMap[17]);
	
	Ogre::Vector3 P0 = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetPivotPosition();
	Ogre::Vector3 P1 = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetPivotPosition();
	std::vector<GFPhysSoftBodyFace*> aroundfaces;
	Dome_Active->DisRigidCollAroundPoint(aroundfaces, OgreToGPVec3(P0), 2.0f, false);
	Dome_Active->DisRigidCollAroundPoint(aroundfaces, OgreToGPVec3(P1), 2.0f, false);
	//

	m_DetectCameraIntersect = true;

	//DeferredRendFrameWork::Get()->SetBloomBrightPassThreshold(0.9f);

	CShadowMap::Instance()->m_ShadowLightPos = Ogre::Vector3(0,-0.4f,0);

	MisMedicOrganInterface * oif = GetOrgan(EODT_UTERUS);
	MisMedicOrgan_Ordinary * organzigong = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);
	if(organzigong)
	{
		organzigong->SetMinPunctureDist(2.0f);
		organzigong->m_physbody->SetRigidPoseForceCollideInflueceCoeff(0.8f);
		organzigong->m_physbody->SetRigidPoseForceAttachInflueceCoeff(1.0f);
		//MisSoftBodyPoseRigidForce * smforce = organzigong->GetPoseRigidForce();
		//if(smforce)
		//{
		  // smforce->SetCollideCoeff(0.8f);
		  // smforce->SetAttachCoeff(1.0f);
		//}
		MisMedicDynObjConstructInfo & cs = organzigong->GetCreateInfo();
		
		if(cs.m_OrganType == EODT_UTERUS)
		{
		   organzigong->setEffectRender_deviateScale(0.0f);
		   Ogre::ColourValue burncolor = Ogre::ColourValue(255.0f / 255.0f, 251.0f / 255.0f,  230.0f / 255.0f,1);

		   organzigong->SetMaxBloodTrackExistsTime(10.0f);
		   organzigong->SetBurnNodeColor(burncolor);
		   organzigong->SetBurnWhiteColor(burncolor , burncolor);
		organzigong->SetMinPunctureDist(0.5f);
		   //organzigong->SetBurnNodeColor(Ogre::ColourValue(222.0f/255.0f,200.0f/255.0f,176.0f/255.0f,1.0f));
		   organzigong->m_DragForceRate = 1.0f;
		   organzigong->SetIsolatedPartGravity(15.0f);
		   
		   for(size_t c = 0 ;c < m_HiddedFaceIndex.size(); c++)
		   {
			   int faceid = m_HiddedFaceIndex[c];
			   organzigong->m_OriginFaces[faceid].m_NeedRend = false;
			   organzigong->m_OriginFaces[faceid].m_physface->DisableCollideWithRigid();
		   }
		}
	}

	oif = GetOrgan(EODT_PEITAI);
	MisMedicOrgan_Ordinary * organPeiTai = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);
	if(organPeiTai)
	{
		MisMedicObjectEnvelop * envelop = new MisMedicObjectEnvelop();
		envelop->BuildEnvelop(*organPeiTai  , *organzigong, 0.01f  , 1.0f , true , false , 0 , 1);
		m_ObjEnvelops.push_back(envelop);
		m_pEnvelopEmbryoAndUterus = envelop;
		organzigong->SetEletricCutParameter(0.12f, organzigong->m_MaxCutCount);
	}
	else
	{
		//·ÇÈ¡ÅßÑµÁ·
		organzigong->SetEletricCutParameter(0.2f, organzigong->m_MaxCutCount);
	}

	//register train event 
	CMXEventsDump::Instance()->RegisterHandleEventsFunc(AccesoriesTrainingHandleEvent, this);


	//Ogre::Entity * entitystatic  = m_pOms->GetEntity("DomeStatic$1",true);//GetSceneNode("zigongjingtai$1" , true);
	//MisMedicOrgan_Ordinary * DomeActive = dynamic_cast<MisMedicOrgan_Ordinary*>(GetOrgan(EODT_DOME_ACTIVE));

	//if(entitystatic && DomeActive)
	//{

	//	Ogre::SceneNode * nodestatic = entitystatic->getParentSceneNode();

	//	m_StaticPartMeshPtr = entitystatic->getMesh();

	//	m_StaticDynamicUnion.AttachStaticMeshToDynamicOrgan(m_StaticPartMeshPtr , 
	//		                                                nodestatic,
	//											            nodestatic->getPosition() , 
	//											            nodestatic->getOrientation(), 
	//											            nodestatic->getScale() , 
	//											            DomeActive , 0.05f);

	//}




	m_TexturePtr = Ogre::TextureManager::getSingleton().createManual("CopyTargetTex_"+m_trainName ,
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME , 
		Ogre::TEX_TYPE_2D , 
		2048 , 
		2048 , 
		0 , 
		Ogre::PF_A8R8G8B8 , 
		Ogre::TU_DYNAMIC);
    //////////////////////////////////////////////////////////////////////////
    Ogre::HardwarePixelBufferSharedPtr pixelBuffer;
    if (m_Area.isNull())
    {
        return result;
    }
    try
    {
        pixelBuffer = m_Area->getBuffer();
        pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
    }
    catch (...)
    {
        return result;
    }

    Ogre::uint* pDest = static_cast<Ogre::uint*>(pixelBuffer->getCurrentLock().data);

    m_pDest = new Ogre::uint[m_AreaHeight*m_AreaWidth];

    for (int i = 0;i<m_AreaHeight*m_AreaWidth;i++)
    {
        m_pDest[i] = pDest[i];
    }

    pixelBuffer->unlock();

	return result;
}

bool CAcessoriesCutTraining::Update( float dt )
{
	bool result = MisNewTraining::Update(dt);

	bool bPlus =  InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetShowModel_Plus();
	if (m_bToDebugDisplayMode != bPlus && m_AreaMarkTextureName.length() > 0)
	{
		m_bToDebugDisplayMode = bPlus;
		if (m_bToDebugDisplayMode)
		{

			MisMedicOrganInterface * tmp = GetOrgan(EODT_UTERUS);
			MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

			m_materialName = pOO->getMaterialName();
			pOO->setMaterialName("Adnexa_Uteri_Debug");
			Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName("Adnexa_Uteri_Debug");
			Ogre::TexturePtr  srctex = Ogre::TextureManager::getSingleton().load(m_AreaMarkTextureName , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

			ApplyTextureToMaterial(material , srctex , "AreaMarkMap");

		}
		else
		{
			MisMedicOrganInterface * tmp = m_DynObjMap[12];
			MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);
			pOO->setMaterialName(m_materialName);

		}
	}

	if (!m_bFinished)
	{
		checkDianzi();
	}
	else
	{
		doFinishCount();
	}

	return result;
}

MisMedicOrganInterface * CAcessoriesCutTraining::LoadOrganism( MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain)
{
	if (cs.m_objTopologyType == DOT_UNDEF)	// this should be 
	{
		Ogre::String strEntityName = Ogre::String(cs.m_name);//(organconfig->m_Name) 
		+ "$" + Ogre::StringConverter::toString(m_pOms->s_nLoadCount);  
		m_pOms->s_nLoadCount++;
		//////////////////////////////////////////////////////////////////////////
		float mass = cs.m_mass;//organconfig->m_Mass;
		Ogre::String rigidType = cs.m_RigidType;//organconfig->m_RigidType;
		Ogre::String rigidName = cs.m_name;//organconfig->m_Name;
		transform(rigidType.begin(), rigidType.end(), rigidType.begin(), ::tolower);
		transform(rigidName.begin(), rigidName.end(), rigidName.begin(), ::tolower);

		if ((int)rigidType.find("areamark") >= 0)
		{
			Ogre::String m_RigidType;
			cs.m_objTopologyType = DOT_AreaMark;
			m_Area = Ogre::TextureManager::getSingleton().load(cs.m_s3mfilename , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,Ogre::TEX_TYPE_2D,
				0,1.0f,false,Ogre::PF_A8R8G8B8);
			m_AreaWidth = m_Area->getWidth();
			m_AreaHeight = m_Area->getHeight();
			//uint32 pix = GetPixelFromAreaTesture(0,0);
			//pix = GetPixelFromAreaTesture(1,0);
			//pix = GetPixelFromAreaTesture(0,1);
			//pix = GetPixelFromAreaTesture(1,1);
			//pix = GetPixelFromAreaTesture(0.5f,0.5f);
			m_AreaMarkTextureName = cs.m_s3mfilename;
			return NULL;
		}
	}

	return MisNewTraining::LoadOrganism(cs, ptrain);

}

bool CAcessoriesCutTraining::checkDianzi()
{
	if (m_bNeedCheck)
	{
		m_bNeedCheck = false;
// 			AT_Uterus,//	×Ó¹¬Ìå
// 			AT_Fallopian_Tube_Left,//	×óÊäÂÑ¹Ü
// 			AT_Fallopian_Tube_Right,//	ÓÒÊäÂÑ¹Ü
// 			AT_Ovary_Left,//	×óÂÑ³²
// 			AT_Ovary_Right,//	ÓÒÂÑ³²
// 			AT_Ligament_Left,//	×óÐüÈÍ´ø¹ÌÓÐÈÍ´ø
// 			AT_Ligament_Right,//	ÓÒÐüÈÍ´ø¹ÌÓÐÈÍ´ø
// 			AT_Determine_Area_Burn_1,
// 			AT_Determine_Area_Burn_2,
// 			AT_Determine_Area_Cut_1,
// 			AT_Determine_Area_Cut_2,
// 			AT_Focal_Zone_1,
// 			AT_Focal_Zone_2,
// 			AT_Focal_Zone_3,
// 			AT_TOTALNUM
		if (m_ScoreTimes[AT_Determine_Area_Cut_1] > 0 && m_ScoreTimes[AT_Determine_Area_Cut_2] > 0)
		{
			CTipMgr::Instance()->ShowTip("TrainingFinish");
			CScoreMgr::Instance()->Grade("TrainingFinish");
			if(m_ScoreSys)
			   m_ScoreSys->SetTrainSucced();
			m_bFinished = true;
		}
		for (int i = 0 ; i < AT_Determine_Area_Burn_1; i++)
		{
			if (m_ScoreTimes[i] > 12)
			{
				m_bFinished = true;
				CTipMgr::Instance()->ShowTip("TrainingEndedByMistake");
				return false;
			}
		}
		
	}
	return true;
}

uint32 CAcessoriesCutTraining::GetPixelFromAreaTesture( float cx, float cy )
{    
	if (cx >= m_AreaWidth || cy >= m_AreaHeight || cx < 0 || cy < 0)
	{
		assert(0 && "out of range");
		return 0;
	}

    if (m_pDest == NULL)
    {
        Ogre::HardwarePixelBufferSharedPtr pixelBuffer;
        if (m_Area.isNull())
        {
            return 0;
        }
        try
        {
            pixelBuffer = m_Area->getBuffer();
            pixelBuffer->lock(Ogre::HardwareBuffer::HBL_NORMAL);
        }
        catch (...)
        {
            return 0;
        }

        Ogre::uint* pDest = static_cast<Ogre::uint*>(pixelBuffer->getCurrentLock().data);

        m_pDest = new Ogre::uint[m_AreaHeight*m_AreaWidth];

        for (int i = 0;i<m_AreaHeight*m_AreaWidth;i++)
        {
            m_pDest[i] = pDest[i];
        }

        pixelBuffer->unlock();
    }     
    int tcx = cx*(m_AreaWidth - 1);

    int tcy = cy*(m_AreaHeight - 1);

	if (tcy >= m_AreaHeight || tcx >= m_AreaWidth || tcx < 0 || tcy < 0)
		return 0;
   
	uint32 pix = *(m_pDest + tcy * m_AreaWidth + tcx);

	return pix;
}

void CAcessoriesCutTraining::receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif)
{
}

void CAcessoriesCutTraining::CheckClampAreaRegion(const std::vector<Ogre::Vector2> & texCord , ITool * tool , MisMedicOrganInterface * oif)
{
	std::set<AreaType> regions;
	//if(tool->GetType() == )
	for(size_t t = 0 ; t < texCord.size() ; t++)
	{
		AreaType areaType = getAreaType(oif , texCord[t]);
		regions.insert(areaType);
	}
	std::set<AreaType>::iterator itor = regions.begin();
	for( ; itor != regions.end() ; ++itor)
	{
		AreaType regionType = *itor;
		if(regionType == AT_Uterus)
		{
			m_NumOfUterusBeClamped++;
			//AddOperateItem("Clamp_Uterus_Count" , 1.f , true);
		}
		else if(regionType == AT_Fallopian_Tube_Left || regionType == AT_Fallopian_Tube_Right)
		{
			//AddOperateItem("Clamp_Tube_Count"  , 1.f , true);
			m_NumOfFallopianTubeBeClamped++;
		}
		else if(regionType == AT_Ovary_Left || regionType == AT_Ovary_Right)
		{
			//AddOperateItem("Clamp_Ovary_Count" , 1.f , true);
			m_NumOfOvaryBeClamped++;
		}
	}
}

void CAcessoriesCutTraining::RegisterMistake( int mistakeTimes )
{
	m_foundMistake = false;

	if (0 == m_mistakeCounterMax)
	{
		m_foundMistake = false;
		return;
	}

	if (areaMatchNum[AT_Uterus] == m_mistakeCounterMax)
	{
		m_foundMistake = true;
		CTipMgr::Instance()->ShowTip("Tip_Damage_Uterus");
		CScoreMgr::Instance()->Grade("Damage_Uterus");
		m_ScoreTimes[AT_Uterus] += mistakeTimes;
	}
	else if (areaMatchNum[AT_Fallopian_Tube_Left]  == m_mistakeCounterMax)
	{
		m_foundMistake = true;
		CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Left");
		CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Left");
		m_ScoreTimes[AT_Fallopian_Tube_Left] += mistakeTimes;
	}
	else if (areaMatchNum[AT_Fallopian_Tube_Right]  == m_mistakeCounterMax)
	{
		m_foundMistake = true;
		CTipMgr::Instance()->ShowTip("Tip_Damage_Fallopian_Tube_Right");
		CScoreMgr::Instance()->Grade("Damage_Fallopian_Tube_Right");
		m_ScoreTimes[AT_Fallopian_Tube_Right] += mistakeTimes ;
	}
	else if (areaMatchNum[AT_Ovary_Left]  == m_mistakeCounterMax)
	{
		m_foundMistake = true;
		CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Left");
		CScoreMgr::Instance()->Grade("Damage_Ovary_Left");
		m_ScoreTimes[AT_Ovary_Left] += mistakeTimes ;
	}
	else if (areaMatchNum[AT_Ovary_Right]  == m_mistakeCounterMax)
	{
		m_foundMistake = true;
		CTipMgr::Instance()->ShowTip("Tip_Damage_Ovary_Right");
		CScoreMgr::Instance()->Grade("Damage_Ovary_Right");
		m_ScoreTimes[AT_Ovary_Right] += mistakeTimes ;
	}
	else if (areaMatchNum[AT_Ligament_Left]  == m_mistakeCounterMax)
	{
		m_foundMistake = true;
		CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Left");
		CScoreMgr::Instance()->Grade("Damage_Ligament_Left");
		m_ScoreTimes[AT_Ligament_Left] += mistakeTimes ;
	}
	else if (areaMatchNum[AT_Ligament_Right]  == m_mistakeCounterMax)
	{
		m_foundMistake = true;
		CTipMgr::Instance()->ShowTip("Tip_Damage_Ligament_Right");
		CScoreMgr::Instance()->Grade("Damage_Ligament_Right");
		m_ScoreTimes[AT_Ligament_Right] += mistakeTimes ;
	}
	else 
	{
// 		m_foundMistake = true;
// 		CTipMgr::Instance()->ShowTip("MistakeHappen");
	}
}

void CAcessoriesCutTraining::testMistake(int markValue)
{
	if ((markValue & AreaMark[AT_Uterus])!= 0)
	{
		areaMatchNum[AT_Uterus] ++;
		m_mistakeCounterMax = max(areaMatchNum[AT_Uterus], m_mistakeCounterMax);
	}
	if ((markValue & AreaMark[AT_Fallopian_Tube_Left])!= 0)
	{
		areaMatchNum[AT_Fallopian_Tube_Left] ++;
		m_mistakeCounterMax = max(areaMatchNum[AT_Fallopian_Tube_Left], m_mistakeCounterMax);
	}
	if ((markValue & AreaMark[AT_Fallopian_Tube_Right])!= 0)
	{
		areaMatchNum[AT_Fallopian_Tube_Right] ++;
		m_mistakeCounterMax = max(areaMatchNum[AT_Fallopian_Tube_Right], m_mistakeCounterMax);
	}
	if ((markValue & AreaMark[AT_Ovary_Left])!= 0)
	{
		areaMatchNum[AT_Ovary_Left] ++;
		m_mistakeCounterMax = max(areaMatchNum[AT_Ovary_Left], m_mistakeCounterMax);
	}
	if ((markValue & AreaMark[AT_Ovary_Right])!= 0)
	{
		areaMatchNum[AT_Ovary_Right] ++;
		m_mistakeCounterMax = max(areaMatchNum[AT_Ovary_Right], m_mistakeCounterMax);
	}
	if ((markValue & AreaMark[AT_Ligament_Left])!= 0)
	{
		areaMatchNum[AT_Ligament_Left] ++;
		m_mistakeCounterMax = max(areaMatchNum[AT_Ligament_Left], m_mistakeCounterMax);
	}
	if ((markValue & AreaMark[AT_Ligament_Right])!= 0)
	{
		areaMatchNum[AT_Ligament_Right] ++;
		m_mistakeCounterMax = max(areaMatchNum[AT_Ligament_Right], m_mistakeCounterMax);
	}
}

#if 0
void CAcessoriesCutTraining::AreaCutDetermationSave()
{

	MisMedicOrganInterface * tmp = m_DynObjMap[12];
	MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

	MisMedicEffectRender& eff = pOO->GetEffectRender();
	
	Ogre::TexturePtr  texPtr = eff.m_ComposedEffectTexturePtr;

	Ogre::HardwarePixelBufferSharedPtr pixelBuffer;
	if (texPtr.isNull())
	{
		return;
	}

	m_BurnMark_W = texPtr->getWidth();
	m_BurnMark_H = texPtr->getHeight();


	Ogre::Box extents(0, 0, m_BurnMark_W, m_BurnMark_H);
	Ogre::PixelBox pb(extents, Ogre::PF_A8R8G8B8, m_BurnMarkArea);

// 	printf("PixelBox: %d, %d, w: %d, h: %d\n", pb.left, pb.right, pb.getWidth(), pb.getHeight());

	texPtr->getBuffer()->getRenderTarget()->writeContentsToFile("c:\\tttt.bmp");

 	texPtr->getBuffer()->getRenderTarget()->copyContentsToMemory(pb, Ogre::RenderTarget::FB_AUTO);

	int cou = 0;
	for (int i = 0; i < m_BurnMark_H * m_BurnMark_W; i++)
	{
		if (m_BurnMarkArea[i] != 0)
		{
			cou++;

		}
	}
	return ;
}
void CAcessoriesCutTraining::AreaCutDetermationSave()
{
	Ogre::HardwarePixelBufferSharedPtr pixelBufferPtr;

	Ogre::PixelBox lockedpixbox;

	MisMedicOrganInterface * tmp = m_DynObjMap[12];
	MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

	MisMedicEffectRender& eff = pOO->GetEffectRender();

	Ogre::TexturePtr  texPtr = eff.m_ComposedEffectTexturePtr;

	try
	{
		pixelBufferPtr = texPtr->getBuffer();

		pixelBufferPtr->lock(Ogre::HardwareBuffer::HBL_NORMAL); 

		lockedpixbox = pixelBufferPtr->getCurrentLock();
	}
	catch (...)
	{
		return;
	}
	//restore permanet texture
	int pxwidth  = lockedpixbox.getWidth();
	int pxheight = lockedpixbox.getHeight();
	int pxrowpitch = lockedpixbox.rowPitch;

	int cou = 0;

	Ogre::uint8 * pixeldata = static_cast<Ogre::uint8*>(lockedpixbox.data);
	for(int h = 0 ; h < pxheight ; h++)
	{
		for(int w = 0 ; w < pxwidth ; w++)
		{
			if (pixeldata[(h * pxwidth + w) * 4 + 1] != 0)
			{
				cou++;
			}
		}
	}

	pixelBufferPtr->unlock();
}
#else

void CAcessoriesCutTraining::AreaCutDetermationSave()
{
	MisMedicOrganInterface * tmp = m_DynObjMap[12];
	MisMedicOrgan_Ordinary * pOO = dynamic_cast<MisMedicOrgan_Ordinary*>(tmp);

	MisMedicEffectRender& eff = pOO->GetEffectRender();

	Ogre::TexturePtr  texPtr = eff.m_QuantityTexturePtr;//check their may cause grade error

	texPtr->convertToImage(m_TexImage);

// 	m_TexImage.save("c:\\tttt.bmp");
}

Ogre::ColourValue CAcessoriesCutTraining::GetColorFromImage( float u, float v )
{
	Ogre::PixelBox lockedpixbox = m_TexImage.getPixelBox();
	int tcx = u*(lockedpixbox.getWidth());

	int tcy = v*(lockedpixbox.getHeight());


	Ogre::ColourValue color = lockedpixbox.getColourAt(u,v, 0);
	return color;
}

void CAcessoriesCutTraining::onOrganTopologyRefreshed(MisMedicOrgan_Ordinary* organ)
{
	if (organ->GetOrganType() == EODT_UTERUS)
	{
		MisMedicOrgan_Ordinary * womb = dynamic_cast<MisMedicOrgan_Ordinary*>(organ);
		OnWombBeCutted(womb);
	}
}

void CAcessoriesCutTraining::OnWombBeCutted(MisMedicOrgan_Ordinary * womb)
{
	for (int c = 0; c < womb->m_physbody->GetSoftBodyShape().GetNumSubParts(); c++)
	{
		GFPhysSubConnectPart * subPart = womb->m_physbody->GetSoftBodyShape().GetSubPart(c);// m_OrganSubParts[c];

		bool IsolatedPart = true;

		for (size_t n = 0; n < subPart->m_Nodes.size(); n++)
		{
			GFPhysSoftBodyNode * pnode = subPart->m_Nodes[n];
			if (pnode->m_StateFlag & GPSESF_CONNECTED)
			{
				IsolatedPart = false;
				break;
			}
		}

		//total isolated part (i.e not connect to any other object)
		if (IsolatedPart)
		{
			for (int n = 0; n < (int)subPart->m_Tetras.size(); n++)
			{
				GFPhysSoftBodyTetrahedron * tetra  = subPart->m_Tetras[n];
				
				for (int f = 0; f < 4; f++)
				{
					GFPhysSoftBodyFace * face = tetra->m_TetraFaces[f]->m_surface;
					if (face)
					{
						face->m_RSCollisionMask |= ColFloorCat;
					}
				}
			}
		}
		else
		{
			for (int n = 0; n < (int)subPart->m_Tetras.size(); n++)
			{
				GFPhysSoftBodyTetrahedron * tetra = subPart->m_Tetras[n];

				for (int f = 0; f < 4; f++)
				{
					GFPhysSoftBodyFace * face = tetra->m_TetraFaces[f]->m_surface;
					if (face)
					{
						face->m_RSCollisionMask &= (~ColFloorCat);
					}
				}
			}
		}
	}
}
void CAcessoriesCutTraining::KeyPress(QKeyEvent * event)
{
#if 0
	if(event->key() == Qt::Key_J)
	{
		for(int w = 0 ; w < m_WaterPools.size() ; w++)
		{
			WaterPool *pWaterPool = m_WaterPools[w];
			pWaterPool->AddWater(5);
		}
	}
	else 	if(event->key() == Qt::Key_K)
	{
		for(int w = 0 ; w < m_WaterPools.size() ; w++)
		{
			WaterPool *pWaterPool = m_WaterPools[w];
			pWaterPool->AddBlood(5);
		}
	}
	else 	if(event->key() == Qt::Key_L)
	{
		for(int w = 0 ; w < m_WaterPools.size() ; w++)
		{
			WaterPool *pWaterPool = m_WaterPools[w];
			pWaterPool->Reduce(5);
		}
	}
#endif
}

#endif

