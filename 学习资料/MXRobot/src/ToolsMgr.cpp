/**Supplement:zx**/
#include "ToolsMgr.h"
#include "XMLWrapperTraining.h"
#include "XMLWrapperToolConfig.h"
#include "ResourceManager.h"
#include "InputSystem.h"
#include "KeyboardInput.h"
#include "HardwareInterface.h"
#include "MXOgreWrapper.h"
#include "Instruments/Tool.h"
#include "Inception.h"
#include "MisRobotInput.h"
#include "XMLWrapperHardwareConfig.h"
#include "XMLWrapperToolPlace.h"
#include "../Instruments/SuctionAndlrrigationTube.h"
//#include "NeedleHolder.h"
//#include "SurgicalSutures.h"
#include "stdafx.h"

#define USE_CONFIG_ORIENTATION 1


int CToolsMgr::s_nCount = 0;

CToolsMgr::CToolsMgr(void)
{
	map_NameTool.clear();
	m_pLeftTool = NULL;
	m_pRightTool = NULL;
	m_pTrainingConfig = NULL;
	m_pTraining = NULL;
    m_pHardwareConfig = NULL;
    m_pXMLContentManager = NULL;
	m_nLeftDeviceLabelID = -1;
	m_nRightDeviceLabelID = -1;
	m_strToolPlaceName = "Default";
	m_totalElectricTime = 0.f;
	m_totalValidElectricTime = 0.f;
	m_maxKeeppingElectricBeginTime = 0.f;
	m_maxKeeppingElectricTime = 0.f;
	m_toolSuctionTime = 0.f;
	m_toolIrrigationTime = 0.f;
	m_nReleasedTitanicClip = 0;
	m_leftToolClosedTimes = 0;
	m_rightToolClosedTimes = 0;
	m_leftToolMovedDistance = 0.f;
	m_rightToolMovedDistance = 0.f;
	m_leftToolMovedTime = 0.f;
	m_rightToolMovedTime = 0.f;
	m_electricAffectTimeForHemoClip = 0.f;
	m_electricAffectTimeForOrdinaryOrgan = 0.f;
	m_toolIsClosedInsertion = true;
	m_toolInAndOutTimes = 0;
	m_toolIsClosedInSeparteTime = true;
}
//==================================================================================
CToolsMgr::~CToolsMgr(void)
{
	if ( !map_NameTool.empty() )
	{
		RemoveAllCurrentTool();
		map_NameTool.clear();
	}
}
//======================================================================================
void CToolsMgr::OnCurrentTrainDeleted()
{
	if(m_pLeftTool)
	   m_pLeftTool->SetOwnerTraining(0);
	
	if(m_pRightTool)
	   m_pRightTool->SetOwnerTraining(0);

	for(std::map<Ogre::String,ITool*>::iterator itr = m_fixedTools.begin();itr != m_fixedTools.end();++itr)
	{
		itr->second->SetOwnerTraining(NULL);
	}
	
	m_pTraining = 0;
}
//======================================================================================
void CToolsMgr::AddTool( const Ogre::String& strToolName, ITool *pTool )
{
	map_NameTool[strToolName] = pTool;
}
//======================================================================================
void CToolsMgr::RemoveTool( const Ogre::String& strToolName)
{
	CTool* pTool = NULL;

	MAP_NAME_TOOL::iterator it = map_NameTool.find(strToolName);
	if (it != map_NameTool.end())
	{
		pTool = (CTool *)it->second;
		map_NameTool.erase(it);
	}
	else
	{
		std::map<Ogre::String,ITool*>::iterator itr = m_fixedTools.find(strToolName);
		if(itr != m_fixedTools.end())
		{
			pTool = (CTool*)itr->second;
			m_fixedTools.erase(itr);
		}
	}

	if(pTool)
	{
		StatisticToolData(pTool);

		CResourceManager::Instance()->RemoveToolInPool(pTool);

		if(m_pTraining)
			m_pTraining->OnToolRemoved(pTool);

		SAFE_DELETE(pTool);
	}
}
//======================================================================================
void CToolsMgr::StatisticToolData(ITool* tool)
{
	CTool* pTool = static_cast<CTool*>(tool);
	if (pTool)
	{
		ITool::ToolOwner toolOwner = pTool->GetToolOwner();
		//固定的器械将不再更新数据，因为即将被删除.保证固定后不在接受外部输入，如通电
		if(toolOwner == ITool::TO_None)
			return;

		m_totalElectricTime += pTool->GetTotalElectricTime();
		m_totalValidElectricTime += pTool->GetValidElectricTime();
		m_nReleasedTitanicClip += pTool->GetNumberOfReleasedTitanicClip();
		int closedTimes = pTool->GetToolClosedTimes();
		float movedDistance = pTool->GetMovedDistance();
		float movedTime = pTool->GetMovedTime();
		
		if(toolOwner == ITool::TO_LeftHand)
		{
			m_leftToolClosedTimes += closedTimes;
			m_leftToolMovedDistance += movedDistance;
			m_leftToolMovedTime += movedTime;
		}
		else if(toolOwner == ITool::TO_RightHand)
		{
			m_rightToolClosedTimes += closedTimes;
			m_rightToolMovedDistance += movedDistance;
			m_rightToolMovedTime += movedTime;
		}

		//update keepping electric time
		float keeppingElectrcTime = pTool->GetMaxKeeppingElectricTime();
		if(keeppingElectrcTime > m_maxKeeppingElectricTime)
		{
			m_maxKeeppingElectricTime = keeppingElectrcTime;
			m_maxKeeppingElectricBeginTime = pTool->GetMaxKeeppingElectricBeginTime();
		}

		m_electricAffectTimeForHemoClip += pTool->GetElectricAffectTimeForHemoClip();
		m_electricAffectTimeForOrdinaryOrgan += pTool->GetElectricAffectTimeForOrdinaryOrgan();

		if(m_toolIsClosedInsertion)
			m_toolIsClosedInsertion = pTool->IsClosedInsertion();

		if(pTool->GetType() == TT_SUCTION_AND_IRRIGATION_TUBE)
		{
			m_toolSuctionTime += pTool->GetLeftPadElectricTime();
			m_toolIrrigationTime += pTool->GetRightPadElectricTime();
			//移出冲吸器同时需要移出水柱效果
			static_cast<SuctionAndIrrigationTube*>(pTool)->RemoveWaterColumn();
		}

		++m_toolInAndOutTimes;

		if(m_toolIsClosedInSeparteTime)
			m_toolIsClosedInSeparteTime = pTool->IsClosedInSeparateTime();
	}
}
//======================================================================================
void CToolsMgr::Update(float dt)
{
	MAP_NAME_TOOL::iterator it = map_NameTool.begin();
	while (it != map_NameTool.end())
	{
		it->second->Update(dt);
		++it;
	}

	//更新固定的器械
	for(std::map<Ogre::String,ITool*>::iterator itr = m_fixedTools.begin();itr != m_fixedTools.end();++itr)
	{
		itr->second->Update(dt);
	}
}
//======================================================================================
bool CToolsMgr::Initialize(Ogre::SceneManager * pSceneManager, CXMLWrapperTraining * pTrainingConfig, ITraining * pTraining )
{
	m_pTrainingConfig = pTrainingConfig;
	m_pTraining  = pTraining;
	CResourceManager::Instance()->m_bChangeTool = false;

	vector<CXMLWrapperToolPlace *> & vtToolPlaces = m_pTrainingConfig->m_ToolPlaces;
	for (vector<CXMLWrapperToolPlace *>::iterator it = vtToolPlaces.begin(); it != vtToolPlaces.end(); it++)
	{
		if ((*it)->m_flag_Name && (*it)->m_Name == m_strToolPlaceName)
		{
			if ((*it)->m_flag_HardwareConfigXML)
			{
				m_pHardwareConfig = dynamic_cast<CXMLWrapperHardwareConfig *> (m_pXMLContentManager->Load((*it)->m_HardwareConfigXML));
				LoadHardwareConfig();
			}
		}
	}
	
	return true;
}
//======================================================================================
void CToolsMgr::Terminate()
{
   	CResourceManager::Instance()->m_bChangeTool = true;
}
//======================================================================================
ITool * CToolsMgr::SetLeftTool(ITool * pTool)
{
	if (pTool == m_pLeftTool) 
		return m_pLeftTool;

	if (m_pLeftTool)
		m_pLeftTool->NoUse();
	
	m_pLeftTool = pTool;
   
	pTool->SetToolOwner(ITool::TO_LeftHand);
	pTool->SetToolSide(ITool::TSD_LEFT);
	
	pTool->Use();

	vector<CXMLWrapperToolPlace *> & vtToolPlaces = pTool->m_pTrainingConfig->m_ToolPlaces;
	
	for (vector<CXMLWrapperToolPlace *>::iterator it = vtToolPlaces.begin(); it != vtToolPlaces.end(); it++)
	{
		if ((*it)->m_flag_Name && (*it)->m_Name == m_strToolPlaceName)
		{
			if ((*it)->m_flag_HardwareConfigXML)
			{
				m_pHardwareConfig = dynamic_cast<CXMLWrapperHardwareConfig *> (m_pXMLContentManager->Load((*it)->m_HardwareConfigXML));
				LoadHardwareConfig();
			}
			if ((*it)->m_flag_LeftToolPos)
			{
				pTool->GetKernelNode()->setPosition((*it)->m_LeftToolPos);
			}
			if ((*it)->m_flag_LeftToolOrientation)
			{
				pTool->GetKernelNode()->setOrientation((*it)->m_LeftToolOrientation);
			}

			//if ((*it)->m_flag_LeftToolFixAngle)
			//{
				//InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->SetExternalAngle((*it)->m_LeftToolFixAngle);
			//}
		}
	}

	InputSystem::GetInstance(DEVICETYPE_LEFT)->SetDefaultPosition(pTool->GetKernelNode()->getPosition());
	InputSystem::GetInstance(DEVICETYPE_LEFT)->SetDefaultOrientation(pTool->GetKernelNode()->getOrientation());


	InputSystem::GetInstance(DEVICETYPE_LEFT)->ResetInput();

	if ( TT_CLIP_APPLICATOR == pTool->GetType() )
	{
		InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->SetShaftAside( pTool->m_pToolConfig->m_MaxShaftAside );		//修复在真机上重置钛夹钳后有1个钛夹会自动掉落的bug		
		CResourceManager::Instance()->m_bChangeTool = true;
	}

	//银夹钳初始状态设置为半张开
	if ( TT_Y_CLIP_APPLIER == pTool->GetType() )
	{		
		InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->SetShaftAside(30);			
		CResourceManager::Instance()->m_bChangeTool = true;
	}

	return m_pLeftTool;
}
//===================================================================================================
ITool * CToolsMgr::SetRightTool(ITool * pTool)
{
	if (pTool == m_pRightTool) return m_pRightTool;
	if (m_pRightTool) m_pRightTool->NoUse();
	m_pRightTool = pTool;
	pTool->SetToolOwner(ITool::TO_RightHand);
    pTool->SetToolSide(ITool::TSD_RIGHT);//m_enmSide = ;
	pTool->Use();
	
	vector<CXMLWrapperToolPlace *> & vtToolPlaces = pTool->m_pTrainingConfig->m_ToolPlaces;
	for (vector<CXMLWrapperToolPlace *>::iterator it = vtToolPlaces.begin(); it != vtToolPlaces.end(); it++)
	{
		if ((*it)->m_flag_Name && (*it)->m_Name == m_strToolPlaceName)
		{
			if ((*it)->m_flag_HardwareConfigXML)
			{
				m_pHardwareConfig = dynamic_cast<CXMLWrapperHardwareConfig *> (m_pXMLContentManager->Load((*it)->m_HardwareConfigXML));
				LoadHardwareConfig();
			}
			if ((*it)->m_flag_RightToolPos)
			{
				pTool->GetKernelNode()->setPosition((*it)->m_RightToolPos);
			}
			if ((*it)->m_flag_RightToolOrientation)
			{
				pTool->GetKernelNode()->setOrientation((*it)->m_RightToolOrientation);
			}
			//if ((*it)->m_flag_RightToolFixAngle)
			//{
			//	InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->SetExternalAngle((*it)->m_RightToolFixAngle);
			//}
		}
	}

	InputSystem::GetInstance(DEVICETYPE_RIGHT)->SetDefaultPosition(pTool->GetKernelNode()->getPosition());
	InputSystem::GetInstance(DEVICETYPE_RIGHT)->SetDefaultOrientation(pTool->GetKernelNode()->getOrientation());

	InputSystem::GetInstance(DEVICETYPE_RIGHT)->ResetInput();

	if ( TT_CLIP_APPLICATOR == pTool->GetType() )
	{
		InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->SetShaftAside(pTool->m_pToolConfig->m_MaxShaftAside);			//修复在真机上重置钛夹钳后有1个钛夹会自动掉落的bug	
	}

	//银夹钳初始状态设置为半张开
	if ( TT_Y_CLIP_APPLIER == pTool->GetType() )
	{		
		InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->SetShaftAside(30);			
		CResourceManager::Instance()->m_bChangeTool = true;
	}

	return m_pRightTool;
}
//===================================================================================================
ITool * CToolsMgr::SwitchTool(bool bLeft, Ogre::String strToolType, Ogre::String strSubType)
{
	if (!m_pTrainingConfig)
		return NULL;
	ITool * pTool = NULL;
  
	// check if tool chosen is the same as tool in hand 
	ITool * pCurTool = bLeft?GetLeftTool():GetRightTool();
	
	if (pCurTool)
	{
		//if switch tool is the same return exist one
		if (pCurTool->IsInUse() && 
			pCurTool->m_pToolConfig->m_Type == strToolType &&
			pCurTool->m_pToolConfig->m_SubType == strSubType)
		{
			return pCurTool;
		}
		else//else delete old one 
		{
			RemoveTool(pCurTool->GetName());
			bLeft?(m_pLeftTool = NULL):(m_pRightTool=NULL);
			pCurTool = NULL;
		}
	}

	// different to tool in hand, try create a new one
	pTool = CreateTool(strToolType,false,strSubType,bLeft);

	return pTool;
}

