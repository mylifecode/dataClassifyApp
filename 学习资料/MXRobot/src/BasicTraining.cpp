#include "stdafx.h"
#include "BasicTraining.h"
#include "MXOgreWrapper.h"
#include "XMLWrapperStaticScene.h"
#include "XMLWrapperSceneNode.h"
#include "XMLWrapperToolConfig.h"
#include "XMLWrapperTraining.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperConnect.h"
#include "XMLWrapperAdhere.h"
#include "XMLWrapperAdhesion.h"
#include "XMLWrapperCollision.h"
#include "XMLWrapperPart.h"
#include "XMLWrapperPursue.h"
#include "XMLWrapperMucous.h"
#include "XMLWrapperMovie.h"
#include "XMLWrapperLight.h"
#include "XMLWrapperShadow.h"
#include "ToolsMgr.h"
#include "OgreHardwareBuffer.h"
#include "OgreSubMesh.h"
#include "InputSystem.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreMaterialManager.h"
#include "Inception.h"
#include "MXEventsDump.h"
#include "MXEvent.h"
#include "EffectManager.h"
#include "ScreenEffect.h"
#include "ResourceManager.h"

#include "OgreLight.h"
#include "OgreCompositor.h"
#include "OgreCompositorManager.h"
#include "HelperLogics.h"
#include "OgreMaxScene.hpp"

#include "LightMgr.h"
#include <QDir>
#include <QTextStream>

#include "PerformanceDebug.h"
#include "PathputTool.h"

#include "ShadowMap.h"
#include "EngineCore.h"
#include "Instruments/Tool.h"

#include"../EditorTool/HotchpotchEditor.h"

#include "NullSchemeHandler.h"

#include "XMLWrapperToolPlace.h"
#include "MXGlobalConfig.h" //bacon add
#include "SYTrainingReport.h"
#include "SimpleUI.h"
#include "DeferredRendFrameWork.h"

#include <QKeyEvent>


#define TOTALTIME 10000 //10s

extern const int GRAVITY = 20;

CBasicTraining::CBasicTraining(void)
{
	m_pOms= new OgreMax::OgreMaxScene;
	m_pToolsMgr = new CToolsMgr;

	//m_pSmallCamera = NULL;
	m_pLargeCamera = NULL;


	m_strConfigPath = "";
	m_nSceneCount = 0;
	m_eCS = MXOgreWrapper::CS_DEGREE_0;
	m_bStaticCamera = false;
	m_nDefaultCS = 0;

	m_bIsContinue=false;
	m_bTimeOut=false;
	m_bIsTerminating=false;
	m_bTrainingRunning = false;
	m_bTrainingIlluminated = false;

	OgreMax::OgreMaxScene::s_nLoadCount=0;
	m_viewport=NULL;
	//  [4/16/2012 yl]
	m_pRdWindow=NULL;
	m_fToolDistance=30;

	m_vStartLeftPosition=Ogre::Vector3(0.0,0.0,0.0);
	m_vStartRightPosition=m_vStartLeftPosition;
	m_dwStartTime=0;

//	m_pMovieTexture = NULL;
	m_pVideoOverlay = NULL;
	m_pOverlayContainer = NULL;
	m_bPlaying = false;

	for (int i = 0; i < EDOT_ORGAN_LIMIT; i++)
	{
		m_bCutMark[i] = false;
	}

	m_vecLeftToolPos=Ogre::Vector3::ZERO;
	m_vecRightToolPos=Ogre::Vector3::ZERO;

	m_dwLeftStartTime=0;
	m_dwRightStartTime=0;

	m_bLeftFirstTip=false;
	m_bLeftFirstTip=false;

	m_bLeftFirstTip=false;
	m_bRightFirstTip=false;

	m_lastmilliseconds = 0;

	m_pathtool = 0;
	//m_veineditor = 0;
	//m_veineditor_v2 = 0;
	m_pHotchpotchEditor = NULL;

	//m_pDynStrips = NULL;

	m_pManualObjectForDebugLeft = NULL;
	m_pSceneNodeForDebugLeft = NULL;
	m_pManualObjectForDebugRight = NULL;
	m_pSceneNodeForDebugRight = NULL;

	m_bVideoRecording = MxGlobalConfig::Instance()->EnabledRecord();//bacon add
	m_fLastTime = -1;
	m_IsSeriousFault = false;
	m_SeriousFaultID = -1;

	//////////////////////////////////////////////////////////////////////////
	// test code
//	m_Debug = false;
//	m_TextBasic = NULL;
	//m_Testboundbox1 = NULL;
	//m_Testboundbox2 = NULL;
	//m_Testboundbox3 = NULL;
	m_Testboundbox4 = NULL;

#ifdef USE_SSAO
	m_LogicInstance = NULL;
#endif
	m_IsInEditMode = false;
	CMXEventsDump::Instance()->RegisterHandleEventsFunc(this);
}

CBasicTraining::~CBasicTraining(void)
{
	if(m_pToolsMgr)
	   m_pToolsMgr->OnCurrentTrainDeleted();
	SAFE_DELETE(m_pToolsMgr);

	Terminate();
	SAFE_DELETE(m_pOms);

	SAFE_DELETE(m_pathtool);
	//SAFE_DELETE(m_veineditor);
	//SAFE_DELETE(m_veineditor_v2);
	SAFE_DELETE(m_pHotchpotchEditor);

	// test code
//	m_Debug = false;
	//m_TextBasic = NULL;
	//m_Testboundbox1 = NULL;
	//m_Testboundbox2 = NULL;
	//m_Testboundbox3 = NULL;
	//m_Testboundbox4 = NULL;



	//Ogre::CompositorManager::getSingleton().removeAll();	
	////unload
	//Ogre::CompositorManager::getSingleton().unregisterCompositorLogic("SSAOLogic");

	//SAFE_DELETE(m_LogicInstance);
#ifndef USE_SSAO
	//SAFE_DELETE(m_listenerDrawDepth);
#else
	SAFE_DELETE(m_listenerGbuffer);
	SAFE_DELETE(m_listenerNoGbuffer);
#endif
	CMXEventsDump::Instance()->UnRegisterHandleEventsFunc();
}

void CBasicTraining::LoadFSM(CXMLWrapperTraining * pTrainingConfig)
{
	
}

