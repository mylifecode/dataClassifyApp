#include "StdAfx.h"
#include "XMLWrapperGlobalConfig.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperGlobalConfig)
REGISTER_CLASS(GlobalConfig)
//dir
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,SkinDir,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,StyleSheetDir,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,ToolIconDir,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,VideoDir,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,AudioDir,VALUE)
//config file name
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,DatabaseFileName,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,ToolXMLConfigFileName,VALUE)

//REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,DegreeListXMLConfigFileName,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,CourseTrainXMLConfigFileName,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, SelectModuleXMLConfigFileName, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, KnowledgeLibConfigFile, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, SurgeryTrainConfigFile, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, ScanUrl, VALUE)
//hardware
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,EnableForceFeedback,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,Left_Shaft_K,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,Left_Shaft_B,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,Right_Shaft_K,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,Right_Shaft_B,VALUE)
//vr
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,VR,VALUE)
//video
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,EnableRecord,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,VideoFrameRate,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,VideoWidth,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,VideoHeight,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,VideoSaveDir,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, HardwareEncode, VALUE)
//network
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,ServerIp,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,ServerPort,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,ClientPort,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,PackageSize,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,SendFrequency,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,MulticastMode,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,MulticastGroupAddress,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,DecodeDelayTime, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,EnableSendScreenData,VALUE)
//database
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,DatabaseHostName,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,DatabasePort,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,DatabaseName,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,DatabaseUserName,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,DatabasePassword,VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig,SelectLogin, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, OnLineConfig, VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, UserMode, VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, RTrainModuleDir, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, VTrainModuleDir, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperGlobalConfig, QuestModuleDir, VALUE)

END_IMPL_SERIALIZATION_CLASS

CXMLWrapperGlobalConfig::CXMLWrapperGlobalConfig(void)
{
	//dir
 	INIT_ATTRIBUTE_STRING(SkinDir)
	INIT_ATTRIBUTE_STRING(StyleSheetDir)
	INIT_ATTRIBUTE_STRING(ToolIconDir)
	INIT_ATTRIBUTE_STRING(VideoDir)
	INIT_ATTRIBUTE_STRING(AudioDir)
	//config file name
	INIT_ATTRIBUTE_STRING(ToolXMLConfigFileName)
	INIT_ATTRIBUTE_STRING(DatabaseFileName)
	
	//INIT_ATTRIBUTE_STRING(DegreeListXMLConfigFileName)
	INIT_ATTRIBUTE_STRING(CourseTrainXMLConfigFileName)
	INIT_ATTRIBUTE_STRING(SelectModuleXMLConfigFileName)
	INIT_ATTRIBUTE_STRING(KnowledgeLibConfigFile)
	INIT_ATTRIBUTE_STRING(SurgeryTrainConfigFile)
	//hadrware
	INIT_ATTRIBUTE_BOOL_VALUE(EnableForceFeedback,true)
	INIT_ATTRIBUTE_FLOAT_VALUE(Left_Shaft_K,0.1)
	INIT_ATTRIBUTE_FLOAT_VALUE(Left_Shaft_B,1)
	INIT_ATTRIBUTE_FLOAT_VALUE(Right_Shaft_K,0.2)
	INIT_ATTRIBUTE_FLOAT_VALUE(Right_Shaft_B,2)
	//VR
	INIT_ATTRIBUTE_BOOL_VALUE(VR,false)
	//video
	INIT_ATTRIBUTE_BOOL_VALUE(EnableRecord,false)
	INIT_ATTRIBUTE_LONG_VALUE(VideoFrameRate,50)
	INIT_ATTRIBUTE_LONG_VALUE(VideoWidth,1024)
	INIT_ATTRIBUTE_LONG_VALUE(VideoHeight,600)
	INIT_ATTRIBUTE_STRING_VALUE(VideoSaveDir,"data")
	INIT_ATTRIBUTE_BOOL_VALUE(HardwareEncode, false)
	//network
	INIT_ATTRIBUTE_STRING(ServerIp)
	INIT_ATTRIBUTE_LONG_VALUE(ServerPort,0)
	INIT_ATTRIBUTE_LONG_VALUE(PackageSize,0)
	INIT_ATTRIBUTE_LONG_VALUE(SendFrequency,0)
	INIT_ATTRIBUTE_STRING_VALUE(MulticastGroupAddress,"239.0.0.1")
	INIT_ATTRIBUTE_LONG_VALUE(DecodeDelayTime, 0)
	INIT_ATTRIBUTE_BOOL_VALUE(EnableSendScreenData,false)
	//database
	INIT_ATTRIBUTE_STRING(DatabaseHostName)
	INIT_ATTRIBUTE_LONG_VALUE(DatabasePort,3306)
	INIT_ATTRIBUTE_STRING(DatabaseName)
	INIT_ATTRIBUTE_STRING(DatabaseUserName)
	INIT_ATTRIBUTE_STRING(DatabasePassword)

	INIT_ATTRIBUTE_BOOL_VALUE(SelectLogin, false)
	INIT_ATTRIBUTE_BOOL_VALUE(OnLineConfig, false)


	INIT_ATTRIBUTE_LONG_VALUE(DatabasePort, 0)

	INIT_ATTRIBUTE_STRING(RTrainModuleDir)
	INIT_ATTRIBUTE_STRING(VTrainModuleDir)
	INIT_ATTRIBUTE_STRING(QuestModuleDir)
	
}

CXMLWrapperGlobalConfig::~CXMLWrapperGlobalConfig(void)
{
}
