#include <WinSock2.h>
#include <stdlib.h>
#include <time.h>
#include <QSound>
#include <Qfontdatabase>
#include "MXApplication.h"
#include "RegClass.h"
#include "MxGlobalConfig.h"
#include <SYScoreTableManager.h>
#include "SYTrainTaskStruct.h"
#include "SYUserInfo.h"
#include "SYLoading.h"
#include "SYMessageBox.h"
#include "DataStoreService.h"
#include "tinyxml.h"
#include "SYMainWindow.h"
#include "SYStringTable.h"
#pragma comment(lib,"Ws2_32.lib")

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



MXApplication::MXApplication(int argc, char** argv)
	:QApplication(argc, argv)
{
	//init network
	int ret;
	WSADATA data;
	WORD word = MAKEWORD(2, 2);
	ret = WSAStartup(word, &data);
	if(ret || LOBYTE(data.wVersion) != 2 || HIBYTE(data.wHighVersion) != 2)
		MessageBoxA(NULL, "网络初始化失败！", NULL, MB_OK);
	srand(time(NULL));

	QDir::setCurrent(qApp->applicationDirPath());

	//register xml class
	RegXMLClass();

	setPaths();
	
	QString fontName = loadFontFamilyFromFiles("..\\res\\Skin\\SourceHanSansCN-Normal.ttf");
	
	QFont font(fontName);
	setFont(font);
	
	m_AutoUpdateTimer = new QTimer;

	m_MessageProcessTimer = new QTimer;

	bool bRet = QObject::connect(m_AutoUpdateTimer, SIGNAL(timeout()), this, SLOT(Update()));

	QObject::connect(m_MessageProcessTimer, SIGNAL(timeout()), this, SLOT(ProcessMessages()));

	m_IsActive = false;

	m_updateinterval = 0;

	m_communicator = 0;

	m_SubModuleLoadingUI = 0;

	StartupProcessCommunicator("SYLapModulePipe");
	m_communicator->AddMessageListener(this);

	SYStringTable::Intialize();

	SYTaskTrainDataMgr::GetInstance().Initialize();
	
	SYScoreTableManager::GetInstance()->Initialize();
}

MXApplication::~MXApplication(void)
{
	disconnect(m_AutoUpdateTimer, SIGNAL(timeout()), this, SLOT(Update()));
	disconnect(m_MessageProcessTimer, SIGNAL(timeout()), this, SLOT(ProcessMessages()));


	SAFE_DELETE(m_AutoUpdateTimer);
	SAFE_DELETE(m_MessageProcessTimer);


	WSACleanup();

	//	InceptionImp::Destroy();
	if(m_communicator)
	{
		m_communicator->RemoveMessageListener(this);
		delete m_communicator;
		m_communicator = 0;
	}
}

