/**Author:zx**/
#include "stdafx.h"
#include "ResourceManager.h"
#include <QFile>
#include "RegClass.h"
#include "XMLSerialize.h"
#include "XMLWrapperToolConfig.h"
#include "XMLWrapperHardwareConfig.h"
#include "XMLWrapperGlobalTraining.h"
#include "XMLWrapperTraining.h"
#include "OgreSceneManager.h"
#include "MXOgreWrapper.h"

//tools
#include "Instruments/AirNeedle.h"
#include "Instruments/ElectricHook.h"
#include "Instruments/ElectricNeedle.h"
#include "Instruments/ElectricSpatula.h"
#include "Instruments/GraspingForceps.h"
#include "Instruments/HemoClip.h"
#include "Instruments/BigHemoClip.h"
#include "Instruments/Knotter.h"
#include "Instruments/Scissors.h"
#include "Instruments/DissectingForceps.h"
#include "Instruments/DissectingForcepsSize10.h"
#include "Instruments/Straw.h"
#include "Instruments/ClipApplier.h"
#include "Instruments/SilverClip.h"
#include "Instruments/BipolarElecForceps.h"
#include "Instruments/SilverClipHolder.h"
#include "Instruments/LinearCuttingStapler.h"
#include "Instruments/Cautery.h"
#include "Instruments/HarmonicScalpel.h"
#include "Instruments/LosslessForceps.h"
#include "Instruments/StraightMiniScissors.h"
#include "Instruments/CurvedScissors.h"
#include "Instruments/Nail.h"
#include "Instruments/HookClipPliers.h"
#include "Instruments/HookClip.h"
#include "Instruments/ThinMenbraneForceps.h"
#include "Instruments/BiopsyForceps.h"
#include "Instruments/Trielcon.h"
#include "Instruments/InjectNeedle.h"
#include "Instruments/SuctionAndlrrigationTube.h"
#include "Instruments/SpecimenBag.h"
#include "Instruments/RingForceps.h"
#include "Instruments/NeedleHolder.h"
#include "Instruments/GallForceps.h"
#include "Trochar.h"

// training
#include "ITraining.h"
#include "TrainingMgr.h"
#include "BasicTraining.h"

#include "SYCameraSkillA.h"
#include "SYCameraSkillB.h"


#include "PickupObjectTraining.h"
#include "SYCuttingTubeTrain.h"
#include "SYSealAndCut.h"
#include "BasicNewTraining_Level12.h"
#include "ElectrocoagulationTrain.h"
#include "RingTransferTraining.h"
#include "ACTitaniumClipTraining.h"
#include "ACEyeHandCoordTrain.h"
#include "ACPreciseNavigationTrain.h"
#include "SYElectrichookTrain.h"
#include "SYStopBleedTrain.h"
// hardware
#include "MisRobotInput.h"

// particles
#include "XMLWrapperParticles.h"
#include "XMLWrapperParticle.h"

#include "Inception.h"

#include "../NewTrain/GallNewTraining.h"
#include "AppendectomyTraining.h"
#include "../NewTrain/AcessoriesCutTrain.h"



#include "../NewTrain/BasicNewTraining_Level13.h"

#include "../NewTrain/AcessoriesCutTrainSub.h"
#include "../NewTrain/AccessorieCutTrain2.h"
#include "../NewTrain/AdhesionResectionTrain.h"
#include "../NewTrain/SutureTrain.h"
#include "../NewTrain/SutureKnotTrain.h"

#include "../NewTrain/LigatingLoopTrain.h"
#include "../NewTrain/KnotTestTraining.h"
#include "../NewTrain/Sigmoidectomy.h"

#include "../NewTrain/ThoracoscopyTrain.h"
#include "../NewTrain/Nephrectomy.h"
#include "../NewTrain/HysterectomyTraining.h"
#include "SutureTrainV2.h"
#include "SutureKnotTrainV2.h"
CResourceManager::CResourceManager()
: m_bOgreResourceLoaded(false), m_bClassRegistered(false), m_bXMLConfigsLoaded(false),
  m_pXMLContentManager(NULL), m_pGlobalTraining(NULL), m_pToolConfig(NULL), m_pParticlesConfig(NULL),
  m_pOms(NULL),m_controlMode(MULTIL_PORT_MODEL)
{

}