bool CBasicTraining::Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig)
{
	m_pTrainingConfig=pTrainingConfig;

	Ogre::SceneManager * pSMG =  MXOgre_SCENEMANAGER;

	m_pToolsMgr->Initialize(pSMG, pTrainingConfig, this);

	//set global shaft
	HardwareInterface::m_GlobalLeftShaftB = MxGlobalConfig::Instance()->GetLeftShaftB();
	HardwareInterface::m_GlobalLeftShaftK = MxGlobalConfig::Instance()->GetLeftShaftK();

	HardwareInterface::m_GlobalRightShaftB = MxGlobalConfig::Instance()->GetRightShaftB();
	HardwareInterface::m_GlobalRightShaftK = MxGlobalConfig::Instance()->GetRightShaftK();

	//
// 	SetSceneMgr(MXOgreWrapper::Get()->GetDefaultSceneManger());
	LoadConfig(pTrainingConfig);
	m_strSkinModelName = pTrainingConfig->m_SkinModelName;

	char szAppName[MAX_PATH];
	int len = GetModuleFileNameA(NULL, szAppName, sizeof(szAppName));

	Ogre::String strRoot(szAppName);

	int nIndex = strRoot.find_last_of('\\');
	strRoot = strRoot.substr(0, nIndex);
	nIndex = strRoot.find_last_of('\\');
	strRoot = strRoot.substr(0, nIndex);

	int modeType = CResourceManager::Instance()->GetControlModeType();//

	if (modeType == SINGLE_PORT_MODEL)
	{
		m_strConfigPath = strRoot + "\\config\\SinglePortConfig\\";
	}else if(modeType == MULTIL_PORT_MODEL)
	{
		m_strConfigPath = strRoot + "\\config\\Train\\";
	}
	else
	{
		m_strConfigPath = strRoot + "\\config\\";
	}

	if (pTrainingConfig->m_SupportFSM)
	{
		LoadFSM(pTrainingConfig);
	}

    m_strName = pTrainingConfig->m_Name;
    m_strTrainingType = pTrainingConfig->m_Type;
	m_bStaticCamera = pTrainingConfig->m_StaticCamera;
	m_strMediaSource = pTrainingConfig->m_MoviePath;
	m_usenewblood = pTrainingConfig->m_UseNewBlood;
	CScoreMgr::Instance()->SetTrainingTime(Inception::Instance()->m_remainTime);

	CMXEventsDump::Instance()->ClearEvent();

	TrainingIntialized();

	/*m_pDynStrips = new DynamicStripsObjectEx( "MucousExEx" );
	for ( size_t i = 0; i < pTrainingConfig->m_vecMucousObject.size(); i++ )
	{
		//Ogre::String sz = pTrainingConfig->m_vecMucousObject[ i ]->m_Name;
		float fMax = pTrainingConfig->m_vecMucousObject[ i ]->m_MaxLength;
		MucousSlot::setSlotMaxRadius( fMax );
		break;
	}
	m_pDynStrips->setVisible( false );
	m_pDynStrips->get_setName() = "MyStrips" ;
	m_pSceneMgr->getRootSceneNode()->attachObject(m_pDynStrips);*/
	
	if(pTrainingConfig->m_Lights.size() > 0)
	{
		Ogre::ColourValue ambientColor = pTrainingConfig->m_Lights[0]->m_ColorAmbient;
		pSMG->setAmbientLight(ambientColor);//reset there is a bug the organ 's scene will set envoriment ambient
	}
	else
	{
		pSMG->setAmbientLight(Ogre::ColourValue(0.54f,0.54f,0.54f));
	}
	OgreWidget *  widget = MXOgreWrapper::Instance()->GetOgreWidgetByName(RENDER_WINDOW_LARGE);
		
	//widget->m_windoweventlistener = this;

	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceLeft)
	{
		m_pManualObjectForDebugLeft = pSMG->createManualObject();
		m_pSceneNodeForDebugLeft = pSMG->getRootSceneNode()->createChildSceneNode();
		m_pSceneNodeForDebugLeft->attachObject(m_pManualObjectForDebugLeft);
	}

	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceRight)
	{
		m_pManualObjectForDebugRight = pSMG->createManualObject();
		m_pSceneNodeForDebugRight = pSMG->getRootSceneNode()->createChildSceneNode();
		m_pSceneNodeForDebugRight->attachObject(m_pManualObjectForDebugRight);
	}

	MapDeviceWorkspaceModel();
	//
	//create force feed back debug draw
	m_ForceFeedBackDebugDraw = pSMG->createManualObject("force_fb");
	m_ForceFeedBackDebugDraw->setDynamic(true);
	Ogre::SceneNode * scenenode = pSMG->getRootSceneNode()->createChildSceneNode("force_fb");
	if(m_ForceFeedBackDebugDraw)
		scenenode->attachObject(m_ForceFeedBackDebugDraw);

	//path tool for vein connect
	//m_pathtool = new PathPutTool();
	//m_pathtool->Construct(m_pOms->GetSceneManager() , this);
	
    //m_veineditor = new VeinConnEditor();
    //m_veineditor->Construct(m_pOms->GetSceneManager() , this);

	//m_veineditor_v2 = new VeinConnEditorV2();
	//m_veineditor_v2->Construct(m_pOms->GetSceneManager() , this);

	//////////////////////////////////////////////////////////////////////////
	/*if(m_Debug)
	{
		m_Testboundbox1 = MXOgre_SCENEMANAGER->getRootSceneNode()->createChildSceneNode("m_Testboundbox1");
		Ogre::Entity* box4 =  MXOgre_SCENEMANAGER->createEntity("m_Testboundbox1Entity",Ogre::SceneManager::PT_CUBE);
		m_Testboundbox1->attachObject(box4);
		m_Testboundbox1->setScale(0.01, 0.01,0.01);
		m_Testboundbox1->setVisible(true);
		m_Testboundbox1->showBoundingBox(true);

		m_Testboundbox2 = MXOgre_SCENEMANAGER->getRootSceneNode()->createChildSceneNode("m_Testboundbox2");
		Ogre::Entity* box5 =  MXOgre_SCENEMANAGER->createEntity("m_Testboundbox2Entity", Ogre::SceneManager::PT_CUBE);
		m_Testboundbox2->attachObject(box5);
		m_Testboundbox2->setScale(0.01, 0.01,0.01);
		m_Testboundbox2->setVisible(true);
		m_Testboundbox2->showBoundingBox(true);

		m_Testboundbox3 = MXOgre_SCENEMANAGER->getRootSceneNode()->createChildSceneNode("m_Testboundbox3");
		Ogre::Entity* box6 =  MXOgre_SCENEMANAGER->createEntity("m_Testboundbox3Entity",Ogre::SceneManager::PT_CUBE);
		m_Testboundbox3->attachObject(box6);
		m_Testboundbox3->setScale(0.01, 0.01,0.01);
		m_Testboundbox3->setVisible(true);
		m_Testboundbox3->showBoundingBox(true);

		m_Testboundbox4 = MXOgre_SCENEMANAGER->getRootSceneNode()->createChildSceneNode("m_Testboundbox4");
		Ogre::Entity* box7 =  MXOgre_SCENEMANAGER->createEntity("m_Testboundbox4Entity", Ogre::SceneManager::PT_CUBE);
		m_Testboundbox4->attachObject(box7);
		m_Testboundbox4->setScale(0.01, 0.01,0.01);
		m_Testboundbox4->setVisible(true);
		m_Testboundbox4->showBoundingBox(true);

		Ogre::Overlay* pMyOVerlay = (Ogre::Overlay*)Ogre::OverlayManager::getSingleton().getByName("basitext");
		pMyOVerlay->show();
		Ogre::OverlayContainer* textcontainer = pMyOVerlay->getChild("basitext/MyContainer");
		m_TextBasic = textcontainer->getChild("basitext/text");
		m_TextBasic->show();
	}*/
	EffectManager::Instance()->CreateSmokeManager();
	EffectManager::Instance()->CreateBubbleManager();
	EffectManager::Instance()->CreateWaterManager();

	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceLeft)
	{
		InputSystem::GetInstance(DEVICETYPE_LEFT)->CreateDebugInfo(m_pOms->GetSceneManager());
	}else if (m_pTrainingConfig->m_DebugForDeviceWorkspaceRight)
	{
		InputSystem::GetInstance(DEVICETYPE_RIGHT)->CreateDebugInfo(m_pOms->GetSceneManager());
	}

	if (HardwareInterface::m_HardwareConfig.Adjust_Parameter)
	{
		InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->CreateDebugInfo(m_pOms->GetSceneManager());
	}

	CSimpleUIManger::Instance()->OnTrainingBeInitialized(this , pTrainingConfig->m_SimpleUI);
	m_ElapsedTime = 0.0f;
	return true;
}

