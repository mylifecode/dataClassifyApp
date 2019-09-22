#include <WinSock2.h>
#include <stdlib.h>
#include <time.h>
#include <QSound>
#include <Qfontdatabase>
#include "RegClass.h"
#include "MxQuestionModuleApp.h"
#include "MxProcessCommunicator.h"
#include "tinyxml.h"
#include "MxGlobalConfig.h"
#include "SYMainWindow.h"
#include "SYExamGlobal.h"
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

MxQuestionModuleApp::MxQuestionModuleApp(int argc, char** argv)
:QApplication(argc, argv)
{
	//init network
	int ret;
	WSADATA data;
	WORD word = MAKEWORD(2, 2);
	ret = WSAStartup(word, &data);
	if (ret || LOBYTE(data.wVersion) != 2 || HIBYTE(data.wHighVersion) != 2)
		MessageBoxA(NULL,"ÍøÂç³õÊ¼»¯Ê§°Ü£¡",NULL,MB_OK);
	srand(time(NULL));

	QDir::setCurrent(qApp->applicationDirPath());

	//register xml class
	RegXMLClass();

	setPaths();
	
	QString fontName = loadFontFamilyFromFiles("..\\res\\Skin\\SourceHanSansCN-Normal.ttf");

	QFont font(fontName);
	setFont(font);

	m_IsDisabled = false;

	m_updateinterval = 0;

	m_communicator = 0;

	m_HasPingMsgSended = false;

	StartupProcessCommunicator("SYLapModulePipe");
}

MxQuestionModuleApp::~MxQuestionModuleApp(void)
{
    WSACleanup();
}

void MxQuestionModuleApp::setPaths()
{
	//set style sheet path
	//QDir::addSearchPath("qss", "../config/StyleSheet/StyleSheet1");
	//QDir::addSearchPath("icons","../res/Skin/SkinConfig3");

	QDir::addSearchPath("qss", MxGlobalConfig::Instance()->GetStyleSheetDir());
	QDir::addSearchPath("icons", MxGlobalConfig::Instance()->GetSkinDir());
	QDir::addSearchPath("video", MxGlobalConfig::Instance()->GetVideoDir());
	QDir::addSearchPath("audio", MxGlobalConfig::Instance()->GetAudioDir());
	//QDir::addSearchPath("video",MxGlobalConfig::Instance()->GetVideoDir());
	//QDir::addSearchPath("audio",MxGlobalConfig::Instance()->GetAudioDir());
}
//void MxQuestionModuleApp::PauseUpdateLoop()
//{
	//if (!m_IsActive)
	//{
	//	return;
	//}
	//m_AutoUpdateTimer->stop();
	//MxSoundManager::GetInstance()->MuteDevice(true);
//}

//void MxQuestionModuleApp::ResumeUpdateLoop()
//{
	//if (!m_IsActive)
	//{
	//	return;
	//}
	//m_AutoUpdateTimer->start(m_updateinterval);
	//MxSoundManager::GetInstance()->MuteDevice(false);
//}

void MxQuestionModuleApp::Update()
{
	
	
}

//void MxQuestionModuleApp::Initialize()
//{
	
	//mark as need update
	//m_IsActive = true;
//}

//void MxQuestionModuleApp::Terminate()
//{
	
	
	//m_IsActive = false;
//}

void MxQuestionModuleApp::StartupProcessCommunicator(const std::string& communicationName)
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

				g_UserInfor.m_userid = userId;
				
				g_UserInfor.m_name = QString::fromStdString(username);
				
				if (permission >= UP_Teacher)
				{

				}
				else
				{
					
				}
			}

		}
	}
}

void MxQuestionModuleApp::SendPintMsgToParentModule()
{
	//send ping message to note parent process launch succed
	if (m_communicator)
	{
		PingMsg * msg = new PingMsg();
		m_communicator->SendMessage(msg);
		delete msg;
	}
	m_HasPingMsgSended = true;

}

void MxQuestionModuleApp::exitModuleAndShowParentWindow(int pWindowToShow)
{
	if (m_IsDisabled)
		return;

	if (m_HasPingMsgSended == false)
		return;

	//send ping message to note parent process launch succed
	if (m_communicator)
	{
		ShowParentWindowMsg * msg = new ShowParentWindowMsg(pWindowToShow);
		m_communicator->SendMessage(msg);
		delete msg;
	}

	//do real quit
	QTimer::singleShot(800, SYMainWindow::GetInstance(), SLOT(close()));
	m_IsDisabled = true;
}
	