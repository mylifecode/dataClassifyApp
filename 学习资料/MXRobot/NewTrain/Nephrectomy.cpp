#include "Nephrectomy.h"
#include "InputSystem.h"
#include "XMLWrapperTraining.h"
#include "XMLWrapperDataForDeviceCandidate.h"
#include "VeinConnectObject.h"
CNephrectomy::CNephrectomy(const Ogre::String & strName)
{
    m_Gerotas = 0;
    m_MesoColon = 0;
    m_ActiveDome = 0;

    DeferredRendFrameWork::Get()->AddStageRendListener(this);
    DeferredRendFrameWork::Get()->m_UseCustomRendStage = true;
}
//======================================================================================================================
CNephrectomy::~CNephrectomy(void)
{
    DeferredRendFrameWork::Get()->RemoveStageRendListener(this);
}
//======================================================================================================================
bool CNephrectomy::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
    bool result = MisNewTraining::Initialize(pTrainingConfig, pToolConfig);

    for (DynObjMap::iterator obj = m_DynObjMap.begin(); obj != m_DynObjMap.end(); obj++)
    {
        VeinConnectObject* connect = dynamic_cast<VeinConnectObject*>(obj->second);
        if (connect)
        {
            m_DepthLowOrgans.insert(connect);
        }
        if (obj->first == EODT_DOME_ACTIVE)
        {
            m_ActiveDome = dynamic_cast<MisMedicOrgan_Ordinary*>(obj->second);
        }
        else if (obj->first == EDOT_GEROTAS)
        {
            m_Gerotas = dynamic_cast<MisMedicOrgan_Ordinary*>(obj->second);
            m_Gerotas->SetEletricCutParameter(0.05f,10);
            m_Gerotas->m_CutWidthScale = 2.0f;
            m_DepthLowOrgans.insert(m_Gerotas);
        }
        else if(obj->first == EDOT_KIDNEY_VESSELS)
        {
            m_KidneyVessels = dynamic_cast<MisMedicOrgan_Ordinary*>(obj->second);
            m_DepthLowOrgans.insert(m_KidneyVessels);
        }
        else if(obj->first == EDOT_MESOCOLON)
        {
            m_MesoColon = dynamic_cast<MisMedicOrgan_Ordinary*>(obj->second);
            m_MesoColon->SetTimeNeedToEletricCut(0.8f);

            m_MesoColon->SetEletricCutParameter(0.5f, 10);
            m_MesoColon->SetBurnNodeColor(Ogre::ColourValue(180.0 / 255.0, 125 / 255.0, 54.0 / 255.0, 1));
            m_MesoColon->m_BurnShrinkRate = 1.0f;
            m_MesoColon->m_CutWidthScale = 1.2f;
        }
    }

    m_ToolPlaceIndex = 1;
    return result;
}
//======================================================================================================================

void CNephrectomy::CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig)
{
    MisNewTraining::CreateTrainingScene(pTrainingConfig);
}

void CNephrectomy::BuildOrgansVolumeTextureCoord()
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

//======================================================================================================================
bool CNephrectomy::BeginRendOneFrame(float timeelapsed)
{
    bool result = MisNewTraining::BeginRendOneFrame(timeelapsed);
    return result;
}
//======================================================================================================================
bool CNephrectomy::Update(float dt)
{
    bool result = MisNewTraining::Update(dt);

    VeinConnectObject * btconnect = dynamic_cast<VeinConnectObject*>(GetOrgan(EDOT_KIDNEYCONNECT_STRONG));

    if (btconnect)
    {
        btconnect->m_SolveFFVolumeCS = true;
		if(m_Gerotas && m_Gerotas->IsClamped())
        {
            btconnect->m_SolveFFVolumeCS = false;
        }
    }
    return result;
}
//======================================================================================================================
void CNephrectomy::BeforeRendStage(DeferredRendSceneManager::DeferredRendStage stage)
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

    //re-render cut part depth
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
            if (organ == m_MesoColon)
            {
                organ->setVisible(true);
                Ogre::Pass * pass = m_MesoColon->GetOwnerMaterialPtr()->getTechnique(0)->getPass(0);
                float constdbias = pass->getDepthBiasConstant();
                pass->setDepthBias(1000);
                pass->setColourWriteEnabled(false);//only draw depth
                pass->setFragmentProgram("MisMedical/TessTemplateDX11_PSWriteDepth");
            }
            else
                organ->setVisible(false);
            itor++;
        }
        //m_Gerotas->m_OwnerMaterialPtr->setColourWriteEnabled(false);
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
//======================================================================================================================

