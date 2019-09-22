#include <WinSock2.h>
#include <stdlib.h>
#include <time.h>
#include <QSound>
#include <Qfontdatabase>
#include "StdAfx.h"
#include "MXApplication.h"
#include "MXOgreWrapper.h"
#include "InceptionImp.h"
#include "../MXRobot/Common/PerformanceDebug.h"
#include "../MXRobot/ResourceManager/ResourceManager.h"
#include "../MXCommon/include/MxSoundManager.h"
#include "MisRobotInput.h"
#include "InputSystem.h"
#include "RegClass.h"
#include "MxGlobalConfig.h"
#include "SYUserInfo.h"
#include "MxProcessCommunicator.h"
#include "SYTrainWindow.h"
#include "SYStringTable.h"
#include <SYScoreTableManager.h>
#pragma comment(lib,"Ws2_32.lib")


QWidget * g_extAppWindow = 0;

extern QEvent::Type loadfinishEventType;

QString loadFontFamilyFromFiles(const QString &fontFileName)
{
	static QHash<QString, QString> tmd;
	if (tmd.contains(fontFileName)) {
		return tmd.value(fontFileName);
	}
	QString font = "";
	QFile fontFile(fontFileName);
	if (!fontFile.open(QIODevice::ReadOnly)) {
		qDebug() << "Open font file error";
		return font;
	}

	QByteArray content = fontFile.readAll();

	int loadedFontID = QFontDatabase::addApplicationFontFromData(content);
	QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
	if (!loadedFontFamilies.empty()) {
		font = loadedFontFamilies.at(0);
	}
	fontFile.close();

	if (!(font.isEmpty()))
		tmd.insert(fontFileName, font);
	return font;
}



MXApplication::MXApplication(int argc, 
	                         char** argv)
:QApplication(argc, argv)
{
	//init network
	int ret;
	WSADATA data;
	WORD word = MAKEWORD(2, 2);
	ret = WSAStartup(word, &data);
	if (ret || LOBYTE(data.wVersion) != 2 || HIBYTE(data.wHighVersion) != 2)
		MessageBoxA(NULL,"网络初始化失败！",NULL,MB_OK);
	srand(time(NULL));

	QDir::setCurrent(qApp->applicationDirPath());

	//register xml class
	RegXMLClass();

	setPaths();

	QString fontName = loadFontFamilyFromFiles("..\\res\\Skin\\SourceHanSansCN-Normal.ttf");

	setFont(QFont(fontName));

	InceptionImp::Create();

	//CPerformanceDebug::Init();

	m_pEngineCore = new EngineCore;
	/*m_DegreeMenu = new RbDegreeMenu(QString(""),false,0);
	m_DegreeMenu->setPositon(QPoint(867,0));*/
	
	m_AutoUpdateTimer = new QTimer;
	m_MessageProcessTimer = 0;
	MXOgreWrapper::Create();

	//load score table from database
	SYStringTable::Intialize();
	SYScoreTableManager::GetInstance()->Initialize();

	bool bRet = QObject::connect(m_AutoUpdateTimer, SIGNAL(timeout()), this, SLOT(Update()));
	
	m_IsActive = false;

	m_updateinterval = 0;

	m_communicator = 0;

	m_IsDisabled = false;

	m_HasPingMsgSended = false;

	m_TrainWindow = 0;
}

MXApplication::~MXApplication(void)
{
	disconnect(m_AutoUpdateTimer, SIGNAL(timeout()), this, SLOT(Update()));
	
    SAFE_DELETE( m_pEngineCore );
    SAFE_DELETE( m_AutoUpdateTimer );
	
	if (m_MessageProcessTimer)
	{
		disconnect(m_MessageProcessTimer, SIGNAL(timeout()), this, SLOT(ProcessMessages()));
		SAFE_DELETE(m_MessageProcessTimer);
	}
	
	MXOgreWrapper::Destroy();
	//CPerformanceDebug::Release();
	WSACleanup();

	InceptionImp::Destroy();
	//if (m_communicator)
	//{
	//	m_communicator->RemoveMessageListener(this);
	//	delete m_communicator;
	//	m_communicator = 0;
	//}
}