void CBasicTraining::ShowDebugFrame()
{
	// show phantomboxdeub
	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceLeft)
	{
		//QFile filel("config_left.txt");
		//if (!filel.open(QIODevice::ReadOnly))
		//{
		   Inception::Instance()->EmitShowPhantomBoxDebug();
		//filel.close();
		//}
	}
	else if (m_pTrainingConfig->m_DebugForDeviceWorkspaceRight)
	{
		//QFile filer("config_right.txt");
		//if (!filer.open(QIODevice::ReadOnly))
		//{
		  Inception::Instance()->EmitShowPhantomBoxDebug();
		//filer.close();
		//}
	}

		// show camera
		/*
		if (MisRobotInput::m_HardwareConfig.Adjust_Parameter)
		{
			QFile filec("adjust_camera.txt");
			if (!filec.open(QIODevice::ReadOnly))
			{
				Inception::Instance()->EmitShowAdjustCamera();
				filec.close();
			}
		}

		// show shaft
		if (MisRobotInput::m_HardwareConfig.Adjust_Parameter)
		{
			QFile filesl("adjust_left_shaft.txt");
			if (!filesl.open(QIODevice::ReadOnly))
			{
				Inception::Instance()->EmitShowAdjustShaft();
				filesl.close();
			}
			QFile filesr("adjust_right_shaft.txt");
			if (!filesr.open(QIODevice::ReadOnly))
			{
				Inception::Instance()->EmitShowAdjustShaft();
				filesr.close();
			}
		}
		if (MisRobotInput::m_HardwareConfig.Adjust_Parameter)
		{
			//Inception::Instance()->EmitShowAdjustGyro();
		}
		*/
}

float CBasicTraining::GetElapsedTime()
{
	//return (Inception::Instance()->m_totalTime - Inception::Instance()->m_remainTime);
	return m_ElapsedTime;
}

bool CBasicTraining::Update(float dt)
{
	m_ElapsedTime += dt;
	//PERFORMANCE_DEBUG
	float elapsedseconds = 0;
    static float videoUpdateTime = MxGlobalConfig::Instance()->GetVideoFrameRate();
	
	unsigned long currmseconds = Ogre::Root::getSingleton().getTimer()->getMilliseconds();

	//if(m_lastmilliseconds > 0)
	{	
		elapsedseconds = currmseconds-m_lastmilliseconds;//(float)(currmseconds-m_lastmilliseconds) / (float)1000.0f;

		if(elapsedseconds < 0)
			elapsedseconds = 0;
	}
	//m_lastmilliseconds = currmseconds; //去掉,现在在用于截屏

	// deal events
	CMXEventsDump::Instance()->DumpEvents();
	
	m_pToolsMgr->Update(dt);
	
	//  [4/16/2012 yl 每次器械出来显示小窗口]
	if (m_pRdWindow!=NULL)
	{
		bool m_bSmallWindow=Inception::Instance()->m_bCenterWindow;
		if (m_bSmallWindow)
		{
			if (m_pToolsMgr->GetLeftTool()!=NULL)
			{
				if (m_pToolsMgr->GetLeftTool()->GetKernelNode()!=NULL)
				{
					if (m_dwStartTime==0)
					{
						m_dwStartTime=GetTickCount();
					}
				}
			}

			if (m_pToolsMgr->GetRightTool()!=NULL)
			{
				if (m_pToolsMgr->GetRightTool()->GetKernelNode()!=NULL)
				{
					if (m_dwStartTime==0)
					{
						m_dwStartTime=GetTickCount();
					}
				}
			}

			if (GetTickCount()-m_dwStartTime>TOTALTIME)
			{
				m_pRdWindow->hide();
				Inception::Instance()->m_bCenterWindow=false;
				Inception::Instance()->EmitShowSmallWindow();
				m_dwStartTime=0;
			}
		}
	}
	

	//EffectManager::Instance()->UpdateCamera(m_pOms);//move to mis new train
	CScreenEffect::Instance()->UpdateScreenColor();
	//EffectManager::Instance()->Update(dt);

	//PERFORMANCE_DEBUG_BEGIN_BY_NAME(CheckToolSpeed)
	//tips
	CheckToolSpeed();
	//PERFORMANCE_DEBUG_END_BY_NAME(CheckToolSpeed)


	//PERFORMANCE_DEBUG_BEGIN_BY_NAME(CheckToolFighting)
	//tools collision
	CheckToolFighting();
	//PERFORMANCE_DEBUG_END_BY_NAME(CheckToolFighting)
	//red screen
	CScreenEffect::Instance()->CheckWarn();


	//shadow upadte

	/*b::Instance()->update();*/

	//PERFORMANCE_DEBUG_BEGIN_BY_NAME(Animations)
	m_pOms->Update(MXOgreWrapper::Instance()->m_FrameElapsedTime);
	//PERFORMANCE_DEBUG_END_BY_NAME(Animations)

	if (m_bTrainingRunning && Inception::Instance()->m_remainTime == 0)
	{
		TimeOver();
	}

	/*if( m_pDynStrips != NULL )
	{
		m_pDynStrips->Update( m_pLargeCamera );
		bool in = m_pDynStrips->isInScene()? true: false;
		bool visible = m_pDynStrips->isVisible()?true:false;
		const Ogre::AxisAlignedBox& box = m_pDynStrips->getBoundingBox();
	}*/

	MapDeviceWorkspaceModel();

	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceLeft)
	{
		InputSystem::GetInstance(DEVICETYPE_LEFT)->DrawReferenceLine(m_pManualObjectForDebugLeft);
	}
	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceRight)
	{
		InputSystem::GetInstance(DEVICETYPE_RIGHT)->DrawReferenceLine(m_pManualObjectForDebugRight);
	}

	if ( m_bVideoRecording&&elapsedseconds>videoUpdateTime )
	{
		//m_funcStartVideoRecording();
		m_lastmilliseconds = currmseconds;
	}

	//Editor tool update
	if (m_IsInEditMode)
	{
		if (m_pathtool)
		{
			if (m_pathtool->m_EditedObject)
				m_pathtool->update(0, m_pLargeCamera);
			if (m_pathtool->m_SendDebuginfo)
			{
				for (set<int>::iterator set_iter = m_pathtool->m_SelectedClusterId.begin(); set_iter != m_pathtool->m_SelectedClusterId.end(); set_iter++)
				{
					showDebugInfo(Ogre::StringConverter::toString(*set_iter));
				}
				m_pathtool->m_SelectedClusterId.clear();
				m_pathtool->m_SendDebuginfo = false;
			}
		}
		
		if (m_pHotchpotchEditor)
			m_pHotchpotchEditor->Update(dt, m_pLargeCamera);
	}
    //

	return true;
}

bool CBasicTraining::Terminate()
{
	//CSSAO::Instance()->OnTrainingTerminate();
    

	CSimpleUIManger::Instance()->Clear();

	EffectManager::Instance()->DestorySmokeManager();
	EffectManager::Instance()->DestoryWaterManager();
	EffectManager::Instance()->DestoryBubbleManager();

	Ogre::SceneManager * pSMG =  MXOgre_SCENEMANAGER;

	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceLeft && m_pManualObjectForDebugLeft && m_pSceneNodeForDebugLeft)
	{
		m_pSceneNodeForDebugLeft->detachObject(m_pManualObjectForDebugLeft);
		pSMG->destroyManualObject(m_pManualObjectForDebugLeft);
		pSMG->getRootSceneNode()->removeChild(m_pSceneNodeForDebugLeft);
		pSMG->destroySceneNode(m_pSceneNodeForDebugLeft);
	}
	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceRight && m_pManualObjectForDebugRight && m_pSceneNodeForDebugRight)
	{
		m_pSceneNodeForDebugRight->detachObject(m_pManualObjectForDebugRight);
		pSMG->destroyManualObject(m_pManualObjectForDebugRight);
		pSMG->getRootSceneNode()->removeChild(m_pSceneNodeForDebugRight);
		pSMG->destroySceneNode(m_pSceneNodeForDebugRight);
	}
	ReleaseStaticData();
	//ReleaseDynamicData();
	ReleasePartData();

	CLightMgr::Instance()->RemoveAllLight();
	//g_ShadowMap->enableShadow(false);

	pSMG->setShadowTechnique(Ogre::SHADOWTYPE_NONE);

	if ( pSMG )
	{
		pSMG->clearScene();
		pSMG->destroyAllCameras();
		pSMG->destroyAllManualObjects();
		pSMG->destroyAllLights();
	}
	m_pOms->Destroy();

	return true;
}