ITool* CToolsMgr::ChooseOneFixedTool(const Ogre::String& toolName,bool isLeftHand)
{
	ITool* pTool = NULL;

	std::map<Ogre::String,ITool*>::iterator itr = m_fixedTools.find(toolName);
	if(itr != m_fixedTools.end())
	{
		pTool = itr->second;
		if(isLeftHand)
		{
			RemoveLeftCurrentTool();
			m_pLeftTool = CreateTool(pTool->m_pToolConfig->m_Type,
									false,
									pTool->m_pToolConfig->m_SubType,
									isLeftHand);

			RemoveTool(pTool->GetName());
			pTool = m_pLeftTool;
		}
		else
		{
			RemoveRightCurrentTool();
			m_pRightTool =CreateTool(pTool->m_pToolConfig->m_Type,
									false,
									pTool->m_pToolConfig->m_SubType,
									isLeftHand);

			RemoveTool(pTool->GetName());
			pTool = m_pRightTool;
		}

		m_fixedTools.erase(itr);
	}

	return pTool;
}

void CToolsMgr::RemoveAllFixedTools()
{
	for(std::map<Ogre::String,ITool*>::iterator itr = m_fixedTools.begin();itr != m_fixedTools.end();++itr)
	{
		ITool* pTool = itr->second;
		RemoveTool(pTool->GetName());
	}

	m_fixedTools.clear();
}


