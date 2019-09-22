#pragma once
#include "XMLSerialize.h"

class CXMLWrapperGlobalConfig : public CXMLSerialize
{
public:
	CXMLWrapperGlobalConfig(void);
	~CXMLWrapperGlobalConfig(void);
	
	//dir
	IMPL_ATTRIBUTE_STRING(SkinDir)
		IMPL_ATTRIBUTE_STRING(StyleSheetDir)
		IMPL_ATTRIBUTE_STRING(ToolIconDir)
		IMPL_ATTRIBUTE_STRING(VideoDir)
		IMPL_ATTRIBUTE_STRING(AudioDir)

		
		//config file name
		IMPL_ATTRIBUTE_STRING(DatabaseFileName)
		IMPL_ATTRIBUTE_STRING(ToolXMLConfigFileName)
		
		//IMPL_ATTRIBUTE_STRING(DegreeListXMLConfigFileName)
		IMPL_ATTRIBUTE_STRING(CourseTrainXMLConfigFileName)
		IMPL_ATTRIBUTE_STRING(SelectModuleXMLConfigFileName)
		IMPL_ATTRIBUTE_STRING(KnowledgeLibConfigFile)
		IMPL_ATTRIBUTE_STRING(SurgeryTrainConfigFile)
		IMPL_ATTRIBUTE_STRING(ScanUrl)

		//hardware
		IMPL_ATTRIBUTE_BOOL(EnableForceFeedback)
		IMPL_ATTRIBUTE_FLOAT(Left_Shaft_K)
		IMPL_ATTRIBUTE_FLOAT(Left_Shaft_B)
		IMPL_ATTRIBUTE_FLOAT(Right_Shaft_K)
		IMPL_ATTRIBUTE_FLOAT(Right_Shaft_B)

		//是否为虚实结合，用于控制在界面模块是否显示CameraWindow
		IMPL_ATTRIBUTE_BOOL(VR)

		//video
		IMPL_ATTRIBUTE_BOOL(EnableRecord)
		IMPL_ATTRIBUTE_LONG(VideoFrameRate)
		IMPL_ATTRIBUTE_LONG(VideoWidth)
		IMPL_ATTRIBUTE_LONG(VideoHeight)
		IMPL_ATTRIBUTE_STRING(VideoSaveDir)
		IMPL_ATTRIBUTE_BOOL(HardwareEncode)

		//network
		IMPL_ATTRIBUTE_STRING(ServerIp)
		IMPL_ATTRIBUTE_LONG(ServerPort)
		IMPL_ATTRIBUTE_LONG(ClientPort)
		IMPL_ATTRIBUTE_LONG(PackageSize)
		IMPL_ATTRIBUTE_LONG(SendFrequency)		//发送频率，单位ms
		IMPL_ATTRIBUTE_STRING(MulticastGroupAddress)
		IMPL_ATTRIBUTE_BOOL(MulticastMode)
		IMPL_ATTRIBUTE_LONG(DecodeDelayTime)	//解码延迟时间，单位ms
		IMPL_ATTRIBUTE_BOOL(EnableSendScreenData)

		//database
		IMPL_ATTRIBUTE_STRING(DatabaseHostName)
		IMPL_ATTRIBUTE_LONG(DatabasePort)
		IMPL_ATTRIBUTE_STRING(DatabaseName)
		IMPL_ATTRIBUTE_STRING(DatabaseUserName)
		IMPL_ATTRIBUTE_STRING(DatabasePassword)

		IMPL_ATTRIBUTE_BOOL(SelectLogin);
		IMPL_ATTRIBUTE_BOOL(OnLineConfig);

		IMPL_ATTRIBUTE_LONG(UserMode);

		IMPL_ATTRIBUTE_STRING(RTrainModuleDir)
		IMPL_ATTRIBUTE_STRING(VTrainModuleDir)
		IMPL_ATTRIBUTE_STRING(QuestModuleDir)


	DECLARE_SERIALIZATION_CLASS(CXMLWrapperGlobalConfig)
};

