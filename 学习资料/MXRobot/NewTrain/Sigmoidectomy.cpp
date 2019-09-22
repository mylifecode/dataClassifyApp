#include "Sigmoidectomy.h"
#include "MxEventsDump.h"
#include "InputSystem.h"
#include "MXOgreGraphic.h"
#include "MisMedicRigidPrimtive.h"
#include "VeinConnectObject.h"
#include "MisMedicObjectSerializer.h"
#include "BasicTraining.h"
#include "XMLWrapperTraining.h"
#include "XMLWrapperDataForDeviceCandidate.h"
#include "Instruments\Tool.h"
#include "OgreQuaternion.h"
#include "Inception.h"
#include "MisMedicObjectUnion.h"
#include "ShadowMap.h"
#include "qevent.h"
#include "DeferredRendFrameWork.h"

void SigmoidCutHandleEvent(MxEvent * pEvent, ITraining * pTraining)
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

void CSigmoidectomy::BeforeRendStage(DeferredRendSceneManager::DeferredRendStage stage)
{
	if (stage == DeferredRendSceneManager::Stage_CustomFirst)
	{
		DynObjMap::iterator itor = m_DynObjMap.begin();
		while (itor != m_DynObjMap.end())
		{
			MisMedicOrganInterface * organ = dynamic_cast<MisMedicOrganInterface*>(itor->second);
            if (organ == NULL)
            {
                itor++;
                continue;
            }
			if (m_DepthLowOrgans.find(organ) == m_DepthLowOrgans.end())
				organ->setVisible(true);
			else
				organ->setVisible(false);
			itor++;
		}
		
	}

	//re-redn cut part depth
	if (stage == DeferredRendSceneManager::Stage_CustomMiddle)
	{
		DynObjMap::iterator itor = m_DynObjMap.begin();
		while (itor != m_DynObjMap.end())
		{
			MisMedicOrganInterface * organ = dynamic_cast<MisMedicOrganInterface*>(itor->second);
            if (organ == NULL)
            {
                itor++;
                continue;
            }
			if (organ == m_SigmoidCutpart)
			{
				organ->setVisible(true);
				Ogre::Pass * pass = m_SigmoidCutpart->GetOwnerMaterialPtr()->getTechnique(0)->getPass(0);
				float constdbias = pass->getDepthBiasConstant();
				pass->setDepthBias(1000);
				pass->setColourWriteEnabled(false);//only draw depth
				pass->setFragmentProgram("MisMedical/TessTemplateDX11_PSWriteDepth");
			}
			else
				organ->setVisible(false);
			itor++;
		}
		//m_SigmoidCutpart->m_OwnerMaterialPtr->setColourWriteEnabled(false);
	}

	//rend dynamic dome
	if (stage == DeferredRendSceneManager::Stage_CustomFinal)
	{
		DynObjMap::iterator itor = m_DynObjMap.begin();
		while (itor != m_DynObjMap.end())
		{
			MisMedicOrganInterface * organ = dynamic_cast<MisMedicOrganInterface*>(itor->second);
            if (organ == NULL)
            {
                itor++;
                continue;
            }
			if (m_DepthLowOrgans.find(organ) != m_DepthLowOrgans.end())
				organ->setVisible(true);
			else
				organ->setVisible(false);
			itor++;
		}
	}
}
void CSigmoidectomy::AfterRendStage(DeferredRendSceneManager::DeferredRendStage stage)
{
	if (stage == DeferredRendSceneManager::Stage_CustomFinal)
	{
		DynObjMap::iterator itor = m_DynObjMap.begin();
		while (itor != m_DynObjMap.end())
		{
			MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(itor->second);
            if (organ == NULL)
            {
                itor++;
                continue;
            }
			if (organ)
			{
				organ->setVisible(true);
			}
			itor++;
		}
		//
		m_pToolsMgr->SetVisibleForAllTools(true);
		if (m_SigmoidCutpart)
		{
			Ogre::Pass * pass = m_SigmoidCutpart->GetOwnerMaterialPtr()->getTechnique(0)->getPass(0);
			float constdbias = pass->getDepthBiasConstant();
			pass->setDepthBias(0);
			pass->setColourWriteEnabled(true);
			pass->setFragmentProgram("MisMedical/TessTemplateDX11_ps_2");
		}
	}
	else
	{
		m_pToolsMgr->SetVisibleForAllTools(false);
	}
}
CSigmoidectomy::CSigmoidectomy(const Ogre::String & strName): MisNewTraining()
{
	m_SigmoidCutpart = 0;
	m_DynamicDomPart = 0;
	m_DetectCameraIntersect = false;// true;
#if(1)
	DeferredRendFrameWork::Get()->AddStageRendListener(this);
	DeferredRendFrameWork::Get()->m_UseCustomRendStage = true;
#endif
}

