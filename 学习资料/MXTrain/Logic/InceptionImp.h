//用于MXROBOT与MXUI交互
//MXROBOT调用EmitXXX发射signal
//MXUI设置slot响应
#ifndef INCEPTIONIMP_H
#define INCEPTIONIMP_H
#include <QObject>
#include "Inception.h"
#include "TipInfo.h"
#define HIGH_CLARITY_VALUE 5000000
#define STANDARD_CLARITY_VALUE 1000000
#define NORMAL_CLARITY_VALUE 100000


class InceptionImp : public Inception
{
	Q_OBJECT

public:
	static Inception * Create()
	{
		if (ms_pObject == 0)
		{
			ms_pObject = new InceptionImp();
		}
		return ms_pObject;
	}

	static void Destroy()
	{
		if (ms_pObject)
		{
			delete ms_pObject;
			ms_pObject = 0;
		}
	}
public:
	
	void readyToTiming(){ emit BeginTiming(); }

signals:
	//void StartTimer(int sec);
	void MissionCompleted();
	void ShowTip(TipInfo);
	//void ChangeShot(int eCA);
    void SetLaparotomyReason(int errorCode);
	void ShowSmallWindow();
	void ShowToolSelectWindow(bool bLeft);
	void ChooseToolCategory(bool bLeft,int usage,bool bMoveClockwise );
	void ShowMovie(QString strMovieName);
	//void LoadMovie(QString strMovieName);
	void EnableTool(std::vector<ToolType>);

	void SelectTool(int handType, int msgType, int submsgType);
	void SelectDegree( int msgType);
    void QuitScene();
    void DataBaseChanged();
    void retPlanChange(int nIndex);
    void finishedRetPlanChange();
	
	void ShowPhantomBoxDebug();
	void GetPhantomBoxDebugInfo(QString& strinfo);
	//void ShowAdjustShaft();
	//void GetAdjustShaftInfo(QString& strinfo);
	//void ShowAdjustCamera();
	//void GetAdjustCameraInfo(QString& strinfo);
	void ShowAdjustGyro();
	void GetAdjustGyroInfo(QString& strinfo);

	//void trainPopupWidget(int mode, const std::string& title, const std::string& description, const std::string& leftButtonText, const std::string& rightButtongText);

	void BeginTiming();										//显示训练场景中左上角的计时窗口

	void ShowDebugInfo(const std::string& debugInfo);

	void ShowFixToolMenue();

	void ShowTrainProgressBar();

	void ShowTrainCompletness(int curr , int total);
public slots:
	
	void EmitEnableTool(std::vector<ToolType> toolTypes) 
	{
		emit EnableTool(toolTypes); 
	}

	void EmitShowTip(TipInfo tipInfo) 
	{ 
		emit ShowTip(tipInfo); 
	}

	//void EmitChangeShot(int eCA) 
	////{ 
	//	emit ChangeShot(eCA);
	//}

	void EmitShowSmallWindow()
	{ 
		emit ShowSmallWindow();
	}

    void EmitDataBaseChanged() 
	{ 
		emit DataBaseChanged();
	} 

    void EmitRetPlanChange(int nIndex)
	{ 
		emit retPlanChange(nIndex); 
	}

    void EmitFinishedRetPlanChange()
	{ 
		emit finishedRetPlanChange();
	}

	void EmitShowToolSelectWindow( bool bLeft)
	{ 
	    emit ShowToolSelectWindow(bLeft);
	}

	void EmitChooseToolType( bool bLeft, int usage, bool  bMoveClockwise)
	{ 
		emit ChooseToolCategory(bLeft,usage,bMoveClockwise);
	}

	void EmitToolSelect(int handType, int msgType, int submsgType)
	{ 
		emit SelectTool(handType,msgType,submsgType);
	}

	void EmitDegreeSelect( int msgType)
	{ 
		emit SelectDegree(msgType);
	}

	void EmitShowMovie(QString strMovieName)
	{ 
		emit ShowMovie(strMovieName);
	}

	void EmitQuitScene()
	{ 
		emit QuitScene();
	}
	
	void EmitShowPhantomBoxDebug()
	{
		emit ShowPhantomBoxDebug();
	};

	void EmitGetPhantomBoxDebugInfo(QString& strinfo)
	{
		emit GetPhantomBoxDebugInfo(strinfo);
	};

	//void EmitShowAdjustShaft()
	//{
	//	emit ShowAdjustShaft();
	//};

	//void EmitGetAdjustShaftInfo(QString& strinfo)
	//{
	//	emit GetAdjustShaftInfo(strinfo);
	//};

	//void EmitShowAdjustCamera()
	//{
	//	emit ShowAdjustCamera();
	//};

	////void EmitGetAdjustCameraInfo(QString& strinfo)
	//{
	//	emit GetAdjustCameraInfo(strinfo);
	//};
	
	void EmitShowAdjustGyro()
	{
		emit ShowAdjustGyro();
	};
	
	void EmitGetAdjustGyroInfo(QString& strinfo)
	{
		emit GetAdjustGyroInfo(strinfo);
	};

	//void EmitTrainPopupWidgetInfo(int mode, const std::string& title, const std::string& description, const std::string& leftButtonText, const std::string& rightButtongText)
	//{
		//emit trainPopupWidget(mode, title, description, leftButtonText, rightButtongText);
	//}

	void EmitShowFixToolMenue()
	{
		emit ShowFixToolMenue();
	}
	void EmitShowDebugInfo(const std::string& debugInfo)
	{
		emit ShowDebugInfo(debugInfo);
	}
	void EmitShowTrainProgressBar()
	{
		emit ShowTrainProgressBar();
	}

	void EmitShowTrainCompletness(int curr , int total)
	{
		emit ShowTrainCompletness(curr , total);
	}
};

#endif