void CToolsMgr::AllFixedToolsRelease()
{
	for (std::map<Ogre::String, ITool*>::iterator itr = m_fixedTools.begin(); itr != m_fixedTools.end(); ++itr)
	{
		CTool* pTool = dynamic_cast<CTool*>(itr->second);

		pTool->ReleaseClampedOrgans();
		pTool->ReleaseHoldRigid();
		pTool->ReleaseClampedRope();
	}
}

void CToolsMgr::RemoveLeftFixedTool()
{
    for (std::map<Ogre::String, ITool*>::iterator itr = m_fixedTools.begin(); itr != m_fixedTools.end(); ++itr)
    {
        ITool* pTool = itr->second;
        RemoveTool(pTool->GetName());
    }

    m_fixedTools.clear();
}

void CToolsMgr::RemoveRightFixedTool()
{
    for (std::map<Ogre::String, ITool*>::iterator itr = m_fixedTools.begin(); itr != m_fixedTools.end(); ++itr)
    {
        ITool* pTool = itr->second;
        RemoveTool(pTool->GetName());
    }

    m_fixedTools.clear();
}

ITool* CToolsMgr::FixTool(bool isLeftHand)
{
	ITool* fixedTool = NULL;
	//固定左手器械
	if(isLeftHand)
	{
		if(m_pLeftTool)
		{
			fixedTool = m_pLeftTool;
			StatisticToolData(fixedTool);
			fixedTool->SetToolOwner(ITool::TO_None);		//后面不再统计该器械的相关数据
            m_fixedTools.insert(make_pair("L" + m_pLeftTool->GetName(), m_pLeftTool));
			m_pLeftTool = NULL;
		}
	}
	else
	{
		if(m_pRightTool)
		{
			fixedTool = m_pRightTool;
			StatisticToolData(fixedTool);
			fixedTool->SetToolOwner(ITool::TO_None);
			m_fixedTools.insert(make_pair("R" + m_pRightTool->GetName(),m_pRightTool));
			m_pRightTool = NULL;
		}
	}

	return fixedTool;
}