void CNephrectomy::AfterRendStage(DeferredRendSceneManager::DeferredRendStage stage)
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
        if (m_MesoColon)
        {
            Ogre::Pass * pass = m_MesoColon->GetOwnerMaterialPtr()->getTechnique(0)->getPass(0);
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
//======================================================================================================================

MisMedicOrganInterface * CNephrectomy::LoadOrganism(MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain)
{
    if (cs.m_objTopologyType == DOT_UNDEF)
    {        
        m_pOms->s_nLoadCount++;
        //////////////////////////////////////////////////////////////////////////
        
        Ogre::String rigidType = cs.m_RigidType;
        
        transform(rigidType.begin(), rigidType.end(), rigidType.begin(), ::tolower);
        
        if ((int)rigidType.find("areamark") >= 0)
        {            
            cs.m_objTopologyType = DOT_AreaMark;
            m_Area = Ogre::TextureManager::getSingleton().load(
                cs.m_s3mfilename, 
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
                Ogre::TEX_TYPE_2D,
                0, 
                1.0f, 
                false, 
                Ogre::PF_A8R8G8B8);
            m_AreaWidth = m_Area->getWidth();
            m_AreaHeight = m_Area->getHeight();
            m_AreaMarkTextureName = cs.m_s3mfilename;
            return NULL;
        }
    }

    return MisNewTraining::LoadOrganism(cs, ptrain);
}
//======================================================================================================================

void CNephrectomy::OnSimpleUIEvent(const SimpleUIEvent & event)
{
    if (event.m_EventAction == SimpleUIEvent::eEA_MouseReleased)
    {
        Ogre::OverlayElement *pElement = event.m_pEventElement;        
        if (pElement->getName() == "KidneyCutOverlayElement/LeftArrow")
        {
            if (--m_ToolPlaceIndex < 1)
                m_ToolPlaceIndex = 2;

            ChangeToolPlace(m_ToolPlaceIndex);

            Ogre::OverlayElement *pTrocarInfoElement = CSimpleUIManger::Instance()->GetElement("KidneyCutOverlayElement/TrocarInfo");
            if (pTrocarInfoElement)
            {
                Ogre::String materialName = "KidneyCut/Trocar_Config_" + Ogre::StringConverter::toString(m_ToolPlaceIndex);
                pTrocarInfoElement->setMaterialName(materialName);
            }
        }
        else if (pElement->getName() == "KidneyCutOverlayElement/RightArrow")
        {
            if (++m_ToolPlaceIndex > 2)
                m_ToolPlaceIndex = 1;

            ChangeToolPlace(m_ToolPlaceIndex);

            Ogre::OverlayElement *pTrocarInfoElement = CSimpleUIManger::Instance()->GetElement("KidneyCutOverlayElement/TrocarInfo");
            if (pTrocarInfoElement)
            {
                Ogre::String materialName = "KidneyCut/Trocar_Config_" + Ogre::StringConverter::toString(m_ToolPlaceIndex);
                pTrocarInfoElement->setMaterialName(materialName);
            }
        }
    }
}
//======================================================================================================================

int CNephrectomy::ChangeToolPlace(int id)
{
    if (!(m_pTrainingConfig->m_DataForDeviceCandidates.empty()))
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
        default:
            return 0;
        }
    }
    return 0;
}