CSigmoidectomy::~CSigmoidectomy(void)
{
	//DeferredRendFrameWork::Get()->RemoveStageRendListener(this);
    //删除约束
    for (int i = 0; i != m_ObjAdhersions.size(); ++i)
    {
        if (m_ObjAdhersions[i])
        {
            delete m_ObjAdhersions[i];
            m_ObjAdhersions[i] = NULL;
        }
    }
    m_ObjAdhersions.clear();
}

bool CSigmoidectomy::OrganShouldBleed(MisMedicOrgan_Ordinary* organ, int faceID, float* pFaceWeight)
{
	if (organ->m_OrganID == EDOT_SIGMOIDCUTPART
	 || organ->m_OrganID == EDOT_IMA
	 || organ->m_OrganID == EDOT_IMV)
	{
		return true;
	}
	else
	{
		return false;
	}
}
bool CSigmoidectomy::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
    //遍历所有器官
    // int oldmask =  m_physBOdy->getcollisionmask；
    //如果器官是操作部分
    //器官->m_physBody->setCollisionMask(oldmask | OPC_RIGIDDOME);
    //否则
    //器官->m_physBody->setCollisionMask(oldmask & (~OPC_RIGIDDOME));
    bool result = MisNewTraining::Initialize(pTrainingConfig,pToolConfig);

	//DeferredRendFrameWork::Get()->SetBloomBrightPassThreshold(1.0f);///this means disable

    for (DynObjMap::iterator obj = m_DynObjMap.begin(); obj != m_DynObjMap.end();obj++)
    {
       // MisMedicOrgan_Ordinary * organor = dynamic_cast<MisMedicOrgan_Ordinary*>(obj->second);
		//if(organor)
		{
			if (obj->first == EDOT_SIGMOIDCUTPART)
			{
				m_SigmoidCutpart = dynamic_cast<MisMedicOrgan_Ordinary*>(obj->second);
				//m_SigmoidCutpart->m_MaxSoftNodeSpeed = 3.0f;
				m_SigmoidCutpart->SetTimeNeedToEletricCut(1.5f);
				m_SigmoidCutpart->SetTetraCollapseParam(0.15f, 2);
				m_SigmoidCutpart->SetEletricCutParameter(0.4f ,20);
				m_SigmoidCutpart->SetBurnNodeColor(Ogre::ColourValue(180.0 / 255.0 ,  125 / 255.0 , 54.0 / 255.0 ,1));
				m_SigmoidCutpart->m_BurnShrinkRate = 0.98f;
				m_SigmoidCutpart->m_CutWidthScale = 1.4f;
				m_SigmoidCutpart->SetBleedRadius(0.004f);
			}
			else if(obj->first == EODT_DOME_ACTIVE)
			{				
				m_DynamicDomPart = dynamic_cast<MisMedicOrgan_Ordinary*>(obj->second);
				m_DepthLowOrgans.insert(m_DynamicDomPart);
				for (int f = 0, nf = m_DynamicDomPart->m_physbody->GetNumFace(); f < nf; f++)
				{
					GFPhysSoftBodyFace * face = m_DynamicDomPart->m_physbody->GetFaceAtIndex(f);

					bool disablecollide =  false;
					for(int n = 0 ; n < 3 ; n++)
					{
						float height = face->m_Nodes[n]->m_UnDeformedPos.m_y;
						if(height > 10.0f)
						{
							disablecollide = true;
							face->m_Nodes[n]->SetMass(0);
						}
					}
					if(disablecollide)
					{
					   face->DisableCollideWithRigid();
					}
				}
			}
			else if(obj->first == 9)
			{
				MisMedicOrgan_Ordinary * Ureter = dynamic_cast<MisMedicOrgan_Ordinary*>(obj->second);
				m_DepthLowOrgans.insert(Ureter);
			}
			else if (obj->first == 40)
			{
				MisMedicOrgan_Ordinary * Gonadal = dynamic_cast<MisMedicOrgan_Ordinary*>(obj->second);
				m_DepthLowOrgans.insert(Gonadal);
			}
			else if (obj->first == 24)
			{
				VeinConnectObject * connect = dynamic_cast<VeinConnectObject*>(obj->second);
				m_DepthLowOrgans.insert(connect);
			}
			//if (obj->first != EDOT_SIGMOIDCUTPART)
			//{        
			//	uint32 oldmask = organor->m_physbody->GetCollisionCategory();        
			   // uint32 mask = (oldmask & (~OPC_RIGIDDOME));
			   // organor->m_physbody->SetCollisionCategory(mask);            
			//}
		}
    }
	m_CameraPlaceIndex = 1;
    m_ToolPlaceIndex = 1;

	//Ogre::Entity * domeStatic = m_pOms->GetEntity("StaticDome$1", true);

	//if (domeStatic && m_DynamicDomPart)
	//{
	//	Ogre::SceneNode * nodestatic = domeStatic->getParentSceneNode();

	//	m_StaticDomeMeshPtr = domeStatic->getMesh();

	//	m_StaDynDomeUnion.AttachStaticMeshToDynamicOrgan(m_StaticDomeMeshPtr,
	//		nodestatic,
	//		nodestatic->getPosition(),
	//		nodestatic->getOrientation(),
	//		nodestatic->getScale(),
	//		m_DynamicDomPart, 0.05f);
	//}
	
    return result;
}