void CBasicTraining::OnSaveTrainingReport()
{
	SYTrainingReport::Instance()->Reset();
	SYTrainingReport::Instance()->SetTotalTime(Inception::Instance()->m_totalTime);
	SYTrainingReport::Instance()->SetRemainTime(Inception::Instance()->m_remainTime);
	SYTrainingReport::Instance()->SetElectricTime(m_pToolsMgr->GetTotalElectricTime());
	SYTrainingReport::Instance()->SetValidElectricTime(m_pToolsMgr->GetValidElectricTime());
	SYTrainingReport::Instance()->SetNumberOfReleasedTitanicClip(m_pToolsMgr->GetNumberOfReleasedTitanicClip());
	SYTrainingReport::Instance()->SetLeftToolClosedTimes(m_pToolsMgr->GetLeftToolClosedTimes());
	SYTrainingReport::Instance()->SetRightToolClosedTimes(m_pToolsMgr->GetRightToolClosedTimes());
	SYTrainingReport::Instance()->SetLeftToolMovedDistance(m_pToolsMgr->GetLeftToolMovedDistance());
	SYTrainingReport::Instance()->SetRightToolMovedDistance(m_pToolsMgr->GetRightToolMovedDistance());
	SYTrainingReport::Instance()->SetLeftToolMovedSpeed(m_pToolsMgr->GetLeftToolMovedSpeed());
	SYTrainingReport::Instance()->SetRightToolMovedSpeed(m_pToolsMgr->GetRightToolMovedSpeed());
	SYTrainingReport::Instance()->SetMaxKeeppingElectricBeginTime(m_pToolsMgr->GetMaxKeeppingElectricBeginTime());
	SYTrainingReport::Instance()->SetMaxKeeppingElectricTime(m_pToolsMgr->GetMaxKeeppingElectricTime());
	SYTrainingReport::Instance()->SetElectricAffectTimeForHemoClip(m_pToolsMgr->GetElectricTimeForHemoClip());
	SYTrainingReport::Instance()->SetElectricAffectTimeForOrdinaryOrgan(m_pToolsMgr->GetElectricTimeForOrdinaryOrgan());
}

void CBasicTraining::setDeletedDynamicStripsIndex( int nIndex )
{
	SY_ASSERT( 0 && "Subclass need app this function\n" );
}
void CBasicTraining::passingToolKernel2mID( int nIndex )
{
	SY_ASSERT( 0 && "Subclass need apply this function\n" );
}
void CBasicTraining::passingToolKernelVector( Ogre::Vector3 vec, int index )
{
	SY_ASSERT( 0 && "Subclass need apply this function\n" );
}


std::vector<int> CBasicTraining::getDeletedDynamicStripsIndexVector()
{
	SY_ASSERT( 0 && "Subclass need app this function\n" );
	static vector< int > vec;
	return vec;
}

//void CBasicTraining::RemoveDynamicObject(TEDOT type)
//{
		//ReleaseDynamicData(type);
//}
void CBasicTraining::LoadStaticData(CXMLWrapperTraining * pTrainingConfig)
{
	if (pTrainingConfig->m_StaticScene->m_Scene.size() == 0) return; // zx
	Ogre::String strStaticDataPath = (*pTrainingConfig->m_StaticScene->m_Scene.begin())->m_Path;
	if (strStaticDataPath == "")
	{
		return;
	}

	char szLoadCount[256];
	OgreMax::OgreMaxScene::s_nLoadCount++;
	m_nSceneCount = OgreMax::OgreMaxScene::s_nLoadCount;
	sprintf_s(szLoadCount, "%d", OgreMax::OgreMaxScene::s_nLoadCount );
	if (!m_pOms)
	{
		m_pOms= new OgreMax::OgreMaxScene;
	}
	Ogre::String STRR = (*pTrainingConfig->m_StaticScene->m_Scene.begin())->m_Path;
	m_pOms->Load((*pTrainingConfig->m_StaticScene->m_Scene.begin())->m_Path,
		MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_LARGE),
		0,MXOgre_SCENEMANAGER);

	if (m_pOms->GetSceneNode("trocar_gallbladder$1", true))
	{
		m_quatTrocar = m_pOms->GetSceneNode("trocar_gallbladder$1", true)->getOrientation();
	}
	m_pLargeCamera = m_pOms->GetCamera(Ogre::String(CAMERA_NAME_1) + Ogre::String(szLoadCount),false);
	m_quatLargetCamera = m_pLargeCamera->getOrientation();

	m_viewport = MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_LARGE)->addViewport(m_pLargeCamera);
	m_pLargeCamera->setAspectRatio(static_cast<Ogre::Real>(m_viewport->getActualWidth()) / static_cast<Ogre::Real>(m_viewport->getActualHeight()));
	const Ogre::RenderTarget::FrameStats& stats = MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_LARGE)->getStatistics();
	
	
	std::string fps = Ogre::StringConverter::toString((int)stats.avgFPS);

	CXMLWrapperToolPlace * toolPlaceConfig = (m_pTrainingConfig->m_ToolPlaces.size() > 0 ? m_pTrainingConfig->m_ToolPlaces[0] : 0);

	InputSystem * pInputSys = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE);
	pInputSys->ResetInput();
	pInputSys->SetDefaultOrientation(m_pLargeCamera->getParentNode()->getOrientation());
	pInputSys->SetDefaultPosition(m_pLargeCamera->getParentNode()->getPosition());
	pInputSys->SetState(m_eCS);


	pInputSys = InputSystem::GetInstance(DEVICETYPE_LEFT);
	pInputSys->ResetInput();
	if (toolPlaceConfig)
	{
		pInputSys->SetDefaultPosition(toolPlaceConfig->m_LeftToolPos);
		pInputSys->SetDefaultOrientation(toolPlaceConfig->m_LeftToolOrientation);
	}

	pInputSys = InputSystem::GetInstance(DEVICETYPE_RIGHT);
	pInputSys->ResetInput();
	if (toolPlaceConfig)
	{
		pInputSys->SetDefaultPosition(toolPlaceConfig->m_RightToolPos);
		pInputSys->SetDefaultOrientation(toolPlaceConfig->m_RightToolOrientation);
	}

	pInputSys = InputSystem::GetInstance(DEVICETYPE_KEYCODE);
	pInputSys->ResetInput();

	

	//高光效果  [3/20/2012 yl]
	if (m_viewport && pTrainingConfig->m_HDR)
	{
		Ogre::CompositorManager::getSingleton().addCompositor(m_viewport, "HDR");
		Ogre::CompositorManager::getSingleton().setCompositorEnabled(m_viewport, "HDR", true);	
// 		Ogre::CompositorManager::getSingleton().addCompositor(m_viewport, "Bloom");
// 		Ogre::CompositorManager::getSingleton().setCompositorEnabled(m_viewport, "Bloom", true);		
	}

	//Ogre::CompositorManager::getSingleton().addCompositor(m_viewport, "MisMedical/SAO");
	//Ogre::CompositorManager::getSingleton().setCompositorEnabled(m_viewport, "MisMedical/SAO", true);	

	//deferred rendering
 
	//set training time
	Inception::Instance()->m_remainTime = (pTrainingConfig->m_TrainTime==0)?3600:pTrainingConfig->m_TrainTime;
	Inception::Instance()->m_totalTime = Inception::Instance()->m_remainTime;

	m_fCollisionRadius=pTrainingConfig->m_CollisionRadius;
	m_bCanStaticCollision=pTrainingConfig->m_CanStaticCollision;

	EffectManager::Instance()->SetOgreMaxScene(m_pOms);
}

