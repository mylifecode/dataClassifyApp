
#pragma once
#include "Forceps.h"

class ITool;
class CXMLWrapperTool;
/*钛夹钳*/


//以下为钛夹钳的两种种状态0代表完全打开，1代表完全闭合
//当且仅当当前状态为1并且上一次状态为0时才能在器官上夹钛夹
#define CLIPAPLLIER_COMPLETELY_OPEN_STATUS		0
#define CLIPAPLLIER_COMPLETELY_CLOSE_STATUS		1

class MisCTool_PluginClamp;
class MisCTool_PluginClipTitanic;
class CClipApplier : public CForceps
{
public:
	CClipApplier();
	
	CClipApplier(CXMLWrapperTool * pToolConfig);
	
	virtual ~CClipApplier(void);

	void InternalSimulationStart(int currStep , int TotalStep , float dt);

	inline void SetOpenCloseStatus(int status){m_nStatus = status;}
	inline int GetOpenCloseStatus(){return m_nStatus;}

	void CreateEmptyClip();

	virtual bool Update(float dt);

	virtual void Updates2m();

	virtual bool Initialize(CXMLWrapperTraining * pTraining);

	std::string GetCollisionConfigEntryName();

	//overridden for new train
	//virtual void OnCutBladeClampedTube(MisMedicOrgan_Tube & tubeclamp , int segment , int localsection , Real sectionWeight);

	//夹子夹住物体时是否与物体垂直
	bool IsClampedVertical();
	//是否下半部分可见
	bool IsLowerHalfPartVisiable();

protected:
	//每一片器械头可以移动的距离
	const float m_fHalfHeadMoveDistance;

	std::vector< ITool* > m_pEmptyClips;
	//int m_nLastShaftAsideForEmptyClip;

	int m_nStatus;										//钛夹钳状态

	Ogre::Vector3 m_vDirection;

	bool m_bClampBeforeRealse;

	MisCTool_PluginClipTitanic * m_TitanicPlugin;
};