void CSigmoidectomy::BuildOrgansVolumeTextureCoord()
{
	DynObjMap::iterator itor = m_DynObjMap.begin();
	while (itor != m_DynObjMap.end())
	{
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(itor->second);

		if (organ && organ->GetCreateInfo().m_objTopologyType == DOT_VOLMESH)
		{
			organ->BuildTetrahedronNodeTextureCoord(GFPhysVector3(0, 1, 0), true);
		}

		itor++;
	}
}
void CSigmoidectomy::SerializerReadFinish(MisMedicOrgan_Ordinary * organ , MisMedicObjetSerializer & serializer)
{
   /*
   Real coff = organ->GetCreateInfo().m_ExpandValue;
    GFPhysVector3 center = organ->GetCreateInfo().m_ExpandCenterPos;
    GFPhysVector3 point1 = organ->GetCreateInfo().m_ExpandPlanePoint1;
    GFPhysVector3 point2 = organ->GetCreateInfo().m_ExpandPlanePoint2;
    GFPhysVector3 point3 = organ->GetCreateInfo().m_ExpandPlanePoint3;
    if(fabs(coff) > GP_EPSILON)
    {                
        //serializer.GetExpandPosAlongNormal(coff,points);
        serializer.ExpandByScale(coff,center, point1, point2, point3);
    }

//////////////////////////////////////////////////////////////////////////        
    if (organ->GetOrganType() == EDOT_SIGMOIDNOCUTPART)//set expandvalue to use undeformedpos to build links
    {
        GFPhysVector3 offset = GFPhysVector3(0.0f,0.0f,0.0f);
        serializer.Rearrange(offset);
    }*/
}