//===================================================================================================
void CToolsMgr::LoadHardwareConfig()
{
    HardwareConfig hardwareConfig;
	hardwareConfig.Use_ShaftLeft = m_pHardwareConfig->m_Use_ShaftLeft;
	hardwareConfig.Use_ShaftRight = m_pHardwareConfig->m_Use_ShaftRight;
	hardwareConfig.Use_CameraLarge = m_pHardwareConfig->m_Use_CameraLarge;
	//////////////////////////////////////////////////////////////////////////
    hardwareConfig.Use_MisRobot = m_pHardwareConfig->m_Use_MisRobot;
    hardwareConfig.Use_MisRobot_Spec = m_pHardwareConfig->m_Use_MisRobot_Spec;
    hardwareConfig.Use_MisRobot_Left = m_pHardwareConfig->m_Use_MisRobot_Left;
    hardwareConfig.Use_MisRobot_Right = m_pHardwareConfig->m_Use_MisRobot_Right;
	hardwareConfig.Use_Phantom_SelectTool = m_pHardwareConfig->m_Use_Phantom_SelectTool;
	hardwareConfig.Adjust_Parameter = m_pHardwareConfig->m_Adjust_Parameter;
	
	hardwareConfig.Left_Angle_K[0] = m_pHardwareConfig->m_Left_Angle_K.x;
	hardwareConfig.Left_Angle_K[1] = m_pHardwareConfig->m_Left_Angle_K.y;
	hardwareConfig.Left_Angle_K[2] = m_pHardwareConfig->m_Left_Angle_K.z;
	hardwareConfig.Left_Movement_K[0] = m_pHardwareConfig->m_Left_Movement_K.x;
	hardwareConfig.Left_Movement_K[1] = m_pHardwareConfig->m_Left_Movement_K.y;
	hardwareConfig.Left_Movement_K[2] = m_pHardwareConfig->m_Left_Movement_K.z;
    hardwareConfig.Left_Shaft_K = m_pHardwareConfig->m_Left_Shaft_K;
    hardwareConfig.Left_Shaft_B = m_pHardwareConfig->m_Left_Shaft_B;
	hardwareConfig.ClAppliper_Left_Shaft_K = m_pHardwareConfig->m_ClAppliper_Left_Shaft_K;
	hardwareConfig.ClAppliper_Left_Shaft_B = m_pHardwareConfig->m_ClAppliper_Left_Shaft_B;

	hardwareConfig.Right_Angle_K[0] = m_pHardwareConfig->m_Right_Angle_K.x;
	hardwareConfig.Right_Angle_K[1] = m_pHardwareConfig->m_Right_Angle_K.y;
	hardwareConfig.Right_Angle_K[2] = m_pHardwareConfig->m_Right_Angle_K.z;
	hardwareConfig.Right_Movement_K[0] = m_pHardwareConfig->m_Right_Movement_K.x;
	hardwareConfig.Right_Movement_K[1] = m_pHardwareConfig->m_Right_Movement_K.y;
	hardwareConfig.Right_Movement_K[2] = m_pHardwareConfig->m_Right_Movement_K.z;
    hardwareConfig.Right_Shaft_K = m_pHardwareConfig->m_Right_Shaft_K;
    hardwareConfig.Right_Shaft_B = m_pHardwareConfig->m_Right_Shaft_B;
	hardwareConfig.ClAppliper_Right_Shaft_K = m_pHardwareConfig->m_ClAppliper_Right_Shaft_K;
	hardwareConfig.ClAppliper_Right_Shaft_B = m_pHardwareConfig->m_ClAppliper_Right_Shaft_B;


	hardwareConfig.Camera_Angle_K[0] = m_pHardwareConfig->m_Camera_Angle_K.x;
	hardwareConfig.Camera_Angle_K[1] = m_pHardwareConfig->m_Camera_Angle_K.y;
	hardwareConfig.Camera_Angle_K[2] = m_pHardwareConfig->m_Camera_Angle_K.z;
	hardwareConfig.Camera_Movement_K[0] = m_pHardwareConfig->m_Camera_Movement_K.x;
	hardwareConfig.Camera_Movement_K[1] = m_pHardwareConfig->m_Camera_Movement_K.y;
	hardwareConfig.Camera_Movement_K[2] = m_pHardwareConfig->m_Camera_Movement_K.z;

    hardwareConfig.Force_X_K = m_pHardwareConfig->m_Force_X_K;
    hardwareConfig.Force_X_B = m_pHardwareConfig->m_Force_X_B;
    hardwareConfig.Force_Y_K = m_pHardwareConfig->m_Force_Y_K;
    hardwareConfig.Force_Y_B = m_pHardwareConfig->m_Force_Y_B;
    hardwareConfig.Force_Z_K = m_pHardwareConfig->m_Force_Z_K;
    hardwareConfig.Force_Z_B = m_pHardwareConfig->m_Force_Z_B;

	//hardwareConfig.Force_Angles_X = m_pHardwareConfig->m_Force_Angles_X;
	//hardwareConfig.Force_Angles_Y = m_pHardwareConfig->m_Force_Angles_Y;
	//hardwareConfig.Force_Angles_Z = m_pHardwareConfig->m_Force_Angles_Z;

	hardwareConfig.Force_Transport_Multiplier  = m_pHardwareConfig->m_Force_Transport_Multiplier;
	hardwareConfig.Force_Impulse_Multiplier  = m_pHardwareConfig->m_Force_Impulse_Multiplier;

    hardwareConfig.Switch_Distance = m_pHardwareConfig->m_Switch_Distance;
	hardwareConfig.Force_Release_Limit = m_pHardwareConfig->m_Force_Release_Limit;
    hardwareConfig.Force_Release_Limit_ElectricHook = m_pHardwareConfig->m_Force_Release_Limit_ElectricHook;
	hardwareConfig.Distance_Release_Limit = m_pHardwareConfig->m_Distance_Release_Limit;
    hardwareConfig.Distance_Release_Limit_ElectricHook = m_pHardwareConfig->m_Distance_Release_Limit_ElectricHook;

	hardwareConfig.Force_Radius = m_pHardwareConfig->m_Force_Radius;
	hardwareConfig.Force_Strength = m_pHardwareConfig->m_Force_Strength;

	hardwareConfig.Stiffness = m_pHardwareConfig->m_Stiffness;
	hardwareConfig.Clamp_Stiffness = m_pHardwareConfig->m_Clamp_Stiffness;

    HardwareInterface::SetHardwareConfig(hardwareConfig);

}
//===================================================================================================
ITool* CToolsMgr::CreateTool(const Ogre::String & strType, const bool bReuse, const Ogre::String & strSubType,bool bLeft)
{
	ITool* pTool = NULL;
	try
	{
		pTool = CResourceManager::Instance()->GetOneTool(strType, bReuse, strSubType);
	}
	catch (...)
	{
		SY_ASSERT(0 && "can't get tool");
	}

	if (!pTool) 
		return NULL;

	pTool->SetToolSide(bLeft ? ITool::TSD_LEFT : ITool::TSD_RIGHT);
	pTool->SetOwnerTraining(m_pTraining);

	pTool->Initialize(m_pTrainingConfig);

	if (bLeft)
	{
		//SetLeftToolDeviceLable(pTool);
		SetLeftTool(pTool);
	}
	else
	{
		//SetRightToolDeviceLable(pTool);
		SetRightTool(pTool);
	}

	pTool->GetOwnerTraining()->OnToolCreated(pTool , bLeft ? 0 : 1);

	AddTool(pTool->GetName(), pTool);

	// 	if (bLeft)
	// 	{
	// 		//SetLeftToolDeviceLable(pTool);
	// 		SetLeftTool(pTool);
	// 	}
	// 	else
	// 	{
	// 		//SetRightToolDeviceLable(pTool);
	// 		SetRightTool(pTool);
	// 	}

	pTool->SetBackupMaterial();

	ITraining * pTraining = pTool->GetOwnerTraining();

	//if (strToolType == "NeedleHolder" && pTraining->m_strName.find("SurgicalSutures") != string::npos)
	// {
	// ((CNeedleHolder *)pTool)->SetSuturesId(((CSurgicalSutures *)pTraining)->GetSuturesId());
	//((CNeedleHolder *)pTool)->SetNeedleId(((CSurgicalSutures *)pTraining)->GetNeedleId());
	// }

	return pTool;
}
//==============================================================================
void CToolsMgr::SetVisibleForAllTools(bool vis)
{
	if (m_pLeftTool)
		((CTool*)m_pLeftTool)->SetVisible(vis);

	if (m_pRightTool)
		((CTool*)m_pRightTool)->SetVisible(vis);

	for (std::map<Ogre::String, ITool*>::iterator itr = m_fixedTools.begin(); itr != m_fixedTools.end(); ++itr)
	{
		((CTool*)itr->second)->SetVisible(vis);
	}
}
//===================================================================================================
bool CToolsMgr::RemoveAllCurrentTool(bool includefixedtool)
{
	if (m_pLeftTool)
	{
		RemoveTool(m_pLeftTool->GetName());
		m_pLeftTool = NULL;
	}
	if (m_pRightTool)
	{
		RemoveTool(m_pRightTool->GetName());
		m_pRightTool = NULL;
	}

    if (includefixedtool)
    {
        for (std::map<Ogre::String, ITool*>::iterator itr = m_fixedTools.begin(); itr != m_fixedTools.end(); ++itr)
        {
            RemoveTool(itr->first);
        }
        m_fixedTools.clear();
    }
	

	return true;
}
//===================================================================================================
bool CToolsMgr::RemoveLeftCurrentTool()
{
	if (m_pLeftTool)
	{
		RemoveTool(m_pLeftTool->GetName());
		m_pLeftTool = NULL;
	}
	return true;
}
//===================================================================================================
bool CToolsMgr::RemoveRightCurrentTool()
{
	if (m_pRightTool)
	{
		RemoveTool(m_pRightTool->GetName());
		m_pRightTool = NULL;
	}
	return true;
}