CResourceManager::~CResourceManager()
{

}

bool CResourceManager::LoadTraining( CTrainingMgr * pTrainingManager )
{
	return LoadTraining(Inception::Instance()->m_strTrainingName, m_pToolConfig, pTrainingManager);
}

bool CResourceManager::LoadResource(Ogre::vector<Ogre::String>::type & addResloacte)
{
	if (!m_bClassRegistered)
	{
		RegXMLClass();
		m_bClassRegistered = true;
	}

	if (/*!m_bXMLConfigsLoaded && */m_bClassRegistered)
	{
		if(LoadXMLConfigs())
		{
			m_bXMLConfigsLoaded = true;
		}
	}

	Ogre::String s = Inception::Instance()->m_strTrainingName;
	CXMLWrapperTraining * currTrainConfig = CResourceManager::Instance()->GetTrainingConfigByName(Inception::Instance()->m_strTrainingName);
	if(currTrainConfig && currTrainConfig->m_flag_AddOgreResLocation)
	   addResloacte = Ogre::StringUtil::split(currTrainConfig->m_AddOgreResLocation);


	if (m_bClassRegistered && m_bXMLConfigsLoaded) return true;

	return false;
}


bool CResourceManager::LoadTraining(const Ogre::String strTrainingName, CXMLWrapperToolConfig * pToolConfig, CTrainingMgr * pTrainingManager)
{
	bool bFound = false;
	if (!pTrainingManager) return false;

	ITraining * pTraining = pTrainingManager->GetTrainingByName(strTrainingName);
	if (!pTraining) // not loaded
	{
		if (m_pGlobalTraining && m_pGlobalTraining->m_flag_Trainings)
		{
			vector<CXMLWrapperTraining *> & vecTrainings = m_pGlobalTraining->m_Trainings;
			vector<CXMLWrapperTraining *>::iterator iter;
			for (iter = vecTrainings.begin(); iter != vecTrainings.end(); iter++)
			{
				CXMLWrapperTraining * pTrainingConfig = *iter;
				if (pTrainingConfig->m_Name == strTrainingName)
				{
					ITraining * pRealTraining = CreateTrainingByTypeAndName(pTrainingConfig->m_Type, pTrainingConfig->m_Name);
					
					pRealTraining->Initialize(pTrainingConfig, pToolConfig);
					
					pTrainingManager->AddTraining(strTrainingName, pRealTraining);
					bFound = true;
					break;
				}
			}
		}
	}
	return bFound;
}

CXMLWrapperTraining * CResourceManager::GetTrainingConfigByName(const Ogre::String & strTrainingName) const
{
	CXMLWrapperTraining *pTrainingConfig = NULL;

	if (m_pGlobalTraining)
	{
		vector<CXMLWrapperTraining *> & vecTrainings = m_pGlobalTraining->m_Trainings;
		vector<CXMLWrapperTraining *>::iterator iter;
		for (iter = vecTrainings.begin(); iter != vecTrainings.end(); iter++)
		{
			if ((*iter)->m_Name == strTrainingName)
			{
				pTrainingConfig = *iter;
				break;
			}
		}
	}

	return pTrainingConfig;
}