void MXApplication::setPaths()
{
	//set style sheet path
	QDir::addSearchPath("qss",MxGlobalConfig::Instance()->GetStyleSheetDir());
	QDir::addSearchPath("icons",MxGlobalConfig::Instance()->GetSkinDir());
	QDir::addSearchPath("video",MxGlobalConfig::Instance()->GetVideoDir());
	QDir::addSearchPath("audio",MxGlobalConfig::Instance()->GetAudioDir());
}
void MXApplication::PauseUpdateLoop()
{
	if (!m_IsActive)
	{
		return;
	}
	m_AutoUpdateTimer->stop();
	MxSoundManager::GetInstance()->MuteDevice(true);
}

void MXApplication::ResumeUpdateLoop()
{
	if (!m_IsActive)
	{
		return;
	}
	m_AutoUpdateTimer->start(m_updateinterval);
	MxSoundManager::GetInstance()->MuteDevice(false);
}

void MXApplication::Update()
{
	if (!m_IsActive)//Inception::Instance()->GetRenderCircleState())
	{
		//Terminate();
		return;
	}

#if defined(TEST_EFFICIENCY)
	clock_t now1 = clock();
#endif

	//PERFORMANCE_DEBUG_BEGIN_BY_NAME(m_pEngineCore)
	float dt = m_pEngineCore->Update();

	ITraining * train = m_pEngineCore->GetTrainingMgr()->GetCurTraining();

	//PERFORMANCE_DEBUG_END_BY_NAME(m_pEngineCore)
	//PERFORMANCE_DEBUG_BEGIN_BY_NAME(MXOgreWrapper)
	if (train)
	{
		MXOgreWrapper::Get()->Update(train);
	}
	//PERFORMANCE_DEBUG_END_BY_NAME(MXOgreWrapper)

#if defined(TEST_EFFICIENCY)
	clock_t now2 = clock();
	double updateTime = ( (double)(now2 - now1) ) / (double) CLOCKS_PER_SEC;
	m_gWriteFile<<"主程序更新时间"<<updateTime<<"秒"<<"\n";
#endif

	//check device select menu
	int MsgLeft   = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->GetDeviceMessageType();
	int MsgRight  = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->GetDeviceMessageType();
	int MsgCamera = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDevice()->GetDeviceMessageType();

	if (MsgLeft >= 0)
	{
		int subMsg = InputSystem::GetInstance(DEVICETYPE_LEFT)->GetDevice()->GetDeviceSubMessage();
		Inception::Instance()->EmitToolSelect(DEVICETYPE_LEFT, MsgLeft, subMsg);
	}
	if (MsgRight >= 0)
	{
		int subMsg = InputSystem::GetInstance(DEVICETYPE_RIGHT)->GetDevice()->GetDeviceSubMessage();
		Inception::Instance()->EmitToolSelect(DEVICETYPE_RIGHT, MsgRight, subMsg);
	}
	
	bool IsCameraPullOut = InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetDevice()->IsPullOut();
	
	//if(MsgCamera==1)
	//Inception::Instance()->EmitDegreeSelect(MsgCamera);
	if (m_TrainWindow)
	    m_TrainWindow->SetCameraPullOutImgVisible(IsCameraPullOut);
	//
	m_communicator->ProcessMessages(dt, 10);
}

void MXApplication::ProcessMessages()
{
	m_communicator->ProcessMessages(0.1f, 10);
}

void MXApplication::OnTrainLoadingFinished()
{
	// start Loop
	m_AutoUpdateTimer->start(m_updateinterval);

	//mark as need update
	m_IsActive = true;
}
//=================================================================================================
void MXApplication::StartLoadingTrain(const std::string& trainingName, QWidget * invokeWidget)
{
	g_extAppWindow = invokeWidget;
	m_LoadingThread.SetTrainLoadInfo(m_pEngineCore, trainingName);
	m_LoadingThread.start();
}
//=================================================================================================
void MXApplication::Terminate()
{
	m_AutoUpdateTimer->stop();
	
	m_pEngineCore->Terminate();
	
	MXOgreWrapper::Get()->Terminate();
	
	m_IsActive = false;

	m_MessageProcessTimer = new QTimer;
	QObject::connect(m_MessageProcessTimer, SIGNAL(timeout()), this, SLOT(ProcessMessages()));
	m_MessageProcessTimer->start(100);
}

