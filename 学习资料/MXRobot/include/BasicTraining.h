#pragma once
#include "ITraining.h"
#include "ToolsMgr.h"
#include "OgreMaxScene.hpp"
#include "MXOgreWrapper.h"
#include "IObjDefine.h"
//#include "MovieMgr.h"
#include "ScoreMgr.h"
#include "OnLineGradeMgr.h"
#include "TipMgr.h"
//#include <boost/function.hpp>

class CFSMachine;
class PathPutTool;
class DynamicStripsObject;
class DynamicStripsObjectEx;
class VeinConnectPair;

class VeinConnEditor;
class VeinConnEditorV2;
class HotchpotchEditor;

extern const int GRAVITY; 

//connp用于S4M和S1M绑定，辅助形变，1是S4M，2是S1M
/*
typedef struct {
	unsigned int ms1;
	unsigned int mp1;
	unsigned int ms2;
	unsigned int mp2;
	double dis;
} connp;
*/

class ObjectLoadReleaseListener
{
public:
	virtual void onDynamicObjectChanged() = 0;
};

class CBasicTraining :public ITraining
{
public:
	enum enmShadowType
	{
		NONE,
		STENCIL_ADDITIVE,
		STENCIL_MODULATIVE,
		TEXTURE_ADDITIVE,
		TEXTURE_MODULATIVE,
	};
public:
	CBasicTraining(void);
	virtual ~CBasicTraining(void);

	virtual void KeyPress(QKeyEvent * event);
	void ReloadMaterial();

public:
	
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	virtual void ShowDebugFrame();

	float GetElapsedTime();

	virtual bool Update(float dt);

	virtual void setAbdomenVisible(bool b);

	virtual void setAbdomenTransparent(int  alpha);

	virtual void OnOrganCutByTool(MisMedicOrganInterface * organ, bool iselectriccut)
	{}

	virtual void OnVeinConnectCuttingByElecTool(std::vector<VeinConnectPair*> & cuttingPair)
	{}

	virtual void OrganBeginAddToDynamicWorld(MisMedicOrganInterface * organ)
	{}
	
	virtual void OrganBeginRemoveFromDynamicWorld(MisMedicOrganInterface * organ)
	{}

	virtual bool OrganShouldBleed(MisMedicOrgan_Ordinary* organ, int faceID, float* pFaceWeight)
	{
		return false;
	}

	virtual void onOrganTopologyRefreshed(MisMedicOrgan_Ordinary* organ)
	{

	}

	bool Terminate();
	virtual void TrainingIntialized();
	virtual void OnTrainingIlluminated();
	
	virtual void TimeOver();
	virtual void TrainingFinish();
	//void TrainingFinish(Ogre::String strTipName);

	void Statistics();

	/**
	@基础训练场景和胆囊场景用的坐标系不同，
	 所以重力的方向使用的坐标轴也不同，需要
	 根据各自使用的坐标系来分别设置。
	*/
	
	void addDynObjLoadReleaseListener(ObjectLoadReleaseListener * listener);

	void removeDynObjLoadReleaseListener(ObjectLoadReleaseListener * listener);

	void SetEditorMode(bool set);

	//set get当前被删的DynamicStrips的索引
	virtual void setDeletedDynamicStripsIndex( int nIndex );
	virtual std::vector<int> getDeletedDynamicStripsIndexVector(); 
	virtual void passingToolKernel2mID( int nIndex );
	virtual void passingToolKernelVector( Ogre::Vector3 vec, int index );
	
	//void RemoveDynamicObject(TEDOT type);
	//void SetAutoRecordFunction(boost::function<void()> func);
	/**
		在设置训练报告，在场景结束时调用
	*/
	virtual void OnSaveTrainingReport();

	bool ToolInsideFascia(ITool *curTool);

	/**
		下面几个函数将用于发送调试信息到UI层，在调试模式下，将此调试信息现在在调试窗口上！
	*/
	void showDebugInfo(const std::string& debugInfo);

	void showDebugInfo(float value,int precision = -1);
	
	void showDebugInfo(int value);
	void showDebugInfo(unsigned int value);

	void showDebugInfo(int value1,int value2,int value3,int value4);

	void showDebugInfo(const std::string& prefix,float value,int precision = -1);

	void showDebugInfo(float value1,float value2,float value3,char split = ' ',int precision = -1);

	void SetEditMode(bool set)
	{
		m_IsInEditMode = set;
	}
protected:
	void CreateDefaultSceneMgr();
	virtual void LoadConfig(CXMLWrapperTraining * pTrainingConfig);

	virtual void LoadDynamicData(CXMLWrapperTraining * pTrainingConfig);

	void LoadStaticData(CXMLWrapperTraining * pTrainingConfig);
	
	void LoadDynamicDataOne(CXMLWrapperTraining * pTrainingConfig, int i);
	void LoadPartData(CXMLWrapperTraining * pTrainingConfig);
	//void LoadMucous( CXMLWrapperTraining * pTrainingConfig );
	//bool ConnectObject(CXMLWrapperTraining * pTrainingConfig);
	//bool CreateCollision( CXMLWrapperTraining * pTrainingConfig);