float CToolsMgr::GetTotalElectricTime()
{
	float curToolElectricTime = 0.f;
	if(m_pLeftTool)
		curToolElectricTime += m_pLeftTool->GetTotalElectricTime();
	if(m_pRightTool)
		curToolElectricTime += m_pRightTool->GetTotalElectricTime();
	return curToolElectricTime + m_totalElectricTime;
}

float CToolsMgr::GetValidElectricTime()
{
	float curToolValidElectricTime = 0.f;
	if(m_pLeftTool)
		curToolValidElectricTime += m_pLeftTool->GetValidElectricTime();
	if(m_pRightTool)
		curToolValidElectricTime += m_pRightTool->GetValidElectricTime();
	return curToolValidElectricTime + m_totalValidElectricTime;
}

float CToolsMgr::GetMaxKeeppingElectricBeginTime()
{
	float beginTime = m_maxKeeppingElectricBeginTime;
	float maxTime = m_maxKeeppingElectricTime;

	if(m_pLeftTool)
	{
		float time = m_pLeftTool->GetMaxKeeppingElectricTime();
		if(time > maxTime)
		{
			beginTime = m_pLeftTool->GetMaxKeeppingElectricBeginTime();
			maxTime = time;
		}
	}

	if(m_pRightTool)
	{
		float time = m_pRightTool->GetMaxKeeppingElectricTime();
		if(time > maxTime)
		{
			beginTime = m_pRightTool->GetMaxKeeppingElectricBeginTime();
			maxTime = time;
		}
	}

	return beginTime;
}

