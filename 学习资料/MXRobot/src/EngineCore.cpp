#include "EngineCore.h"
#include "ResourceManager.h"
#include "InputSystem.h"
#include "KeyboardInput.h"
#include "Instruments/Tool.h"
#include <OgreCamera.h>
#include <OgreSceneQuery.h>
#include <OgreSceneNode.h>
#include "MXOgreWrapper.h"
#include "ITraining.h"
#include "ToolsMgr.h"
#include "ScreenEffect.h"
#include "Inception.h"
#include "EffectManager.h"
#include "Common/CollisionTools.h"
#include "MisRobotInput.h"
#include "XMLWrapperHardwareConfig.h"
#include "PerformanceDebug.h"
#include "GlobalKeyValue.h"
#include "SerialHardwareManager.h"
#include "math.h"
#include "ToolSpenetrateMgr.h"

//#include "math.h"
#define LIGHTNAME "VR001$1"
#include "TipMgr.h"
#include "SimpleUI.h"
#include "MxSoundManager.h"

EngineCore::EngineCore(void)
	:m_pTrainingMgr(NULL)
	, m_flastframe(0)
	//,m_fTimeSlice(0)
	, m_bShowFrame(false)
	, m_bShowProfiler(false)
	, m_fClAppliperDistance(0.01f)
	, m_fClAppliperOffset(0.2f)
	//,m_fTempAside(100.0f)
	, m_fTempKey(0.0f)
	//,m_bMaxAside(true)
	//,m_bMinAside(false)
	, m_bReturn(false)
	, m_bCameraFixed(false)
{
	//m_pTrainingMgr = new CTrainingMgr;
	m_pTrainingMgr = CTrainingMgr::Instance();	
	//CSoundManager::Create();
	//CSoundManager::Get()->Initialize("../res/audio/music.wav");
	//CPhysicCore::Create();

	//timeSliceFile.open("..\\Config\\timeslice.ini",ios::in);
	//if(timeSliceFile.fail())
	//{
	//	return;
	//}

	//string strTimeSlice;
	//getline(timeSliceFile,strTimeSlice);

	//m_fTimeSlice = atof(strTimeSlice.c_str());

	//getline(timeSliceFile,strTimeSlice);


	//m_iUpdateTimes = atoi(strTimeSlice.c_str());
	//m_writeFile.open("time.log", ios::trunc); 

	m_vecLeftLastPos=Ogre::Vector3::ZERO;
	m_vecRightLastPos=Ogre::Vector3::ZERO;

	m_RightDistance=0.0;
	m_LeftDistance=0.0;

	//GetCurrTouchManager().CreateTouch(DEVICETYPE_LEFT_NAME);
	//GetCurrTouchManager().CreateTouch(DEVICETYPE_RIGHT_NAME);

	//
	QueryPerformanceFrequency(&m_litmp);	
	m_lastframetime = 0;

	m_FrameCount = 0;
	m_AccumulateTime = 0;
	m_UpdateFPS = 0;

	
}

EngineCore::~EngineCore(void)
{
	m_writeFile.close();
	delete m_pTrainingMgr;

	SerialHardwareManager::GetInstance()->DeInitialize();
	SerialHardwareManager::DestroyIntance();
}

/*
bool EngineCore::frameStarted( const Ogre::FrameEvent& evt )
{
	Update();

	ProcessInput();

	return true;
}

bool EngineCore::frameRenderingQueued( const Ogre::FrameEvent& evt )
{
	return true;
}

bool EngineCore::frameEnded( const Ogre::FrameEvent& evt )
{
	Terminate();

	return true;
}
*/