void CBasicTraining::LoadDynamicDataOne(CXMLWrapperTraining * pTrainingConfig, int i)
{ 

}

void CBasicTraining::LoadDynamicData(CXMLWrapperTraining * pTrainingConfig)
{
	for (int i = 0; i < (int)pTrainingConfig->m_DynamicScene.size(); i++)
	{
		LoadDynamicDataOne(pTrainingConfig, i);
	}

	for(size_t i = 0 ;i < m_loadlistener.size() ; i++)
	{
		if(m_loadlistener[i])
			m_loadlistener[i]->onDynamicObjectChanged();
	}
}

void CBasicTraining::LoadPartData(CXMLWrapperTraining * pTrainingConfig)
{
	/*
	CPartObject *pPartDynamicObj;

    Ogre::String dir = (*pTrainingConfig->m_StaticScene->m_Scene.begin())->m_DataDir;

	if (dir == "")     return;

	for (int i = 0; i < (int)pTrainingConfig->m_PartScene.size(); i++)
	{
        if (pTrainingConfig->m_PartScene[i]->m_flag_FileName)
        {
			Ogre::String fileName = Ogre::String( pTrainingConfig->m_PartScene[i]->m_FileName );
			Ogre::String strS1mFileName = dir + Ogre::String(pTrainingConfig->m_PartScene[i]->m_FileName);
            pPartDynamicObj = new CPartObject(pTrainingConfig->m_PartScene[i]->m_Name,
                    strS1mFileName, pTrainingConfig->m_PartScene[i]->m_Type,
                    pTrainingConfig->m_PartScene[i]->m_Mass,
                    pTrainingConfig->m_PartScene[i]->m_EdgeStiff, pTrainingConfig->m_PartScene[i]->m_BendStiff,
                    pTrainingConfig->m_PartScene[i]->m_TorsionStiff, pTrainingConfig->m_PartScene[i]->m_EdgeDamp,
                    pTrainingConfig->m_PartScene[i]->m_ExtraEdgeDamp, pTrainingConfig->m_PartScene[i]->m_BendDamp,
                    pTrainingConfig->m_PartScene[i]->m_TorsionDamp, pTrainingConfig->m_PartScene[i]->m_Interval,
                    pTrainingConfig->m_PartScene[i]->m_Radius, pTrainingConfig->m_PartScene[i]->m_SelfCollisionStrength,
					pTrainingConfig->m_PartScene[i]->m_TetraStiff, pTrainingConfig->m_PartScene[i]->m_TetraDamp, 
					pTrainingConfig->m_PartScene[i]->m_TetraPointDamp, 
                    MXOgre_SCENEMANAGER,pTrainingConfig->m_PartScene[i]->m_CanShow,
					pTrainingConfig->m_PartScene[i]->m_FixedPoints,
					pTrainingConfig->m_PartScene[i]->m_HomingForce);
        }
        else
        {
            pPartDynamicObj = new CPartObject(pTrainingConfig->m_PartScene[i]->m_Name,
                    pTrainingConfig->m_PartScene[i]->m_Type, pTrainingConfig->m_PartScene[i]->m_Length, 
                    pTrainingConfig->m_PartScene[i]->m_FirstPointPos, pTrainingConfig->m_PartScene[i]->m_Orientation,
                    pTrainingConfig->m_PartScene[i]->m_Mass,
                    pTrainingConfig->m_PartScene[i]->m_EdgeStiff, pTrainingConfig->m_PartScene[i]->m_BendStiff,
                    pTrainingConfig->m_PartScene[i]->m_TorsionStiff, pTrainingConfig->m_PartScene[i]->m_EdgeDamp,
                    pTrainingConfig->m_PartScene[i]->m_ExtraEdgeDamp, pTrainingConfig->m_PartScene[i]->m_BendDamp,
                    pTrainingConfig->m_PartScene[i]->m_TorsionDamp, pTrainingConfig->m_PartScene[i]->m_Interval,
                    pTrainingConfig->m_PartScene[i]->m_Radius, pTrainingConfig->m_PartScene[i]->m_SelfCollisionStrength,
					pTrainingConfig->m_PartScene[i]->m_TetraStiff, pTrainingConfig->m_PartScene[i]->m_TetraDamp, 
					pTrainingConfig->m_PartScene[i]->m_TetraPointDamp, 
                    MXOgre_SCENEMANAGER,pTrainingConfig->m_PartScene[i]->m_CanShow,
					pTrainingConfig->m_PartScene[i]->m_FixedPoints,
					pTrainingConfig->m_PartScene[i]->m_HomingForce);
        }

		CPhysicCore::Instance()->GetPhysicsCore()->SetForceFeedbackRatio( pTrainingConfig->m_PartScene[i]->m_Type, 
			pTrainingConfig->m_PartScene[i]->m_ForceFeedbackRatio );

		pPartDynamicObj->Initialize(m_pOms);

		m_mapIDPartObj[pTrainingConfig->m_PartScene[i]->m_Type] = pPartDynamicObj;
	}
	*/
}

void CBasicTraining::LoadConfig(CXMLWrapperTraining * pTrainingConfig)
{
	LoadMovies(pTrainingConfig);
	LoadScores(pTrainingConfig);
	LoadProGrades(pTrainingConfig);
	LoadTips(pTrainingConfig);
	LoadStaticData(pTrainingConfig);
	if (!pTrainingConfig->m_CustomLoadDynamicScene)
	{
		LoadDynamicData(pTrainingConfig);
	}
	LoadPartData(pTrainingConfig);
	//LoadMucous( pTrainingConfig );
	//ConnectObject(pTrainingConfig);

	// Create organ collision,but too slowly!!!
	//CreateCollision(pTrainingConfig);

	//Environment Effect
	CreateEnvironment(pTrainingConfig);
	
	//shadowmap init
	if (m_pOms)
	{
		CLightMgr::Instance()->GetLight()->setPosition(0,0,0);
#ifdef USE_SSAO//in ssao use cmera
		CShadowMap::Get()->initConfig(m_pOms->GetSceneManager(),m_pLargeCamera);
#else//but in classic shadow we use light
		CShadowMap::Get()->initConfig(m_pOms->GetSceneManager(),m_pLargeCamera->getFOVy() , m_pLargeCamera->getAspectRatio());
#endif
		DeferredRendFrameWork::Instance()->OnInitialize(m_pLargeCamera , MXOgreWrapper::Instance()->GetMainRenderWindow() , pTrainingConfig->m_CanSSAO);
		DeferredRendFrameWork::Instance()->SetBloomAndCCParameter(pTrainingConfig->m_BloomFactor.x,
			                                                      pTrainingConfig->m_CCFactor.x,
			                                                      pTrainingConfig->m_CCFactor.y,
			                                                      pTrainingConfig->m_CCFactor.z);
	}
}

