#include "GallNewTraining.h"
//#include "MisMedicOrganTube.h"
#include "VeinConnectObject.h"
#include "TextureBloodEffect.h"
//#include "TextureWaterEffect.h"//__/__
#include "MisMedicEffectRender.h"
#include "MXEventsDump.h"
#include "EffectManager.h"
#include "Dynamic/Constraint/GoPhysSoftBodyVolumeConstraint.h"
//#include "time.h"
#include "MisMedicOrganAttachment.h"
#include "../Instruments/GraspingForceps.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "XMLWrapperTraining.h"
#include "ShadowMap.h"
#include "ScreenEffect.h"
#include "DeferredRendFrameWork.h"
#include "MisMedicRigidPrimtive.h"
REGISTERTRAINING(CNewGallTraining,NewGallTrain,GallTrain1)


void GallbNewTrainHandleEvent(MxEvent * pEvent, ITraining * pTraining)
{
	if (!pEvent || !pTraining)
		return;

	MisNewTraining * pNewTraining = dynamic_cast<MisNewTraining *> (pTraining);

	if(pNewTraining)
	{
		TrainScoreSystem * scoreSys = pNewTraining->GetScoreSystem();
		if(scoreSys)
		{
			scoreSys->SetEnabledAddOperateItem(true);
			scoreSys->ProcessTrainEvent(pEvent);
		}
	}
}
//=============================================================================================
CNewGallTraining::CNewGallTraining(void)
:m_dynamicTextureName("DynamicTextureForGallTraining")
{
	srand((unsigned)time(NULL));
	m_min=1;
	m_max=5;
	
	m_cysticDuct = NULL;
	m_gallbladder = NULL;
	m_veinConnect = NULL;

	m_GallPressConstraint = 0;
	m_minDisOfClampedFaceAndClip = -1.f;
	m_minDisOfClampedFaceAndGallbladderNeck = -1.f;
	m_minDisOfClampedFace = -1.f;

	m_sceneTexture = Ogre::TextureManager::getSingletonPtr()->createManual(m_dynamicTextureName,
																		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																		Ogre::TEX_TYPE_2D,
																		960,
																		540,
																		0,
																		Ogre::PF_R8G8B8,
																		Ogre::TU_RENDERTARGET);
	m_cameraIsFixed = false;
    m_bFinish = false;

	m_isDetectDomeFinish = false;
	m_CholeyStripPercent = 0;
	m_BedStripPercent = 0;
	m_HasCysticDuctBeIngure = false;
	m_HasCysticArteryBeIngure = false;



	m_HasCysticDuctBeFatalError = false;
	m_HasCysticArteryBeFatalError = false;

	m_HasClipCysticDuctGood = false;
	m_NumClipInCysticDuct = 0;
	m_HasCysticDuctBeCutInRightPlace = -1;

	m_HasClipCysticArteryGood = false;
	m_NumClipInCysticArtery = 0;
	m_HasCysticArteryBeCutInRightPlace = -1;
}
//=============================================================================================
CNewGallTraining::~CNewGallTraining(void)
{
	if(m_GallPressConstraint)
	{
		if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(m_GallPressConstraint);

		delete m_GallPressConstraint;
		m_GallPressConstraint = 0;
	}

	m_sceneTexture.setNull();
	Ogre::TextureManager::getSingletonPtr()->remove(m_dynamicTextureName);
}

float CalcGaussianWeight(int iSamplePoint,  float fBlurSigma)
{
	float g = 1.0f / sqrt(2.0f * 3.14159 * fBlurSigma * fBlurSigma);
	return (g * exp(-(iSamplePoint * iSamplePoint) / (2 * fBlurSigma * fBlurSigma)));
}


bool CNewGallTraining::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
    bool result = MisNewTraining::Initialize(pTrainingConfig, pToolConfig);

    m_TrainName = pTrainingConfig->m_Name;

    DynObjMap::iterator itor = m_DynObjMap.begin();

	int collidePlaneCat = (MMRC_UserStart << 1);

    while (itor != m_DynObjMap.end())
    {
        MisMedicOrganInterface * oif = itor->second;

        MisMedicOrgan_Ordinary * organmesh = dynamic_cast<MisMedicOrgan_Ordinary*>(oif);

		MisMedicRigidPrimtive  * collidePlane = dynamic_cast<MisMedicRigidPrimtive*>(oif);
        
		if (collidePlane)
		{
			collidePlane->m_body->SetCollisionCategory(collidePlaneCat);

		}
		else if (organmesh)
        {
            MisMedicDynObjConstructInfo & cs = organmesh->GetCreateInfo();

            if (cs.m_OrganType == EDOT_GALLBLADDER)
            {
                organmesh->m_DragForceRate = 1.0f;
                organmesh->setEffectRender_deviateScale(1.0f);
                organmesh->GetEffectRender().SetBloodBleedTexture("gall_biletex.dds");
                organmesh->GetEffectRender().SetDynBldPar_Tex("surface_blood.dds");
                organmesh->GetBloodTextureEffect()->SetBloodTexture("surface_blood.dds");//m_BloodSystem->SetBloodColor(Ogre::ColourValue(0.9f , 0.9f , 0.9f , 1.0f));
                organmesh->SetBloodColor(Ogre::ColourValue(0.182f, 0.086f, 0));
                cs.m_BloodRadius = 0.004f;
                organmesh->SetMinPunctureDist(0.5f);
              
                m_gallbladder = organmesh;
            }
            else if (cs.m_OrganType == EDOT_LIVER)
            {
            
                organmesh->setEffectRender_deviateScale(0.32f);
                organmesh->GetEffectRender().SetBloodBleedTexture("Strip_Blood.dds");
                organmesh->GetEffectRender().SetDynBldPar_Tex("surface_blood.dds");
                organmesh->GetBloodTextureEffect()->SetBloodTexture("surface_blood.dds");//???
				organmesh->SetBloodColor(Ogre::ColourValue(0.1333f, 0.0f, 0));
                cs.m_BloodRadius = 0.002f;
                organmesh->SetMinPunctureDist(0.8f);
            }
            else if (cs.m_OrganType == EDOT_CYSTIC_DUCT)
            {
                organmesh->GetEffectRender().SetBloodBleedTexture("gall_biletex.dds");
                organmesh->GetEffectRender().SetDynBldPar_Tex("partcl_bile.dds");
                organmesh->GetBloodTextureEffect()->SetBloodTexture("partcl_bile.dds");
                organmesh->SetTimeNeedToEletricCut(0.5f);
                organmesh->SetBloodPointDensity(0.2f);
                organmesh->SetMaxBloodTrackCount(3);
                organmesh->SetMinPunctureDist(0.2f);
				organmesh->SetBloodColor(Ogre::ColourValue(0.19, 0.0011, 0.0003, 0));
				organmesh->SetBleedRadius(0.05f);
				organmesh->SetBleedWhenStripBreak(true);
                cs.m_BloodRadius = 0.02f;

                m_cysticDuct = organmesh;
            }
            else if (cs.m_OrganType == EDOT_BRAVERY_ARTERY)
            {
                organmesh->SetMinPunctureDist(0.2f);
				organmesh->SetBleedWhenStripBreak(true);
                cs.m_BloodRadius = 0.008f;
            }
            else if (cs.m_OrganType == EDOT_COMMON_BILE_DUCT)
            {
                organmesh->GetEffectRender().SetBloodBleedTexture("Strip_Blood.dds");
                organmesh->GetBloodTextureEffect()->SetBloodTexture("surface_blood.dds");
                organmesh->GetEffectRender().SetDynBldPar_Tex("surface_blood.dds");
                organmesh->SetTimeNeedToEletricCut(0.5f);
                organmesh->SetBloodPointDensity(0.2f);
                organmesh->SetMaxBloodTrackCount(3);
				organmesh->SetBleedWhenStripBreak(true);
                cs.m_BloodRadius = 0.008f;
            }

			else if (cs.m_OrganType == EODT_DOME_ACTIVE)//only for test
            {
				organmesh->SetBloodColor(Ogre::ColourValue(0.1333f, 0.0f, 0));
                
            }

			else if (cs.m_OrganType == 100)
			{

			}

			if (cs.m_OrganType != EDOT_GALLBLADDER)
			{
				organmesh->m_physbody->SetCollisionMask(organmesh->m_physbody->m_MaskBits & (~collidePlaneCat));
			}
        }
        else
        {
            if (oif->GetCreateInfo().m_OrganType == EODT_VEINCONNECT)
            {
                m_veinConnect = static_cast<VeinConnectObject*>(oif);
                m_veinMaterialName = oif->getMaterialName();
            }
            if (oif->GetCreateInfo().m_OrganType == EODT_VEINBOTTOMCONNECT)
            {
                VeinConnectObject * bottomObj = dynamic_cast<VeinConnectObject*>(oif);
                if (bottomObj)
                {
                    bottomObj->m_CanBlood = false;
                }
            }
        }
        itor++;
    }


    //register train event 
    CMXEventsDump::Instance()->RegisterHandleEventsFunc(GallbNewTrainHandleEvent, this);

    LoadGallbladderNeckInfo();

    //Ogre::Viewport* viewport = m_renderTarget->addViewport(m_pLargeCamera);
    //viewport->setClearEveryFrame(false);
    //viewport->setBackgroundColour(Ogre::ColourValue::Red);

    if (m_TrainName == "GallTrain3")
    {
        //DeferredRendFrameWork::Get()->SetBloomBrightPassThreshold(0.8f);
    }
    m_ElectricCutVeinBottom = false;
    m_ElectricCutVeinConnect = false;
    m_HurtDuringSeperateTriangleCount = false;
    m_Cut_BRAVERY_ARTERY = false;
    m_Cut_COMMON_BILE_DUCT = false;
    m_Cut_BRAVERY_CYSTIC_DUCT = false;

	m_DetectCameraIntersect = true;