MisMedicOrganInterface * CSigmoidectomy::LoadOrganism(MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain)
{
    return MisNewTraining::LoadOrganism(cs, ptrain);
}

void CSigmoidectomy::KeyPress(QKeyEvent * event)
{
     //if (!(m_pTrainingConfig->m_DataForDeviceCandidates.empty()))//BCED1D2 <--> 01234
     //{
     //    Ogre::Camera* sourceCamera1 = m_pOms->GetCamera(Ogre::String(CAMERA_NAME_1) + Ogre::String("1"),false); 
     //    Ogre::Quaternion quat1 = sourceCamera1->getParentNode()->getInitialOrientation();  //sourceCamera1->getParentNode()->getInitialOrientation();//getOrientation
     //    Ogre::Vector3  pos1 = sourceCamera1->getParentNode()->getInitialPosition(); //sourceCamera1->getParentNode()->getInitialPosition();
 
     //    Ogre::Camera* sourceCamera2 = m_pOms->GetCamera(Ogre::String(CAMERA_NAME_2) + Ogre::String("1"),false);
     //    Ogre::Quaternion quat2 = sourceCamera2->getParentNode()->getInitialOrientation();
     //    Ogre::Vector3  pos2 = sourceCamera2->getParentNode()->getInitialPosition();
 
     //    Ogre::Camera* sourceCamera3 = m_pOms->GetCamera(Ogre::String(CAMERA_NAME_3) + Ogre::String("1"),false);
     //    Ogre::Quaternion quat3 = sourceCamera3->getParentNode()->getInitialOrientation();
     //    Ogre::Vector3  pos3 = sourceCamera3->getParentNode()->getInitialPosition();
 
     //    if (event->key() == Qt::Key_1)//B1C
     //    {           
     //        if ( m_pTrainingConfig->m_DataForDeviceCandidates[1] && m_pTrainingConfig->m_DataForDeviceCandidates[0] && sourceCamera1)
     //        {
     //            m_pToolsMgr->RemoveAllCurrentTool();
     //            m_pTrainingConfig->m_DataForDeviceWorkspaceRight = m_pTrainingConfig->m_DataForDeviceCandidates[1]->m_DataCandidate;
     //            m_pTrainingConfig->m_DataForDeviceWorkspaceLeft = m_pTrainingConfig->m_DataForDeviceCandidates[0]->m_DataCandidate;
 
     //            InputSystem::GetInstance(DEVICETYPE_LEFT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceLeft,false);
     //            InputSystem::GetInstance(DEVICETYPE_RIGHT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceRight,false);
 
     //            //int zorder = m_pLargeCamera->getViewport()->getZOrder();
     //            //MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_LARGE)->removeViewport(zorder);
     //            //m_pLargeCamera->getParentNode()->setOrientation(quat1);
     //            //m_pLargeCamera->getParentNode()->setPosition(pos1);
     //            InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(quat1);
     //            InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(pos1);
 
     //            //m_viewport = MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_LARGE)->addViewport(sourceCamera1);
     //        }       
     //    }
     //    else if(event->key() == Qt::Key_2)//D1E
     //    {          
     //        if ( m_pTrainingConfig->m_DataForDeviceCandidates[2] && m_pTrainingConfig->m_DataForDeviceCandidates[3]  && sourceCamera2)
     //        {
     //            m_pToolsMgr->RemoveAllCurrentTool();
     //            m_pTrainingConfig->m_DataForDeviceWorkspaceRight = m_pTrainingConfig->m_DataForDeviceCandidates[2]->m_DataCandidate;           
     //            m_pTrainingConfig->m_DataForDeviceWorkspaceLeft = m_pTrainingConfig->m_DataForDeviceCandidates[3]->m_DataCandidate;
     //            InputSystem::GetInstance(DEVICETYPE_LEFT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceLeft,false);
     //            InputSystem::GetInstance(DEVICETYPE_RIGHT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceRight,false);
     //            InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(quat2);
     //            InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(pos2);
     //        } 
     //    }         
     //    else if(event->key() == Qt::Key_3)//B2D2
     //    {           
     //        if ( m_pTrainingConfig->m_DataForDeviceCandidates[4] && m_pTrainingConfig->m_DataForDeviceCandidates[5] && sourceCamera3)
     //        {
     //            m_pToolsMgr->RemoveAllCurrentTool();
     //            m_pTrainingConfig->m_DataForDeviceWorkspaceRight = m_pTrainingConfig->m_DataForDeviceCandidates[4]->m_DataCandidate;
     //            m_pTrainingConfig->m_DataForDeviceWorkspaceLeft = m_pTrainingConfig->m_DataForDeviceCandidates[5]->m_DataCandidate;
     //            InputSystem::GetInstance(DEVICETYPE_LEFT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceLeft,false);
     //            InputSystem::GetInstance(DEVICETYPE_RIGHT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceRight,false);
     //            InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(quat3);
     //            InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(pos3);
     //        }        
     //    } 
 
     //}    
}

