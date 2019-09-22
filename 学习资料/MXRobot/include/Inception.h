//用于MXROBOT与MXUI交互
//MXROBOT调用EmitXXX发射signal
//MXUI设置slot响应
#ifndef INCEPTION_H
#define INCEPTION_H
#include <vector>
#include <QObject>
#include <QString>
#include "TipInfo.h"
#define HIGH_CLARITY_VALUE 5000000
#define STANDARD_CLARITY_VALUE 1000000
#define NORMAL_CLARITY_VALUE 100000

struct ToolType
{
	enum HandType
	{
		HT_None,
		HT_LeftHand,
		HT_RightHand
	};

	HandType enmHandType;
	std::string strToolType;
	std::string strToolSubType;

	ToolType()
	{
		enmHandType = HT_None;
		strToolType = "";
		strToolSubType = "";
	}

	ToolType(HandType enmHandType, const std::string& strToolType, const std::string& strToolSubType)
	{
		this->enmHandType = enmHandType;
		this->strToolType = strToolType;
		this->strToolSubType = strToolSubType;
	}
};

		
class Inception : public QObject
{
public:
	static Inception * Instance()
	{
		return ms_pObject;
	}
	std::string m_strTrainingName;
	std::string m_strTargetCameraName;
	bool m_bCenterWindow;//提示窗体居中
	bool m_bCenterShow;
	int m_remainTime;	//训练操作剩余时间
	int m_totalTime;	//训练操作总时间
	bool m_bTrainRunning;
	bool m_bLeftButton;
	bool m_bRightButton;
	int	    m_mousexlargeview;
	int	    m_mouseylargeview;
	//bool m_bTimeOut;
	float m_fPercent;//百分比
	bool m_bErrorEnd;//是否严重操作结束

    int m_clarity;
public:
	
	inline void SetLeftPedalState(bool state){ m_bLeftButton = state;}
	
	inline void SetRightPedalState(bool state){ m_bRightButton = state;}
	
	inline void SetErrorEndState(bool state){m_bErrorEnd = state;}

	virtual void readyToTiming() = 0;
	
	virtual void EmitEnableTool(std::vector<ToolType> toolTypes) = 0;

	virtual void EmitShowTip(TipInfo tipInfo) = 0;

	//virtual void EmitChangeShot(int eCA) = 0;

	virtual void EmitShowSmallWindow() = 0;

	virtual void EmitDataBaseChanged() = 0;

	virtual void EmitRetPlanChange(int nIndex) = 0;

	virtual void EmitFinishedRetPlanChange() = 0;

	virtual void EmitShowToolSelectWindow(bool bLeft) = 0;

	virtual void EmitChooseToolType(bool bLeft, int usage, bool  bMoveClockwise) = 0;

	virtual void EmitToolSelect(int handType, int msgType , int submsgType) = 0;

	virtual void EmitDegreeSelect(int msgType) = 0;

	virtual void EmitShowMovie(QString strMovieName) = 0;

	virtual void EmitQuitScene() = 0;

	virtual void EmitShowPhantomBoxDebug() = 0;
	virtual void EmitGetPhantomBoxDebugInfo(QString& strinfo) = 0;

	//virtual void EmitShowAdjustShaft() = 0;
	//virtual void EmitGetAdjustShaftInfo(QString& strinfo) = 0;

	//virtual void EmitShowAdjustCamera() = 0;
	//virtual void EmitGetAdjustCameraInfo(QString& strinfo) = 0;

	virtual void EmitShowAdjustGyro() = 0;
	virtual void EmitGetAdjustGyroInfo(QString& strinfo) = 0;

	//virtual void EmitTrainPopupWidgetInfo(int mode, const std::string& title, const std::string& description, const std::string& leftButtonText, const std::string& rightButtongText) = 0;

	virtual void EmitShowFixToolMenue() = 0;

	virtual void EmitShowDebugInfo(const std::string& debugInfo) = 0;

	virtual void EmitShowTrainProgressBar() = 0;

	virtual void EmitShowTrainCompletness(int curr , int total) = 0;

protected:
	static Inception * ms_pObject;

};



#endif