#if(0)
	float sample9Value[9];
	float sample11Value[11];
	float sample13Value[13];

	float fBLurSigma = 1.65f;
	for (int t = 0; t < 9; t++)
	{
		sample9Value[t] = CalcGaussianWeight(t - 4, fBLurSigma);
	}
	for (int t = 0; t < 11; t++)
	{
		sample11Value[t] = CalcGaussianWeight(t - 5, fBLurSigma);
	}

	for (int t = 0; t < 13; t++)
	{
		sample13Value[t] = CalcGaussianWeight(t - 6, fBLurSigma);
	}
#endif
	//


	//test instance
#if(0)
	if (Ogre::Root::getSingleton().getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_VERTEX_PROGRAM) == false)
	{
		OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "Your video card doesn't support batching", "Demo_Instance::createScene");
	}
	Ogre::InstanceManager * mCurrentManager = MXOgre_SCENEMANAGER->createInstanceManager(
		                                            "InstanceMgrHWBasix", "instancespehere.mesh",
													Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, Ogre::InstanceManager::HWInstancingBasic,
													100, Ogre::IM_USEALL);



	for (int j = 0; j < 10; ++j)
	{
		//Create the instanced entity
		Ogre::InstancedEntity * ent = mCurrentManager->createInstancedEntity("Examples/Instancing/HWBasic/Robot");
		ent->setOrientation(Ogre::Quaternion::IDENTITY);
		ent->setPosition(Ogre::Vector3(0 , j , j));
		ent->setScale(Ogre::Vector3(0.1f, 0.1f, 0.1f));
	}
#endif
	//
    return result;
}
void CNewGallTraining::SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ)
{
	//if(organ->m_OrganID == EDOT_GALLBLADDER || organ->m_OrganID == EDOT_LIVER)
	{
		organ->GetCreateInfo().m_distributemass = false;
	}
}

bool CNewGallTraining::Update(float dt)
{
	bool result = MisNewTraining::Update(dt);

	VeinConnectObject * btconnect = dynamic_cast<VeinConnectObject*>(GetOrgan(EODT_VEINBOTTOMCONNECT));

	if(btconnect)
	{
		btconnect->m_SolveFFVolumeCS = true;

		if(m_gallbladder && m_gallbladder->IsClamped())
		{
		   btconnect->m_SolveFFVolumeCS = false;
		}
	}
	
	//test whether train finished
	if (m_ScoreSys->IsTrainFinished() && m_IsFinalMovieShowed == false)
	{
		//Inception::Instance()->EmitLoadMovie("End");
		Inception::Instance()->EmitShowMovie("End");
		//TrainingFinish("TrainingFinish");
		m_IsFinalMovieShowed = true;
        m_bFinish = true;
        COnLineGradeMgr::Instance()->SendGrade("SpecimenBag");
		//AddOperateItem("TakeOutGallbladder",1,true);
	}

	UpdateTargetPointSeeState();
    if (m_ScoreSys->IsSeperateVeinFinished() && !m_HurtDuringSeperateTriangleCount)
    {
        int gallHurtCount = m_gallbladder->m_BleedingRecords.size();

        for (int i = 0; i < gallHurtCount; i++)
        {
            m_gallbladder->m_BleedingRecords[i].m_HavingBeenCount = true;
        }

        int liverHurtCount = 0;
        MisMedicOrganInterface * pOrgan = NULL;
        for (DynObjMap::iterator itr = m_DynObjMap.begin(); itr != m_DynObjMap.end(); ++itr)
        {
            pOrgan = itr->second;
            if (pOrgan->m_OrganID == EDOT_LIVER)
            {
                liverHurtCount = pOrgan->m_BleedingRecords.size();
                for (int i = 0; i < liverHurtCount; i++)
                {
                    pOrgan->m_BleedingRecords[i].m_HavingBeenCount = true;
                }
                break;
            }
        }

        if (gallHurtCount + liverHurtCount < 6)
        {
            COnLineGradeMgr::Instance()->SendGrade("GallbladderTrianglePerfect");
        }
        else
        {
            COnLineGradeMgr::Instance()->SendGrade("GallbladderTriangleNotPerfect");
        }
        m_HurtDuringSeperateTriangleCount = true;
    }

	//目前没有取胆囊的操作，所以在血管被剪断后就可以开始判断是否安全夹持与计算夹持距离
	//if(m_ScoreSys && m_ScoreSys->FinishCutTube())
	{//测试
		UpdateSafeClampDistance();
		UpdateMixTexture();//夹住胆囊动脉10秒后修改贴图
	}


	if (m_IsSeriousFault)
	{
		if (m_fLastTime < 0)
		{
			m_IsSeriousFault = false;
			m_fLastTime = 0;
		}
		else
			m_fLastTime -= dt;
	}

	//测试
	if(m_TrainName == "GPUTess")
	{
		//CScreenEffect::Get()->HideAllOverLays();
	}

	return result;
}