void MXApplication::StartupProcessCommunicator(const std::string& communicationName,
	                                           QString &QtrainCategoryName,
	                                           QString &QtrainEnName,
	                                           QString &QtrainChName,
											   QString &QtrainCode,
											   int & TaskRecordID,
											   int & TaskTrainID)
{
	if (m_communicator == nullptr)
	{
		m_communicator = new MxProcessCommunicator(communicationName, false);
		bool succeed = m_communicator->Startup();

		if (succeed)
		{
			char strBuffer[1024 * 100];
			int ReadByte = m_communicator->ReadFromSharedMemory(strBuffer, 1024 * 100);

			TiXmlDocument xmldoc;
			xmldoc.Parse(strBuffer);
			TiXmlElement * rootElement = xmldoc.RootElement();

			if (rootElement)
			{
				TiXmlElement * userData = rootElement->FirstChildElement("userdata");

				//user data
				int userId;
				int permission;
				std::string username = userData->Attribute("username");
				userData->Attribute("userid", &userId);
				userData->Attribute("userpermission", &permission);
				std::string userName = userData->Attribute("username");
				std::string realName = userData->Attribute("realName");

				SYUserInfo::Instance()->m_userId = userId;
				SYUserInfo::Instance()->m_userName = QString::fromStdString(userName);
				SYUserInfo::Instance()->m_realName = QString::fromStdString(realName);
				SYUserInfo::Instance()->m_permission = (UserPermission)permission;

				//train data
				TiXmlElement * trainData = rootElement->FirstChildElement("traindata");
				if (trainData)
				{
					std::string trainCat = trainData->Attribute("trainCategry");
					std::string trainName = trainData->Attribute("trainName");
					std::string trainChName = trainData->Attribute("trainChName");
					std::string trainCode = trainData->Attribute("trainCode");

					TaskRecordID = -1;
					TaskTrainID = -1;
					trainData->Attribute("TaskRecordID", &TaskRecordID);
					trainData->Attribute("trainTaskID", &TaskTrainID);

					QtrainCategoryName = QString::fromStdString(trainCat);
					QtrainEnName = QString::fromStdString(trainName);
					QtrainChName = QString::fromStdString(trainChName);
					QtrainCode = QString::fromStdString(trainCode);
				}
			}

			m_communicator->AddMessageListener(this);
			m_communicator->StartReceiveMessage();
		}
	}
}

MxProcessCommunicator* MXApplication::getCommunicator()
{
	return m_communicator;
}

void MXApplication::SendPintMsgToParentModule()
{
	//send ping message to note parent process launch succed
	PingMsg * msg = new PingMsg();
	m_communicator->SendMessage(msg);
	delete msg;

	m_HasPingMsgSended = true;
}

void MXApplication::exitModuleAndShowParentWindow(QWidget * trainWindow)
{
	if (m_IsDisabled)
		return;

	if (m_HasPingMsgSended == false)
		return;

	//send ping message to note parent process launch succed
	if (m_communicator)
	{
		ShowParentWindowMsg * msg = new ShowParentWindowMsg(-1);
		m_communicator->SendMessage(msg);
		delete msg;
	}

	//do real quit
	QTimer::singleShot(500, trainWindow, SLOT(close()));
	m_IsDisabled = true;
}

//loading thread 
LoadingThread::LoadingThread()
{

}

void LoadingThread::SetTrainLoadInfo(EngineCore * pEngineCore, const std::string & trainname)
{
	m_trainingName = trainname;
	m_pEngineCore = pEngineCore;
}

void LoadingThread::run()
{
	//read train config first because we need know addition resource information
	Ogre::vector<Ogre::String>::type AdditionResLocations;

	bool result = CResourceManager::Instance()->LoadResource(AdditionResLocations);

	// init Ogre with additional resource
	MXOgreWrapper::Get()->Initialize(AdditionResLocations);

	// init EngineCore
	m_pEngineCore->Initialize();

	//mark as need update

	//LoadingFinishEvent * event = new LoadingFinishEvent(loadfinishEventType);

	QApplication::postEvent(g_extAppWindow, new QEvent(QEvent::Type(FINISHED_LOADING_EVENT)));
}

void MXApplication::OnReceiveCommunicatorMessage(MXPCMessageType type, MxProcessCommunicateMsg * msg)
{
	if (!msg)
		return;

	if (msg->m_MsgHeader.m_MsgType == MT_ONLINEGRADCALLBACK)
	{
		OnlineGradeCallBackMsg* msgReal = (OnlineGradeCallBackMsg*)msg;		
		SYTrainWindow::StopDataServiceCallBack(msgReal->m_retCode, msgReal->m_strUrl);
	}

}

void MXApplication::OnReceiveSubModuleTerminate()
{

}