bool CResourceManager::LoadTraining(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig, CTrainingMgr * pTrainingManager)
{
	if (!pTrainingManager) return false;

	ITraining * pTraining = pTrainingManager->GetTrainingByName(pTrainingConfig->m_Name);
	if (!pTraining) // not loaded
	{
		ITraining * pRealTraining = CreateTrainingByTypeAndName(pTrainingConfig->m_Type, pTrainingConfig->m_Name);
		
		pRealTraining->Initialize(pTrainingConfig, pToolConfig);
		
		pTrainingManager->AddTraining(pTrainingConfig->m_Name, pRealTraining);
	}
	return true;
}
bool CResourceManager::LoadXMLConfigs()
{
	if (!m_pXMLContentManager)
	{
		m_pXMLContentManager = new CXMLContentManager();
	}

	Ogre::String  m_globalConfigFilePath;


	//switch (m_controlMode)
	//{
	//case SINGLE_PORT_MODEL:
		 // m_globalConfigFilePath = SINGLEPORT_GLOBAL_CONFIG_FILE;
		//break;
	//case MULTIL_PORT_MODEL:
		 m_globalConfigFilePath = MULTIPORT_GLOBAL_CONFIG_FILE;
		//break;
	//default:
			//break;
	//}
	// load global.xml
	if (m_pGlobalTraining)
	{
		SAFE_DELETE(m_pGlobalTraining);
	}
	m_pGlobalTraining = dynamic_cast<CXMLWrapperGlobalTraining *>( m_pXMLContentManager->Load(m_globalConfigFilePath));

	// load tool.xml
	if (m_pToolConfig)
	{
		SAFE_DELETE(m_pToolConfig);
	}
	m_pToolConfig =  dynamic_cast<CXMLWrapperToolConfig *> (m_pXMLContentManager->Load(TOOL_CONFIG_FILE));

    //load hardware.xml
    /*if (m_pHardwareConfig)
    {
        delete m_pHardwareConfig;
    }
    m_pHardwareConfig =  dynamic_cast<CXMLWrapperHardwareConfig *> (m_pXMLContentManager->Load(HARDWARE_CONFIG_FILE));
    LoadHardwareConfig();*/

	// load particles.xml
	if (m_pParticlesConfig)
	{
		SAFE_DELETE(m_pParticlesConfig);
	}
	m_pParticlesConfig = dynamic_cast<CXMLWrapperParticles *>(m_pXMLContentManager->Load(PARTICLES_CONIFG_FILE));

	if (!m_pGlobalTraining && !m_pToolConfig) return false;

	return true;
}

const Ogre::String CResourceManager::LoadDefaultTraining(CTrainingMgr * pTrainingManager)
{
	if (m_pGlobalTraining && m_pGlobalTraining->m_flag_Trainings)
	{
		vector<CXMLWrapperTraining *> & vecTrainings = m_pGlobalTraining->m_Trainings;
		vector<CXMLWrapperTraining *>::iterator iter;
		for (iter = vecTrainings.begin(); iter != vecTrainings.end(); iter++)
		{
			CXMLWrapperTraining * pTrainingConfig = *iter;
			if (pTrainingConfig->m_AutoLoad)
			{
				LoadTraining(pTrainingConfig, m_pToolConfig, pTrainingManager);
				return pTrainingConfig->m_Name;
			}
		}
	}
	
	return Ogre::String("");
}

