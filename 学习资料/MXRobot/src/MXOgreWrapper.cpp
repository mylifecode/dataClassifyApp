#include "StdAfx.h"
#include "MXOgreWrapper.h"
#include <QFile>
#include <OgreConfigFile.h>
#include "OgreTextureManager.h"
#include "OgreMaterialManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreOverlayManager.h"
#include "OgreCompositorManager.h"
#include "OgreCompositor.h"
#include "HelperLogics.h"
#include "DynamicObjectRenderable.h"
#include "ITraining.h"
#include "ShadowMap.h"
#include "NewTrain\VeinConnectRender.h"
#include "LightMgr.h"
#include "DeferredRendFrameWork.h"
#include "DeferredRendSceneManager.h"
#include "MisMedicOrganRender.h"
MXOgreWrapper::MXOgreWrapper(void):m_ActiveRenderSystem(NULL)
,m_OpenGLRenderSystem(NULL)
,m_Direct3D9RenderSystem(NULL)
,m_Root(NULL)
,m_SceneMgr(NULL)
,m_Camera(NULL)
,m_HDRLogic( NULL )
{
	m_mapOgreWidget.clear();
	m_FrameElapsedTime = 0;
	m_bInitialized = false;
	m_TimeSinceLastRend = 0;
	m_RawTimeSLR = 0;
	mOverlaySystem = 0;
}

MXOgreWrapper::~MXOgreWrapper(void)
{
    SAFE_DELETE( m_HDRLogic );
}


