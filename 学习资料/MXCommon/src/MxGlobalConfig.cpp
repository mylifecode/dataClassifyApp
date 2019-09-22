#include "MxGlobalConfig.h"
#include "XMLWrapperGlobalConfig.h"
#include <cassert>
#include "XMLSerialize.h"

MxGlobalConfig::MxGlobalConfig(void)
	:m_xmlFileName("../Config/globalConfig.xml"),
	//dir
	m_skinDir(""),
	m_styleSheetDir(""),
	m_toolIconDir(""),
	m_videoDir(""),
	m_audioDir(""),
	//config file name
	m_databaseFileName(""),
	m_toolXmlConfigFileName(""),
	//m_toolForTaskXmlConfigFileName(""),
	//m_degreeListXmlConfigFileName(""),
	m_courseTrainXmlConfigFileName(""),
	m_selectModuleXmlConfigFileName(""),
	m_knowledgeLibConfigFile(""),
	//hardware
	m_enabledForceFeedback(false),
	m_left_shaft_k(0.1f),
	m_left_shaft_b(1.f),
	m_right_shaft_k(0.2f),
	m_right_shaft_b(2.f),
	//vr
	m_isVR(false),
	//video
	m_enabledRecord(false),
	m_videoFrameRate(50),
	m_videoWidth(1024),
	m_videoHeight(768),
	m_videoSaveDir(""),
	m_useHardwareEncode(false),
	//network
	m_serverIp("127.0.0.1"),
	m_serverPort(8000),
	m_clientPort(8100),
	m_packageSize(60 * 1024),
	m_sendFrequency(10),
	m_multicastGroupAddress("239.0.0.1"),
	m_multicastMode(false),
	m_decodeDelayTime(0),
	m_enabledSendScreenData(false),
	//database
	m_databaseHostName("SYServer"),
	m_databasePort(3306),
	m_databaseName("SYdatabase"),
	m_databaseUserName("suoyan"),
	m_databasePassword("sy.3721"),
	//other dir and file name
	m_curTrainCaseDir(""),
	m_curTrainCaseFileName(""),
	m_selectLogin(false),
	m_onLineConfig(false),
	m_scanUrl(""),
	//mode
	m_userMode(0),

	m_RealTrainModule("..\\data\\RTModule\\ACRealObjectModule.exe"),
	m_VirtualTrainModule("MxTrain.exe"),
    m_QuestionModule("QuestioinModule.exe")

{
	Initialize();
}

MxGlobalConfig::~MxGlobalConfig(void)
{

}

MxGlobalConfig* MxGlobalConfig::Instance()
{
	static MxGlobalConfig globalPath;
	return &globalPath;
}

