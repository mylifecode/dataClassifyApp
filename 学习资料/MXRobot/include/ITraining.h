#pragma once
#include "IMXDefine.h"
//#include "MXEventsDump.h"
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "ResourceManager.h"
#include "SimpleUI.h"
class CXMLWrapperTraining;
class CXMLWrapperToolConfig;
class CToolsMgr;
class CollisionTools;
class VeinConnectObject;
//class VeinConnectObjectV2;
class MisMedicOrganInterface;
using namespace GoPhys;

class QKeyEvent;		//后期去除！！！

#define MXOgre_SCENEMANAGER    MXOgreWrapper::Get()->GetDefaultSceneManger()


#define FUNCNAME(a,b,c) gen_a##b##c

#define REGISTERTRAINING(className,typeName,trainingName)	\
ITraining* FUNCNAME(className,typeName,trainingName)()\
{	\
return new className;	\
}	\
static CResourceManager::Register<className> className##trainingRegister(#typeName"-"#trainingName,FUNCNAME(className,typeName,trainingName));


class ITool;
class MxEvent;


class ITraining
{
public:
	struct Register
	{
		Register(const std::string& trainingName,REGISTER generator)
		{
			CResourceManager::Instance()->RegisterTraining(trainingName,generator);
		}
	};


	ITraining()
	{
		m_ineditormode = false;
		m_IsNewTrainMode = false;
		m_IsJustUseCamera = false;
	}
	
	virtual ~ITraining(void)
	{

	}

	virtual void UpdateTrocarTransform() = 0;

	//事件回调函数，当有事件时到达时，该函数被CMXEventsDump类所调用;
	//所有ITraining类的子类都可以重写该函数，免去了通过旧方式来注册事件回调函数;
	//如果在当前训练类中不需要处理任何事件，则不重写该函数即可
	virtual void OnHandleEvent(MxEvent* pEvent)
	{
		
	}

	void SetUseNewBloodSystem(bool set)
	{
		m_usenewblood = set;
	}

	bool GetUseNewBloodSystem()
	{
		return m_usenewblood;
	}

	void SetNewTrainMode(bool set)
	{
		m_IsNewTrainMode = set;
	}

	bool IsInNewTrainMode()
	{
		return m_IsNewTrainMode;
	}

	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig) = 0;

	virtual bool Update(float dt) = 0;

	virtual bool BeginRendOneFrame(float timeelapsed) = 0;
	virtual bool FinishRendOneFrame(float timeelapsed) = 0;

	virtual bool Terminate() = 0;

	virtual void OnSaveTrainingReport() = 0;

	//场景被照亮时该函数被调用
	virtual void OnTrainingIlluminated()		
	{

	}

	/**
		设摄像机的状态发生改变时将调用此函数，如：1、摄像机从固定状态到可移动状态；2、摄像机从可移动状态到固定状态
		参数isFixed表示当前摄像机是否为固定状态
	*/
	virtual void OnCameraStateChanged(bool isFixed)
	{
		
	}

    virtual void CalcCameraSpeed(Real dt, bool isFixed)
    {

    }

	virtual float GetCameraSpecialAngle()
	{
		return 0;
	}

	// 设定腹腔是否透明
	virtual void setAbdomenVisible(bool b){};

	virtual void setAbdomenTransparent(int  alpha) = 0;

	virtual void TrainingFinish() = 0;

	//set get当前被删的DynamicStrips的索引
	virtual void setDeletedDynamicStripsIndex( int nIndex ) = 0;
	virtual std::vector<int> getDeletedDynamicStripsIndexVector() = 0;
	virtual void passingToolKernel2mID( int nIndex ) = 0;
	virtual void passingToolKernelVector( Ogre::Vector3 vec, int index ) = 0;

	virtual void SetEditorMode(bool set) = 0;

	//New Train start
	virtual std::vector<VeinConnectObject*> GetVeinConnectObjects()
	{
		return std::vector<VeinConnectObject*>();
	}

	virtual VeinConnectObject  * GetVeinConnect(int connectType)
	{
		return 0;
	}

	//virtual VeinConnectObjectV2  * GetVeinObjV2(int connectType)
	//{
	//	return 0;
	//}

	//virtual std::vector<VeinConnectObjectV2*> GetVeinConnectObjectV2s()
	//{
	//	return std::vector<VeinConnectObjectV2*>();
	//}

	virtual MisMedicOrganInterface * GetOrgan(int organId)
	{
		return 0;
	}

	virtual MisMedicOrganInterface * GetOrgan(GFPhysSoftBody * body)
	{
		return 0;
	}

	virtual void GetAllOrgan(std::vector<MisMedicOrganInterface*>& ogans)
	{
		return;
	}

	//virtual void NotifySoftBodyFaceDeleted(const std::set<GFPhysSoftBodyFace*> & facedelete)
	//{

	//}

	virtual GFPhysVector3 GetTrainingForceFeedBack(ITool* tool)
	{
		return GFPhysVector3(0,0,0);
	}

	virtual void OnToolCreated(ITool * tool ,int side)//side = 0 left side = 1 right
	{

	}

	virtual void OnToolRemoved(ITool * tool)
	{

	}

	virtual float GetElapsedTime() = 0;

    virtual void KeyPress(QKeyEvent * event)
    {

    }

	virtual void OnSimpleUIEvent(const SimpleUIEvent & event){}

	/** 
		在启动调试界面的情况下，通过调用此函数来向训练发送一个调试信息值 
		调试界面类为：MxTrainingDebugWindow
	*/
	virtual void onDebugMessage(const std::string&){}

	/**
		用于显示调试信息，在调试模式下，此信息的内容会被显示到调试窗口！
	*/
	virtual void showDebugInfo(const std::string& debugInfo){}
	virtual void showDebugInfo(float value,int precision = -1){}
	virtual void showDebugInfo(int value){}
	virtual void showDebugInfo(unsigned int value){}
	virtual void showDebugInfo(int value1,int value2,int value3,int value4){}
	virtual void showDebugInfo(const std::string& prefix,float value,int precision = -1){}
	virtual void showDebugInfo(float vlaue1,float value2,float value3,int precision = -1){}

	//New Train End
    Ogre::String m_strName;
	Ogre::String m_strTrainingType;
	CToolsMgr * m_pToolsMgr;
//  	Ogre::SceneManager * m_pSceneMgr;
	//Ogre::Camera* m_pSmallCamera;
	Ogre::Camera* m_pLargeCamera;
	Ogre::Quaternion m_quatTrocar;
	Ogre::Quaternion m_quatLargetCamera;

	Ogre::String m_strConfigPath;
	
	int m_nSceneCount;

	bool m_bStaticCamera;
	Ogre::String m_strMediaSource;

	int m_nDefaultCS;

	CollisionTools* m_pCollisionTool;

	float m_fCollisionRadius;
	bool m_bCanStaticCollision;

	bool m_usenewblood;

	bool m_ineditormode;

	bool m_IsNewTrainMode;

	bool m_IsJustUseCamera;
};