bool EngineCore::Initialize()
{
	m_bCalibrationFinished = false;
	m_bLeftTouchCalibrationFinished = false;
	m_bRightTouchCalibrationFinished = false;
	m_bCameraCalibrationFinished = false;
	m_bUnlock = false;

	/*CTouch * touchLeft  = GetCurrTouchManager().GetTouch(DEVICETYPE_LEFT_NAME);

	CTouch * touchRight = GetCurrTouchManager().GetTouch(DEVICETYPE_RIGHT_NAME);

	bool isValidLeft = false;

	bool isValidRight = false;

	if(touchLeft  && touchLeft-> m_hHD != HD_INVALID_HANDLE)
	isValidLeft = true;

	if(touchRight && touchRight-> m_hHD != HD_INVALID_HANDLE)
	isValidRight = true;

	if (isValidLeft || isValidRight)
	{
	GetCurrTouchManager().DisableForce(DEVICETYPE_LEFT_NAME);
	GetCurrTouchManager().DisableForce(DEVICETYPE_RIGHT_NAME);
	GetCurrTouchManager().StartScheduler();
	}*/

	//if (InitConfig())
	{
		InitLogic();
	}

	GetCurrTouchManager().Intialize(DEVICETYPE_LEFT_NAME , DEVICETYPE_RIGHT_NAME);
	SerialHardwareManager::CreateIntance()->Intialize();
	
	CScreenEffect::Instance()->SetScreenColor(Ogre::ColourValue(0,0,0,0.95),5000.0f);
	return true;
}

bool EngineCore::InitLogic()
{
	m_pTrainingMgr->Initialize();

	return true;
}

/*
bool EngineCore::InitConfig()
{
bool result = CResourceManager::Instance()->LoadResource(&(Ogre::Root::getSingleton()));

CXMLWrapperTraining * currTrainConfig = CResourceManager::Instance()->GetTrainingConfigByName(Inception::Instance()->m_strTrainingName);

if(currTrainConfig->m_flag_AddOgreResLocation)
{
Ogre::vector<Ogre::String>::type AddResLocations = Ogre::StringUtil::split(currTrainConfig->m_AddOgreResLocation);
for(size_t c = 0 ; c < AddResLocations.size() ; c++)
{
Ogre::String resString = AddResLocations[c];
Ogre::ResourceGroupManager::getSingleton().addResourceLocation(resString, "FileSystem");
}
Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}


return result;
}
*/

double EngineCore::GetCurrentTimeInSeconds()
{
	LARGE_INTEGER  countFreq;

	QueryPerformanceCounter(&countFreq) ; 

	double TimeSeconds = (double)(countFreq.QuadPart) / (double)m_litmp.QuadPart;

	return TimeSeconds;
}


float EngineCore::Update()
{
#define TEST_EFFICIENCY

#if defined(TEST_EFFICIENCY)
	//clock_t now1 = clock();
#endif
	float dt = 0;
	double currentTime = GetCurrentTimeInSeconds();
	if(m_lastframetime < FLT_EPSILON)
		dt = 0.01f;
	else
		dt = (float)(currentTime-m_lastframetime);
	m_lastframetime = currentTime;

	TouchUpdate();

	ProcessInput(dt);

	ToolSpenetrateMgr::GetInstance()->CorrectToolTransform(m_pTrainingMgr,dt); 

	//fps statics
	m_FrameCount++;
	m_AccumulateTime += dt;
	if(m_AccumulateTime > 1.0f)
	{
		m_UpdateFPS = (int)((float)m_FrameCount / (float)m_AccumulateTime);
		m_FrameCount = 0;
		m_AccumulateTime = 0;
	}
#if defined(TEST_EFFICIENCY)

#endif

#if defined(TEST_EFFICIENCY)
	ProfilerUpdate();
	//ProfilerUpdate( 1 / updateTime );
#endif
	return dt;
}