ITool * CResourceManager::GetOneTool(const Ogre::String & strType, const bool bReuse, const Ogre::String & strSubType /*= ""*/)
{
	if (strType == "") return NULL;
	ITool * pTool = NULL;
	
	vector<CXMLWrapperTool *> & vecConfig = m_pToolConfig->m_Configs;
	vector<CXMLWrapperTool *>::iterator iterConfig;

	CXMLWrapperTool * pToolConfig = NULL;
	for (iterConfig = vecConfig.begin(); iterConfig != vecConfig.end(); iterConfig++)
	{
		pToolConfig = *iterConfig;
		if (strType == pToolConfig->m_Type && strSubType == pToolConfig->m_SubType)
		{
			// find right config, chech if there is one available
			if (bReuse)
			{
				std::vector<ITool *>::iterator iter;
				for (iter = m_vectToolsPool.begin(); iter != m_vectToolsPool.end(); iter++)
				{
					ITool * pToolInPool = *iter;
					CXMLWrapperTool * pToolInPoolConfig = pToolInPool->GetConfig();
					if (!pToolInPool->IsInUse() && !pToolInPool->IsDisable() && pToolInPoolConfig->m_Type == strType && pToolInPoolConfig->m_SubType == strSubType)
					{ // one not in use
						return pToolInPool;
					}
				}
			}

			// not available or specified as Reuse, create a new one
			break;
		}
	}

	if (!pToolConfig) return NULL; // bad type, no config matches

	OgreMax::OgreMaxScene::s_nLoadCount++;
	int nIndex = OgreMax::OgreMaxScene::s_nLoadCount;
	char szIndex[256];
	sprintf_s(szIndex, "%d", nIndex);

	Ogre::SceneNode * pTopNode = NULL;
	const Ogre::String & strSceneFilePath = pToolConfig->m_Path;
	const Ogre::String & strLeft = pToolConfig->m_Left   + "$" + szIndex;
	const Ogre::String & strRight = pToolConfig->m_Right + "$" + szIndex;
	const Ogre::String & strRoot = pToolConfig->m_Root   + "$" + szIndex;
	const Ogre::String & strLeft_a1 = pToolConfig->m_Left_a1  + "$" + szIndex;
	const Ogre::String & strRight_b1 = pToolConfig->m_Right_b1+ "$" + szIndex;
	const Ogre::String & strLeft_a2= pToolConfig->m_Left_a2  + "$" + szIndex;
	const Ogre::String & strRight_b2= pToolConfig->m_Right_b2 + "$" + szIndex;
	const Ogre::String & strCenter = pToolConfig->m_Center  + "$" + szIndex;

	if (!m_pOms)
	{
		m_pOms = new OgreMax::OgreMaxScene;
	}

	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();

	try 
	{
		m_pOms->Load(strSceneFilePath, MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_SMALL), 0, pSceneManager, pSceneManager->getRootSceneNode());	// load from scene
	}
	catch(...)
	{
		SY_ASSERT( 0 && "can't find tool scene");
	}
	
	pTool = CreateToolInstance(pToolConfig);	// create tool instance

	// set essential nodes in toll instance
	Ogre::SceneNode * pNodeKernel = (Ogre::SceneNode *)pSceneManager->getRootSceneNode()->getChild(strRoot);
	pTool->SetKernelNode(pNodeKernel);
	
	if (pToolConfig->m_Left != "")
	{		
		Ogre::SceneNode * pNodeLeft = (Ogre::SceneNode *)pSceneManager->getSceneNode(strLeft);
		pTool->SetLeftNode(pNodeLeft);
	}
	else
	{
		Ogre::SceneNode * pNodeLeft = pSceneManager->createSceneNode();
        pNodeKernel->addChild( pNodeLeft );
		pTool->SetLeftNode(pNodeLeft);
	}
	
	if (pToolConfig->m_Right != "")
	{
		Ogre::SceneNode * pNodeRight = (Ogre::SceneNode *)pSceneManager->getSceneNode(strRight);
		pTool->SetRightNode(pNodeRight); 
	}
	else
	{
		Ogre::SceneNode * pNodeRight = pSceneManager->createSceneNode();
        pNodeKernel->addChild( pNodeRight );
		pTool->SetRightNode(pNodeRight);
	}

	if (pToolConfig->m_Left_a1 !="")
	{
		Ogre::SceneNode * pNodeLeft_a1 = (Ogre::SceneNode *)pSceneManager->getSceneNode(strLeft_a1);
		pTool->SetLeft_a1Node(pNodeLeft_a1); 
	}
    
	if (pToolConfig->m_Left_a2 !="")
	{
		Ogre::SceneNode * pNodeLeft_a2 = (Ogre::SceneNode *)pSceneManager->getSceneNode(strLeft_a2);
		pTool->SetLeft_a2Node(pNodeLeft_a2); 
	}
    
	if (pToolConfig->m_Right_b1 != "")
	{
		Ogre::SceneNode * pNodeRight_b1 = (Ogre::SceneNode *)pSceneManager->getSceneNode(strRight_b1);
		pTool->SetRight_b1Node(pNodeRight_b1); 
	}
     
	if (pToolConfig->m_Right_b2 != "")
	{
		Ogre::SceneNode * pNodeRight_b2 = (Ogre::SceneNode *)pSceneManager->getSceneNode(strRight_b2);
		pTool->SetRight_b2Node(pNodeRight_b2); 
	}
     
	if (pToolConfig->m_Center != "")
	{
		Ogre::SceneNode * pNodeCenter = (Ogre::SceneNode *)pSceneManager->getSceneNode(strCenter);
		pTool->SetCenterNode(pNodeCenter); 
	}
 	m_pOms->Destroy();
	SAFE_DELETE(m_pOms);
	
	pTool->SetName(pToolConfig->m_Name + "_" + szIndex);
    pTool->SetType(pToolConfig->m_Type);
	pTool->SetSubType(pToolConfig->m_SubType);

	bool bElectric = false;
	if ( pToolConfig->m_CanFire || pToolConfig->m_CanCut )
	{
		bElectric = true;
	}
	pTool->SetElectricAttribute(bElectric);

	if(pToolConfig->m_flag_CanClosed)
		pTool->SetCanClosed(pToolConfig->m_CanClosed);

    // physics route
	Ogre::String strPhysicFilePrefix = "..\\media\\models\\tools\\" + Ogre::String( pToolConfig->m_Type );
	if ( pToolConfig->m_SubType != "" )
	{
		strPhysicFilePrefix += "_" + Ogre::String( pToolConfig->m_SubType );
	}
	strPhysicFilePrefix += "\\";

	m_vectToolsPool.push_back(pTool);
	
	return pTool;
}