float CToolsMgr::GetMaxKeeppingElectricTime()
{
	float maxTime = m_maxKeeppingElectricTime;

	if(m_pLeftTool)
	{
		float time = m_pLeftTool->GetMaxKeeppingElectricTime();
		if(time > maxTime)
			maxTime = time;
	}

	if(m_pRightTool)
	{
		float time = m_pRightTool->GetMaxKeeppingElectricTime();
		if(time > maxTime)
			maxTime = time;
	}

	return maxTime;
}

float CToolsMgr::GetToolSuctionTime()
{
	float curSuctionTime = 0.f;

	if(m_pLeftTool && m_pLeftTool->GetType() == TT_SUCTION_AND_IRRIGATION_TUBE)
		curSuctionTime += m_pLeftTool->GetLeftPadElectricTime();
	if(m_pRightTool && m_pRightTool->GetType() == TT_SUCTION_AND_IRRIGATION_TUBE)
		curSuctionTime += m_pRightTool->GetLeftPadElectricTime();

	return curSuctionTime + m_toolSuctionTime;
}

float CToolsMgr::GetToolIrrigationTime()
{
	float curIrrigationTime = 0.f;

	if(m_pLeftTool && m_pLeftTool->GetType() == TT_SUCTION_AND_IRRIGATION_TUBE)
		curIrrigationTime += m_pLeftTool->GetRightPadElectricTime();
	if(m_pRightTool && m_pRightTool->GetType() == TT_SUCTION_AND_IRRIGATION_TUBE)
		curIrrigationTime += m_pRightTool->GetRightPadElectricTime();

	return curIrrigationTime + m_toolIrrigationTime;
}