// process user inputs, zx
bool EngineCore::ProcessInput(float dt)
{
	ITraining * pTraining = m_pTrainingMgr->GetCurTraining();

	if (!pTraining) return false;

	MisRobotInput::RefreshAllTouchStates();//refresh all position orientation state from touch manager first

	InputSystem::GetInstance(DEVICETYPE_LEFT)->Update(dt);
	InputSystem::GetInstance(DEVICETYPE_RIGHT)->Update(dt);
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->Update(dt);

	// process left tool
	ITool * pLeftTool = pTraining->m_pToolsMgr->GetLeftTool();
	if (pLeftTool && pLeftTool->IsInUse())
	{
		Ogre::Vector3 v3Pos = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetPosition_OGRE(E_EULER_ANGLES_YXZ, E_MOVE_AXIS_Z);
		Ogre::Quaternion quatKernel = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetOrientation_OGRE(E_EULER_ANGLES_YXZ);
		if (InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->m_InputDeviceType == InputGyoscope)
		{
			v3Pos = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetPosition_OGRE(E_EULER_ANGLES_XYZ, E_MOVE_AXIS_Z);
			quatKernel = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetOrientation_OGRE(E_EULER_ANGLES_XYZ);
		}

		float fShaftAside = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetShaftAside();

		if (false == HardwareInterface::m_HardwareConfig.Use_Phantom_SelectTool)
		{
			if (InputSystem::GetInputDeviceType(DEVICETYPE_LEFT) == InputPhantom || InputSystem::GetInputDeviceType(DEVICETYPE_LEFT) == InputGyoscope )
			{
				KeyboardInput* keycode =  dynamic_cast<KeyboardInput*>(InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetDevice());
				fShaftAside = keycode->GetShaftAside(DEVICETYPE_LEFT);
			}
		}
		//update tool posture & shaft
		pLeftTool->SyncToolPostureByHardware(InputSystem::GetInstance(DEVICETYPE_LEFT)->GetPivotPosition(), v3Pos, quatKernel);
		fShaftAside = pLeftTool->SyncShaftAsideByHardWare(fShaftAside, dt);
	}

	// process right tool
	ITool * pRightTool = pTraining->m_pToolsMgr->GetRightTool();
	if (pRightTool && pRightTool->IsInUse())
	{
		Ogre::Vector3 v3Pos = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetPosition_OGRE(E_EULER_ANGLES_YXZ, E_MOVE_AXIS_Z);
		Ogre::Quaternion quatKernel = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetOrientation_OGRE(E_EULER_ANGLES_YXZ);

		if (InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->m_InputDeviceType == InputGyoscope)
		{
			v3Pos = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetPosition_OGRE(E_EULER_ANGLES_XYZ, E_MOVE_AXIS_Z);
			quatKernel = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetOrientation_OGRE(E_EULER_ANGLES_XYZ);
		}
	
		// update two heads, if exist
		float nShaftAside = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetShaftAside();
		if (false == HardwareInterface::m_HardwareConfig.Use_Phantom_SelectTool)
		{
			if (InputSystem::GetInputDeviceType(DEVICETYPE_RIGHT) == InputPhantom || InputSystem::GetInputDeviceType(DEVICETYPE_RIGHT) == InputGyoscope )
			{
				KeyboardInput* keycode =  dynamic_cast<KeyboardInput*>(InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetDevice());
				nShaftAside = keycode->GetShaftAside(DEVICETYPE_RIGHT);
			}
		}

		pRightTool->SyncToolPostureByHardware(InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetPivotPosition(), v3Pos, quatKernel);
		nShaftAside = pRightTool->SyncShaftAsideByHardWare(nShaftAside, dt);
	}

	// electric
	bool bLeftElectricButton  = SerialHardwareManager::GetInstance()->IsLeftPadDown()  || InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetLeftElectricButton();
	bool bRightElectricButton = SerialHardwareManager::GetInstance()->IsRightPadDown() || InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetRightElectricButton();
	bool bCanElectric = false;
	if (pLeftTool)
	{
		bCanElectric = bCanElectric || pLeftTool->GetElectricAttribute();
	}
	if (pRightTool)
	{
		bCanElectric = bCanElectric || pRightTool->GetElectricAttribute();
	}

 
	// End of Play Sound for ElectricButton
	
	//同一优先级 右手器械优先
	bool IsRightToolElectric = true;
	bool IsLeftToolElectric = true;

	if(pRightTool && pLeftTool)
	{
		int leftPriority = pLeftTool->GetElecPriority();
		int rightPriority = pRightTool->GetElecPriority();

		if(leftPriority > rightPriority)
		{
			IsLeftToolElectric = true;
			IsRightToolElectric = false;
		}
		else
		{
			IsLeftToolElectric = false;
			IsRightToolElectric = true;
		}

//无视通电优先级，如冲吸器等使用电按键但非通电器械//不用 直接设置更高优先级
// 		IsRightToolElectric = IsRightToolElectric || pRightTool->IsIgnoreElecPriority();
// 		IsLeftToolElectric = IsLeftToolElectric || pLeftTool->IsIgnoreElecPriority();
	}

	if(pRightTool)
	{
		pRightTool->SetElectricButton(IsRightToolElectric && bCanElectric && (bLeftElectricButton || bRightElectricButton));
		pRightTool->SetElectricLeftPad(IsRightToolElectric && bLeftElectricButton);
		pRightTool->SetElectricRightPad(IsRightToolElectric && bRightElectricButton);
	}
	if(pLeftTool)
	{	
		pLeftTool->SetElectricButton(IsLeftToolElectric && bCanElectric && (bLeftElectricButton || bRightElectricButton));
		pLeftTool->SetElectricLeftPad(IsLeftToolElectric && bLeftElectricButton );
		pLeftTool->SetElectricRightPad(IsLeftToolElectric && bRightElectricButton );
	}


	// Add for Play Sound of ElectricButton
	std::string soundstr = "";

	ITool * toolPerformElectric = 0;
	
	if(pRightTool && pRightTool->GetElectricButton())
	   toolPerformElectric = pRightTool;
	
	else if(pLeftTool && pLeftTool->GetElectricButton())
	   toolPerformElectric = pLeftTool;

	if(toolPerformElectric)
	{
		if( toolPerformElectric-> GetType() == TT_ULTRASONIC_SCALPE)
		{
			if(bLeftElectricButton)
			   soundstr = "../res/audio/HarmonicScalpel1.wav";
			else if(bRightElectricButton)
			   soundstr = "../res/audio/HarmonicScalpel2.wav";
		}
		else
		{
			if(bLeftElectricButton)
			   soundstr = "../res/audio/music1.wav";
			else if(bRightElectricButton)
			   soundstr = "../res/audio/music2.wav";
		}
		MxSoundManager:: GetInstance()-> Play( soundstr , false);
	}
	else
	{
		MxSoundManager::GetInstance ()->StopSound ("../res/audio/music1.wav" );
		MxSoundManager::GetInstance ()->StopSound ("../res/audio/music2.wav" );

		MxSoundManager::GetInstance ()->StopSound ("../res/audio/HarmonicScalpel1.wav" );
		MxSoundManager::GetInstance ()->StopSound ("../res/audio/HarmonicScalpel2.wav" );

	}

	bool bCurShowFrame =  InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetShowFrame();
	
	bool curState = SerialHardwareManager::GetInstance()->IsCameraLocked();
	
	if (pTraining->m_pLargeCamera && curState == false)
	{
		Ogre::Vector3 v3Pos ;
		Ogre::Vector3 v3PosTrocar;
		Ogre::Quaternion quat;
		Ogre::Quaternion quatTrocar;

		//if( 0 )//测试代码
		if(pTraining->m_ineditormode == false)
		{
			v3Pos = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetPosition_OGRE(E_EULER_ANGLES_ZXY, E_MOVE_AXIS_Y);		
			v3PosTrocar = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetPositionTrocar_OGRE(E_EULER_ANGLES_ZXY, E_MOVE_AXIS_Y);
			quat = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetOrientation_OGRE(E_EULER_ANGLES_ZXY);
			quatTrocar = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetOrientationTrocar_OGRE(E_EULER_ANGLES_ZX0);

			if (InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDevice()->m_InputDeviceType == InputLaparoscope)
			{
				v3Pos = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetPosition_OGRE(E_EULER_ANGLES_XZY, E_MOVE_AXIS_Y);
				v3PosTrocar = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetPositionTrocar_OGRE(E_EULER_ANGLES_XZY, E_MOVE_AXIS_Y);
				quat = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetOrientation_OGRE(E_EULER_ANGLES_XZY);
				quatTrocar = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetOrientationTrocar_OGRE(E_EULER_ANGLES_XZY);
			}
		}
		else
		{
			Ogre::Camera * pCamera = pTraining->m_pLargeCamera;

			m_bShowFrame = bCurShowFrame;
			if (m_bShowFrame)
			{
				pCamera->setPolygonMode(Ogre::PM_WIREFRAME);
			}
			else
			{
				pCamera->setPolygonMode(Ogre::PM_SOLID);
			}
			return true;
			//////////////////////////////////////////////////////////////////////////
		}

		Ogre::Camera * pCamera = pTraining->m_pLargeCamera;
		Ogre::Node * pTrocar = NULL;

		if (MXOgreWrapper::Get()->GetDefaultSceneManger()->hasSceneNode("trocar_gallbladder$1"))
		{
			pTrocar = MXOgreWrapper::Get()->GetDefaultSceneManger()->getSceneNode("trocar_gallbladder$1");
		}

		if (pTraining->m_bStaticCamera == false)
		{
			if (pTrocar)
			{
				pTrocar->setPosition(v3PosTrocar);
				pTrocar->setOrientation(pTraining->m_quatTrocar * quatTrocar) ;
			}
			pCamera->getParentNode()->setOrientation(quat);
			pCamera->getParentNode()->setPosition(v3Pos);
		}
		if (m_pTrainingMgr->GetCurTraining())
		   m_pTrainingMgr->GetCurTraining()->UpdateTrocarTransform();

		if (MXOgreWrapper::Get()->GetDefaultSceneManger()->hasLight(LIGHTNAME))
		{
			//Ogre::Vector3 v3Deta=pCamera->getDerivedOrientation()*Ogre::Vector3(0.0,0.0,-1.0)*0.5;
			//Ogre::Vector3 v3Deta = -pCamera->getDerivedDirection()*0.0f;
			MXOgreWrapper::Get()->GetDefaultSceneManger()->getLight(LIGHTNAME)->getParentNode()->setPosition(pCamera->getDerivedPosition());// +v3Deta);
			MXOgreWrapper::Get()->GetDefaultSceneManger()->getLight(LIGHTNAME)->getParentNode()->setOrientation(pCamera->getDerivedOrientation());
		}

		float specialAngle = pTraining->GetCameraSpecialAngle();// MXOgreWrapper::CS_DEGREE_30;
		
	//	if(InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE))
		//  enumCameraState = (MXOgreWrapper::CameraState)InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetState();
		
		MXOgreWrapper::Instance()->SetCameraSpecialAngle(pCamera, pTraining->m_quatLargetCamera, Ogre::Degree(specialAngle));
		if (m_bShowFrame != bCurShowFrame)
		{
			m_bShowFrame = bCurShowFrame;
			pTraining->m_pLargeCamera->setPolygonMode(m_bShowFrame?Ogre::PM_WIREFRAME:Ogre::PM_SOLID);

		}
	}
    
	if(curState != m_bCameraFixed)
		pTraining->OnCameraStateChanged(curState);
	m_bCameraFixed = curState;
	
    pTraining->CalcCameraSpeed(dt, m_bCameraFixed);

	m_bShowProfiler = InputSystem::GetInstance(DEVICETYPE_KEYCODE)->GetShowProfiler();
	return true;
}