void CNewGallTraining::UpdateSafeClampDistance()
{
	m_minDisOfClampedFace = -1.f;
	CTool * pLeftTool = GetLeftTool();
	CTool * pRightTool = GetRightTool();
	CGraspingForceps* graspForceps = NULL;

	graspForceps = dynamic_cast<CGraspingForceps*>(pLeftTool);
	if(graspForceps == NULL)
		graspForceps = dynamic_cast<CGraspingForceps*>(pRightTool);

	if(graspForceps)
	{
		MisCTool_PluginClamp * clampPlugin = NULL;
		std::vector<MisMedicCToolPluginInterface*>& plugins = graspForceps->m_ToolPlugins;

		for(std::size_t i = 0;i < plugins.size();++i)
		{
			if(clampPlugin = dynamic_cast<MisCTool_PluginClamp*>(plugins[i]))
				break;
		}

		std::vector<MisMedicOrgan_Ordinary *> organsClamped;
		clampPlugin->GetOrgansBeClamped(organsClamped);

		//
		for(size_t c = 0 ; c < organsClamped.size() ; c++)
		{
			if(organsClamped[c] == m_gallbladder)
			{   
				//获取抓取点
				float weights[3] = {0.3333f,0.3333f,0.3333f};
				std::vector<GFPhysSoftBodyFace*> clampedFaces;
				GFPhysVector3 clampPoint(0,0,0);

				clampPlugin->GetFacesBeClamped(clampedFaces , m_gallbladder);

				for(size_t f = 0;f < clampedFaces.size();++f)
				{
					for(int n =0;n < 3;++n)
					{
						clampPoint += clampedFaces[f]->m_Nodes[n]->m_UnDeformedPos;
					}
				}

				if(clampedFaces.size())
					clampPoint /= clampedFaces.size() * 3;

				CalClampDistance(clampedFaces,clampPoint);

				std::vector<MisMedicOrganAttachment*> attachments;
				if(m_cysticDuct)
				   m_cysticDuct->GetAttachment(MOAType_TiantumClip,attachments);
				const int nAttachment = attachments.size();

				if(nAttachment)
				{
					//获取切割点
					GFPhysVector3 cutPoint(0,0,0);
					float dis;
					float minDis = 10000.f;
					int pointIndex = -1;

					GFPhysAlignedVectorObj<MMO_Face>& cutFaces = m_cysticDuct->m_CutCrossfaces;
					for(size_t p = 0;p < cutFaces.size();++p)
					{
						GFPhysSoftBodyFace * physFace = cutFaces[p].m_physface;
						if(physFace)
						{
							dis = physFace->m_Nodes[0]->m_UnDeformedPos.Distance(clampPoint);
							if(dis < minDis)
							{
								minDis = dis;
								pointIndex = p;
							}
						}
					}

					if(pointIndex != -1)
						cutPoint = cutFaces[pointIndex].m_physface->m_Nodes[0]->m_UnDeformedPos;

					//获取远端钛夹
					MisMedicTitaniumClampV2 * pAttachment = NULL;
					GFPhysVector3 massCenter;
					minDis = 10000.f;
					for(size_t a = 0;a < attachments.size();++a)
					{
						MisMedicTitaniumClampV2* tempAttachment = static_cast<MisMedicTitaniumClampV2*>(attachments[a]);
						if (tempAttachment->getRelativeFace())
						{
							massCenter = tempAttachment->getRelativeFace()->GetUndeformedMassCenter(weights);
							dis = massCenter.Distance(cutPoint);

							if (dis < minDis)
							{
								minDis = dis;
								pAttachment = tempAttachment;
							}
						}
					}

					//更新安全加持距离
					if (pAttachment->getRelativeFace())
					{
						massCenter = pAttachment->getRelativeFace()->GetUndeformedMassCenter(weights);
						dis = massCenter.Distance(clampPoint);
						if (m_minDisOfClampedFaceAndClip < 0 || dis < m_minDisOfClampedFaceAndClip)
							m_minDisOfClampedFaceAndClip = dis;
					}

					//计算到胆囊颈的距离
					bool bFind = false;
					std::set<GFPhysSoftBodyFace*>::iterator itr;
					for(size_t cf = 0;cf < clampedFaces.size();++cf)
					{
						itr = m_neckFaces.find(clampedFaces[cf]);
						if(itr != m_neckFaces.end())
						{
							m_minDisOfClampedFaceAndGallbladderNeck = 0.f;
							bFind = true;
							break;
						}
					}
					
					if(!bFind)
					{
						dis = clampPoint.Distance(m_neckPos);
						if(m_minDisOfClampedFaceAndGallbladderNeck < 0 || dis < m_minDisOfClampedFaceAndGallbladderNeck)
							m_minDisOfClampedFaceAndGallbladderNeck = dis;
					}
				}
				break;
		    }
		}
	}
}

bool CNewGallTraining::CalClampDistance(std::vector<GFPhysSoftBodyFace*>& clampedFaces, GFPhysVector3& clampPoint)
{
	//计算到胆囊颈的距离
	bool bFind = false;
	std::set<GFPhysSoftBodyFace*>::iterator itr;
	for(size_t cf = 0;cf < clampedFaces.size();++cf)
	{
		itr = m_neckFaces.find(clampedFaces[cf]);
		if(itr != m_neckFaces.end())
		{
			m_minDisOfClampedFace = 0.f;
			bFind = true;
			break;
		}
	}
	float dis = 0.f;
	if(!bFind)
	{
		dis = clampPoint.Distance(m_neckPos);
		if(m_minDisOfClampedFace < 0 || dis < m_minDisOfClampedFace)
			m_minDisOfClampedFace = dis;
	}
	return bFind;
}

