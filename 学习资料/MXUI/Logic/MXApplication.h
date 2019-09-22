#pragma once

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>
#include <QApplication>

#include <MxProcessCommunicator.h>
class MxProcessCommunicator;
class SYLoading;
class MXApplication : public QApplication, public MxProcessCommunicator::CommunicatorMsgListener
{
	Q_OBJECT
public:
	MXApplication(int argc, char** argv);

	~MXApplication(void);

	int  LaunchQuestionModuleProcess();

	int  LaunchModule(const std::string & modulename,
					  const QString & trainName,
					  const QString & trainChName,
					  const QString & trainCat,
					  const QString & trainCode,
					  int TaskRecordID,
					  int trainTaskID,
					  bool showLoadingUI);

	//@ over ridden MxProcessCommunicator::CommunicatorMsgListener
	void OnReceiveCommunicatorMessage(MXPCMessageType type, MxProcessCommunicateMsg * msg);
	void OnReceiveSubModuleTerminate();

	static void StopDataServiceCallBack(int, char *pUrl);
	static void LogoutCallBack();

	MxProcessCommunicator* getCommunicator();
	public slots:
	void ProcessMessages();

	void Update();

	void PauseUpdateLoop();

	void ResumeUpdateLoop();

public:
	void Initialize();

	void Terminate();

	//	inline EngineCore* GetEngineCore() const { return m_pEngineCore; }
	//inline void SetEngineCore( EngineCore* pEngineCore ) { m_pEngineCore = pEngineCore; }

private:
	//bool winEventFilter ( MSG * msg, long * result );

	/// 设置程序所需的相关路径
	void setPaths();

	void StartupProcessCommunicator(const std::string& communicationName);


	private slots:/** 外部模块进程退出时调用该函数 */

	void TerminateSubMoudle();

	void InternalLaunchSubModule();

signals :
	void ReceiveMessage(MXPCMessageType type, const QString& message);
		void ReceiveStop();



private:

	std::string m_SubModuleName;

	std::string m_SubModuleParam;

	SYLoading * m_SubModuleLoadingUI;
	bool m_IsActive;

	QTimer * m_AutoUpdateTimer;

	QTimer * m_MessageProcessTimer;

	//EngineCore * m_pEngineCore;

	float m_updateinterval;

	MxProcessCommunicator* m_communicator;
};