void CBasicTraining::CreateDefaultSceneMgr()
{

}

void CBasicTraining::ReleaseStaticData()
{

}

//void CBasicTraining::ReleaseDynamicData()
//{
//}

//void CBasicTraining::ReleaseDynamicData(int nType)
//{
	
//}

void CBasicTraining::ReleasePartData()
{

}

void CBasicTraining::setAbdomenVisible( bool b )
{
	if (m_strSkinModelName.empty())
	{
		return;
	}	
	stringstream ss;
	ss << m_nSceneCount;
	
 	Ogre::String s = m_strSkinModelName + "$";
	s.append(ss.str());
	MXOgre_SCENEMANAGER->getSceneNode(s)->setVisible(b);	
	//m_pSceneMgr->getSceneNode("diaphragm$1")->setVisible(b);
}
void CBasicTraining::setAbdomenTransparent(int  alpha)
{
	 EffectManager::Instance()->SetNodeTransparent(std::string("Skin") ,alpha);
	 EffectManager::Instance()->SetNodeTransparent(std::string("Head") ,alpha);
	 //EffectManager::Instance()->SetNodeTransparent(std::string("Iris") ,alpha);
};

void CBasicTraining::addDynObjLoadReleaseListener(ObjectLoadReleaseListener * listener)
{
	for(size_t i = 0 ;i < m_loadlistener.size() ; i++)
	{
		if(m_loadlistener[i] == listener)
			return;
	}
	m_loadlistener.push_back(listener);
}

void CBasicTraining::removeDynObjLoadReleaseListener(ObjectLoadReleaseListener * listener)
{
	for(size_t i = 0 ;i < m_loadlistener.size() ; i++)
	{
		if(m_loadlistener[i] == listener)
		{
			m_loadlistener.erase(m_loadlistener.begin()+i);
			return;
		}
	}

}

//void CBasicTraining::LoadMucous( CXMLWrapperTraining * pTrainingConfig )
//{
	//for ( size_t i = 0; i < pTrainingConfig->m_vecMucousObject.size(); i++ )
	//{
		//Ogre::String sz = pTrainingConfig->m_vecMucousObject[ i ]->m_Name;
		//float fMax = pTrainingConfig->m_vecMucousObject[ i ]->m_MaxLength;
	//}
//}

void CBasicTraining::LoadMovies( CXMLWrapperTraining * pTrainingConfig )
{
	
}

void CBasicTraining::LoadScores(CXMLWrapperTraining * pTrainingConfig)
{
	CScoreMgr::Instance()->LoadScores(pTrainingConfig->m_Scores);
}

void CBasicTraining::LoadProGrades(CXMLWrapperTraining * pTrainingConfig)
{
	COnLineGradeMgr::Instance()->LoadGrade(pTrainingConfig->m_OnLineGrades);
}
void CBasicTraining::LoadTips(CXMLWrapperTraining * pTrainingConfig)
{
	CTipMgr::Instance()->LoadTips(pTrainingConfig->m_Tips);
}

void CBasicTraining::CheckToolSpeed()
{
	if (m_pToolsMgr->GetLeftTool()!=NULL)
	{
		if (m_pToolsMgr->GetLeftTool()->GetKernelNode()!=NULL)
		{
			//防止选择器械提示移动速度过快信息
			if (m_strLeftToolName=="")
			{
				m_strLeftToolName=m_pToolsMgr->GetLeftTool()->GetName();
			}
			if (m_strLeftToolName!=m_pToolsMgr->GetLeftTool()->GetName())
			{
				m_vecLeftToolPos=Ogre::Vector3::ZERO;
				m_bLeftFirstTip=false;
				m_strLeftToolName=m_pToolsMgr->GetLeftTool()->GetName();
			}

			Ogre::Vector3 vecCurrentTool=m_pToolsMgr->GetLeftTool()->GetKernelNode()->_getDerivedPosition();
			if (m_vecLeftToolPos!=Ogre::Vector3::ZERO)
			{
				if (m_vecLeftToolPos!=vecCurrentTool)
				{
					float dist=vecCurrentTool.distance(m_vecLeftToolPos);
					if (dist>m_pTrainingConfig->m_DistanceLimit)
					{
						if (m_dwLeftStartTime==0)
						{
							if (m_bLeftFirstTip)
							{
								CTipMgr::Instance()->ShowTip("LeftSpeedWarn");
							}
							else
							{
								m_bLeftFirstTip=true;
							}
							
							m_dwLeftStartTime=GetTickCount();
						}
						if (GetTickCount()-m_dwLeftStartTime>=5000)//5s
						{
							m_dwLeftStartTime=0;
						}
					}
					m_vecLeftToolPos=vecCurrentTool;
				}	
			}
			else
			{
				m_vecLeftToolPos=vecCurrentTool;
			}
		}
	}

	if (m_pToolsMgr->GetRightTool()!=NULL)
	{
		if (m_pToolsMgr->GetRightTool()->GetKernelNode()!=NULL)
		{
			if (m_strRightToolName=="")
			{
				m_strRightToolName=m_pToolsMgr->GetRightTool()->GetName();
			}
			if (m_strRightToolName!=m_pToolsMgr->GetRightTool()->GetName())
			{
				m_vecRightToolPos=Ogre::Vector3::ZERO;
				m_bRightFirstTip=false;
				m_strRightToolName=m_pToolsMgr->GetRightTool()->GetName();
			}
			Ogre::Vector3 vecCurrentTool=m_pToolsMgr->GetRightTool()->GetKernelNode()->_getDerivedPosition();
			if (m_vecRightToolPos!=Ogre::Vector3::ZERO)
			{
				if (m_vecRightToolPos!=vecCurrentTool)
				{
					float dist=vecCurrentTool.distance(m_vecRightToolPos);
					if (dist>m_pTrainingConfig->m_DistanceLimit)
					{
						if (m_dwRightStartTime==0)
						{
							if (m_bRightFirstTip)
							{
								CTipMgr::Instance()->ShowTip("RightSpeedWarn");
							}
							else
							{
								m_bRightFirstTip=true;
							}
							
							m_dwRightStartTime=GetTickCount();
						}
						if (GetTickCount()-m_dwRightStartTime>=5000)//5s
						{
							m_dwRightStartTime=0;
						}
					}
					m_vecRightToolPos=vecCurrentTool;
				}	
			}
			else
			{
				m_vecRightToolPos=vecCurrentTool;
			}
		}
	}
}


bool CBasicTraining::ToolInsideFascia(ITool *curTool)
{
	if (curTool != NULL)
	{

	}

	return false;
}

void CBasicTraining::showDebugInfo(const std::string& debugInfo)
{
	Inception::Instance()->EmitShowDebugInfo(debugInfo);
}

void CBasicTraining::showDebugInfo(float value,int precision)
{
	std::stringstream ss;
	if(precision != -1)
		ss<<std::setprecision(precision);
	ss << value;
	showDebugInfo(ss.str());
}

void CBasicTraining::showDebugInfo(int value)
{
	std::stringstream ss;
	ss <<value;
	showDebugInfo(ss.str());
}

void CBasicTraining::showDebugInfo(unsigned int value)
{
	std::stringstream ss;
	ss <<value;
	showDebugInfo(ss.str());
}

void CBasicTraining::showDebugInfo(int value1,int value2,int value3,int value4)
{
	std::stringstream ss;
	ss << value1<<" "<<value2<<" "<<value3<<" "<<value4;
	showDebugInfo(ss.str());
}