void CNewGallTraining::UpdateMixTexture(void)
{
	static DWORD nGallClampedTime = 0;
	static DWORD nGallReleaseTime = ::GetTickCount()-1000;

	if(m_gallbladder == 0)
	   return;

	if(m_minDisOfClampedFace<0.0f)
	{
		nGallClampedTime = 0;
		if(nGallReleaseTime==0)
		{
			nGallReleaseTime = ::GetTickCount();
			m_gallbladder->SetBaseTextureMix(1.0f);
		}
		else
		{
			float fMix = (::GetTickCount()-nGallReleaseTime)/1000.0f;//1秒渐变
			if(fMix<=1.0f)
				m_gallbladder->SetBaseTextureMix(1.0f-fMix);
		}
	}
	else
	{
		nGallReleaseTime = 0;
		if(nGallClampedTime==0)
			nGallClampedTime = ::GetTickCount();
		else
		{
			float fMix = (::GetTickCount()-nGallClampedTime)/1000.0f;//1秒渐变
			if(fMix>10.0f&&fMix<=11.0f)
				m_gallbladder->SetBaseTextureMix(fMix-10.0f);
		}
	}
}

void CNewGallTraining::CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig)
{
	MisNewTraining::CreateTrainingScene(pTrainingConfig);
	VeinConnectObject * connectCalot = 0;
	std::map<int , MisMedicOrganInterface*>::iterator itor = m_DynObjMap.find(EODT_VEINCONNECT);

	//MisMedicOrganInterface * oif = m_DynObjMap.find(EODT_VEINCONNECT)->second;
	if(itor != m_DynObjMap.end())//oif)
	{
		MisMedicOrganInterface * oif  = itor->second;
		connectCalot = dynamic_cast<VeinConnectObject*>(oif);
		connectCalot->m_Actived = false;
	}

	float simInterval = 1.0f / PhysicsWrapper::GetSingleTon().m_SimulateFrequency;
	for(int c = 0 ; c < (int)(1.0f / simInterval) ; c++)
	{
		PhysicsWrapper::GetSingleTon().UpdateWorld(simInterval);
	}

	if(connectCalot)
	{
		connectCalot->m_Actived = true;
		connectCalot->BuildConnectConstraint(m_DynObjMap);//rebuild connection reset value
	}
}
//======================================================================================================================