int CToolsMgr::GetNumberOfReleasedTitanicClip()
{
	int nCurReleasedClip = 0;
	if(m_pLeftTool)
		nCurReleasedClip = m_pLeftTool->GetNumberOfReleasedTitanicClip();
	if(m_pRightTool)
		nCurReleasedClip = m_pRightTool->GetNumberOfReleasedTitanicClip();

	return nCurReleasedClip + m_nReleasedTitanicClip;
}

int CToolsMgr::GetLeftToolClosedTimes()
{
	return m_leftToolClosedTimes + (m_pLeftTool ? m_pLeftTool->GetToolClosedTimes() : 0);
}

int CToolsMgr::GetRightToolClosedTimes()
{
	return m_rightToolClosedTimes + (m_pRightTool ? m_pRightTool->GetToolClosedTimes() : 0);
}

float CToolsMgr::GetLeftToolMovedDistance()
{
	return m_leftToolMovedDistance + (m_pLeftTool ? m_pLeftTool->GetMovedDistance() : 0.f);
}

float CToolsMgr::GetRightToolMovedDistance()
{
	return m_rightToolMovedDistance + (m_pRightTool ? m_pRightTool->GetMovedDistance() : 0.f);
}

float CToolsMgr::GetLeftToolMovedTime()
{
	float movedTime = m_leftToolMovedTime;
	if (m_pLeftTool)
		movedTime += m_pLeftTool->GetMovedTime();
	return movedTime;
}