void MXOgreWrapper::Initialize(const Ogre::vector<Ogre::String>::type & AddResLocations)
{
	//if (m_bInitialized) return;
	//m_bInitialized = true;
	mOverlaySystem = new Ogre::OverlaySystem();

	if(QFile::exists("..\\Config\\resources.cfg"))
	{
		LoadResourcePathsFromConfigFile("..\\Config\\resources.cfg" , AddResLocations);
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
	}

	CreateSceneMgr();

	CreateCamera();

	if(m_SceneMgr)
	{
	   m_SceneMgr->addRenderQueueListener(mOverlaySystem);
	}
	
	//for render have no fix function support , we use our program shader material replace the content
	//e.g. "BaseWhiteNoLighting"
	if(m_ActiveRenderSystem->getCapabilities()->hasCapability(Ogre::RSC_FIXED_FUNCTION) == false)
	{
		Ogre::MaterialPtr baseWhiteNoLight = Ogre::MaterialManager::getSingleton().getByName("BaseWhiteNoLighting", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
		Ogre::MaterialPtr replaceMat = Ogre::MaterialManager::getSingleton().getByName("MisMedical/BaseOpaqueTemplate_NoLit", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
		replaceMat->copyDetailsTo(baseWhiteNoLight);

		Ogre::MaterialPtr baseWhite = Ogre::MaterialManager::getSingleton().getByName("BaseWhite", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
		replaceMat = Ogre::MaterialManager::getSingleton().getByName("MisMedical/BaseOpaqueTemplateLitNoTex", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
		replaceMat->copyDetailsTo(baseWhite);
	}
	//also assign vertex pixel shader to some font material
	std::vector<Ogre::String> fontsVec;
	fontsVec.push_back("StarWars");
	fontsVec.push_back("mx_number");

	for (size_t f = 0; f < fontsVec.size(); f++)
	{
		Ogre::String  fontName = fontsVec[f];
		Ogre::FontPtr fontObject  = Ogre::FontManager::getSingleton().getByName(fontName);
		fontObject->load();
		Ogre::String fontmaterial = fontObject->getMaterial()->getName();
		Ogre::MaterialPtr matefontPtr = Ogre::MaterialManager::getSingleton().getByName(fontmaterial);
		if (matefontPtr->getNumTechniques() > 0)
		{
			Ogre::Technique * FontTech = matefontPtr->getTechnique(0);

			Ogre::Pass * FontPass = FontTech->getPass(0);

			//this is a hack for font material since dx11 not support fix pipeline so we manually add shader
			if (FontPass->hasVertexProgram() == false || FontPass->hasFragmentProgram() == false)
			{
				FontPass->setVertexProgram("FontShader_VS");
				FontPass->setFragmentProgram("FontShader_PS");
			}
		}
	}
	//create shadow map object
	CShadowMap::Create();
	CShadowMap::Get()->createShadowTexture(2048, 2048);

	DeferredRendFrameWork::Create();
		
}

void MXOgreWrapper::Terminate()
{
	//Ogre::StringVector rgs = Ogre::ResourceGroupManager::getSingletonPtr()->getResourceGroups();
	//Ogre::StringVector::iterator i = rgs.begin();
	//while (i != rgs.end())
	//{
	//	Ogre::ResourceGroupManager::getSingletonPtr()->clearResourceGroup(*i);
	//	//Ogre::ResourceGroupManager::getSingletonPtr()->destroyResourceGroup(*i);
	//	i++;
	//}
	////rgs = Ogre::ResourceGroupManager::getSingletonPtr()->getResourceGroups();
	//Ogre::TextureManager::getSingletonPtr()->destroyAllResourcePools();
	//Ogre::TextureManager::getSingletonPtr()->removeAll();
	//Ogre::MaterialManager::getSingletonPtr()->destroyAllResourcePools();
	//Ogre::MaterialManager::getSingletonPtr()->removeAll();
	//Ogre::ParticleSystemManager::getSingletonPtr()->removeAllTemplates(true);
	//Ogre::ParticleSystemManager::getSingletonPtr()->removeAllTemplates(false);
	//Ogre::OverlayManager::getSingletonPtr()->destroyAllOverlayElements(true);
	//Ogre::OverlayManager::getSingletonPtr()->destroyAllOverlayElements(false);
	//Ogre::OverlayManager::getSingletonPtr()->destroyAll();
	//PostAndPreEffect::GetSingleton().Terminate();
	CShadowMap::Destroy();

	DeferredRendFrameWork::Destroy();
	if (mOverlaySystem) 
	   delete mOverlaySystem;
}

bool MXOgreWrapper::InitializeOgreWidget( int viewType, Ogre::String strOgreWidgetName, OgreWidget *pOgreWidget )
{
	MAP_OGRE_WIDGET::iterator iter = m_mapOgreWidget.find(strOgreWidgetName);

	if (iter == m_mapOgreWidget.end())
	{
		m_mapOgreWidget[strOgreWidgetName] = pOgreWidget;   //map
		Ogre::NameValuePairList ogreWindowParams;
		pOgreWidget->initialise(viewType, strOgreWidgetName, &ogreWindowParams);
		return true;
	}
	else
	{
		return false;
	}
}


OgreWidget* MXOgreWrapper::GetOgreWidgetByName(Ogre::String strOgreWidgetName)
{
	MAP_OGRE_WIDGET::iterator iter = m_mapOgreWidget.find(strOgreWidgetName);

	if (iter == m_mapOgreWidget.end())
	{
		return NULL;
	}
	else
	{
		return iter->second;
	}
}

Ogre::RenderWindow* MXOgreWrapper::GetRenderWindowByName( Ogre::String strOgreWidgetName )
{
	MAP_OGRE_WIDGET::iterator iter = m_mapOgreWidget.find(strOgreWidgetName);

	if (iter == m_mapOgreWidget.end())
	{
		return NULL;
	}
	else
	{
		return iter->second->getOgreRenderWindow();
	}
}

Ogre::RenderWindow* MXOgreWrapper::GetMainRenderWindow()
{
	return GetRenderWindowByName(RENDER_WINDOW_LARGE);
}

void MXOgreWrapper::Update(ITraining * currtrain)
{
	if(currtrain == 0)
	   return;

	Ogre::Root::getSingleton()._fireFrameStarted();

	if (currtrain != NULL)
	{
		currtrain->Update(m_FrameElapsedTime);
	}

	//限制每秒最高渲染帧率
	float maxRendFramePerSecond = 100.0f;
	
	float maxrendInterval = 1.0f / maxRendFramePerSecond;

	m_TimeSinceLastRend += m_FrameElapsedTime;

	m_RawTimeSLR += m_FrameElapsedTime;

	if(m_TimeSinceLastRend > maxrendInterval)
	{
		currtrain->BeginRendOneFrame(m_RawTimeSLR);
		
		m_TimeSinceLastRend = m_TimeSinceLastRend-maxrendInterval;
		
		if(m_TimeSinceLastRend > maxrendInterval)
		   m_TimeSinceLastRend = maxrendInterval;
		
		m_RawTimeSLR = 0;
		//ruo yu add rend depth of the scene first (for soft particle and strips)
		//PostAndPreEffect::GetSingleton().update(currtrain->m_pLargeCamera);

	#ifdef USE_SSAO
		CShadowMap::Get()->_update((Ogre::Camera*)m_pLargeCamera);
	#else
		CShadowMap::Get()->_update(CLightMgr::Instance()->GetLight() , currtrain->m_pLargeCamera->getRealUp());
	#endif

		if (m_mapOgreWidget.size() > 0)
		DeferredRendFrameWork::Instance()->_rendScene(m_mapOgreWidget.begin()->second->getOgreRenderWindow());

		//MAP_OGRE_WIDGET::iterator it = m_mapOgreWidget.begin();
		//while (it != m_mapOgreWidget.end())
		//{
			//it->second->getOgreRenderWindow()->update();
			//++it;
		//}	
	}
	Ogre::Root::getSingleton()._fireFrameRenderingQueued();
	Ogre::Root::getSingleton()._fireFrameEnded();
}

bool MXOgreWrapper::CreateOgreRoot()
{
	bool bRet = true;

#if defined(NDEBUG)
	m_Root = new Ogre::Root("..\\Config\\plugins.cfg");	
#else
	m_Root = new Ogre::Root("..\\Config\\plugins_d.cfg");
#endif
	
	if (m_Root == NULL)
	{
		bRet = false;
	}

	m_OpenGLRenderSystem = m_Root->getRenderSystemByName("OpenGL Rendering Subsystem");
	m_Direct3D9RenderSystem = m_Root->getRenderSystemByName("Direct3D11 Rendering Subsystem");
	if(!(m_OpenGLRenderSystem || m_Direct3D9RenderSystem))
	{
		qCritical("No rendering subsystems found");
		bRet = false;
	}

	m_ActiveRenderSystem = m_Direct3D9RenderSystem;

	Ogre::Root::getSingletonPtr()->setRenderSystem(m_ActiveRenderSystem);

	Ogre::Root::getSingletonPtr()->initialise(false);

	Ogre::Root::getSingletonPtr()->addFrameListener(this);
	
	m_ActiveRenderSystem->addListener(this);

	//高光 [3/20/2012 yl]
	//Ogre::CompositorManager& compMgr = Ogre::CompositorManager::getSingleton();
	//m_HDRLogic = new HDRLogic;
	//compMgr.registerCompositorLogic("HDR", m_HDRLogic );

	
	//initialize moveable object factory
	DynamicObjectRenderableFactory::Initalize();
	
	VeinConnectStripsObjectFactory::Initalize();

	MisMedicOrganRenderFactory::Initalize();

	DeferredRendSceneManagerFactory::Create();
	Ogre::Root::getSingleton().addSceneManagerFactory(DeferredRendSceneManagerFactory::Get());

	return bRet;

}

void MXOgreWrapper::RemoveOgreRoot()
{
	if(m_Root!=NULL)
	{
		Ogre::Root::getSingleton().destroySceneManager(m_SceneMgr);
		m_SceneMgr=NULL;

		DynamicObjectRenderableFactory::Terminate();
		//DynamicStripsObjectFactory::Terminate();
		VeinConnectStripsObjectFactory::Terminate();
		MisMedicOrganRenderFactory::Terminate();

		Ogre::Root::getSingleton().removeSceneManagerFactory(DeferredRendSceneManagerFactory::Get());
		DeferredRendSceneManagerFactory::Destroy();

		SAFE_DELETE(m_Root);
	}
}

Ogre::Root* MXOgreWrapper::GetOgreRoot()
{
	return m_Root;
}

Ogre::OverlaySystem * MXOgreWrapper::GetOverlaySystem()
{
	return mOverlaySystem;
}

void MXOgreWrapper::CreateSceneMgr()
{
	// temp,wait config file
	if(m_SceneMgr == NULL)
	{
		m_SceneMgr = Ogre::Root::getSingleton().createSceneManager(DeferredRendSceneManagerFactory::FACTORY_TYPE_NAME/*Ogre::ST_GENERIC*/, "GenericSceneManager");
	}

}

void MXOgreWrapper::ReCreateSceneMgr()
{
	// temp,wait config file
	if(m_SceneMgr != NULL)
	{
		Ogre::Root::getSingleton().destroySceneManager(m_SceneMgr);
		m_SceneMgr=NULL;
		m_SceneMgr = Ogre::Root::getSingleton().createSceneManager(DeferredRendSceneManagerFactory::FACTORY_TYPE_NAME/*Ogre::ST_GENERIC*/, "GenericSceneManager");
	}
}

Ogre::SceneManager* MXOgreWrapper::GetDefaultSceneManger()
{
	return m_SceneMgr;	
}


void MXOgreWrapper::LoadResourcePathsFromConfigFile(const Ogre::String& strFileName , const Ogre::vector<Ogre::String>::type & AddResLocations)
{
	// Load resource paths from config file
	Ogre::ConfigFile cf;
	cf.load(strFileName);

	// Go through all sections & settings in the file
	Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements())
	{
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i)
		{
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
				archName, typeName, secName);
		}
	}
	for(size_t c = 0 ; c < AddResLocations.size() ; c++)
	{
		Ogre::String resString = AddResLocations[c];
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation(resString, "FileSystem");
	}
}

void MXOgreWrapper::SetCameraSpecialAngle(Ogre::Camera * pCamera, const Ogre::Quaternion& quatDefaultQuaternion, Ogre::Degree degree)
{
	degree = (-1) * degree;
	//Ogre::Matrix4 mx4Camera = GetCameraMatrix(pCamera);
	Ogre::Quaternion quatRotate(Ogre::Radian(degree), Ogre::Vector3::UNIT_X);
	Ogre::Quaternion quatNow = quatDefaultQuaternion * quatRotate;

	pCamera->setOrientation(quatNow);
}

void MXOgreWrapper::CreateCamera()
{

}

Ogre::Matrix4 MXOgreWrapper::GetNodeMatrix(Ogre::SceneNode * pNode)
{
	Ogre::Vector3 v3Pos = pNode->_getDerivedPosition();

	Ogre::Matrix4 mxTrans(1,0,0,0, 0,1,0,0, 0,0,1,0, v3Pos.x, v3Pos.y, v3Pos.z, 1);
	Ogre::Matrix4 mxScale(pNode->_getDerivedScale().x,0,0,0, 0,pNode->_getDerivedScale().y,0,0, 0,0,pNode->_getDerivedScale().z,0, 0,0,0,1);
	Ogre::Matrix3 mx3Rotate;
	pNode->_getDerivedOrientation().ToRotationMatrix(mx3Rotate);

	Ogre::Matrix4 mx4Rotate(mx3Rotate[0][0], mx3Rotate[1][0], mx3Rotate[2][0], 0,
		mx3Rotate[0][1], mx3Rotate[1][1], mx3Rotate[2][1], 0,
		mx3Rotate[0][2], mx3Rotate[1][2], mx3Rotate[2][2], 0,
		0, 0, 0, 1);
	Ogre::Matrix4 mxNode = mxScale * mx4Rotate * mxTrans;	
	return mxNode;
}

Ogre::Matrix4 MXOgreWrapper::GetCameraMatrix(Ogre::Camera * pCamera)
{
	Ogre::Vector3 v3Pos = pCamera->getPosition();

	Ogre::Matrix4 mxTrans(1,0,0,0, 0,1,0,0, 0,0,1,0, v3Pos.x, v3Pos.y, v3Pos.z, 1);
	Ogre::Matrix4 mxScale = Ogre::Matrix4::IDENTITY;
	Ogre::Matrix3 mx3Rotate;
	pCamera->getOrientation().ToRotationMatrix(mx3Rotate);
	//Ogre::Matrix4 mx4Rotate(mx3Rotate[0][0], mx3Rotate[0][1], mx3Rotate[0][2], 0,
	//						mx3Rotate[1][0], mx3Rotate[1][1], mx3Rotate[1][2], 0,
	//						mx3Rotate[2][0], mx3Rotate[2][1], mx3Rotate[2][2], 0,
	//						0, 0, 0, 1);
	Ogre::Matrix4 mx4Rotate(mx3Rotate[0][0], mx3Rotate[1][0], mx3Rotate[2][0], 0,
							mx3Rotate[0][1], mx3Rotate[1][1], mx3Rotate[2][1], 0,
							mx3Rotate[0][2], mx3Rotate[1][2], mx3Rotate[2][2], 0,
							0, 0, 0, 1);
	Ogre::Matrix4 mxNode = mxScale * mx4Rotate * mxTrans;	
	return mxNode;
}

void MXOgreWrapper::RemoveAllOgreWidget( void )
{
	MAP_OGRE_WIDGET::iterator it = m_mapOgreWidget.begin();
	while (it != m_mapOgreWidget.end())
	{
		Ogre::RenderSystem * rs = Ogre::Root::getSingletonPtr()->getRenderSystem();
		
		rs->destroyRenderTarget(it->first);
		it->second->Destroy();//不要delete Qt在析构父窗口时会自动delete 这里delete了后父窗口（SYTrainWindow的bigglwidget）会有野指针！
		//delete it->second;
		it = m_mapOgreWidget.erase(it);
	}
	RemoveOgreRoot();
}

void MXOgreWrapper::DetachOgreWidget(OgreWidget * ogrewidget)
{
	MAP_OGRE_WIDGET::iterator itor = m_mapOgreWidget.begin();
	
	while(itor != m_mapOgreWidget.end())
	{
	   if(itor->second == ogrewidget)
	   {
		  m_mapOgreWidget.erase(itor);
	      break;
	   }
	   itor++;
	}
}
/*void MXOgreWrapper::DelayStop(int nDelayedSeconds)
{
	m_bDelayStopOn = true;
	m_dwDelayStopStartTime = GetTickCount();
	m_nDelayedSeconds = nDelayedSeconds;
}
*/

//  [4/16/2012 yl]
//bool MXOgreWrapper::InitializeRbWindowWidget(Ogre::String strRbWindowName, RbWindow *pRbWindow)
//{
//	MAP_RBWINDOW::iterator iter=m_mapRbWindow.find(strRbWindowName);
//	if (iter == m_mapRbWindow.end())
//	{
//		m_mapRbWindow[strRbWindowName]=pRbWindow;
//		return true;
//	}
//	else
//	{
//		return false;
//	}
//}
//
//RbWindow* MXOgreWrapper::GetRbWindowByName(Ogre::String strRbWindowName)
//{
//	MAP_RBWINDOW::iterator iter=m_mapRbWindow.find(strRbWindowName);
//	if (iter == m_mapRbWindow.end())
//	{
//		return NULL;
//	}
//	else
//	{
//		return iter->second;
//	}
//}
//
//void MXOgreWrapper::RemoveAllRbWindow()
//{
//	m_mapRbWindow.clear();
//}


bool MXOgreWrapper::frameStarted(const Ogre::FrameEvent& evt)
{
	m_FrameElapsedTime = evt.timeSinceLastFrame;
	return true;
}

bool MXOgreWrapper::frameEnded(const Ogre::FrameEvent& evt)
{
	//this->elapsedTime=evt.timeSinceLastFrame;
	return true;
}
void MXOgreWrapper::eventOccurred(const Ogre::String& eventName, 
							      const Ogre::NameValuePairList* parameters)
{
	if(eventName== "DeviceLost")
	{

	}
	else if(eventName== "DeviceRestored")
	{

	}
}