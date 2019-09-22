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

class MxProcessCommunicator;
class MxQuestionModuleApp : public QApplication
{
	Q_OBJECT
public:
	MxQuestionModuleApp(int argc, char** argv);
	~MxQuestionModuleApp(void);

public slots:
	void Update();

	//void PauseUpdateLoop();

	//void ResumeUpdateLoop();

	void SendPintMsgToParentModule();

	void exitModuleAndShowParentWindow(int pWindowToShow);
public:
	//void Initialize();

   //	void Terminate();

		
private:
	
	/// 设置程序所需的相关路径
	void setPaths();
	void StartupProcessCommunicator(const std::string& communicationName);
private:
	bool m_IsDisabled;

	bool m_HasPingMsgSended;
	//QTimer * m_AutoUpdateTimer;

	
	float m_updateinterval;

	MxProcessCommunicator* m_communicator;
};