	void ReleaseStaticData();
	//void ReleaseDynamicData();
	//void ReleaseDynamicData(int nType);
	void ReleasePartData();

	bool GetStaticCamera() const { return m_bStaticCamera; }
	void SetStaticCamera(bool val) { m_bStaticCamera = val; }

	void LoadFSM(CXMLWrapperTraining * pTrainingConfig); // load fsm

	void LoadMovies(CXMLWrapperTraining * pTrainingConfig);
	void LoadScores(CXMLWrapperTraining * pTrainingConfig);
	void LoadProGrades(CXMLWrapperTraining * pTrainingConfig);
	void LoadTips(CXMLWrapperTraining * pTrainingConfig);

	void CheckToolSpeed();

	void CheckToolFighting();//检测左右器械打架
	void CreateEnvironment(CXMLWrapperTraining * pTrainingConfig);

	void setUniform(Ogre::String compositor, Ogre::String material, Ogre::String uniform, float value, bool setVisible, int position = -1);

	void LoadStickObjectFromFile(Ogre::String filename);

	void MapDeviceWorkspaceModel();

public:
	typedef std::vector<ObjectLoadReleaseListener *> VecLoadListener;

	//DynamicStripsObjectEx* m_pDynStrips;

	Ogre::Viewport *m_viewport;
	Ogre::RenderWindow *m_pCurrentRenderWindow;
	
	OgreMax::OgreMaxScene *m_pOms; 
	//CInnerDataMgr *m_pInnerDataMgr;
	//MAP_S_OBJ m_mapStrDynObj;
	//MAP_I_OBJ m_mapIDDynObj;

	VecLoadListener m_loadlistener;

	//MAP_PART_OBJECT m_mapIDPartObj;
	Ogre::String m_strSkinModelName;

	MXOgreWrapper::CameraState m_eCS;

	RbWindow *m_pRdWindow;
	
	float m_fToolDistance;//器械深入距离
	std::string m_sCurrentTraingType;	

	//OgreUtils::DirectShowMovieTexture *m_pMovieTexture;
	Ogre::Overlay *m_pVideoOverlay;
	Ogre::OverlayContainer * m_pOverlayContainer;
	bool m_bPlaying;

	bool m_bCutMark[EDOT_ORGAN_LIMIT];

	bool m_bTimeOut;
	bool m_bIsTerminating;
    bool m_bIsContinue;//超时后继续练习标志
	bool m_bTrainingRunning;

	CXMLWrapperTraining * m_pTrainingConfig;
	std::vector<int> m_vtCustomDynamicObjIndex;
	unsigned int m_lastmilliseconds;
	PathPutTool * m_pathtool;
	//VeinConnEditor *m_veineditor;
	//VeinConnEditorV2 *m_veineditor_v2;
	HotchpotchEditor * m_pHotchpotchEditor;

	Ogre::ManualObject * m_ForceFeedBackDebugDraw;
private:
	Ogre::Vector3 m_vecLeftToolPos;
	Ogre::Vector3 m_vecRightToolPos;

	DWORD m_dwLeftStartTime;
	DWORD m_dwRightStartTime;

	Ogre::Vector3 m_vStartLeftPosition;
	Ogre::Vector3 m_vStartRightPosition;

	Ogre::String m_strLeftToolName;
	Ogre::String m_strRightToolName;

	DWORD m_dwStartTime;

	bool m_bLeftFirstTip;
	bool m_bRightFirstTip;

	Ogre::ManualObject *m_pManualObjectForDebugLeft;
	Ogre::SceneNode *m_pSceneNodeForDebugLeft;
	Ogre::ManualObject *m_pManualObjectForDebugRight;
	Ogre::SceneNode *m_pSceneNodeForDebugRight;
	//boost::function<void()> m_funcStartVideoRecording;
	bool m_bVideoRecording;
	float m_ElapsedTime;   
	bool m_IsInEditMode;
protected:
	bool m_IsSeriousFault;
	float m_fLastTime;
	int	 m_SeriousFaultID;
	bool m_bTrainingIlluminated;
#ifndef USE_SSAO
	//Ogre::MaterialManager::Listener* m_listenerDrawDepth;
#else
	SSAOLogic* m_LogicInstance;
	Ogre::MaterialManager::Listener* m_listenerGbuffer;
	Ogre::MaterialManager::Listener* m_listenerNoGbuffer;
#endif
private:
	//////////////////////////////////////////////////////////////////////////
	// test code
	//Ogre::OverlayElement* m_TextBasic;
	//bool m_Debug;
	//Ogre::SceneNode* m_Testboundbox1;
	//Ogre::SceneNode* m_Testboundbox2;
	////Ogre::SceneNode* m_Testboundbox3;
	Ogre::SceneNode* m_Testboundbox4;

};