ITool * CResourceManager::CreateToolInstance(CXMLWrapperTool * pToolConfig)
{
	ITool * pTool = NULL;
	if (pToolConfig->m_Type == TT_AIRNEEDLE)
	{
		pTool = new CAirNeedle(pToolConfig);
		//pTool->m_nDeviceLabel = 1;
	}
	else if (pToolConfig->m_Type == TT_T_HOOK_ELECTRODE || pToolConfig->m_Type == TT_ELECTRIC_HOOK)
	{
		pTool = new CElectricHook(pToolConfig);
		//pTool->m_nDeviceLabel = 2;
	}
	else if(pToolConfig->m_Type == TT_T_NEEDLE_ELECTRODE)
	{
		pTool = new CElectricNeedle(pToolConfig);
	}
	else if (pToolConfig->m_Type == TT_ELECTRIC_SPATULA)
	{
		pTool = new CElectricSpatula(pToolConfig);
		//pTool->m_nDeviceLabel = 3;
	}
	else if (pToolConfig->m_Type == TT_GRASPING_FORCEPS)
	{
		pTool = new CGraspingForceps(pToolConfig);
		//pTool->m_nDeviceLabel = 4;
	}
	else if (pToolConfig->m_Type == TT_GALL_FORCEPS)
	{
		pTool = new CGallForceps(pToolConfig);
	}
	else if (pToolConfig->m_Type == TT_RINGFORCEPS)
	{
		pTool = new CRingForceps(pToolConfig);
		//pTool->m_nDeviceLabel = 4;
	}
	
	else if (pToolConfig->m_Type == TT_DISSECTING_FORCEPS && pToolConfig->m_SubType == "Size10") //分离钳
	{
		pTool = new CDissectingForcepsSize10(pToolConfig);
	}
	else if (pToolConfig->m_Type == TT_DISSECTING_FORCEPS)
	{
		pTool = new CDissectingForceps(pToolConfig);
	}
	//pTool->m_nDeviceLabel = 5;

	//else if (pToolConfig->m_Type == TT_DISSECTING_FORCEPS_SIZE10) 
	//{
		
		//pTool->m_nDeviceLabel = 5;
	//}
	else if (pToolConfig->m_Type == TT_HEMOCLIP)
	{
		pTool = new CHemoClip(pToolConfig);
		//pTool->m_nDeviceLabel = 6;
	}
	else if (pToolConfig->m_Type == TT_BIGHEMOCLIP)
	{
		pTool = new CBigHemoClip(pToolConfig);
	}
	else if (pToolConfig->m_Type == TT_NAIL)
	{
		pTool = new CNail(pToolConfig);
		//pTool->m_nDeviceLabel = 6;
	}
	else if (pToolConfig->m_Type == TT_KNOTTER)
	{
		pTool = new CKnotter(pToolConfig);
		//pTool->m_nDeviceLabel = 7;
	}
	else if (pToolConfig->m_Type == TT_T_STRAIGHT_SCISSORS || pToolConfig->m_Type == TT_SCISSORS)
	{
		pTool = new CStraightScissors(pToolConfig);
		//pTool->m_nDeviceLabel = 8;
	}
	else if (pToolConfig->m_Type == TT_CURVEDSCISSORS)
	{
		pTool = new CCurvedScissors(pToolConfig);
	}
	else if (pToolConfig->m_Type == TT_STRAW)
	{
		pTool = new CStraw(pToolConfig);
		//pTool->m_nDeviceLabel = 9;
	}
	else if (pToolConfig->m_Type == TT_TRIELCON)
	{
		pTool = new CTrielcon(pToolConfig);
		//pTool->m_nDeviceLabel = 10;
	}
	else if (pToolConfig->m_Type == TT_TROCHAR)
	{
		pTool = new CTrochar(pToolConfig);
		//pTool->m_nDeviceLabel = 11;
	}
	else if (pToolConfig->m_Type == TT_CLIP_APPLICATOR || pToolConfig->m_Type == TT_CLIPAPPLIER)
	{
		pTool = new CClipApplier(pToolConfig);
		//pTool->m_nDeviceLabel = 12;
	}
	else if (pToolConfig->m_Type == TT_NEEDLEHOLDER)//持针器
	{
		pTool = new CNeedleHolder(pToolConfig);
		//pTool->m_nDeviceLabel = 13;
	}
	else if (pToolConfig->m_Type == TT_SILVERCLIP)//银夹
	{
		pTool = new CSliverClip(pToolConfig);
		//pTool->m_nDeviceLabel = 14;
	}
	else if (pToolConfig->m_Type == TT_BIPOLARELECFORCEPS) //双极电凝钳
	{
		pTool = new CBipolarElecForceps(pToolConfig);
		//pTool->m_nDeviceLabel = 15;
	}
	else if (pToolConfig->m_Type == TT_Y_CLIP_APPLIER || pToolConfig->m_Type == TT_SILVERCLIPHOLDER) //银夹 + 持夹器
	{
		pTool = new CSilverClipHolder(pToolConfig);
		//pTool->m_nDeviceLabel = 16;
	}
	else if (pToolConfig->m_Type == TT_LINEARCUTTINGSTAPLER) //直线切割吻合器
	{
		pTool = new CLinearCuttingStapler(pToolConfig);
		//pTool->m_nDeviceLabel = 17;
	}
	else if (pToolConfig->m_Type == TT_OBLIQUE_EVEN_MOUTH_BIPOLAR || pToolConfig->m_Type == TT_CAUTERY) //电凝刀
	{
		pTool = new CCautery(pToolConfig);
		//pTool->m_nDeviceLabel = 18;
	}
	else if (pToolConfig->m_Type == TT_ULTRASONIC_SCALPE || pToolConfig->m_Type == TT_HARMONICSCALPEL) //超声刀
	{
		pTool = new CHarmonicScalpel(pToolConfig);
		//pTool->m_nDeviceLabel = 19;
	}
	else if (pToolConfig->m_Type == TT_BIOPSYFORCEPS) //活检钳
	{
		pTool = new CBiopsyForceps(pToolConfig);
		//pTool->m_nDeviceLabel = 19;
	}
	else if (pToolConfig->m_Type == TT_LOSSLESSFORCEPS) //无损抓钳
	{
		pTool = new CLosslessForceps(pToolConfig);
		//pTool->m_nDeviceLabel = 20;
	}

	else if (pToolConfig->m_Type==TT_T_DOUBLE_HOOK_SCISSORS || pToolConfig->m_Type == TT_DOUBLEHOOKSCISSORS)//钩剪
	{
		pTool= 0;
	}
	else if (pToolConfig->m_Type==TT_T_STRAIGHT_MINI_SCISSORS || pToolConfig->m_Type == TT_STRAIGHTMINISCISSORS)//迷你直剪
	{
		pTool= new CStrightMiniScissors(pToolConfig);
	}
	else if ( pToolConfig->m_Type == TT_HEM_O_LOK_PLIERS || pToolConfig->m_Type == TT_HOOKCLIPPLIERS)  //HOOK施夹器
	{
		pTool= new CHookClipPliers(pToolConfig);
	}
	else if (pToolConfig->m_Type == TT_HOOKCLIP)  //HOOK夹子
	{
		pTool= new CHookClip(pToolConfig);
	}
	else if (pToolConfig->m_Type ==TT_THIN_MENBRANE_DISSENCTOR || pToolConfig->m_Type == TT_THINMENBRANEFORCEPS) //系膜剥离钳
	{
		pTool= new CThinMenbraneForceps(pToolConfig);
	}
	else if (pToolConfig->m_Type == TT_INJECT_NEEDLE) //注射器
	{
		pTool= new CInjectNeedle(pToolConfig);
	}
	else if (pToolConfig->m_Type == TT_SUCTION_AND_IRRIGATION_TUBE) //冲吸器
	{
		pTool= new SuctionAndIrrigationTube(pToolConfig);
	}
	else if(pToolConfig->m_Type == TT_SPECIMENBAG)//标本袋
	{
		pTool = new SpecimenBag(pToolConfig);
	}

	return pTool;
}