void CSigmoidectomy::OnSimpleUIEvent(const SimpleUIEvent & event)
{
    if (event.m_EventAction == SimpleUIEvent::eEA_MouseReleased)
    {
        Ogre::OverlayElement *pElement = event.m_pEventElement;
        //if (pElement->getName() == "SigmoidCutOverlayElement/LeftArrow")
        //{
        //    if (--m_CameraPlaceIndex < 1)
        //        m_CameraPlaceIndex = 3;

        //    //ChangeCamera(m_CameraPlaceIndex);

        //    Ogre::OverlayElement *pTrocarInfoElement = CSimpleUIManger::Instance()->GetElement("SigmoidCutOverlayElement/TrocarInfo");
        //    if (pTrocarInfoElement)
        //    {
        //        Ogre::String materialName = "SigmoidCut/Trocar_Config_" + Ogre::StringConverter::toString(m_CameraPlaceIndex);
        //        pTrocarInfoElement->setMaterialName(materialName);
        //    }
        //}
        //else if (pElement->getName() == "SigmoidCutOverlayElement/RightArrow")
        //{
        //    if (++m_CameraPlaceIndex > 3)
        //        m_CameraPlaceIndex = 1;

        //    //ChangeCamera(m_CameraPlaceIndex);

        //    Ogre::OverlayElement *pTrocarInfoElement = CSimpleUIManger::Instance()->GetElement("SigmoidCutOverlayElement/TrocarInfo");
        //    if (pTrocarInfoElement)
        //    {
        //        Ogre::String materialName = "SigmoidCut/Trocar_Config_" + Ogre::StringConverter::toString(m_CameraPlaceIndex);
        //        pTrocarInfoElement->setMaterialName(materialName);
        //    }
        //}
        //else 
        if (pElement->getName() == "SigmoidCutOverlayElement/LeftArrow2")
        {
            if (--m_ToolPlaceIndex < 1)
                m_ToolPlaceIndex = 2;

            ChangeToolPlace(m_ToolPlaceIndex);

            Ogre::OverlayElement *pTrocarInfoElement = CSimpleUIManger::Instance()->GetElement("SigmoidCutOverlayElement/TrocarInfo2");
            if (pTrocarInfoElement)
            {
                Ogre::String materialName = "SigmoidCut/Trocar_Config_" + Ogre::StringConverter::toString(m_ToolPlaceIndex);
                pTrocarInfoElement->setMaterialName(materialName);
            }
        }
        else if (pElement->getName() == "SigmoidCutOverlayElement/RightArrow2")
        {
            if (++m_ToolPlaceIndex > 2)
                m_ToolPlaceIndex = 1;

            ChangeToolPlace(m_ToolPlaceIndex);

            Ogre::OverlayElement *pTrocarInfoElement = CSimpleUIManger::Instance()->GetElement("SigmoidCutOverlayElement/TrocarInfo2");
            if (pTrocarInfoElement)
            {
                Ogre::String materialName = "SigmoidCut/Trocar_Config_" + Ogre::StringConverter::toString(m_ToolPlaceIndex);
                pTrocarInfoElement->setMaterialName(materialName);
            }
        }
    }
}