void CNewGallTraining::LoadGallbladderNeckInfo()
{
	if(m_gallbladder == 0)
	   return;

	Ogre::TexturePtr pTexture = Ogre::TextureManager::getSingletonPtr()->load("gallbladderNeck_Mark.png",
																				Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																				Ogre::TEX_TYPE_2D,
																				Ogre::MIP_DEFAULT,
																				1.f,
																				false,
																				Ogre::PF_A8R8G8B8);
	if(pTexture.isNull())
	{
		throw Ogre::String("the texure ptr is null");
	}

	const int width = pTexture->getWidth();
	const int height = pTexture->getHeight();
	
	Ogre::HardwarePixelBufferSharedPtr pixBuffer = pTexture->getBuffer();
	int * pDest = NULL;

	//lock
	pDest = static_cast<int*>(pixBuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

	const GFPhysAlignedVectorObj<MMO_Face>& faces = m_gallbladder->m_OriginFaces;
	int nFaces = faces.size();

	for(int f = 0 ;f < nFaces;++f)
	{
		const MMO_Face& face = faces[f];
		if(face.m_physface)
		{
			for(int n = 0;n < 3;++n)
			{
				///
				Ogre::Vector2 vertTexCoord = face.GetTextureCoord(n);
				int offset = (width - 1) * vertTexCoord.x + (height - 1) * vertTexCoord.y * (width - 1);
				int color = *(pDest + offset);
				int mask = 0x000000ff;
				if((color & mask) == mask)
				{
					m_neckFaces.insert(face.m_physface);
				}
			}
			
		}
	}

	//unlock
	pixBuffer->unlock();

	GFPhysVector3 tempPos(0,0,0);
	for(std::set<GFPhysSoftBodyFace*>::iterator itr = m_neckFaces.begin();itr != m_neckFaces.end();++itr)
	{
		for(int n = 0;n < 3;++n)
		{
			tempPos += (*itr)->m_Nodes[n]->m_UnDeformedPos;
		}
	}

	if(m_neckFaces.size())
		tempPos /= m_neckFaces.size() * 3;

	m_neckPos = tempPos;
}

//========================================================================================================================
void CNewGallTraining::UpdateTargetPointSeeState()
{
	static Ogre::Vector3 targetPoints[5];

	if (m_isDetectDomeFinish)
		return;

	targetPoints[0] = Ogre::Vector3(-11.717f,12.966f,10.646f);
	targetPoints[1] = Ogre::Vector3(0.316f,19.767f,10.019f);
	targetPoints[2] = Ogre::Vector3(-0.041f,15.465f,6.89f);
	targetPoints[3] = Ogre::Vector3(-8.766f, 21.064f, 12.317f);
	targetPoints[4] = Ogre::Vector3(-6.629f, 12.904f, 6.445f);

	static bool detected[5] = { false, false, false, false, false };

	Ogre::Vector3 camPos = m_pLargeCamera->getDerivedPosition();
	Ogre::Vector3 camDir = m_pLargeCamera->getDerivedDirection();

	for (int c = 0; c < 5; c++)
	{
		if (detected[c] == false)
		{
			Ogre::Vector3 dirTarget = (targetPoints[c] - camPos).normalisedCopy();
			if (dirTarget.dotProduct(camDir) > 0.86f)
			{
				detected[c] = true;
			}
		}
	}

	int numTriggerPoint = 0;
	for (int c = 0; c < 5; c++)
	{
		if (detected[c])
		{
			numTriggerPoint++;
		}
	}

	if (numTriggerPoint == 5)
	{
		m_isDetectDomeFinish = true;
		
	}
}
void CNewGallTraining::OnOrganCutByTool(MisMedicOrganInterface * organ, bool iselectriccut)
{
    if (organ->m_OrganID == EDOT_COMMON_BILE_DUCT || organ->m_OrganID == EDOT_HEPATIC_ARTERY)
    {
        if (organ->m_OrganID == EDOT_COMMON_BILE_DUCT)
        {
            if (iselectriccut)
                TrainingFatalError("CommonBileDuctCutFailed");//errorMessage = "肝总管损伤";
            else
                TrainingFatalError("CommonBileDuctElecBreak");
        }
        else
        {
            if (iselectriccut)
                TrainingFatalError("HepaticArteryClipFailederrorMessage");// = "肝动脉损伤";
            else
                TrainingFatalError("HepaticArteryElecBreak");
        }
    }

    if (m_ScoreSys->IsSeperateVeinFinished() == false && iselectriccut)
    {
        if (organ->m_OrganID == EDOT_BRAVERY_ARTERY && !m_Cut_BRAVERY_ARTERY)// 胆囊动脉
        {
            m_Cut_BRAVERY_ARTERY = !m_Cut_BRAVERY_ARTERY;
        }
        if (organ->m_OrganID == EDOT_COMMON_BILE_DUCT && !m_Cut_COMMON_BILE_DUCT)// 胆总管
        {
            m_Cut_COMMON_BILE_DUCT = !m_Cut_COMMON_BILE_DUCT;
        }
        if (organ->m_OrganID == EDOT_CYSTIC_DUCT && !m_Cut_BRAVERY_CYSTIC_DUCT)// 胆囊管
        {
            m_Cut_BRAVERY_CYSTIC_DUCT = !m_Cut_BRAVERY_CYSTIC_DUCT;
        }
    }
}

//========================================================================================================
void CNewGallTraining::OnVeinConnectCuttingByElecTool(std::vector<VeinConnectPair*> & cuttingPair)
{    
    MisMedicOrganInterface *pGallbladder = GetOrgan(EDOT_GALLBLADDER);
    MisMedicOrganInterface *pLiver = GetOrgan(EDOT_LIVER);    
    if (pGallbladder && pLiver && (!m_ElectricCutVeinConnect || !m_ElectricCutVeinBottom))
    {

        for (size_t i = 0; i < cuttingPair.size(); ++i)
        {
            if (cuttingPair[i]->m_OrganAID == EDOT_GALLBLADDER && cuttingPair[i]->m_OrganBID == EDOT_LIVER)
            {                
                m_ElectricCutVeinBottom = true;
            }
            else
            {
                m_ElectricCutVeinConnect = true;                
            }
        }
    }
    return;
	/*int r=rand()%(m_max-m_min+1)+m_min;
	if(r==m_min)
	{	
		MisMedicOrganInterface *pGallbladder = GetOrgan(EDOT_GALLBLADDER);
		MisMedicOrganInterface *pLiver = GetOrgan(EDOT_LIVER);
		Ogre::Vector2 textureCoord;
		if(pGallbladder && pLiver)
		{
			
			for(size_t i = 0; i < cuttingPair.size(); ++i)
			{
				if(cuttingPair[i]->m_OrganAType == EDOT_GALLBLADDER && cuttingPair[i]->m_OrganBType == EDOT_LIVER)
				{
					((MisMedicOrgan_Ordinary*)pGallbladder)->ApplyEffect_BurnWhite(0 ,cuttingPair[i]->m_faceA,cuttingPair[i]->m_weightsA , 0.1f , textureCoord , 0.25f);
					((MisMedicOrgan_Ordinary*)pLiver)->ApplyEffect_BurnWhite(0 ,cuttingPair[i]->m_faceB,cuttingPair[i]->m_weightsB,0.1f , textureCoord , 0.25f);
				}
				else if(cuttingPair[i]->m_OrganBType == EDOT_GALLBLADDER && cuttingPair[i]->m_OrganAType == EDOT_LIVER)
				{
					((MisMedicOrgan_Ordinary*)pLiver)->ApplyEffect_BurnWhite(0 ,cuttingPair[i]->m_faceA , cuttingPair[i]->m_weightsA , 0.1f , textureCoord , 0.25f);
					((MisMedicOrgan_Ordinary*)pGallbladder)->ApplyEffect_BurnWhite(0 ,cuttingPair[i]->m_faceB , cuttingPair[i]->m_weightsB , 0.1f , textureCoord , 0.25f);
				}
			}
		}
	}*/
}

void CNewGallTraining::OnCameraStateChanged(bool isFixed)
{
	m_cameraIsFixed = isFixed;

	if(isFixed)
	{
		float curTime = GetElapsedTime();
        if (curTime > 0 && m_veinConnect)
		{
			const GFPhysDBVTree& tree = m_veinConnect->GetCollideTree();
			GFPhysVector3 min, max;
			tree.GetAABB(min, max);

			Ogre::Vector3 cameraPosition = m_pLargeCamera->getDerivedPosition();
			if(Ogre::Math::intersects(Ogre::Ray(cameraPosition, m_pLargeCamera->getDerivedDirection()), Ogre::AxisAlignedBox(GPVec3ToOgre(min), GPVec3ToOgre(max))).first)
			{
				LocateInfo info;

				info.startTime = curTime;
				info.stopTime = curTime;
				info.rate = cameraPosition.distance(GPVec3ToOgre((min + max) / 2));
				m_locateInfos.push_back(info);
			}
		}
	}
	else
	{
		//更新定位结束时间
		if(m_locateInfos.size())
			m_locateInfos[m_locateInfos.size() - 1].stopTime = GetElapsedTime();
	}
}

void CNewGallTraining::onDebugMessage(const std::string& value)
{
	if (value != "savecamera")
	{
		return;
	}
	std::string filepath = "..//media//models//GallbladderAll//StaticScene//ga_jingtai.scene";
	TiXmlDocument document;
	bool ret = document.LoadFile(filepath);
	if (!ret)
	{
		return;
	}
	TiXmlNode * rootElement = document.RootElement();
	if (strcmp(rootElement->Value(),"scene") == 0)
	{
		rootElement = rootElement->FirstChild();
	}
	while (rootElement)
	{
		int nodetype = rootElement->Type();
		if (strcmp(rootElement->Value(),"nodes") == 0)
		{
			break;
		}
		rootElement = rootElement->NextSibling();
	}
	if (!m_pLargeCamera)
	{
		document.SaveFile();
		return;
	}
	Ogre::Vector3 parentPos    = m_pLargeCamera->getParentNode()->getPosition();
	Ogre::Quaternion parentOri = m_pLargeCamera->getParentNode()->getOrientation();
	Ogre::Quaternion selfOri = m_pLargeCamera->getOrientation();
	
	TiXmlElement* element = rootElement->FirstChildElement();
	for (; element!=NULL; element=element->NextSiblingElement())
	{
		TiXmlAttribute* elementAttri = element->FirstAttribute();
		if (elementAttri->ValueStr() == "Camera001")
		{
			TiXmlElement* childElement = element->FirstChildElement();
			for (; childElement!=NULL; childElement=childElement->NextSiblingElement())
			{
				if (childElement->ValueStr() == "position")
				{
					TiXmlAttribute* childAttri = childElement->FirstAttribute();
					for (;childAttri!=NULL;childAttri=childAttri->Next())
					{
						if (childAttri->NameTStr()=="x")
						{
							childAttri->SetDoubleValue(parentPos.x);
						} 
						else if (childAttri->NameTStr()=="y")
						{
							childAttri->SetDoubleValue(parentPos.y);
						} 
						else if(childAttri->NameTStr()=="z")
						{
							childAttri->SetDoubleValue(parentPos.z);
						}
					}
				}
				else if (childElement->ValueStr() == "rotation")
				{
					TiXmlAttribute* childAttri = childElement->FirstAttribute();
					for (;childAttri!=NULL;childAttri=childAttri->Next())
					{
						if (childAttri->NameTStr()=="qx")
						{
							childAttri->SetDoubleValue(parentOri.x);
						} 
						else if (childAttri->NameTStr()=="qy")
						{
							childAttri->SetDoubleValue(parentOri.y);
						} 
						else if(childAttri->NameTStr()=="qz")
						{
							childAttri->SetDoubleValue(parentOri.z);
						}
						else if(childAttri->NameTStr()=="qw")
						{
							childAttri->SetDoubleValue(parentOri.w);
						}
					}
				}
				else if (childElement->ValueStr() == "camera")
				{
					TiXmlElement* selfElement = childElement->FirstChildElement("rotation");
					TiXmlAttribute* childAttri = selfElement->FirstAttribute();
					for (;childAttri!=NULL;childAttri=childAttri->Next())
					{
						if (childAttri->NameTStr()=="qx")
						{
							childAttri->SetDoubleValue(selfOri.x);
						} 
						else if (childAttri->NameTStr()=="qy")
						{
							childAttri->SetDoubleValue(selfOri.y);
						} 
						else if(childAttri->NameTStr()=="qz")
						{
							childAttri->SetDoubleValue(selfOri.z);
						}
						else if(childAttri->NameTStr()=="qw")
						{
							childAttri->SetDoubleValue(selfOri.w);
						}
					}
				}
			}
		}
	}
	document.SaveFile();
	return;
}
//////////////////////////////////////////////////////////////////////////
void CNewGallTraining::OnSaveTrainingReport()
{
    Real usedtime = GetElapsedTime();
    if (m_minDisOfClampedFaceAndClip > 0)
        AddOperateItem("SafeClampCysticDuct", m_minDisOfClampedFaceAndClip * 10, false);
    if (m_minDisOfClampedFaceAndGallbladderNeck > 0)
        AddOperateItem("SafeClampGallbladder", m_minDisOfClampedFaceAndGallbladderNeck * 10, false);

    size_t n = m_locateInfos.size();
    if (n)
    {
        //更新定位结束时间
        if (m_cameraIsFixed)
            m_locateInfos[n - 1].stopTime = GetElapsedTime();

        // 		//获取定位时间最长的一次操作
        // 		int locateIndex = 0;
        // 		float maxTime = m_locateInfos[0].stopTime - m_locateInfos[0].startTime;
        // 		float rate = m_locateInfos[0].rate;
        // 		for(std::size_t i = 1;i < m_locateInfos.size();++i)
        // 		{
        // 			float time  = m_locateInfos[i].stopTime - m_locateInfos[1].startTime;
        // 			if(time > maxTime)
        // 			{
        // 				maxTime = time;
        // 				rate = m_locateInfos[i].rate;
        // 				locateIndex = i;
        // 			}
        // 		}
        // 
        // 		if(rate > 0.033f)
        // 		{
        // 			AddOperateItem("LocateSuccess",1,true);
        // 			MxOperateItem * pOperateItem = GetLastOperateItem("LocateSuccess");
        // 			if(pOperateItem)
        // 				pOperateItem->SetLastOperateTime(m_locateInfos[locateIndex].startTime);
        // 		}
        // 		else
        // 			AddOperateItem("LocateSuccess",0,false);

        //只要有一次定位成功则就算正确操作
        bool beAdded = false;
        for (std::size_t i = 0; i < m_locateInfos.size(); ++i)
        {
            float rate = m_locateInfos[i].rate;
            if (rate < 16.7823f)
            {
                AddOperateItem("LocateSuccess", 1, true);
                MxOperateItem * pOperateItem = GetLastOperateItem("LocateSuccess");
                if (pOperateItem)
                    pOperateItem->SetLastOperateTime(m_locateInfos[i].startTime);
                beAdded = true;
                COnLineGradeMgr::Instance()->SendGrade("FixCamera", 0, usedtime);
                break;
            }
        }

        if (!beAdded)
        {
            AddOperateItem("LocateSuccess", 0, false);
            COnLineGradeMgr::Instance()->SendGrade("FixCamera0", 0, usedtime);
        }
    }

    //最后调用父类的函数，才能保证有有效的数据被保存

    //////////////////////////////////////////////////////////////////////////
    if (m_bFinish)
    {
        COnLineGradeMgr::Instance()->SendGrade("Exploration", 0, usedtime);
    }
    else
    {
        COnLineGradeMgr::Instance()->SendGrade("Exploration0", 0, usedtime);
    }

    //解剖胆囊三角////////////////////////////////////////////////////////////////////////

    Real fPercent_tri = 1.0f;
    VeinConnectObject * btconnect_tri = dynamic_cast<VeinConnectObject*>(GetOrgan(EODT_VEINCONNECT));
    if (btconnect_tri)
    {
        int currConnectCount_tri = btconnect_tri->GetCurrConnectCount();

        int initConnectCount_tri = btconnect_tri->GetInitConnectCount();

        fPercent_tri = (Real)currConnectCount_tri / (Real)initConnectCount_tri;
    }

    if (fPercent_tri < 0.55f)
    {
        COnLineGradeMgr::Instance()->SendGrade("ExposedGallbladderTriangle", 0, usedtime);
        if (m_ElectricCutVeinConnect)
        {
            COnLineGradeMgr::Instance()->SendGrade("GallbladderTriangleElecSeperate", 0, usedtime);
        }
        else
        {
            COnLineGradeMgr::Instance()->SendGrade("GallbladderTriangleForceSeperate", 0, usedtime);
        }
        /*<OnLineGrade Name = "GallbladderTriangleElecSeperate" ItemCode = "L120020130" Description = "      胆囊三角通电分离">< / OnLineGrade>
        <OnLineGrade Name = "GallbladderTriangleForceSeperate" ItemCode = "L120020131" Description = "      胆囊三角钝性分离">< / OnLineGrade>*/
    }
    else
    {
        COnLineGradeMgr::Instance()->SendGrade("ExposedGallbladderTriangle0", 0, usedtime);
    }


    /*<OnLineGrade Name = "GallbladderTrianglePerfect" ItemCode = "L120020110" Description = "      镜下操作熟练，没有造成多余损伤">< / OnLineGrade>
    <OnLineGrade Name = "GallbladderTriangleNotPerfect" ItemCode = "L120020111" Description = "      分离胆囊三角过程中造成多余损伤">< / OnLineGrade>
    */
    if (m_Cut_BRAVERY_ARTERY)
    {
        COnLineGradeMgr::Instance()->SendGrade("GallbladderTriangleHurtArteriaeCystica", 0, usedtime);
    }
    if (m_Cut_COMMON_BILE_DUCT)
    {
        COnLineGradeMgr::Instance()->SendGrade("GallbladderTriangleHurtCommonBileDuct", 0, usedtime);
    }
    if (m_Cut_BRAVERY_CYSTIC_DUCT)
    {
        COnLineGradeMgr::Instance()->SendGrade("GallbladderTriangleHurtCysticDuct", 0, usedtime);
    }

    /*<OnLineGrade Name = "GallbladderTriangleHurtCommonBileDuct" ItemCode = "L120020112" Description = "      分离胆囊三角过程中意外离断胆总管">< / OnLineGrade>
    <OnLineGrade Name = "GallbladderTriangleHurtCysticDuct" ItemCode = "L120020113" Description = "      分离胆囊三角过程中意外离断胆囊管">< / OnLineGrade>
    <OnLineGrade Name = "GallbladderTriangleHurtArteriaeCystica" ItemCode = "L120020114" Description = "      分离胆囊三角过程中意外离断胆囊动脉">< / OnLineGrade>
    */

    //用钛夹夹闭后剪断胆囊管和胆囊动脉////////////////////////////////////////////////////////////////////////
    MisMedicOrganInterface * pOrgan = NULL;
    for (DynObjMap::iterator itr = m_DynObjMap.begin(); itr != m_DynObjMap.end(); ++itr)
    {
        pOrgan = itr->second;
        if (pOrgan->m_OrganID == EDOT_BRAVERY_ARTERY || pOrgan->m_OrganID == EDOT_CYSTIC_DUCT)
        {
            int parts = dynamic_cast<MisMedicOrgan_Ordinary*> (pOrgan)->GetNumSubParts();
            int clipCount = pOrgan->GetAttachmentCount(MOAType_TiantumClip);
            if (parts >= 2 && clipCount <= 1)
            {
                if (pOrgan->m_OrganID == EDOT_BRAVERY_ARTERY)
                {
                    COnLineGradeMgr::Instance()->SendGrade("CutArteriaeCysticaWithoutClipApplicator", 0, usedtime);
                }
                else
                {
                    COnLineGradeMgr::Instance()->SendGrade("CutCysticDuctWithoutClipApplicator", 0, usedtime);
                }
            }
            break;
        }
    }

    /*<OnLineGrade Name = "CutCysticDuctWithClipApplicator" ItemCode = "L120030140" Description = "      在适当位置施放钛夹夹闭胆囊管">< / OnLineGrade>
    <OnLineGrade Name = "CutCysticDuctWithoutClipApplicator" ItemCode = "L120030142" Description = "      未夹闭即剪断胆囊管">< / OnLineGrade>

    <OnLineGrade Name = "CutCysticDuctRightPlace" ItemCode = "L120030150" Description = "      在正确位置剪断胆囊管">< / OnLineGrade>
    <OnLineGrade Name = "CutCysticDuctWrongPlace" ItemCode = "L120030151" Description = "      剪断胆囊管位置有误">< / OnLineGrade>

    <OnLineGrade Name = "CutArteriaeCysticaWithClipApplicator" ItemCode = "L120030160" Description = "      在适当位置施放钛夹夹闭胆囊动脉">< / OnLineGrade>
    <OnLineGrade Name = "CutArteriaeCysticaWithoutClipApplicator" ItemCode = "L120030161" Description = "      未夹闭即剪断胆囊动脉">< / OnLineGrade>

    <OnLineGrade Name = "CutArteriaeCysticaRightPlace" ItemCode = "L120030170" Description = "      在正确位置剪断胆囊动脉">< / OnLineGrade>
    <OnLineGrade Name = "CutArteriaeCysticaWrongPlace" ItemCode = "L120030171" Description = "      剪断胆囊动脉位置有误">< / OnLineGrade>
    */

    int sum_ReleasedClip = m_pToolsMgr->GetNumberOfReleasedTitanicClip();
    if (sum_ReleasedClip > 0 && sum_ReleasedClip <= 8)
    {
        COnLineGradeMgr::Instance()->SendGrade("ClipApplicatorExactly", 0, usedtime);
    }
    else if (sum_ReleasedClip > 8)
    {
        COnLineGradeMgr::Instance()->SendGrade("ClipApplicatorTooMany", 0, usedtime);
    }
    else
    {
    }

    //遍历器官把所有器官上的钛夹数量累加起来。
    //int sum_EffectiveClip = 0;
    //pOrgan = NULL;
    //for (DynObjMap::iterator itr = m_DynObjMap.begin(); itr != m_DynObjMap.end(); ++itr)
    //{
    //    pOrgan = itr->second;
    //    int OrganHemiClipCount = pOrgan->GetAttachmentCount(MOAType_TiantumClip);
    //    sum_EffectiveClip += OrganHemiClipCount;
    //}

    //if (sum_ReleasedClip - sum_EffectiveClip > 2)
    //{
    //    COnLineGradeMgr::Instance()->SendGrade("ClipApplicatorFall", 0, usedtime);
    //}

    if (TitanicClipInfo::s_clipEmptyCount >= 2)
    {
        COnLineGradeMgr::Instance()->SendGrade("ClipApplicatorFall", 0, usedtime);
    }


    /*<OnLineGrade Name = "ClipApplicatorExactly" ItemCode = "L120030180" Description = "      施放钛夹数量适中">< / OnLineGrade>
    <OnLineGrade Name = "ClipApplicatorTooMany" ItemCode = "L120030181" Description = "      施放钛夹较多">< / OnLineGrade>
    <OnLineGrade Name = "ClipApplicatorFall" ItemCode = "L120030182" Description = "      钛夹掉落体内">< / OnLineGrade>*/


    //分离胆囊床////////////////////////////////////////////////////////////////////////
    int currConnectCount_bottom = INT_MAX;
    VeinConnectObject * btconnect_bottom = dynamic_cast<VeinConnectObject*>(GetOrgan(EODT_VEINBOTTOMCONNECT));

    if (btconnect_bottom)
    {
        currConnectCount_bottom = btconnect_bottom->GetCurrConnectCount();

        if (currConnectCount_bottom = 0)
        {
            COnLineGradeMgr::Instance()->SendGrade("GallbladderSeperateSuccess", 0, usedtime);
            if (m_ElectricCutVeinBottom)
            {
                COnLineGradeMgr::Instance()->SendGrade("GallbladderBedElecSeperate", 0, usedtime);
            }
            else
            {
                COnLineGradeMgr::Instance()->SendGrade("GallbladderBedForceSeperate", 0, usedtime);
            }
            /*<OnLineGrade Name = "GallbladderBedElecSeperate" ItemCode = "L120040130" Description = "      胆囊通电分离">< / OnLineGrade>
            <OnLineGrade Name = "GallbladderBedForceSeperate" ItemCode = "L120040131" Description = "      胆囊钝性分离">< / OnLineGrade>*/
        }
        else
        {
            COnLineGradeMgr::Instance()->SendGrade("GallbladderSeperateFailed", 0, usedtime);
        }
    }
    /*<OnLineGrade Name = "GallbladderSeperateSuccess" ItemCode = "L120040190" Description = "      成功分离胆囊床">< / OnLineGrade>
    <OnLineGrade Name = "GallbladderSeperateFailed" ItemCode = "L120040191" Description = "      分离胆囊床没有分离完全">< / OnLineGrade>
    */

    int gallHurtCount = 0;
    for (int i = 0; i < m_gallbladder->m_BleedingRecords.size(); i++)
    {        
        if (m_gallbladder->m_BleedingRecords[i].m_HavingBeenCount == false)
        {
            gallHurtCount++;
        }        
    }

    int liverHurtCount = 0;
    pOrgan = NULL;
    for (DynObjMap::iterator itr = m_DynObjMap.begin(); itr != m_DynObjMap.end(); ++itr)
    {
        pOrgan = itr->second;
        if (pOrgan->m_OrganID == EDOT_LIVER)
        {
            for (int i = 0; i < pOrgan->m_BleedingRecords.size(); i++)
            {
                if (pOrgan->m_BleedingRecords[i].m_HavingBeenCount == false)
                {
                    liverHurtCount++;
                }
            }
            break;
        }
    }

    if (m_bFinish)
    {
        if (gallHurtCount + liverHurtCount < 6)
        {
            COnLineGradeMgr::Instance()->SendGrade("GallbladderSeperateNoHurt", 0, usedtime);
        }
        else
        {
            COnLineGradeMgr::Instance()->SendGrade("GallbladderSeperateTooMuchHurt", 0, usedtime);
        }
    }


    /*<OnLineGrade Name = "GallbladderSeperateNoHurt" ItemCode = "L120040110" Description = "      镜下操作熟练，未造成多余损伤">< / OnLineGrade>
    <OnLineGrade Name = "GallbladderSeperateTooMuchHurt" ItemCode = "L120040111" Description = "      分离胆囊床过程中造成多处损伤，出血较多">< / OnLineGrade>
    */

    //出血////////////////////////////////////////////////////////////////////////
    int BleedingPointOnLiver = 0;
    pOrgan = NULL;
    for (DynObjMap::iterator itr = m_DynObjMap.begin(); itr != m_DynObjMap.end(); ++itr)
    {
        pOrgan = itr->second;
        switch (pOrgan->GetOrganType())
        {
        case EDOT_LIVER:
            BleedingPointOnLiver += pOrgan->GetNumberOfActiveBleedPoint();
            break;
        default:break;
        }
    }
    if (m_bFinish)
    {
        if (BleedingPointOnLiver < 6)
        {
            COnLineGradeMgr::Instance()->SendGrade("NoMuchBleedingPoint", 0, usedtime);
        }
        else if (BleedingPointOnLiver >= 6)
        {
            COnLineGradeMgr::Instance()->SendGrade("MuchBleedingPoint", 0, usedtime);
        }
        else
        {
        }
    }
    else
    {
        COnLineGradeMgr::Instance()->SendGrade("Unfinish_BleedingPointUnknow", 0, usedtime);
    }

    Real bleedingVolume = 0.0f;
    for (DynObjMap::iterator itr = m_DynObjMap.begin(); itr != m_DynObjMap.end(); ++itr)
    {
        bleedingVolume += itr->second->GetBleedingVolume();
    }
    if (m_bFinish)
    {
        if (bleedingVolume < 30.0f && bleedingVolume > GP_EPSILON)
        {
            COnLineGradeMgr::Instance()->SendGrade("NoMuchBleedingVolume", 0, usedtime);
        }
        else if (bleedingVolume >= 30.0f)
        {
            COnLineGradeMgr::Instance()->SendGrade("MuchBleedingVolume", 0, usedtime);
        }
        else
        {
        }
    }
    else
    {
        COnLineGradeMgr::Instance()->SendGrade("Unfinish_BleedingVolumeUnknow", 0, usedtime);
    }

    //set max bleed time
    Real maxBloodTime = 0.f;
    pOrgan = NULL;
    for (DynObjMap::iterator itr = m_DynObjMap.begin(); itr != m_DynObjMap.end(); ++itr)
    {
        pOrgan = itr->second;
        float curOrganMaxTime = pOrgan->GetCurMaxBleedTime();
        if (maxBloodTime < curOrganMaxTime)
            maxBloodTime = curOrganMaxTime;
    }

    if (maxBloodTime > GP_EPSILON)
    {
        if (maxBloodTime < 10.0f)
        {
            COnLineGradeMgr::Instance()->SendGrade("StopBleeding", 0, usedtime);
        }
        else
        {
            COnLineGradeMgr::Instance()->SendGrade("StopBleeding0", 0, usedtime);
        }
    }
    //通电////////////////////////////////////////////////////////////////////////
    Real time = m_pToolsMgr->GetMaxKeeppingElectricTime();
    if (time > 1.0f)
    {
        if (time < 5.0f)
        {
            COnLineGradeMgr::Instance()->SendGrade("SingleElecTime", 0, usedtime);
        }
        else
        {
            COnLineGradeMgr::Instance()->SendGrade("SingleElecTime0", 0, usedtime);
        }
    }

    Real totalTime = m_pToolsMgr->GetTotalElectricTime();
    Real validTime = m_pToolsMgr->GetValidElectricTime();
    Real value = 0.0f;
	if (totalTime > 0.0001f)
	{
		value = 100.f * validTime / totalTime;

		if (value > 0.7f)
		{
			COnLineGradeMgr::Instance()->SendGrade("ElecEffecient", 0, usedtime);
		}
		else
		{
			COnLineGradeMgr::Instance()->SendGrade("ElecEffecient0", 0, usedtime);
		}
	}
    //镜头操作平稳////////////////////////////////////////////////////////////////////////

    Real camspeed = GetCameraSpeed();
    if (camspeed > GP_EPSILON)
    {
        if (camspeed <= 2.0f)
            COnLineGradeMgr::Instance()->SendGrade("CameraHandle_Normal", 0, usedtime);
        else if (camspeed <= 5.0f)
            COnLineGradeMgr::Instance()->SendGrade("CameraHandle_Fast", 0, usedtime);
        else
            COnLineGradeMgr::Instance()->SendGrade("CameraHandle_TooFast", 0, usedtime);
    }

    //器械操作平稳////////////////////////////////////////////////////////////////////////
    Real leftToolSpeed = m_pToolsMgr->GetLeftToolMovedSpeed();
    Real rightTooSpeed = m_pToolsMgr->GetRightToolMovedSpeed();
    Real ToolSpeed = std::max(leftToolSpeed, rightTooSpeed);

    /*if (leftToolSpeed > 0.0f && rightTooSpeed > 0.0f)
    {
        ToolSpeed = (leftToolSpeed + rightTooSpeed) / 2;
    }
    else if (leftToolSpeed == 0.0f && rightTooSpeed > 0.0f)
    {
        ToolSpeed = rightTooSpeed;
    }
    else if (leftToolSpeed > 0.0f && rightTooSpeed == 0.0f)
    {
        ToolSpeed = leftToolSpeed;
    }
    else
    {
    }*/
    if (ToolSpeed <= 5.0f && ToolSpeed > GP_EPSILON)
        COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Normal", 0, usedtime);
    else if (ToolSpeed > 5.0f && ToolSpeed <= 10.0f)
        COnLineGradeMgr::Instance()->SendGrade("MachineHandle_Fast", 0, usedtime);
    else if (ToolSpeed > 10.0f)
        COnLineGradeMgr::Instance()->SendGrade("MachineHandle_TooFast", 0, usedtime);
    else
    {
    }
    //训练用时////////////////////////////////////////////////////////////////////////
    if (m_bFinish)
    {
        if (usedtime < 1800)
        {
            COnLineGradeMgr::Instance()->SendGrade("Finished_In30M", 0, usedtime);
        }
    }
    else
    {
        COnLineGradeMgr::Instance()->SendGrade("UnFinish_In30M", 0, usedtime);
    }

    //////////////////////////////////////////////////////////////////////////
    __super::OnSaveTrainingReport();
}