void CResourceManager::RegisterTraining(const std::string& trainingName,ITraining* (*generator)())
{
	SY_ASSERT(m_trainingGeneratorMap.find(trainingName) == m_trainingGeneratorMap.end() && (std::string("already register the training :") + trainingName).c_str() );
	m_trainingGeneratorMap.insert(make_pair(trainingName,generator));
}

/**to do**/
ITraining * CResourceManager::CreateTrainingByTypeAndName(const Ogre::String & strType, const Ogre::String & strName)
{
	//优先检查是否已经注册过训练相关的信息
	std::map<std::string,REGISTER>::iterator itr = m_trainingGeneratorMap.find(strType + "-" + strName);
	if(itr != m_trainingGeneratorMap.end())
	{
		return itr->second();
	}


	if (strType == "Basic")
	{

	}
	else if (strType == "NewGallTrain")     //胆囊手术
	{
		return new CNewGallTraining();		
	}
	else if (strType == "Thoracoscope")
	{
        return new CThoracoscopyTrain();	
	}
	else if (strType == "NewBasicTraining")
	{
		if (strName == "CamSkillA")
		{
			return new SYCameraSkillA(MXOgreWrapper::CS_DEGREE_0);
		}
		else if (strName == "CamSkillB")
	    {
		    return new SYCameraSkillB(MXOgreWrapper::CS_DEGREE_0);
	    }
	    else if(strName == "PickTrainLevel1")//bacon add
	    {
		    return new CPickupObjectTraining();
	    }
		else if(strName == "TitaniumClipTraining"){
			return new ACTitaniumClipTraining();
		}
		else if (strName == "Cutting")
		{
			return new SYCuttingTubeTrain();
		}
		else if (strName == "SealAndCut")
		{
			return new SYSealAndCut();
		}
		else if (strName == "FineDissection")
		{
			return new CBasicNewTraining_Level12();
		}
		else if (strName == "Electro_Coagulation_Train")
		{
			return new CElectroCoagulationTrain();
		}
		
		else if (strName == "RingTransfer")
		{
			return new CRingTransferTrain();
		}

		else if (strName == "EyeHandCoord")
		{
			return new ACEyeHandCoordTrain();
		}

		else if (strName == "PreciseNavigation")
		{
			return new ACPreciseNavigationTrain();
		}

		else if (strName == "SYElectricHookTrain")
		{
			return new SYElectricHookTrain();
		}

		else if (strName == "CNeedleTestTrainV2")
		{
			return new CSutureTrainV2();
		}

		//below is deprecatted train delete it !!
		else if (strName == "StopBleedTrain")
		{
			return new SYStopBleedTrain();
		}
		
		
		else if(strName == "NBT_Training_13")
		{
			return new CBasicNewTraining_Level13();
		}
		
		else if (strName == "CNeedleTestTrain")
		{
			return new CSutureTrain();
		}
		else if (strName == "CKnotTestTrain")
		{
			return new CSutureKnotTrain();
		}
        else if (strName == "CKnotTestTrain1")
        {
            return new CSutureKnotTrain1();
        }
        else if (strName == "CKnotTestTrain2")
        {
            return new CSutureKnotTrain2();
        }
        else if (strName == "CKnotTestTrain3")
        {
            return new CSutureKnotTrain3();
		}
		else if (strName == "CKnotTestTrain4")
		{
			return new CSutureKnotV2Train1();
		}
		else if (strName == "CKnotTestTrain5")
		{
			return new CSutureKnotV2Train2();
		}
		else if (strName == "CKnotTestTrain6")
		{
			return new CSutureKnotV2Train3();
		}
		
		else if (strName == "LigatingLoop")
		{
			return new CLigatingLoopTrain();
		}
	}
	else if (strType == "NewAccCutTrain")     //胆囊手术
	{
		//if (strName == "JueyuTrain")//电凝绝育
		//{
			//return new CAcessoriesCutTraining_2nd(strName);		
		//}
		//else if (strName == "SilverClipTrain")//银夹绝育
		//{
		//	return new CAcessoriesCutTraining_3rd(strName);		
		//}
		//else 
		if (strName == "Salpingectomy")//左侧输卵管切除
		{
			return new CAcessoriesCutTraining_4th(strName);
		}
		else if (strName == "oophorectomy")//附件切除
		{
			return new CAccessorieCutTrain2(strName);
		}

		else if (strName == "TubalEmbryo"||strName == "EctopicPregnancy")//取胚
		{
			return new CAcessoriesCutTraining_5th(strName);
		}

		else if (strName == "adhesiondecompose")//粘连分解
		{
			return new CAccessorieCutTrain2(strName);
		}
	}
	else if (strType == "Hysterectomy")
	{
		return new HysterectomyTraining(strName);
	}
	else if (strType == "Sigmoidectomy")  //乙状结肠切除
	{
        if (strName == "Sigmoidectomy1" )
        {
            return new CSigmoidectomy(strName);		
        }		
	}
    else if (strType == "Nephrectomy")  //肾切除
    {
        if (strName == "Nephrectomy1")
        {
            return new CNephrectomy(strName);
        }
        if (strName == "Nephrectomy2")
        {
            return new CNephrectomy(strName);
        }
    }
	else if (strType == "Appendectomy")
	{	
        //if (strName == "AdultAppendectomyB")
       // {
         //   return new CAppendectomyTraining(strName);
        //}
       // if (strName == "ChronicAppendectomy")
       // {
            return new CAppendectomyTraining(strName);            
       // }		
	}
	return new MisNewTraining();
}

void CResourceManager::RemoveToolInPool(ITool * pTool)
{
	std::vector<ITool *>::iterator iter;
	for (iter = m_vectToolsPool.begin(); iter != m_vectToolsPool.end(); ++iter)
	{
		ITool * pPoolTool = *iter;
		if (pPoolTool == pTool)
		{
			m_vectToolsPool.erase(iter);
			// dont release, it's done by toolsMgr
			return;
		}
	}
}

CXMLWrapperParticle * CResourceManager::GetParticleConfigByNameAndType(const Ogre::String & strParticleName, const Ogre::String & strParticleType)
{
	if (!m_pParticlesConfig)
	{
		return NULL;
	}

	int nParticleConfigsCount = m_pParticlesConfig->m_Particles.size();
	for (int i = 0; i < nParticleConfigsCount; i++)
	{
		CXMLWrapperParticle * pParticle = m_pParticlesConfig->m_Particles[i];
		if (pParticle)
		{
			if (pParticle->m_Name == strParticleName && pParticle->m_Type == strParticleType)
			{
				return pParticle;
			}
		}
	}
	return NULL;
}