bool EngineCore::Terminate()
{
	/*CTouch * touchLeft  = GetCurrTouchManager().GetTouch(DEVICETYPE_LEFT_NAME);

	CTouch * touchRight = GetCurrTouchManager().GetTouch(DEVICETYPE_RIGHT_NAME);

	bool isValidLeft = false;

	bool isValidRight = false;

	if(touchLeft  && touchLeft-> m_hHD != HD_INVALID_HANDLE)
	isValidLeft = true;

	if(touchRight && touchRight-> m_hHD != HD_INVALID_HANDLE)
	isValidRight = true;

	if (isValidLeft || isValidRight)
	{
	GetCurrTouchManager().StopScheduler();
	}*/

	m_pTrainingMgr->Terminate();
	//CPhysicCore::Instance()->Reset();
	ITouch * touchLeft  = GetCurrTouchManager().GetTouch(DEVICETYPE_LEFT_NAME);
	ITouch * touchRight = GetCurrTouchManager().GetTouch(DEVICETYPE_RIGHT_NAME);
	if (touchLeft || touchRight)
	{
		GetCurrTouchManager().Termiante(DEVICETYPE_LEFT_NAME , DEVICETYPE_RIGHT_NAME);
	}

	ToolSpenetrateMgr::Destroy();
	EffectManager::Destroy();
	CScreenEffect::Destroy();
	CCollisionTools::Destroy();
	CSimpleUIManger::Destroy();

	MxSoundManager::GetInstance()->StopAllSound();

	InputSystem::GetInstance(DEVICETYPE_RIGHT)->Terminate();
	InputSystem::GetInstance(DEVICETYPE_LEFT)->Terminate();
	InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->Terminate();
	InputSystem::GetInstance(DEVICETYPE_KEYCODE)->Terminate();
	
	return true;
}