float CToolsMgr::GetRightToolMovedTime()
{
	float movedTime = m_rightToolMovedTime;
	if (m_pRightTool)
		movedTime += m_pRightTool->GetMovedTime();
	return movedTime;
}

unsigned int CToolsMgr::GetLeftToolFastestSpeedTimes()
{
	return m_pLeftTool ? m_pLeftTool->GetMovedFastestTimes() : 0;
}

unsigned int CToolsMgr::GetRightToolFastestSpeedTimes()
{
	return m_pRightTool ? m_pRightTool->GetMovedFastestTimes() : 0;
}

float CToolsMgr::GetLeftToolMovedSpeed()
{
	float movedTime = m_leftToolMovedTime;
	float moveDistance = GetLeftToolMovedDistance();
	if(m_pLeftTool)
		movedTime += m_pLeftTool->GetMovedTime();
	if(movedTime > FLT_EPSILON)
		return moveDistance / movedTime;
	else
		return 0.f;
}

float CToolsMgr::GetRightToolMovedSpeed()
{
	float movedTime = m_rightToolMovedTime;
	float moveDistance = GetRightToolMovedDistance();
	if(m_pRightTool)
		movedTime += m_pRightTool->GetMovedTime();
	if(movedTime > FLT_EPSILON)
		return moveDistance / movedTime;
	else
		return 0.f;
}

float CToolsMgr::GetElectricTimeForHemoClip()
{
	float time = m_electricAffectTimeForHemoClip;

	if(m_pLeftTool)
		time += m_pLeftTool->GetElectricAffectTimeForHemoClip();
	if(m_pRightTool)
		time += m_pRightTool->GetElectricAffectTimeForHemoClip();

	return time;
}

float CToolsMgr::GetElectricTimeForOrdinaryOrgan()
{
	float time = m_electricAffectTimeForOrdinaryOrgan;

	if(m_pLeftTool)
		time += m_pLeftTool->GetElectricAffectTimeForOrdinaryOrgan();
	if(m_pRightTool)
		time += m_pRightTool->GetElectricAffectTimeForOrdinaryOrgan();

	return time;
}

bool CToolsMgr::ToolIsClosedInsertion()
{
	if(m_toolIsClosedInsertion && m_pLeftTool)
		m_toolIsClosedInsertion = m_pLeftTool->IsClosedInsertion();

	if(m_toolIsClosedInsertion && m_pRightTool)
		m_toolIsClosedInsertion = m_pRightTool->IsClosedInsertion();

	return m_toolIsClosedInsertion;
}

bool CToolsMgr::ToolIsClosedInSeparateTime()
{
	if(m_toolIsClosedInSeparteTime == false)
		return false;

	if(m_pLeftTool)
	{
		m_toolIsClosedInSeparteTime = m_pLeftTool->IsClosedInSeparateTime();
		if(m_toolIsClosedInSeparteTime == false)
			return false;
	}
	
	if(m_pRightTool)
	{
		m_toolIsClosedInSeparteTime = m_pRightTool->IsClosedInSeparateTime();
		if(m_toolIsClosedInSeparteTime == false)
			return false;
	}

	return true;
}