#pragma once
#include <QString>
#include "MXCommon.h"
class CXMLWrapperGlobalConfig;

/**
	此类主要用于获取全局配置：路径、配置文件名、硬件参数、视屏参数
*/
class MXCOMMON_API MxGlobalConfig
{
private:
	MxGlobalConfig(void);
	~MxGlobalConfig(void);

public:
	static MxGlobalConfig* Instance();

	/** 获取全局路径 */
	inline const QString& GetSkinDir() {return m_skinDir;}
	inline const QString& GetStyleSheetDir() {return m_styleSheetDir;}
	inline const QString& GetToolIconDir() {return m_toolIconDir;}
	inline const QString& GetVideoDir() {return m_videoDir;}
	inline const QString& GetAudioDir() {return m_audioDir;}

	inline const std::string& GetRealTrainModule(){ return m_RealTrainModule; }
	inline const std::string& GetVirtualTrainModule(){ return m_VirtualTrainModule; }
	inline const std::string& GetQuestionModule(){ return m_QuestionModule; }


	/** 获取配置文件名 */	
	inline const QString& GetDatabaseFileName() {return m_databaseFileName;}
	inline const QString& GetToolXmlConfigFileName() {return m_toolXmlConfigFileName;}
	//inline const QString& GetToolForTaskXmlConfigFileName(){return m_toolForTaskXmlConfigFileName;}
	//inline const QString& GetDegreeListXmlConfigFileName() {return m_degreeListXmlConfigFileName;}
	inline const QString& GetCourseTrainXmlConfigFileName() {return m_courseTrainXmlConfigFileName;}
	inline const QString& GetSelectModuleXmlConfigFileName() { return m_selectModuleXmlConfigFileName; }
	inline const QString& GetScanUrl() { return m_scanUrl; }

	/** hardware */
	inline bool EnabledForceFeedback() {return m_enabledForceFeedback;}
	inline float GetLeftShaftK() {return m_left_shaft_k;}
	inline float GetLeftShaftB() {return m_left_shaft_b;}
	inline float GetRightShaftK() {return m_right_shaft_k;}
	inline float GetRightShaftB() {return m_right_shaft_b;}

	/** vr */
	inline bool IsVR() {return m_isVR;}

	/** video */
	inline bool EnabledRecord() {return m_enabledRecord;}
	inline int GetVideoFrameRate() {return m_videoFrameRate;}
	inline int GetVideoWidth() {return m_videoWidth;}
	inline int GetVideoHeight() {return m_videoHeight;}
	inline const QString& GetVideoSaveDir() {return m_videoSaveDir;}
	inline bool UseHardwareEncode() { return m_useHardwareEncode; }

	/** network */
	inline const QString& GetServerIp() {return m_serverIp;}
	inline int GetServerPort() {return m_serverPort;}
	inline int GetClientPort() { return m_clientPort; }
	inline int GetPackageSize() {return m_packageSize;}
	inline int GetSendFrequency() {return m_sendFrequency;}
	inline bool MulticastMode() { return m_multicastMode; }
	inline const QString& GetMulticastGroupAddress() { return m_multicastGroupAddress; }
	inline unsigned int GetDecodeDelayTime() { return m_decodeDelayTime; }
	inline bool EnabledSendScreenData() {return m_enabledSendScreenData;}

	/** database */
	inline const QString& GetDatabaseHostName() {return m_databaseHostName;}
	inline int GetDatabasePort() {return m_databasePort;}
	inline const QString& GetDatabaseName() {return m_databaseName;}
	inline const QString& GetDatabaseUserName() {return m_databaseUserName;}
	inline const QString& GetDatabasePassword() {return m_databasePassword;}

	/** other dir and file name 
		这部分属性是在程序中设置的，而以上的属性只能通过globalConfig.xml文件进行设置，所以只有读取的操作，没有设置操作。
	*/
	inline const QString& GetCurTrainCaseDir() {return m_curTrainCaseDir;}
	inline const QString& GetCurTrainCaseFileName() {return m_curTrainCaseFileName;}
	inline void SetCurTrainCaseDir(const QString& caseDir) {m_curTrainCaseDir = caseDir;}
	inline void SetCurTrainCaseFileName(const QString& caseFileName) {m_curTrainCaseFileName = caseFileName;}

	void SetKnowledgeLibConfigFile(const QString& fileName) { m_knowledgeLibConfigFile = fileName; }
	const QString& GetKnowledgeLibConfigFile() const { return m_knowledgeLibConfigFile; }
	const QString& GetSurgeryTrainConfigFile() const { return m_surgeryTrainConfigFile; }

	inline bool selectLogin(){ return m_selectLogin; };
	inline bool onLineConfig(){ return m_onLineConfig; };

	enum UserMode{
		UM_Normal = 0x00,
		/// 在此模式下，登录界面显示访客登录按钮，便可以用访客身份登录系统
		UM_Demonstrate = 0x01,
		UM_Debug = UM_Demonstrate | (0x01 << 1)
	};

	UserMode GetUserMode() const { return static_cast<UserMode>(m_userMode); }




private:
	void Initialize();

private:
	/// 用于初始化成员的xml文件
	const QString m_xmlFileName;
	
	/// dir
	QString m_skinDir;
	QString m_styleSheetDir;
	QString m_toolIconDir;
	QString m_videoDir;
	QString m_audioDir;
	
	//module file
	std::string m_RealTrainModule;
	std::string m_VirtualTrainModule;
	std::string m_QuestionModule;

	///config file name
	QString m_toolXmlConfigFileName;
	
	QString m_databaseFileName;
	//QString m_toolForTaskXmlConfigFileName;
	//QString m_degreeListXmlConfigFileName;
	QString m_courseTrainXmlConfigFileName;
	QString m_selectModuleXmlConfigFileName;

	/// other dir and file name
	/// 当前手术训练的病历文件目录，在程序中根据不同的训练进行动态设置
	QString m_curTrainCaseDir;
	/// 当前手术训练的病历文件名，在程序中根据不同的训练进行动态设置（只是文件名，不包括路径）
	QString m_curTrainCaseFileName;

	QString m_knowledgeLibConfigFile;
	QString m_surgeryTrainConfigFile;
	
	/// hardware
	bool m_enabledForceFeedback;
	float m_left_shaft_k;
	float m_left_shaft_b;
	float m_right_shaft_k;
	float m_right_shaft_b;

	/// 是否为虚实结合
	bool m_isVR;

	/// video
	bool m_enabledRecord;
	int m_videoFrameRate;
	int m_videoWidth;
	int m_videoHeight;
	QString m_videoSaveDir;
	bool m_useHardwareEncode;

	/// network
	QString m_serverIp;
	int m_serverPort;
	int m_clientPort;
	int m_packageSize;
	int m_sendFrequency;
	QString m_multicastGroupAddress;
	bool m_multicastMode;
	unsigned int m_decodeDelayTime;
	bool m_enabledSendScreenData;

	/// database
	QString m_databaseHostName;
	int m_databasePort;
	QString m_databaseName;
	QString m_databaseUserName;
	QString m_databasePassword;
	QString m_scanUrl;
	bool m_selectLogin;
	bool m_onLineConfig;


	/// mode
	int m_userMode;
};