int CSigmoidectomy::ChangeToolPlace(int id)
{
    if (!(m_pTrainingConfig->m_DataForDeviceCandidates.empty()))//BCED1D2 <--> 01234
    {
        switch (id)
        {
        case 1:
            if (m_pTrainingConfig->m_DataForDeviceCandidates[0] && m_pTrainingConfig->m_DataForDeviceCandidates[1])
            {
                m_pToolsMgr->RemoveAllCurrentTool(false);
                m_pTrainingConfig->m_DataForDeviceWorkspaceRight = m_pTrainingConfig->m_DataForDeviceCandidates[1]->m_DataCandidate;
                m_pTrainingConfig->m_DataForDeviceWorkspaceLeft = m_pTrainingConfig->m_DataForDeviceCandidates[0]->m_DataCandidate;

                InputSystem::GetInstance(DEVICETYPE_LEFT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceLeft, false);
                InputSystem::GetInstance(DEVICETYPE_RIGHT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceRight, false);
            }
            return 0;
        case 2:
            if (m_pTrainingConfig->m_DataForDeviceCandidates[0] && m_pTrainingConfig->m_DataForDeviceCandidates[2])
            {
                m_pToolsMgr->RemoveAllCurrentTool(false);
                m_pTrainingConfig->m_DataForDeviceWorkspaceRight = m_pTrainingConfig->m_DataForDeviceCandidates[2]->m_DataCandidate;
                m_pTrainingConfig->m_DataForDeviceWorkspaceLeft = m_pTrainingConfig->m_DataForDeviceCandidates[0]->m_DataCandidate;

                InputSystem::GetInstance(DEVICETYPE_LEFT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceLeft, false);
                InputSystem::GetInstance(DEVICETYPE_RIGHT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceRight, false);
            }
            return 0;
        //case 3:
        //    if (m_pTrainingConfig->m_DataForDeviceCandidates[4] && m_pTrainingConfig->m_DataForDeviceCandidates[5])
        //    {
        //        m_pToolsMgr->RemoveAllCurrentTool(false);
        //        m_pTrainingConfig->m_DataForDeviceWorkspaceRight = m_pTrainingConfig->m_DataForDeviceCandidates[4]->m_DataCandidate;
        //        m_pTrainingConfig->m_DataForDeviceWorkspaceLeft = m_pTrainingConfig->m_DataForDeviceCandidates[5]->m_DataCandidate;

        //        InputSystem::GetInstance(DEVICETYPE_LEFT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceLeft, false);
        //        InputSystem::GetInstance(DEVICETYPE_RIGHT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceRight, false);
        //    }
        //    return 0;
        default:
            return 0;
        }
    }
    return 0;
}
int CSigmoidectomy::ChangeCamera(int id)
{
    if (true)
    {
        Ogre::Camera* sourceCamera1 = m_pOms->GetCamera(Ogre::String(CAMERA_NAME_1) + Ogre::String("1"), false);
        Ogre::Quaternion quat1 = sourceCamera1->getParentNode()->getInitialOrientation();  //sourceCamera1->getParentNode()->getInitialOrientation();//getOrientation
        Ogre::Vector3  pos1 = sourceCamera1->getParentNode()->getInitialPosition(); //sourceCamera1->getParentNode()->getInitialPosition();

        Ogre::Camera* sourceCamera2 = m_pOms->GetCamera(Ogre::String(CAMERA_NAME_2) + Ogre::String("1"), false);
        Ogre::Quaternion quat2 = sourceCamera2->getParentNode()->getInitialOrientation();
        Ogre::Vector3  pos2 = sourceCamera2->getParentNode()->getInitialPosition();

        Ogre::Camera* sourceCamera3 = m_pOms->GetCamera(Ogre::String(CAMERA_NAME_3) + Ogre::String("1"), false);
        Ogre::Quaternion quat3 = sourceCamera3->getParentNode()->getInitialOrientation();
        Ogre::Vector3  pos3 = sourceCamera3->getParentNode()->getInitialPosition();

        switch(id)
        {
        case 1:
            if (sourceCamera1)
            {                
                InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(quat1);
                InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(pos1);
            }  
            return 0;
        case 2:
            if (sourceCamera2)
            {
                InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(quat2);
                InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(pos2);
            }
            return 0;
        case 3:
            if (sourceCamera3)
            {                
                InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultOrientation(quat3);
                InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetDefaultPosition(pos3);
            }
            return 0;
        default:
            return 0;
        }    
    }
    return 0;
}

