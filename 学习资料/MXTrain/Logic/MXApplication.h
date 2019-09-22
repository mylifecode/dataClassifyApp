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
#include <QThread>
#include <MxProcessCommunicator.h>
#include "EngineCore.h"

class MxProcessCommunicator;
class SYTrainWindow;
class LoadingThread : public QThread
{
	Q_OBJECT
public:

	LoadingThread();

	void SetTrainLoadInfo(EngineCore * pEngineCore, const std::string & trainname);

protected:

	void run();

	std::string m_trainingName;
	EngineCore *m_pEngineCore;
};

class MXApplication : public QApplication, public MxProcessCommunicator::CommunicatorMsgListener
{
	Q_OBJECT
public:
	MXApplication(int argc, char** argv);
	
	~MXApplication(void);

	void StartupProcessCommunicator(const std::string& communicationName,
		QString &trainCategoryName,
		QString &trainEnName,
		QString &trainChName,
		QString &trainCode,
		int & TaskRecordID,
		int & TaskTrainID);

	void OnReceiveCommunicatorMessage(MXPCMessageType type, MxProcessCommunicateMsg * msg);
	void OnReceiveSubModuleTerminate();

	
	SYTrainWindow * m_TrainWindow;
	int m_TaskID;
	int m_TrainID;
public slots:
	void Update();

	void PauseUpdateLoop();

	void ResumeUpdateLoop();

	void ProcessMessages();

	void SendPintMsgToParentModule();
	
	void exitModuleAndShowParentWindow(QWidget * trainWindow);
public:
	void StartLoadingTrain(const std::string& trainingName , QWidget * invokeWidget);

	void OnTrainLoadingFinished();

   	void Terminate();

	inline EngineCore* GetEngineCore() const { return m_pEngineCore; }
	inline void SetEngineCore( EngineCore* pEngineCore ) { m_pEngineCore = pEngineCore; }
	
	MxProcessCommunicator* getCommunicator();
private:
	//bool winEventFilter ( MSG * msg, long * result );

	/// 设置程序所需的相关路径
	void setPaths();
	


private slots:/** 外部模块进程退出时调用该函数 */
   
signals :
	

private:
	bool m_IsActive;

	QTimer * m_AutoUpdateTimer;

	QTimer * m_MessageProcessTimer;

	EngineCore * m_pEngineCore;
	
	float m_updateinterval;

	MxProcessCommunicator* m_communicator;

	LoadingThread m_LoadingThread;

	bool m_IsDisabled;

	bool m_HasPingMsgSended;
};

