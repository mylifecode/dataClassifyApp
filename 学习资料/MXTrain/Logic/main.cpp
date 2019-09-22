#include "StdAfx.h"
#include <QApplication>
#include <QSplashScreen>
#include <QTranslator>
#include <QDesktopServices>
#include <QProcessEnvironment>
#include "MxDefine.h"
#include "MXApplication.h"
#include "MxGlobalConfig.h"

//#include <vld.h>
#include "SYTrainWindow.h"
#include "minidmp.h"


LONG WINAPI GPTUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);

void setLibrayPaths(const QString& appPath);

int main(int argc, char *argv[])
{


	SYTrainWindow::CaseInfoParam caseParams;
	caseParams.nextTrainName = "";
	caseParams.trainCategoryName = QString::fromLocal8Bit("µ¨ÄÒÇÐ³ýÊõÈ«Ì×ÑµÁ·");
	caseParams.trainEnName = "CamSkillA";
	caseParams.trainChName = QString::fromLocal8Bit("ÂýÐÔµ¨ÄÒÑ×²¡Àý1");
	

	SetUnhandledExceptionFilter(GPTUnhandledExceptionFilter);

	int nRet;
	HANDLE hPrevMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXT("SYLAPTrain-{BE2B769F-C0E6-4923-BF57-2C47EC80021C}"));
	if (hPrevMutex)
	{
		MessageBox(NULL, TEXT("MXUI±»¶þÖØÆô¶¯¡£"), TEXT("Æô¶¯Ê§°Ü"), MB_OK | MB_SYSTEMMODAL | MB_ICONSTOP);
		CloseHandle(hPrevMutex);
		return 0;
	}

	HANDLE hExeCheckMutex = CreateMutex(FALSE, 0, TEXT("MISRobot-{BE2B769F-C0E6-4923-BF57-2C47EC80021C}"));

	setLibrayPaths(argv[0]);

	int TaskID, TaskTrainID;
	TaskID = TaskTrainID = -1;

	MXApplication a(argc, argv);

	a.StartupProcessCommunicator("SYLapModulePipe", caseParams.trainCategoryName, caseParams.trainEnName, caseParams.trainChName,caseParams.trainCode, TaskID, TaskTrainID);

	Mx::setGlobleStyle("qss:Globle.QSS");

	SYTrainWindow * pCaseOperationWidget = new SYTrainWindow(caseParams, 0);//delete this when the window closed ,so no worry memory leak
	
	a.m_TrainWindow = pCaseOperationWidget;
	a.m_TaskID  = TaskID;
	a.m_TrainID = TaskTrainID;
	nRet = a.exec();
	
	ReleaseMutex(hExeCheckMutex);

	return nRet;
}


LONG WINAPI GPTUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	std::stringstream sstream;

	struct tm *pTime;
	time_t ctTime; time(&ctTime);
	pTime = localtime( &ctTime );
	sstream << std::setw(2) << std::setfill('0') << pTime->tm_mon+1 
			<< "-" << std::setw(2) << std::setfill('0') << pTime->tm_mday
			<< "-" << std::setw(2) << std::setfill('0') << pTime->tm_hour
			<< "-" << std::setw(2) << std::setfill('0') << pTime->tm_min
			;

	std::string filename = "dump"+sstream.str()+".dmp";
	CreateMiniDump(pExceptionInfo, filename.c_str());

	//exit(pExceptionInfo->ExceptionRecord->ExceptionCode);

	return EXCEPTION_EXECUTE_HANDLER;  // ³ÌÐòÍ£Ö¹ÔËÐÐ
}

void setLibrayPaths(const QString& appPath)
{
	//set plugin paths
	int index = -1;
	QString path = appPath;

	index = path.lastIndexOf('\\');
	if (index < 0)
	{
		index = path.lastIndexOf('/');
	}

	if (index >= 0)
		path.remove(index, path.length() - index);

	if (QDir(path).exists() == false)
	{
		QString errorInfo("parse app path error!");
		path.clear();
		qDebug() << errorInfo;
	}

	QStringList paths;
	paths = QApplication::libraryPaths();
	paths << path + "./plugins"
		<< path + "./plugins/platforms";

	QApplication::setLibraryPaths(paths);
}