bool CSigmoidectomy::BeginRendOneFrame(float timeelapsed)
{
	bool result = MisNewTraining::BeginRendOneFrame(timeelapsed);

	//if (m_StaticDomeMeshPtr.isNull() == false)
	//{
	//	m_StaDynDomeUnion.UpdateStaticVertexByDynamic(m_StaticDomeMeshPtr);
	//}

	return result;
}
bool CSigmoidectomy::Update(float dt)
{
    bool result = MisNewTraining::Update(dt);
        
#if(0)
    DynObjMap::iterator itor = m_DynObjMap.find(EDOT_IMA);

    if(itor == m_DynObjMap.end())
        return true;

    MisMedicOrgan_Ordinary* m_OrganIma = (MisMedicOrgan_Ordinary *)itor->second;

    MisMedicObjectAdhersion * adhersion = new MisMedicObjectAdhersion();
    adhersion->BuildUniversalLinkFromAToB( *m_OrganIma, *m_SigmoidCutpart,  0.99 );
    m_ObjAdhersions.push_back(adhersion);
#endif
    return result;
}

void CSigmoidectomy::InternalSimulateEnd(int currStep , int TotalStep , Real dt)
{
	MisNewTraining::InternalSimulateEnd(currStep ,  TotalStep ,  dt);
}

void CSigmoidectomy::OnOrganCutByTool(MisMedicOrganInterface * pOrgan, bool iselectriccut)
{
    if (pOrgan->m_OrganID == EDOT_SIGMOIDCUTPART)
    {
        bool finish = Checkfinish();
    }
}
bool CSigmoidectomy::Checkfinish()
{
    int numOfAllNode = 0;
    int numOfNodeinGreatestSubpart = 0;
    bool cutsucess = false;

    if (m_SigmoidCutpart->GetNumSubParts() == 1)
    {
        return false;
    }

	for (int s = 0, ns = m_SigmoidCutpart->GetNumSubParts(); s < ns; ++s)
    {
        numOfAllNode += m_SigmoidCutpart->GetSubPart(s)->m_Nodes.size();
    }

	for (int s = 0,ns = m_SigmoidCutpart->GetNumSubParts(); s < ns; ++s)
    {
		int n = m_SigmoidCutpart->GetSubPart(s)->m_Nodes.size();
        if (n >= numOfNodeinGreatestSubpart)
        {
            numOfNodeinGreatestSubpart = n;
        }
        float ratio = (float)numOfNodeinGreatestSubpart / numOfAllNode;

        if(ratio < 0.8f)
        {
            cutsucess = true;        
            break;
        }
    }

    if (cutsucess)
    {              
        Inception::Instance()->EmitShowMovie("End");
        TrainingFinish();
        if(m_ScoreSys)
        {
            m_ScoreSys->SetTrainSucced();
        }
        return true;
    }
    else
    {
        return false;
    }
    
    
}
