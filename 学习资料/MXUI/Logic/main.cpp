#include "MxDefine.h"
#include "MXApplication.h"
//#include "SYLoginWindow.h"
#include "aslang.h"

#include <QApplication>
#include <QSplashScreen>
#include <QTranslator>
#include <QDesktopServices>
#include <QProcessEnvironment>
//#include <vld.h>
#include "SYMainWindow.h"

#include "minidmp.h"



LONG WINAPI GPTUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);

void setLibrayPaths(const QString& appPath);

int main(int argc, char *argv[])
{
	SetUnhandledExceptionFilter(GPTUnhandledExceptionFilter);

	int nRet;
	
	HANDLE hPrevMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXT(MENTORMODULE_MUTEXID));
	if (hPrevMutex)
	{
		MessageBox(NULL, TEXT("SYLapMentor模块已经存在"), TEXT("启动失败"), MB_OK | MB_SYSTEMMODAL | MB_ICONSTOP);
		CloseHandle(hPrevMutex);
		return 0;
	}
	HANDLE hExeCheckMutex = CreateMutex(FALSE, 0, TEXT(MENTORMODULE_MUTEXID));

	setLibrayPaths(argv[0]);

	MXApplication a(argc, argv);

	g_lang->ChangeLanguage("zh_CN");

	Mx::setGlobleStyle("qss:globle.qss");

	//SYLoginWindow * pLoginWindow = new SYLoginWindow;
	//pLoginWindow->setAttribute(Qt::WA_DeleteOnClose, true);
	//pLoginWindow->showFullScreen();

	SYMainWindow::CreateInstance();
	SYMainWindow::GetInstance()->showFullScreen();

	nRet = a.exec();

	SYMainWindow::DestoryInstance();
	
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

	return EXCEPTION_EXECUTE_HANDLER;  // 程序停止运行
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