int MXApplication::LaunchModule(const std::string & modulename,
								const QString & trainName,
								const QString & trainChName,
								const QString & trainCat,
								const QString & trainCode,
								int TaskRecordID,
								int trainTaskID,
								bool showLoadingUI)
{
	if (m_communicator == 0)
	{
		return -1;
	}

	m_SubModuleName = modulename;

	QString username = SYUserInfo::Instance()->GetUserName();
	QString realname = SYUserInfo::Instance()->GetRealName();
	int userid = SYUserInfo::Instance()->GetUserId();
	int userpermission = SYUserInfo::Instance()->GetUserPermission();
	int groupId = SYUserInfo::Instance()->GetGroupId();
	//user data organize in XML
	TiXmlElement userNode("userdata");
	userNode.SetAttribute("username", username.toStdString());
	userNode.SetAttribute("realName", realname.toStdString());
	userNode.SetAttribute("userid", userid);
	userNode.SetAttribute("userpermission", userpermission);
	userNode.SetAttribute("groupId", groupId);

	TiXmlElement trainNode("traindata");
	trainNode.SetAttribute("trainName", trainName.toStdString());
	trainNode.SetAttribute("trainChName", trainChName.toStdString());
	trainNode.SetAttribute("trainCategry", trainCat.toStdString());
	trainNode.SetAttribute("trainCode", trainCode.toStdString());
	trainNode.SetAttribute("TaskRecordID", TaskRecordID);
	trainNode.SetAttribute("trainTaskID", trainTaskID);


	TiXmlElement rootNode("shareData");
	rootNode.InsertEndChild(userNode);
	rootNode.InsertEndChild(trainNode);

	TiXmlPrinter printer;
	rootNode.Accept(&printer);
	const char * xmlcstr = printer.CStr();
	m_SubModuleParam = xmlcstr;
	//

	if (showLoadingUI)
	{
		if (m_SubModuleLoadingUI == 0)
		{
			m_SubModuleLoadingUI = new SYLoading(0);
			QDesktopWidget *desktop = QApplication::desktop();
			int screenIndex = desktop->primaryScreen();
			QRect dw = desktop->screenGeometry(screenIndex);
			QRect rtScreen = dw;
			m_SubModuleLoadingUI->move((rtScreen.width() - m_SubModuleLoadingUI->width()) / 2, (rtScreen.height() - m_SubModuleLoadingUI->height()) / 2);
		}

		::SetWindowPos(HWND(m_SubModuleLoadingUI->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		::SetWindowPos(HWND(m_SubModuleLoadingUI->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		m_SubModuleLoadingUI->Play();
		m_SubModuleLoadingUI->show();

		QWidget* mainWindow = SYMainWindow::GetInstance();
		mainWindow->setEnabled(false);//防止重复点击按钮

		QTimer::singleShot(100, this, SLOT(InternalLaunchSubModule()));
	}
	else
	{
		DWORD errorCode = m_communicator->LaunchProcess(modulename.c_str(), "", (void*)xmlcstr, strlen(xmlcstr) + 1);

		if (errorCode == 0)
		{
			QWidget* mainWindow = SYMainWindow::GetInstance();
			mainWindow->setEnabled(false);
		}

		return (int)errorCode;
	}

	return 0;
}

void MXApplication::InternalLaunchSubModule()
{
	bool launchSucced = false;
	DWORD errorCode = 0;

	if (m_communicator)
	{
		const char * xmlcstr = m_SubModuleParam.c_str();
		errorCode = m_communicator->LaunchProcess(m_SubModuleName.c_str(), "", (void*)xmlcstr, strlen(xmlcstr) + 1);

		if (errorCode == 0)
		{
			launchSucced = true;
		}
	}
	if (launchSucced == false)
	{
		m_SubModuleLoadingUI->Stop();
		m_SubModuleLoadingUI->hide();

		QWidget* mainWindow = SYMainWindow::GetInstance();
		mainWindow->setEnabled(true);
		//
		SYMessageBox * messageBox = new SYMessageBox(SYMainWindow::GetInstance(), CHS("技能训练模块启动失败"), CHS("error code : %1").arg(errorCode));
		messageBox->showFullScreen();
		messageBox->exec();
	}
}
void MXApplication::TerminateSubMoudle()
{
	if (m_communicator)
	    m_communicator->TerminateChildProcess();
}

int MXApplication::LaunchQuestionModuleProcess()
{
	return LaunchModule("QuestionModule.exe", "", "", "","",-1,-1,false);
}

void MXApplication::StartupProcessCommunicator(const std::string& communicationName)
{
	if(m_communicator == nullptr)
	{
		m_communicator = new MxProcessCommunicator(communicationName, true);
		m_communicator->Startup();
		m_MessageProcessTimer->start(100);

		//m_communicator->SetListener(this);
		//connect(m_communicator, &MxProcessCommunicator::ReceiveMessage, this, &MXApplication::OnReceiveMessage);
		//connect(m_communicator, &MxProcessCommunicator::ReceiveStop, this, &MXApplication::OnReceiveStop);
	}
}

MxProcessCommunicator* MXApplication::getCommunicator()
{
	return m_communicator;
}

void MXApplication::StopDataServiceCallBack(int retCode, char *pUrl)
{
	OnlineGradeCallBackMsg* msg = new OnlineGradeCallBackMsg();

	msg->m_retCode = retCode;

	size_t lenUrl = strlen(pUrl) + 1;
	strcpy_s(msg->m_strUrl, lenUrl, pUrl);

	MxProcessCommunicator* pCommnunicator = (static_cast<MXApplication*>(qApp))->getCommunicator();
	pCommnunicator->SendMessage(msg);

	delete msg;
}

void MXApplication::LogoutCallBack()
{
	//QMessageBox::information(NULL, QString::fromLocal8Bit("超时注销"), QString::fromLocal8Bit("超时注销"), QMessageBox::Yes);
	//std::cout << "@@@@@@@@@@@@@@@LogoutCallBack@@@@@@@@@@@@@@" << endl;

	//MXApplication* pMXApp = (static_cast<MXApplication*>(qApp));
	//pMXApp->OnReceiveSubModuleTerminate();

	SYMainWindow::GetInstance()->ReturnToLoginWindow();

}

void MXApplication::OnReceiveCommunicatorMessage(MXPCMessageType type, MxProcessCommunicateMsg * msg)
{
	if(!msg)
	   return;

	if(msg->m_MsgHeader.m_MsgType == MT_PING)
	{
		QWidget* mainWindow = SYMainWindow::GetInstance();
		mainWindow->hide();
		if (m_SubModuleLoadingUI)
		{
			m_SubModuleLoadingUI->Stop();
			m_SubModuleLoadingUI->hide();
		}
		//QTimer::singleShot(200, mainWindow, SLOT(hide()));
	}
	else if (msg->m_MsgHeader.m_MsgType == MT_SHOW_PARENT_WINDOW)
	{
		SYMainWindow * mainWindow = SYMainWindow::GetInstance();
		
		ShowParentWindowMsg * showMsg = (ShowParentWindowMsg*)msg;
		
		if (showMsg->m_ShowWinType >= 0)
		{
			mainWindow->onShowNextWindow((WindowType)showMsg->m_ShowWinType);
		}

		if (mainWindow->isVisible() == false)
		{
			mainWindow->setEnabled(true);

			::SetWindowPos(HWND(mainWindow->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
			::SetWindowPos(HWND(mainWindow->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

			mainWindow->show();
			mainWindow->activateWindow();
		}

		//QTimer::singleShot(1000, this, SLOT(TerminateSubMoudle()));//一秒后强行关闭子进程（ 一秒内子进程有机会自己关闭）
	}
	else if (msg->m_MsgHeader.m_MsgType == MT_STARTRECORDSCREEN)
	{
		StartRecordTrainMsg* msgReal = (StartRecordTrainMsg*)msg;
		QString FileName = QString::fromUtf8(msgReal->m_RecordFileName);
		
		//
		int i = 0;
		int j = i + 1;

		//
	}
	else if (msg->m_MsgHeader.m_MsgType == MT_STOPTRECORDSCREEN)
	{
		StartRecordTrainMsg* msgReal = (StartRecordTrainMsg*)msg;
		QString FileName = QString::fromUtf8(msgReal->m_RecordFileName);

		//
		int i = 0;
		int j = i + 1;

		//

	}
	/*
	else if(msg->m_MsgHeader.m_MsgType == MT_STARTONLINEGRAD)
	{
		StartOnlineGradeMsg* msgReal = (StartOnlineGradeMsg*)msg;
		QString sheetCode = msgReal->m_sheetCode;
		MX::start(sheetCode.toLocal8Bit().data());
	}

	else if(msg->m_MsgHeader.m_MsgType == MT_PUSHONLINEGRADSCORE)
	{
		PushOnlineCodeMsg* msgReal = (PushOnlineCodeMsg*)msg;
		MX::pushScoreCode(msgReal->m_Code, msgReal->m_StartTime, msgReal->m_EndTime);
	}

	else if(msg->m_MsgHeader.m_MsgType == MT_STOPONLINEGRAD)
	{
		MX::stop(StopDataServiceCallBack);
	}
	*/
}

void MXApplication::OnReceiveSubModuleTerminate()
{
	SYMainWindow * mainWindow = SYMainWindow::GetInstance();

	if (mainWindow->isVisible() == false)
	{
		mainWindow->setEnabled(true);

		::SetWindowPos(HWND(mainWindow->winId()), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		::SetWindowPos(HWND(mainWindow->winId()), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

		mainWindow->show();
		mainWindow->activateWindow();
	}

	TerminateSubMoudle();//mark as terminated
}

void MXApplication::setPaths()
{
	//set style sheet path
	QDir::addSearchPath("qss", MxGlobalConfig::Instance()->GetStyleSheetDir());
	QDir::addSearchPath("icons", MxGlobalConfig::Instance()->GetSkinDir());
	QDir::addSearchPath("video", MxGlobalConfig::Instance()->GetVideoDir());
	QDir::addSearchPath("audio", MxGlobalConfig::Instance()->GetAudioDir());
}
void MXApplication::PauseUpdateLoop()
{

}

void MXApplication::ResumeUpdateLoop()
{

}

void MXApplication::Update()
{

}

void MXApplication::ProcessMessages()
{
	m_communicator->ProcessMessages(0.1f, 10);
}

void MXApplication::Initialize()
{

}

void MXApplication::Terminate()
{

}

// bool MXApplication::winEventFilter( MSG * msg, long * result )
// {
// 	switch (msg->message)
// 	{
// 	case WM_LBUTTONDOWN:
// 		{
// 			//QSound::play(MxGlobalConfig::Instance()->GetAudioDir() + "/button_p.wav" ); 
// 		}
// 		break;
// 	}
// 	return __super::winEventFilter(msg,result);
// }