void CBasicTraining::showDebugInfo(const std::string& prefix,float value,int precision)
{
	std::stringstream ss;
	if(precision != -1)
		ss<<std::setprecision(precision);
	ss << value;
	showDebugInfo(prefix + ss.str());
}

void CBasicTraining::showDebugInfo(float value1,float value2,float value3,char split,int precision)
{
	std::stringstream ss;
	if(precision != -1)
		ss<<std::setprecision(precision);
	ss << value1 << split << value2 << split << value3;
	showDebugInfo(ss.str());
}


void CBasicTraining::CreateEnvironment(CXMLWrapperTraining * pTrainingConfig)
{
	Ogre::SceneManager * pSMG =  MXOgre_SCENEMANAGER;

	//light effect
	for(int i=0; i<(int)pTrainingConfig->m_Lights.size();i++)
	{

		CLightMgr::Instance()->AddLight(pTrainingConfig->m_Lights[i],pTrainingConfig->m_Lights[i]->m_Name);
	}

#ifndef USE_SSAO
	//m_listenerDrawDepth = NULL;
	//create shadows effect
	//if (m_pTrainingConfig->m_Shadows.size()>0)
	//{
	//if (m_pTrainingConfig->m_Shadows[0]->m_CanShadow)
	//{
			//pSMG->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
			//m_listenerDrawDepth =  new DrawDepthSchemeHandler(g_ShadowMap);
			//Ogre::MaterialManager::getSingleton().addListener( m_listenerDrawDepth , "drawdepth" );
			
	//}
	CShadowMap * shadowMap = CShadowMap::Get();
	if(shadowMap)
	{
		shadowMap->enableShadow(m_pTrainingConfig->m_Shadows[0]->m_CanShadow);

		if(m_pTrainingConfig->m_Shadows[0]->m_flag_ShadowPos)
		   shadowMap->m_ShadowLightPos = m_pTrainingConfig->m_Shadows[0]->m_ShadowPos;
		else
		   shadowMap->m_ShadowLightPos = Ogre::Vector3(2.0f , 0 , 0);

		if(m_pTrainingConfig->m_Shadows[0]->m_flag_ShadowDirection)
		   shadowMap->m_ShadowLightDir = m_pTrainingConfig->m_Shadows[0]->m_ShadowDirection;
		else
		   shadowMap->m_ShadowLightDir = Ogre::Vector3::NEGATIVE_UNIT_Z;
	}
	//	else
	//	{
	//		int type=m_pTrainingConfig->m_Shadows[0]->m_ShadowType;
	//		switch(type)
	//		{
	//		case CBasicTraining::STENCIL_ADDITIVE:
	//			pSMG->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_ADDITIVE);
	//			break;
	//		case CBasicTraining::STENCIL_MODULATIVE:
	//			pSMG->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);
	//			break;
	//		case CBasicTraining::TEXTURE_ADDITIVE:
	//			pSMG->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);
	//			break;
	//		case CBasicTraining::TEXTURE_MODULATIVE:
	//			pSMG->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_MODULATIVE);
	//			break;
	//		default:
	//			pSMG->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
	//			break;
	//		}
	//		Ogre::Vector3 color=m_pTrainingConfig->m_Shadows[0]->m_ShadowColor;
	//		pSMG->setShadowColour(Ogre::ColourValue(color.x,color.y,color.z,1.0));//just for MODULATIVE Type
	//		//just for texture
	//		pSMG->setShadowTextureSettings(2048, 1);
	//		Ogre::ShadowCameraSetupPtr mCurrentShadowCameraSetup=Ogre::ShadowCameraSetupPtr(new Ogre::LiSPSMShadowCameraSetup()); //LISPSM
	//		pSMG->setShadowCameraSetup(mCurrentShadowCameraSetup);

	//		pSMG->setShadowTexturePixelFormat(Ogre::PF_X8R8G8B8);
	//		pSMG->setShadowTextureCasterMaterial(Ogre::StringUtil::BLANK);
	//		pSMG->setShadowTextureReceiverMaterial(Ogre::StringUtil::BLANK);
	//		pSMG->setShadowTextureSelfShadow(false);	
	//		pSMG->setShadowCasterRenderBackFaces(true);
	//	}
	//}

	//Ogre::CompositorManager::getSingleton().addCompositor(m_viewport, "classic_shadow");
	//Ogre::CompositorManager::getSingleton().setCompositorEnabled(m_viewport, "classic_shadow", true);	

#else
	m_listenerGbuffer   = 0;
	m_listenerNoGbuffer = 0;
	//真 ssao阴影
	if (m_viewport )
	{
		//MXASSERT( m_LogicInstance==NULL);
		if (!m_LogicInstance)
			m_LogicInstance = new SSAOLogic;
		m_listenerGbuffer =  new GBufferSchemeHandler ;
		m_listenerNoGbuffer = new NullSchemeHandler;
		Ogre::MaterialManager::getSingleton().addListener( m_listenerGbuffer , "GBuffer" );
		Ogre::MaterialManager::getSingleton().addListener( m_listenerNoGbuffer , "NoGBuffer" );
		Ogre::CompositorManager::getSingleton().registerCompositorLogic("SSAOLogic", m_LogicInstance);

		Ogre::CompositorManager::getSingleton().addCompositor(m_viewport, "DeferredShading/SSAO");
		Ogre::CompositorManager::getSingleton().setCompositorEnabled(m_viewport, "DeferredShading/SSAO", true);		
	}
#endif
}

void CBasicTraining::setUniform(Ogre::String compositor, Ogre::String material, Ogre::String uniform, float value, bool setVisible, int position /* = -1 */)
{
	// remove compositor first???
	if (!m_viewport)
	{
		return;
	}
	Ogre::CompositorManager::getSingleton().removeCompositor(m_viewport, compositor);

	(static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton().getByName(material)))->getTechnique(0)->
		getPass(0)->getFragmentProgramParameters()->setNamedConstant(uniform, value);

	// adding again
	Ogre::CompositorManager::getSingleton().addCompositor(m_viewport, compositor, position);
	Ogre::CompositorManager::getSingleton().setCompositorEnabled(m_viewport, compositor, setVisible);
}

void CBasicTraining::SetEditorMode(bool set)
{
	//g_ShadowMap->enableShadow(true);
	m_ineditormode = set;
}

void CBasicTraining::TrainingIntialized()
{
	CScoreMgr::Instance()->OnTrainStart();
	CTipMgr::Instance()->OnTrainStart();
	CTipMgr::Instance()->ShowTip("TrainingReset");//显示重置提示
	/*
	if (EngineCore::IsUnlocked())
	{
		CTipMgr::Instance()->ShowTip("TrainingIntro");
	}
	else
	{*/
	//CTipMgr::Instance()->ShowTip("TrainingReset");//显示重置信息
	//}
	m_bTrainingRunning = true;
}

void CBasicTraining::OnTrainingIlluminated()
{
	CTipMgr::Instance()->ShowTip("TrainingIntro");//显示训练介绍
	Inception::Instance()->readyToTiming();//当场景亮起时才显示训练场景的时间
	m_bTrainingIlluminated = true;
	m_ElapsedTime = 0;
	if (m_pTrainingConfig->m_NeedFixTool)
	{
		Inception::Instance()->EmitShowFixToolMenue();
	}
	
	ShowDebugFrame();
}

void CBasicTraining::TimeOver()
{
	
}

void CBasicTraining::TrainingFinish()
{
	
}

