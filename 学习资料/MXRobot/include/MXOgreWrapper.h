#pragma once

#include "Singleton.h"
#include "OgreRoot.h"
#include "ogrewidget.h"
#include <QWidget>
#include <map>
#include <OgreLog.h>
#include <QApplication>
#include "../amfc/include/Rbwindow.h"
#include "OgreRenderWindow.h"
#include "OgreFrameListener.h"
#include "Overlay/include/OgreOverlaySystem.h"
// #define GLOBAL_VIEW     0
// #define PART_VIEW    1
class HDRLogic;
class ITraining;
class MXOgreWrapper :public CSingleT<MXOgreWrapper>,Ogre::FrameListener , Ogre::RenderSystem::Listener
{
public:
	enum CameraState {
		CS_DEGREE_0 = 0,
		CS_DEGREE_30 = 30,
		CS_DEGREE_45 = 45,
	};

public:
	MXOgreWrapper(void);
	~MXOgreWrapper(void);

public:
	void Initialize(const Ogre::vector<Ogre::String>::type & AddResLocations);
	void ReCreateSceneMgr();
	bool CreateOgreRoot();
	void RemoveOgreRoot();
	Ogre::Root* GetOgreRoot();

	Ogre::OverlaySystem * GetOverlaySystem();

	void Terminate();

	//@overridden
	void eventOccurred(const Ogre::String& eventName, 
		const Ogre::NameValuePairList* parameters);

public:
	bool InitializeOgreWidget(int viewType, Ogre::String strOgreWidgetName, OgreWidget *pOgreWidget);
	void RemoveAllOgreWidget(void);
	void DetachOgreWidget(OgreWidget * ogrewidget);

	OgreWidget* GetOgreWidgetByName(Ogre::String strOgreWidgetName);//get qt window point

	//rbWindow manage
	//bool InitializeRbWindowWidget(Ogre::String strRbWindowName, RbWindow *pRbWindow);
	//void RemoveAllRbWindow(void);
	//RbWindow* GetRbWindowByName(Ogre::String strRbWindowName);
	
	Ogre::RenderWindow* GetRenderWindowByName(Ogre::String strOgreWidgetName);
	Ogre::RenderWindow* GetMainRenderWindow();

	Ogre::SceneManager* GetDefaultSceneManger();

	void Update(ITraining * currtrain);

	Ogre::Matrix4 GetNodeMatrix(Ogre::SceneNode * pNode); // get one node's matrix, zx
	Ogre::Matrix4 GetCameraMatrix(Ogre::Camera * pCamera); // get camera's matrix, zx

	void SetCameraSpecialAngle(Ogre::Camera * pCamera, const Ogre::Quaternion& quatDefaultQuaternion, Ogre::Degree degree); // set camera in a special angle, zx

protected:
	

	void CreateSceneMgr();

	void CreateCamera();

	void LoadResourcePathsFromConfigFile(const Ogre::String& strFileName , const Ogre::vector<Ogre::String>::type & AddResLocations);
	//void LoadResourcePathsFromConfigFile(const std::string& filename);

	//Ogre::FrameListener callbacks
	bool frameStarted(const Ogre::FrameEvent& evt);
	
	bool frameEnded(const Ogre::FrameEvent& evt);

private:
	//Ogre Stuff
	Ogre::RenderSystem* m_ActiveRenderSystem;
	Ogre::RenderSystem* m_OpenGLRenderSystem;
	Ogre::RenderSystem* m_Direct3D9RenderSystem;
	Ogre::Root* m_Root;

	// temp,wait config file. need list
	Ogre::SceneManager* m_SceneMgr;
	Ogre::Camera* m_Camera;
	Ogre::OverlaySystem * mOverlaySystem;
	//QT Widget
	typedef std::map<Ogre::String, OgreWidget*> MAP_OGRE_WIDGET;
	MAP_OGRE_WIDGET m_mapOgreWidget;

	//QT RbWindow
	float m_TimeSinceLastRend;

	float m_RawTimeSLR;

	bool m_bInitialized;
	
	HDRLogic * m_HDRLogic;

public:

	int m_nDelayedSeconds;
	Ogre::Real m_FrameElapsedTime;

	typedef std::map<Ogre::String,RbWindow*> MAP_RBWINDOW;
    //MAP_RBWINDOW m_mapRbWindow;
};