void MxGlobalConfig::Initialize()
{
	CXMLContentManager xmlContentManager;
	CXMLWrapperGlobalConfig * wrapper =static_cast<CXMLWrapperGlobalConfig*>(xmlContentManager.Load(m_xmlFileName.toStdString()));
	assert(wrapper);

	//init member
	//dir
	if(wrapper->m_flag_SkinDir)
		m_skinDir.append(wrapper->m_SkinDir.c_str());	
	if(wrapper->m_flag_StyleSheetDir)
		m_styleSheetDir.append(wrapper->m_StyleSheetDir.c_str());
	if(wrapper->m_flag_ToolIconDir)
		m_toolIconDir.append(wrapper->m_ToolIconDir.c_str());
	if(wrapper->m_flag_VideoDir)
		m_videoDir.append(wrapper->m_VideoDir.c_str());
	if(wrapper->m_flag_AudioDir)
		m_audioDir.append(wrapper->m_AudioDir.c_str());

	//config file name
	if(wrapper->m_flag_ToolXMLConfigFileName)
		m_toolXmlConfigFileName.append(wrapper->m_ToolXMLConfigFileName.c_str());
	if(wrapper->m_flag_DatabaseFileName)
		m_databaseFileName.append(wrapper->m_DatabaseFileName.c_str());
	//if(wrapper->m_flag_ToolForTaskXMLConfigFileName)
	//	m_toolForTaskXmlConfigFileName.append(wrapper->m_ToolForTaskXMLConfigFileName.c_str());
	//if(wrapper->m_flag_DegreeListXMLConfigFileName)
	//	m_degreeListXmlConfigFileName.append(wrapper->m_DegreeListXMLConfigFileName.c_str());
	if(wrapper->m_flag_CourseTrainXMLConfigFileName)
		m_courseTrainXmlConfigFileName.append(wrapper->m_CourseTrainXMLConfigFileName.c_str());
	if (wrapper->m_flag_SelectModuleXMLConfigFileName)
		m_selectModuleXmlConfigFileName.append(wrapper->m_SelectModuleXMLConfigFileName.c_str());
	if(wrapper->m_flag_KnowledgeLibConfigFile)
		m_knowledgeLibConfigFile.append(wrapper->m_KnowledgeLibConfigFile.c_str());
	if(wrapper->m_flag_SurgeryTrainConfigFile)
		m_surgeryTrainConfigFile.append(wrapper->m_SurgeryTrainConfigFile.c_str());
	if (wrapper->m_flag_ScanUrl)
		m_scanUrl.append(wrapper->m_ScanUrl.c_str());

	//hardware
	if(wrapper->m_flag_EnableForceFeedback)
		m_enabledForceFeedback = wrapper->m_EnableForceFeedback;
	if(wrapper->m_flag_Left_Shaft_K)
		m_left_shaft_k = wrapper->m_Left_Shaft_K;
	if(wrapper->m_flag_Left_Shaft_B)
		m_left_shaft_b = wrapper->m_Left_Shaft_B;
	if(wrapper->m_flag_Right_Shaft_K)
		m_right_shaft_k = wrapper->m_Right_Shaft_K;
	if(wrapper->m_flag_Right_Shaft_B)
		m_right_shaft_b = wrapper->m_Right_Shaft_B;

	//vr
	if(wrapper->m_flag_VR)
		m_isVR = wrapper->m_VR;

	//video
	if(wrapper->m_flag_EnableRecord)
		m_enabledRecord = wrapper->m_EnableRecord;
	if(wrapper->m_flag_VideoFrameRate)
		m_videoFrameRate = wrapper->m_VideoFrameRate;
	if(wrapper->m_flag_VideoWidth)
		m_videoWidth = wrapper->m_VideoWidth;
	if(wrapper->m_flag_VideoHeight)
		m_videoHeight = wrapper->m_VideoHeight;
	if(wrapper->m_flag_VideoSaveDir)
		m_videoSaveDir.append(wrapper->m_VideoSaveDir.c_str());
	if(wrapper->m_flag_HardwareEncode)
		m_useHardwareEncode = wrapper->m_HardwareEncode;

	//network
	if(wrapper->m_flag_ServerIp)
	{
		m_serverIp.clear();
		m_serverIp.append(wrapper->m_ServerIp.c_str());
	}
	if(wrapper->m_flag_ServerPort)
		m_serverPort = wrapper->m_ServerPort;
	if(wrapper->m_flag_ClientPort)
		m_clientPort = wrapper->m_ClientPort;
	if(wrapper->m_flag_PackageSize)
		m_packageSize = wrapper->m_PackageSize;
	if(wrapper->m_flag_SendFrequency)
		m_sendFrequency = wrapper->m_SendFrequency;
	if(wrapper->m_flag_MulticastGroupAddress)
	{
		m_multicastGroupAddress.clear();
		m_multicastGroupAddress.append(wrapper->m_MulticastGroupAddress.c_str());
	}
	if(wrapper->m_flag_MulticastMode)
		m_multicastMode = wrapper->m_MulticastMode;
	if(wrapper->m_flag_DecodeDelayTime)
		m_decodeDelayTime = std::max(0L, wrapper->m_DecodeDelayTime);
	if(wrapper->m_flag_EnableSendScreenData)
		m_enabledSendScreenData = wrapper->m_EnableSendScreenData;

	//database
	if(wrapper->m_flag_DatabaseHostName)
	{
		m_databaseHostName.clear();
		m_databaseHostName.append(wrapper->m_DatabaseHostName.c_str());
	}
	if(wrapper->m_flag_DatabasePort)
		m_databasePort = wrapper->m_DatabasePort;
	if(wrapper->m_flag_DatabaseName)
	{
		m_databaseName.clear();
		m_databaseName.append(wrapper->m_DatabaseName.c_str());
	}
	if(wrapper->m_flag_DatabaseUserName)
	{
		m_databaseUserName.clear();
		m_databaseUserName.append(wrapper->m_DatabaseUserName.c_str());
	}
	if(wrapper->m_flag_DatabasePassword)
	{
		m_databasePassword.clear();
		m_databasePassword.append(wrapper->m_DatabasePassword.c_str());
	}

	if (wrapper->m_flag_SelectLogin)
		m_selectLogin = wrapper->m_SelectLogin;

	if (wrapper->m_flag_OnLineConfig)
		m_onLineConfig = wrapper->m_OnLineConfig;

	if(wrapper->m_flag_UserMode)
		m_userMode = wrapper->m_UserMode;


	if (wrapper->m_flag_RTrainModuleDir)
		m_RealTrainModule = wrapper->m_RTrainModuleDir;
	
	if (wrapper->m_flag_VTrainModuleDir)
		m_VirtualTrainModule = wrapper->m_VTrainModuleDir;

	if (wrapper->m_flag_QuestModuleDir)
		m_QuestionModule = wrapper->m_QuestModuleDir;

	delete wrapper;
}