/*
void CBasicTraining::TrainingFinish(Ogre::String strTipName)
{
	if (m_bTrainingRunning)
	{
		
	}
}
*/
#define ST_TIME 300
#define TOOL_RADIUS  0.15
#define COLLISON_SOUND  "toolCollison.wav"
void CBasicTraining::CheckToolFighting()
{
	///////为了提高帧率，减少运算量，每隔一段时间才检测一次//////
	static double lastCpuTick = 0;
	double curTick = GetTickCount();
	if ( (curTick-lastCpuTick) < ST_TIME)  return;
	lastCpuTick = curTick;
	

	CTool* pLeftTool = dynamic_cast<CTool*>(m_pToolsMgr->GetLeftTool());
	CTool* pRightTool = dynamic_cast<CTool*>(m_pToolsMgr->GetRightTool());
	if (!pLeftTool || !pRightTool)    return;

	GFPhysVector3 nor = GFPhysVector3(0,0,0);
	Ogre::Vector3 lv1(0, 0, 0);
	Ogre::Vector3 lv2(0, 0, 0);
	Ogre::Vector3 rv1(0, 0, 0);
	Ogre::Vector3 rv2(0, 0, 0);
	pLeftTool->GetKernelLine(lv1,lv2);
	pRightTool->GetKernelLine(rv1,rv2);
	
	GFPhysVector3 p1(lv1.x, lv1.y, lv1.z); 
	GFPhysVector3 p2(lv2.x, lv2.y, lv2.z); 
	GFPhysVector3 q1(rv1.x, rv1.y, rv1.z); 
	GFPhysVector3 q2(rv2.x, rv2.y, rv2.z); 
	float s, t;
	GFPhysVector3 c1, c2;

	float dis = GPClosestPtSegmentSegment(p1, p2, q1, q2, s, t, c1, c2);
	dis = sqrt(dis);
	/*if (dis < 0.65f)
	{
		//QSound::play( qApp->applicationDirPath() + "/sound/"+COLLISON_SOUND);
		nor =c2 - c1;
		nor.Normalize();
		float forceline = std::min(1.0f, std::max(0.0f, 1-dis));
		pRightTool->m_ToolFightingForce =  nor * 1.0f * forceline;
		nor = c1 - c2;
		pLeftTool->m_ToolFightingForce =  nor * 1.0f * forceline;
	}else*/
	{
		//pRightTool->m_ToolFightingForce =  GFPhysVector3(0,0,0);
		//pLeftTool->m_ToolFightingForce =  GFPhysVector3(0,0,0);
	}
}

void CBasicTraining::MapDeviceWorkspaceModel()
{
	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceLeft)
	{
		QFile file("config_left.txt");
		if (file.open(QIODevice::ReadOnly))
		{
			QTextStream fin(&file);
			InputSystem::GetInstance(DEVICETYPE_LEFT)->MapDeviceWorkspaceModel(fin.readAll().toStdString(),false);
			file.close();
		}else
		{
			QString strinfo("");
			Inception::Instance()->EmitGetPhantomBoxDebugInfo(strinfo);
			InputSystem::GetInstance(DEVICETYPE_LEFT)->MapDeviceWorkspaceModel(strinfo.toStdString(),false);
		}
	}
	else
	{
		InputSystem::GetInstance(DEVICETYPE_LEFT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceLeft,true);
	}

	if (m_pTrainingConfig->m_DebugForDeviceWorkspaceRight)
	{
		QFile file("config_right.txt");
		if (file.open(QIODevice::ReadOnly))
		{
			QTextStream fin(&file);
			InputSystem::GetInstance(DEVICETYPE_RIGHT)->MapDeviceWorkspaceModel(fin.readAll().toStdString() , false);
			file.close();
		}else
		{
			QString strinfo("");
			Inception::Instance()->EmitGetPhantomBoxDebugInfo(strinfo);
			Ogre::String datainfo = strinfo.toStdString();
			InputSystem::GetInstance(DEVICETYPE_RIGHT)->MapDeviceWorkspaceModel(datainfo,false);
		}
	}
	else
	{
		InputSystem::GetInstance(DEVICETYPE_RIGHT)->MapDeviceWorkspaceModel(m_pTrainingConfig->m_DataForDeviceWorkspaceRight , true);
	}
}

//void CBasicTraining::SetAutoRecordFunction( boost::function<void()> func )
//{
	//m_funcStartVideoRecording = func;
//}

void CBasicTraining::KeyPress(QKeyEvent * event)
{
	/*if (event->key() == Qt::Key_A)
	{
		Ogre::SceneManager::MovableObjectIterator objIter = MXOgre_SCENEMANAGER->getMovableObjectIterator("Entity");
		while (objIter.hasMoreElements())
		{
			Ogre::Entity* entityObj = (Ogre::Entity*)objIter.getNext();
			entityObj->setVisible(!entityObj->getVisible());
		}

	}

	else */if (event->key() == Qt::Key_R)
	{
		//ReloadMaterial();
	}

	ITraining::KeyPress(event);
}

void reloadShader(Ogre::MaterialPtr matReload)
{
	Ogre::Technique* matTech = matReload->getTechnique(0);
	Ogre::Pass* techPass = matTech->getPass(0);

	const Ogre::GpuProgramPtr passVP = techPass->getVertexProgram();
	passVP->setParameter("compile_arguments", "-DENABLE_XX_FEATURE");
	passVP->escalateLoading();
	passVP->reload();

	const Ogre::GpuProgramPtr passFp = techPass->getFragmentProgram();
	passFp->setParameter("compile_arguments", "-DENABLE_XX_FEATURE");
	passFp->escalateLoading();
	passFp->reload();

	cout << "reload GPUProgram " << matReload->getName() + "  " << passVP->getName() << endl;
}

void CBasicTraining::ReloadMaterial()
{
	Ogre::SceneManager::MovableObjectIterator objIter = MXOgre_SCENEMANAGER->getMovableObjectIterator("Entity");
	while (objIter.hasMoreElements())
	{
		Ogre::Entity* entityObj = (Ogre::Entity*)objIter.getNext();

		int nunSubEntity = entityObj->getNumSubEntities();
		for (int i = 0; i < nunSubEntity; i++)
		{
			Ogre::SubEntity* subentityObj = entityObj->getSubEntity(i);
			Ogre::MaterialPtr materialObj = subentityObj->getMaterial();

			if (materialObj.isNull())
				continue;

			reloadShader(materialObj);

		}
	}

	std::vector<MisMedicOrganInterface*> organs;
	GetAllOrgan(organs);
	for (std::size_t i = 0; i < organs.size(); ++i)
	{
		string nameOrganMat = organs[i]->getMaterialName();

		Ogre::MaterialPtr matOrgan = Ogre::MaterialManager::getSingleton().getByName(nameOrganMat);

		reloadShader(matOrgan);
	}

	//Ogre::MaterialPtr matQuad = DeferredRendFrameWork::Instance()->m_pScreenQuadObj->getSection(0)->getMaterial();
	/*Ogre::MaterialPtr matQuad = Ogre::MaterialManager::getSingleton().getByName("Bloom/BrightPass");
	reloadShader(matQuad);

	matQuad = Ogre::MaterialManager::getSingleton().getByName("Bloom/BlurX");
	reloadShader(matQuad);

	matQuad = Ogre::MaterialManager::getSingleton().getByName("Bloom/BlurY");
	reloadShader(matQuad);

	matQuad = Ogre::MaterialManager::getSingleton().getByName("Bloom/Composite");
	reloadShader(matQuad);
	
	matQuad = Ogre::MaterialManager::getSingleton().getByName("MisMedical/StaticTemplateDX11_Transparent");
	reloadShader(matQuad);*/

}