//void EngineCore::PhysicUpdate()
//{
	//CPhysicCore::Instance()->Update(m_fTimeSlice);
//}

void EngineCore::ProfilerUpdate()
{
	Ogre::RenderWindow* pRWnd = MXOgreWrapper::Get()->GetRenderWindowByName(RENDER_WINDOW_LARGE);
	if (pRWnd)
	{
		const Ogre::RenderTarget::FrameStats& stats = pRWnd->getStatistics();
		std::string RendFps = Ogre::StringConverter::toString(stats.avgFPS);
		std::string UpdateFps = Ogre::StringConverter::toString(m_UpdateFPS);

		std::string strCalibrationStatus = "\t" + Ogre::StringConverter::toString(m_bLeftTouchCalibrationFinished)
			+ "\t" + Ogre::StringConverter::toString(m_bRightTouchCalibrationFinished)
			+ "\t" + Ogre::StringConverter::toString(m_bCameraCalibrationFinished)
			+ "\t" + Ogre::StringConverter::toString(m_bCalibrationFinished);

		if (m_bShowProfiler)
		{
			CScreenEffect::Instance()->ShowNumber(UpdateFps + "\t" + RendFps + strCalibrationStatus);
		}
		else
		{
			CScreenEffect::Instance()->HideNumber();
		}
	}
}

