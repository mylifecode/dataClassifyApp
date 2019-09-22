#ifndef _DEFERREDRENDFRAMEWORK_
#define _DEFERREDRENDFRAMEWORK_
#include "ogre.h"
#include "Singleton.h"
#include "DeferredRendSceneManager.h"
//this is more like ogre's compositor but can let use get more control
class DeferedRendTarget : public Ogre::ManualResourceLoader
{
public:
	DeferedRendTarget();

	~DeferedRendTarget();

	void RebuildScreeQuadVertex(int screenWid , int screenHei);

	void DrawScreenQuad(const Ogre::String & quadMat);

	void DrawExternalScene();

	void CreateRendeTarget(const Ogre::String & name , int Wdi , int Hei , float dimscale , Ogre::PixelFormat pixefmt , Ogre::RenderTargetListener * targetlistener);

	void ResizeRendTarget(int Wdi , int Hei);

	virtual void loadResource(Ogre::Resource* resource);

	
	Ogre::Viewport * addViewPortExternSceneMgr(Ogre::Camera * camera ,
											   bool clearEveryFrame ,
											   unsigned int clearBuffers,
											   Ogre::ColourValue bgcolor,
											   bool enableoverlay,
											   const Ogre::String & materialschema);

//protected:
	void DestroyRenderTarget();
	
	void RemoveRendTexture();

	int m_SrcWidth;
	int m_SrcHeight;
	float m_DimScale;

	Ogre::TexturePtr m_RendTexture;
	Ogre::RenderTarget * m_RendTarget;
	Ogre::Viewport * m_pViewPort;

	//for draw external scene
	Ogre::Camera * m_SceneCamera;
	Ogre::SceneManager * m_SceneMgr;
	//

	//for draw full screen quad
	Ogre::SceneManager * m_pScreenQuadSceneMgr;
	Ogre::Camera* m_pScreenQuadCamera;
	Ogre::ManualObject * m_pScreenQuadObj;
	//
	
	Ogre::RenderTargetListener * m_targetlistener;
	//Ogre::Viewport * m_ViewPort;
	bool m_vpOverlayEnabled;
	Ogre::ColourValue m_vpBgColor;
	bool m_vpClearEveryFrame;
	unsigned int m_vpClearbuffers;

	Ogre::String m_vpMaterialSchema;


	bool m_IsExternSceneMgr;
};

class DeferredRendFrameWork;
class DeferedPostScreenProcessor
{
public:
	virtual ~DeferedPostScreenProcessor(){}
	
	virtual void SceneOpaqueStageFinish(Ogre::RenderWindow * renderWindow  , DeferredRendFrameWork * framwork) = 0;

	virtual void SceneAllStageFinish(Ogre::RenderWindow * renderWindow  , DeferedRendTarget * finalsceneTarget , DeferredRendFrameWork * framwork) = 0;

	virtual void Initialize(Ogre::RenderWindow * renderWindow , DeferredRendFrameWork * framwork) = 0;

};
class CSSAO;
class DeferredRendFrameWork : public CSingleT<DeferredRendFrameWork> , public DeferredRendSceneManager::DeferedSceneRendListener
{
public:
	class DeferreRendStageListener
	{
	public:
		virtual void BeforeRendStage(DeferredRendSceneManager::DeferredRendStage) = 0;
		virtual void AfterRendStage(DeferredRendSceneManager::DeferredRendStage) = 0;
	};

	std::vector<DeferreRendStageListener*> m_Listeners;

	void AddStageRendListener(DeferreRendStageListener * listener)
	{
		m_Listeners.push_back(listener);
	}

	void RemoveStageRendListener(DeferreRendStageListener * listener)
	{
		for (int c = 0; c < (int)m_Listeners.size(); c++)
		{
			if (m_Listeners[c] == listener)
			{
				m_Listeners.erase(m_Listeners.begin()+c);
				break;
			}
		}
	}

	bool m_UseCustomRendStage;

	DeferredRendFrameWork();

	virtual ~DeferredRendFrameWork();

	void SetBloomAndCCParameter(float hilightthreshold, 
		                        float contrast,
		                        float brightness,
		                        float saturate);
	
	void SetSSAOParameter(Ogre::Vector3 lightCorrectParameter);

	void SetGammaCorrectInModulate(float gamma);

	void OnInitialize(Ogre::Camera * pSceneCamera , Ogre::RenderWindow * renderWindow , bool enablessao);

	void _rendScene(Ogre::RenderWindow * renderWindow);

	void RendSceneOnViewPort(Ogre::Viewport * vp);

	Ogre::TexturePtr GetFinalSceneTexture();//get final scene (include opaque object with ssao and transparent object , this is the may post process will start)
protected:
	void RenderWindowSizeChanged(Ogre::RenderWindow * RendWindow , int WinWidth , int WinHeight);//when rend window size changed

	void RebuildScreeQuadVertex(int screenWid , int screenHei);

	/** @copydoc DeferedSceneRendListener*/
	void BeforeRendVisibleObjects(DeferredRendSceneManager * sceneMgr , DeferredRendSceneManager::DeferredRendStage currentStage);

	//for debug
	void writeSceneTexture();

	void writeDepthTexture();

	//GBuffer Start
	DeferedRendTarget * m_DepthTarget;
	DeferedRendTarget * m_DiffuseTarget;//currently unused
	DeferedRendTarget * m_SpecularTarget;//currently unused
	DeferedRendTarget * m_NormalTarget;//currently unused
	//GBuffer End
	Ogre::MultiRenderTarget * m_Mrt;

	DeferedRendTarget * m_LightedSceneTarget;

	DeferedRendTarget * m_FinalSceneTarget;//final lighted scene can be do post-process like hdr , dof etc

	Ogre::RenderWindow * m_RendWindow;

	int m_RendWindowWidth;
	int m_RendWindowHeight;

	std::vector<DeferedPostScreenProcessor*> m_PostProcessor;

public:
	//
	//member for draw screen quad
	Ogre::SceneManager * m_pScreenQuadSceneMgr;
	Ogre::Camera* m_pScreenQuadCamera;
	Ogre::ManualObject * m_pScreenQuadObj;
	//Ogre::Viewport * m_pScreenQuadViewPort;
	
	//
	//
	Ogre::Camera * m_pSceneCamera;
    DeferredRendSceneManager * m_pSceneManager;
	Ogre::Viewport * m_pSceneViewPortOnRendWindow;
};



#endif