void EngineCore::ProfilerUpdate( int fps )
{
	std::string strFps = Ogre::StringConverter::toString(fps);

	//CScreenEffect * pEffect = new CScreenEffect;
	//if (m_bShowProfiler)
	{
		CScreenEffect::Instance()->ShowNumber(strFps);
	}
	/*else
	{
	pEffect->HideNumber();
	}*/
}

void EngineCore::TouchUpdate()
{
	if (m_bCalibrationFinished)
	{
		return;
	}
	else
	{

		bool isValidLeft = false; //判断左右手设备是否连接在线
		bool isValidRight = false;

		//////////////////////////////////////////////////////////////////////////
		ITraining * pTraining = m_pTrainingMgr->GetCurTraining();
		if (NULL == pTraining)
		{
			return ;
		}
		if (pTraining->m_strName == "Basic1" || pTraining->m_strName == "Basic2" || pTraining->m_strName == "jieshi" || pTraining->m_strName == "jieshi_Scale" || pTraining->m_strName == "jieshi_Stay")
		{
			m_bLeftTouchCalibrationFinished = true;
			m_bRightTouchCalibrationFinished = true;
		}
		else if(pTraining->m_IsJustUseCamera)
		{
			m_bLeftTouchCalibrationFinished = true;
			m_bRightTouchCalibrationFinished = true;
		}
		else
		{
			Ogre::SceneNode* pTrocar = NULL;
			if (MXOgreWrapper::Get()->GetDefaultSceneManger()->hasSceneNode("trocar_gallbladder$1"))
			{
				pTrocar = MXOgreWrapper::Get()->GetDefaultSceneManger()->getSceneNode("trocar_gallbladder$1");
			}

			if (pTrocar)
			{
				m_bCameraCalibrationFinished = false;
			}else
			{
				m_bCameraCalibrationFinished = true;
			}
		}
		// m_bLeftTouchCalibrationFinished
		if (InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->m_InputDeviceType == InputPhantom)
		{
			//Get Touch State
			ITouch * touchLeft  = GetCurrTouchManager().GetTouch(DEVICETYPE_LEFT_NAME);
			if(touchLeft  && touchLeft-> m_hHD != HD_INVALID_HANDLE)
			{
				isValidLeft = true;
			}
			// 复位
			if (!m_bLeftTouchCalibrationFinished)
			{
				if (isValidLeft && GetCurrTouchManager().CheckCalibration(DEVICETYPE_LEFT_NAME))
				{
					GetCurrTouchManager().UpdateCalibration(DEVICETYPE_LEFT_NAME);
					m_bLeftTouchCalibrationFinished = true;
					cout << "UpdateCalibration1" << endl;
				}
			}
		}else if (InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->m_InputDeviceType == InputGyoscope)
		{	// 无力反馈拉起来亮屏
			isValidLeft = GlobalKeyValue::gUserGyroEnableDefault;
			if (!m_bLeftTouchCalibrationFinished)
			{
				//if (isValidLeft && GlobalKeyValue::gResetButtonLeft)
				{
					m_bLeftTouchCalibrationFinished = true;
				}
			}
		}
		else
		{
			isValidLeft = true;
			m_bLeftTouchCalibrationFinished = true;
		}



		// m_bRightTouchCalibrationFinished
		if (InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->m_InputDeviceType == InputPhantom)
		{
			ITouch * touchRight = GetCurrTouchManager().GetTouch(DEVICETYPE_RIGHT_NAME);
			if(touchRight && touchRight-> m_hHD != HD_INVALID_HANDLE)
			{
				isValidRight = true;
			}
			if (!m_bRightTouchCalibrationFinished)
			{
				if (isValidRight && GetCurrTouchManager().CheckCalibration(DEVICETYPE_RIGHT_NAME))
				{
					GetCurrTouchManager().UpdateCalibration(DEVICETYPE_RIGHT_NAME);
					m_bRightTouchCalibrationFinished = true;
					cout << "UpdateCalibration2" << endl;
				}
			}
		}	// 无力反馈拉起来亮屏
		else if (InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->m_InputDeviceType == InputGyoscope)
		{
			isValidRight = GlobalKeyValue::gUserGyroEnableDefault;

			if (!m_bRightTouchCalibrationFinished)
			{
				//if (isValidRight && GlobalKeyValue::gResetButtonRight)//ceshi
				{
					m_bRightTouchCalibrationFinished = true;
				}
			}
		}
		else
		{
			isValidRight = true;
			m_bRightTouchCalibrationFinished = true;
		}


		// m_bCameraCalibrationFinished
		if (InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDevice()->m_InputDeviceType == InputPhantom||
			InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDevice()->m_InputDeviceType == InputLaparoscope)
		{
			if ((!m_bCameraCalibrationFinished))
			{
				if (InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDistance() >= -0.1f)
				{
					m_bCameraCalibrationFinished = true;
				}
			}
		}
		else
		{
			m_bCameraCalibrationFinished = true;
		}


		if (m_bUnlock)
		{
			m_fGradualAlpha -= 0.02f;
			if (m_fGradualAlpha < 0.01f)
			{
				m_fGradualAlpha = 0.0f;
				m_bCalibrationFinished = true;

				if ( isValidLeft || isValidRight )
				{
					//GetCurrTouchManager().StopScheduler();
					bool touchleft = GetCurrTouchManager().EnableForce(DEVICETYPE_LEFT_NAME);
					bool touchright = GetCurrTouchManager().EnableForce(DEVICETYPE_RIGHT_NAME); 
					if (touchleft || touchright)
					{
						GetCurrTouchManager().StartScheduler();
					}
					//Sleep(300);//temply fix bug stop and start must have some interval for prevent not totaly stop
					InputSystem::GetInstance(DEVICETYPE_LEFT)->SetClampLimitFactor();
					InputSystem::GetInstance(DEVICETYPE_RIGHT)->SetClampLimitFactor();
				}
			}

			if (isValidLeft || isValidRight)
			{
				CScreenEffect::Instance()->SetScreenColor(Ogre::ColourValue(0x24 / 255.0f, 0x2B / 255.0f, 0x3C / 255.0f, m_fGradualAlpha), 5000.0f);
			}

		}
		else
		{
			if (m_bLeftTouchCalibrationFinished && m_bRightTouchCalibrationFinished && m_bCameraCalibrationFinished)
			{
				m_bUnlock = true;
				pTraining->OnTrainingIlluminated();
				InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->SetButton(false);
				CSimpleUIManger::Instance()->ShowUI();
			}
			if ( isValidLeft || isValidRight)
			{
				m_fGradualAlpha = 0.95f;
				CScreenEffect::Instance()->SetScreenColor(Ogre::ColourValue(0x24 / 255.0f, 0x2B / 255.0f, 0x3C / 255.0f, 0.95), 5000.0f);
			}
		}
	}
}

bool EngineCore::m_bUnlock = false;
