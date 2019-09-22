#include "DeferredRendFrameWork.h"
#include "SSAOV2.h"
#include "PostEffect_Bloom.h"
#define USEMRT 1
DeferedRendTarget::DeferedRendTarget()
{
	m_RendTarget = 0;
	m_pViewPort = 0;
	m_SceneMgr = 0;
	m_SceneCamera = 0;

	m_pScreenQuadSceneMgr = 0;
	m_pScreenQuadCamera = 0;
	m_pScreenQuadObj = 0;

	m_IsExternSceneMgr = false;
	m_SrcWidth = 0;
	m_SrcHeight = 0;
	m_DimScale = 1.0f;
	m_targetlistener = 0;
	

	m_vpOverlayEnabled = false;
	m_vpBgColor = Ogre::ColourValue();
	m_vpClearEveryFrame = false;
	m_vpClearbuffers = 0;
	m_vpMaterialSchema = "";
}
//=====================================================================================
DeferedRendTarget::~DeferedRendTarget()
{
	DestroyRenderTarget();
	RemoveRendTexture();

	//destroy Screen Quad Relative Object
	if(m_pScreenQuadSceneMgr)
	{
		if(m_pScreenQuadObj)
		{
			m_pScreenQuadSceneMgr->getRootSceneNode()->detachObject(m_pScreenQuadObj);
			m_pScreenQuadSceneMgr->destroyMovableObject(m_pScreenQuadObj);
			m_pScreenQuadObj = 0;
		}
		if(m_pScreenQuadCamera)
		{
			m_pScreenQuadSceneMgr->destroyCamera(m_pScreenQuadCamera);
			m_pScreenQuadCamera = 0;
		}
		Ogre::Root::getSingleton().destroySceneManager(m_pScreenQuadSceneMgr);
		m_pScreenQuadSceneMgr = 0;
	}
}
//=====================================================================================
void DeferedRendTarget::CreateRendeTarget(const Ogre::String & texname , int buffWid , int buffHei , float dimscale , Ogre::PixelFormat pixefmt , Ogre::RenderTargetListener * targetlistener)
{
	m_DimScale = dimscale;
	m_SrcWidth  = buffWid * dimscale;
	m_SrcHeight = buffHei * dimscale;
	m_RendTexture = Ogre::TextureManager::getSingleton().createManual(  texname,
																		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
																		Ogre::TEX_TYPE_2D,
																		m_SrcWidth ,	m_SrcHeight,	
																		0,
																		pixefmt, //Ogre::PF_FLOAT32_RGBA,  //Ogre::PF_A8R8G8B8,//
																		Ogre::TU_RENDERTARGET,
																		this);
	m_RendTexture->load();//call load at once , else ogre inner state will be error

	m_RendTarget = m_RendTexture->getBuffer()->getRenderTarget();//get pointer of rend target
	m_RendTarget->setAutoUpdated(false);//manually update
	if(targetlistener)
	{
	   m_RendTarget->addListener(targetlistener);
	   m_targetlistener = targetlistener;
	}

	//create screen quad for draw on this target
	static int DeferRendTargetID = 0;

	Ogre::String nameSceneMgr = "DeferTargetQuadSceneMgr" + Ogre::StringConverter::toString(DeferRendTargetID);

	m_pScreenQuadSceneMgr = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, nameSceneMgr);
	m_pScreenQuadCamera = m_pScreenQuadSceneMgr->createCamera("EffectCamera");

	m_pScreenQuadObj = m_pScreenQuadSceneMgr->createManualObject("QuadObj");
	m_pScreenQuadObj->setRenderQueueGroup(Ogre::RENDER_QUEUE_BACKGROUND);
	m_pScreenQuadSceneMgr->getRootSceneNode()->attachObject(m_pScreenQuadObj);

	m_RendTarget->removeAllViewports();//only allow one view port exists one time
	m_pViewPort = m_RendTarget->addViewport( m_pScreenQuadCamera ); 
	m_pViewPort->setClearEveryFrame(false);
	m_pViewPort->setOverlaysEnabled(false);
	m_pViewPort->setAutoUpdated(false);
	m_IsExternSceneMgr = false;

	RebuildScreeQuadVertex(m_SrcWidth , m_SrcHeight);

	DeferRendTargetID++;
}
//=============================================================================================================
void DeferedRendTarget::DestroyRenderTarget()
{
	if(m_RendTarget)
	{
	   m_RendTarget->removeAllViewports();
	   m_RendTarget->removeAllListeners();
	   Ogre::Root::getSingleton().getRenderSystem()->destroyRenderTarget(m_RendTarget->getName());
	   m_RendTarget = 0;
	}
}
//=============================================================================================================
void DeferedRendTarget::RebuildScreeQuadVertex(int screenWid , int screenHei)
{
	//Screen QuadObject Build
	Real hOffset = Ogre::Root::getSingleton().getRenderSystem()->getHorizontalTexelOffset() / (0.5f * screenWid);
	Real vOffset = Ogre::Root::getSingleton().getRenderSystem()->getVerticalTexelOffset()   / (0.5f * screenHei);

	float mQuadLeft , mQuadTop , mQuadRight , mQuadBottom;
	mQuadLeft   = -1 + hOffset;
	mQuadTop    =  1 - vOffset;
	mQuadRight  =  1 + hOffset;
	mQuadBottom = -1 - vOffset;

	//rebuild screen Quad vertex
	m_pScreenQuadObj->clear();
	m_pScreenQuadObj->begin("SSAO/CrytekSAO", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	m_pScreenQuadObj->position(Ogre::Vector3(mQuadRight, mQuadBottom, -1.0));
	m_pScreenQuadObj->position(Ogre::Vector3(mQuadLeft,  mQuadTop, -1.0));
	m_pScreenQuadObj->position(Ogre::Vector3(mQuadLeft,  mQuadBottom, -1.0));
	m_pScreenQuadObj->position(Ogre::Vector3(mQuadRight, mQuadTop, -1.0));

	//using indices
	m_pScreenQuadObj->index(0);
	m_pScreenQuadObj->index(1);
	m_pScreenQuadObj->index(2);
	m_pScreenQuadObj->index(0);
	m_pScreenQuadObj->index(3);
	m_pScreenQuadObj->index(1);
	m_pScreenQuadObj->end();
	m_pScreenQuadObj->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);//make always visible (i.e be draw)
}
//=============================================================================================================
void DeferedRendTarget::DrawScreenQuad(const Ogre::String & quadMat)
{
	if(m_IsExternSceneMgr)
	{
		OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR , "target contains extern scene, DrawScreenQuad failed", "DeferedRendTarget::DrawScreenQuad");
	}
	else
	{
		m_pScreenQuadObj->setMaterialName(0 , quadMat, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		Ogre::RenderTarget * rt = m_pViewPort->getTarget();
		rt->_beginUpdate();
		rt->_updateViewport(m_pViewPort , true);
		rt->_endUpdate();
		rt->swapBuffers();
	}
}
//=============================================================================================================
void DeferedRendTarget::DrawExternalScene()
{
	if(m_IsExternSceneMgr)
	{
		Ogre::RenderTarget * rt = m_pViewPort->getTarget();
		rt->_beginUpdate();
		rt->_updateViewport(m_pViewPort , true);
		rt->_endUpdate();
		rt->swapBuffers();
	}
	else
	{
		OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR , "target has no extern scene", "DeferedRendTarget::DrawExternalScene");
	}
}
//=============================================================================================================
void DeferedRendTarget::RemoveRendTexture()
{
	//remove render texture
	if(m_RendTexture.isNull() == false)
	{
		Ogre::TextureManager::getSingleton().remove(m_RendTexture->getName());
		m_RendTexture.setNull();
	}
}
//=====================================================================================
void DeferedRendTarget::ResizeRendTarget(int Wid , int Hei)
{
	//
	DestroyRenderTarget();
	m_SrcWidth  = Wid * m_DimScale;
	m_SrcHeight = Hei * m_DimScale;
	if(m_RendTexture->isLoaded())
	   m_RendTexture->reload();
	else
	   m_RendTexture->load();
	
	//after recreate texture reget the render target
	m_RendTarget = m_RendTexture->getBuffer()->getRenderTarget();
	m_RendTarget->setAutoUpdated(false);//manually update
	if(m_targetlistener)
	   m_RendTarget->addListener(m_targetlistener);

	if(m_RendTarget)
	{
		m_RendTarget->removeAllViewports();
		
		if (m_IsExternSceneMgr)//this target is used to rend scene
		{
			m_pViewPort = m_RendTarget->addViewport( m_SceneCamera); 
			m_pViewPort->setClearEveryFrame(m_vpClearEveryFrame , m_vpClearbuffers); //clear tag
			m_pViewPort->setBackgroundColour(m_vpBgColor);   //back buff clear color
			m_pViewPort->setOverlaysEnabled(m_vpOverlayEnabled);
			if(m_vpMaterialSchema != "")
			   m_pViewPort->setMaterialScheme(m_vpMaterialSchema);
		}
		else//this target is used to rend screen quad
		{
			m_pViewPort = m_RendTarget->addViewport( m_pScreenQuadCamera ); 
			m_pViewPort->setClearEveryFrame(false);
			m_pViewPort->setOverlaysEnabled(false);
			m_pViewPort->setAutoUpdated(false);
		}
	}

	RebuildScreeQuadVertex(m_SrcWidth , m_SrcHeight);
}
//=====================================================================================
void DeferedRendTarget::loadResource(Ogre::Resource* resource)
{
	Ogre::Texture * renderTex = dynamic_cast<Ogre::Texture * >(resource);

	if(renderTex)
	{
		renderTex->setWidth(m_SrcWidth);
		renderTex->setHeight(m_SrcHeight);
		renderTex->createInternalResources();
	}
}
//=====================================================================================
Ogre::Viewport * DeferedRendTarget::addViewPortExternSceneMgr(Ogre::Camera * camera ,
															  bool clearEveryFrame ,
															  unsigned int clearBuffers,
															  Ogre::ColourValue bgcolor,
															  bool enableoverlay,
															  const Ogre::String & materialschema)
{
	m_IsExternSceneMgr = true;
	m_SceneMgr = camera->getSceneManager();
	m_SceneCamera = camera;
	m_RendTarget->removeAllViewports();//only allow one view port at a time

	m_pViewPort = m_RendTarget->addViewport( m_SceneCamera ); 

	m_vpClearEveryFrame = clearEveryFrame;
	m_vpClearbuffers = clearBuffers;
	m_pViewPort->setClearEveryFrame(clearEveryFrame , clearBuffers);

	m_vpBgColor = bgcolor;
	m_pViewPort->setBackgroundColour(bgcolor);

	m_vpOverlayEnabled = enableoverlay;
	m_pViewPort->setOverlaysEnabled(enableoverlay); 

	if(materialschema != "")
	{
	   m_vpMaterialSchema = materialschema;
	   m_pViewPort->setMaterialScheme(materialschema);
	}
	else
	{
	   m_vpMaterialSchema = "";
	}

	m_pViewPort->setAutoUpdated(false);
	
	return m_pViewPort;
}
//=====================================================================================
DeferredRendFrameWork::DeferredRendFrameWork()
{
	m_UseCustomRendStage = false;
	m_DepthTarget = 0;
	m_DiffuseTarget = 0;
	m_SpecularTarget = 0;
	m_NormalTarget = 0;
	m_LightedSceneTarget = 0;
    m_FinalSceneTarget = 0;

	//member for draw screen quad
	m_pScreenQuadSceneMgr = 0;
	m_pScreenQuadCamera = 0;
	m_pScreenQuadObj = 0;
	//m_pScreenQuadViewPort = 0;
	m_pSceneViewPortOnRendWindow = 0;
	//m_SSAO = 0;
	//
	m_Mrt = 0;
}
//=====================================================================================
DeferredRendFrameWork::~DeferredRendFrameWork()
{
	//remove scene & depth target
	if(m_LightedSceneTarget)
	{
	   delete m_LightedSceneTarget;
	   m_LightedSceneTarget = 0;
	}

	if(m_FinalSceneTarget)
	{
		delete m_FinalSceneTarget;
		m_FinalSceneTarget = 0;
	}

	if(m_DepthTarget)
	{
	   delete m_DepthTarget;
	   m_DepthTarget = 0;
	}

	if(m_Mrt)
	{
	   Ogre::Root::getSingleton().destroyRenderTarget(m_Mrt->getName());
	   m_Mrt = 0;
	}
	//delete all post effect
	for(size_t c = 0 ; c < m_PostProcessor.size() ; c++)
	{
		delete m_PostProcessor[c];
	}
	m_PostProcessor.clear();

	//if(m_SSAO)
	//{
	 //  delete m_SSAO;
	 //  m_SSAO = 0;
	//}
	//
	//destroy Screen Quad Relative Object
	if(m_pScreenQuadSceneMgr)
	{
		if(m_pScreenQuadObj)
		{
			m_pScreenQuadSceneMgr->getRootSceneNode()->detachObject(m_pScreenQuadObj);
			m_pScreenQuadSceneMgr->destroyMovableObject(m_pScreenQuadObj);
			m_pScreenQuadObj = 0;
		}
		if(m_pScreenQuadCamera)
		{
			m_pScreenQuadSceneMgr->destroyCamera(m_pScreenQuadCamera);
			m_pScreenQuadCamera = 0;
		}
		Ogre::Root::getSingleton().destroySceneManager(m_pScreenQuadSceneMgr);
		m_pScreenQuadSceneMgr = 0;
	}
}
void DeferredRendFrameWork::SetBloomAndCCParameter(float hilightthreshold,
	                                               float contrast,
	                                               float brightness,
	                                               float saturate)
{
	for(size_t c = 0 ; c < m_PostProcessor.size() ; c++)
	{
		PostEffect_Bloom * effectbloom = dynamic_cast<PostEffect_Bloom*>(m_PostProcessor[c]);
		if(effectbloom)
		{
		   effectbloom->SetBloomAndCCParameter(hilightthreshold, contrast , brightness , saturate);
		}
	}
}

void DeferredRendFrameWork::SetSSAOParameter(Ogre::Vector3 lightCorrectParameter)
{
	for (size_t c = 0; c < m_PostProcessor.size(); c++)
	{
		CSSAO * ssao = dynamic_cast<CSSAO*>(m_PostProcessor[c]);
		if (ssao)
		{
			ssao->SetAOLightCorrect(Ogre::Vector4(lightCorrectParameter.x, lightCorrectParameter.y , 0 ,0));
		}
	}
}
void DeferredRendFrameWork::SetGammaCorrectInModulate(float gamma)
{
	Ogre::GpuProgramParametersSharedPtr params = GetShaderParamterPtr("Post/ModulateScene", FRAGMENT_PROGRAME, 0, 0);
	if (params.isNull() == false)
		params->setNamedConstant("gammaCorrection", gamma);
}
//=====================================================================================
void DeferredRendFrameWork::OnInitialize(Ogre::Camera * pSceneCamera , Ogre::RenderWindow * renderWindow , bool enablessao)
{
	m_RendWindowWidth  = renderWindow->getWidth();
	m_RendWindowHeight = renderWindow->getHeight();
	
	m_RendWindow = renderWindow;
	m_pSceneCamera = pSceneCamera;
	m_pSceneManager = dynamic_cast<DeferredRendSceneManager*>(pSceneCamera->getSceneManager());
	m_pSceneManager->m_Listener = this;

	Ogre::DepthBuffer * depthBuff = renderWindow->getDepthBuffer();

	//construct screen Quad object and screen Quad scene manager
	m_pScreenQuadSceneMgr = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, "SSAOQuadSceneMgr");
	m_pScreenQuadCamera = m_pScreenQuadSceneMgr->createCamera("EffectCamera");

	m_pScreenQuadObj = m_pScreenQuadSceneMgr->createManualObject("QuadObj");
	m_pScreenQuadObj->setRenderQueueGroup(Ogre::RENDER_QUEUE_BACKGROUND);
	m_pScreenQuadSceneMgr->getRootSceneNode()->attachObject(m_pScreenQuadObj);
	RebuildScreeQuadVertex(m_RendWindowWidth , m_RendWindowHeight);

	if(m_pSceneViewPortOnRendWindow == 0)
	{
#if(1)
	   renderWindow->removeAllViewports();
	   int numViewPorts = 0;
#else
	   int numViewPorts = renderWindow->getNumViewports();
#endif
	   m_pSceneViewPortOnRendWindow = renderWindow->addViewport(m_pSceneCamera , numViewPorts++); 
	   m_pSceneViewPortOnRendWindow->setClearEveryFrame(false);
	   m_pSceneViewPortOnRendWindow->setOverlaysEnabled(true); 
	   m_pSceneViewPortOnRendWindow->setAutoUpdated(false);
	 }
	//

	//Ogre::String depthTechScheme = "SSAO_DrawDepth";
	m_LightedSceneTarget = new DeferedRendTarget();
		
	m_DepthTarget = new DeferedRendTarget();
	
	Ogre::PixelFormat solidLightSceneFmt = Ogre::PF_FLOAT16_RGBA;
	Ogre::PixelFormat solidDepthSceneFmt = Ogre::PF_FLOAT16_RGBA;

	Ogre::String rendsysname = Ogre::Root::getSingleton().getRenderSystem()->getName();

	if(rendsysname == "Direct3D11 Rendering Subsystem")//dx11 allow multi rend target with different format
	{
	   solidLightSceneFmt = Ogre::PF_A8R8G8B8;
	   solidDepthSceneFmt = Ogre::PF_FLOAT16_RGBA;
	}
	m_LightedSceneTarget->CreateRendeTarget("SceneTexture" , m_RendWindowWidth , m_RendWindowHeight , 1.0f , solidLightSceneFmt , 0);
	bool succeed0 = m_LightedSceneTarget->m_RendTarget->attachDepthBuffer(depthBuff);

#if(1)
	m_DepthTarget->CreateRendeTarget("DepthTexture" , m_RendWindowWidth , m_RendWindowHeight , 1.0f , solidDepthSceneFmt , 0);//this);
#else
	m_DepthTarget->CreateRendeTarget("DepthTexture" , m_RendWindowWidth , m_RendWindowHeight , 1.0f , solidDepthSceneFmt , 0);//this);
#endif
	bool succeed1 = m_DepthTarget->m_RendTarget->attachDepthBuffer(depthBuff);

	if(succeed0 == false || succeed1 == false)
	{
	   OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR , "can not attach depth buffer", "DeferredRendFrameWork::RenderWindowSizeChanged");
	}

#if(USEMRT)
	m_Mrt = Ogre::Root::getSingleton().getRenderSystem()->createMultiRenderTarget("SceneAndDepthMrt");
	
	Ogre::RenderTexture * sceneRTTex = dynamic_cast<Ogre::RenderTexture*>(m_LightedSceneTarget->m_RendTarget);
	
	Ogre::RenderTexture * depthRTTex = dynamic_cast<Ogre::RenderTexture*>(m_DepthTarget->m_RendTarget);
	
	sceneRTTex->setAutoUpdated(false);//ensure not auto update;
	
	depthRTTex->setAutoUpdated(false);//ensure not auto update;

	m_Mrt->bindSurface(0 , sceneRTTex);//target0 - scene
	
	m_Mrt->bindSurface(1 , depthRTTex);//target1 - depth

	m_Mrt->attachDepthBuffer(depthBuff);
	
	m_Mrt->removeAllViewports();//only allow one view port at a time

	Ogre::Viewport * mrtVp = m_Mrt->addViewport( pSceneCamera ); 

	mrtVp->setClearEveryFrame(true , Ogre::FBT_COLOUR|Ogre::FBT_DEPTH);

	mrtVp->setBackgroundColour(Ogre::ColourValue(0,0,0,0));

	mrtVp->setOverlaysEnabled(false);
#else
	//set scene manager to current scene
	m_DepthTarget->addViewPortExternSceneMgr(pSceneCamera , true , Ogre::FBT_COLOUR|Ogre::FBT_DEPTH , Ogre::ColourValue() , false , "");//depthTechScheme);

	m_LightedSceneTarget->addViewPortExternSceneMgr(pSceneCamera , true , Ogre::FBT_COLOUR , Ogre::ColourValue(1,1,1,0) , false , "");
#endif
	//add post processor
	if(enablessao)
	   m_PostProcessor.push_back(new CSSAO());
#if(1)
	m_PostProcessor.push_back(new PostEffect_Bloom());
#endif
	for(size_t c = 0 ; c < m_PostProcessor.size() ; c++)
	{
		m_PostProcessor[c]->Initialize(renderWindow , this);
	}

	if(m_PostProcessor.size() > 1)//if we have other post-process like bloom etc
	{
	   m_FinalSceneTarget = new DeferedRendTarget();
	  
	   m_FinalSceneTarget->CreateRendeTarget("FinalSceneTexture" , m_RendWindowWidth , m_RendWindowHeight , 1.0f , solidLightSceneFmt , 0);
	   
	   bool succeed0 = m_FinalSceneTarget->m_RendTarget->attachDepthBuffer(depthBuff);
	   
	   m_FinalSceneTarget->addViewPortExternSceneMgr(pSceneCamera , false , 0 , Ogre::ColourValue(1,1,1,0) , false , "");
	   
	   m_FinalSceneTarget->m_pViewPort->setOverlaysEnabled(false);
	   
	   //enable overlay in screen listener
	   m_pScreenQuadSceneMgr->addRenderQueueListener(MXOgreWrapper::Get()->GetOverlaySystem());
	   m_pSceneViewPortOnRendWindow->setOverlaysEnabled(true);
	}

	if(enablessao)
	{
	   Ogre::MaterialPtr matPtr = Ogre::MaterialManager::getSingleton().load("Post/ModulateScene" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).dynamicCast<Ogre::Material>();
	   Ogre::TexturePtr  occlusionptr = Ogre::TextureManager::getSingleton().getByName("SAOOcclusionTexture_NoBlur");
	   ApplyTextureToMaterial(matPtr , occlusionptr , "modulateTex");
	}
}
//=============================================================================================================
void DeferredRendFrameWork::RebuildScreeQuadVertex(int screenWid , int screenHei)
{
	//Screen QuadObject Build
	Real hOffset = Ogre::Root::getSingleton().getRenderSystem()->getHorizontalTexelOffset() / (0.5f * screenWid);
	Real vOffset = Ogre::Root::getSingleton().getRenderSystem()->getVerticalTexelOffset()   / (0.5f * screenHei);

	float mQuadLeft , mQuadTop , mQuadRight , mQuadBottom;
	mQuadLeft   = -1 + hOffset;
	mQuadTop    =  1 - vOffset;
	mQuadRight  =  1 + hOffset;
	mQuadBottom = -1 - vOffset;

	//rebuild screen Quad vertex
	m_pScreenQuadObj->clear();
	m_pScreenQuadObj->begin("SSAO/CrytekSAO", Ogre::RenderOperation::OT_TRIANGLE_LIST);
	m_pScreenQuadObj->position(Ogre::Vector3(mQuadRight, mQuadBottom, -1.0));
	m_pScreenQuadObj->position(Ogre::Vector3(mQuadLeft,  mQuadTop, -1.0));
	m_pScreenQuadObj->position(Ogre::Vector3(mQuadLeft,  mQuadBottom, -1.0));
	m_pScreenQuadObj->position(Ogre::Vector3(mQuadRight, mQuadTop, -1.0));

	//using indices
	m_pScreenQuadObj->index(0);
	m_pScreenQuadObj->index(1);
	m_pScreenQuadObj->index(2);
	m_pScreenQuadObj->index(0);
	m_pScreenQuadObj->index(3);
	m_pScreenQuadObj->index(1);
	m_pScreenQuadObj->end();
	m_pScreenQuadObj->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);//make always visible (i.e be draw)
}
//==========================================================================================================
void DeferredRendFrameWork::BeforeRendVisibleObjects(DeferredRendSceneManager * sceneMgr , DeferredRendSceneManager::DeferredRendStage currentStage)
{
	if(currentStage == DeferredRendSceneManager::Stage_TransparentLit
	|| currentStage == DeferredRendSceneManager::Stage_CustomFinal)
	{
		Ogre::MaterialPtr ssaomodulate = Ogre::MaterialManager::getSingleton().load("Post/ModulateScene" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).dynamicCast<Ogre::Material>();
		
		Ogre::Pass * ssaoPass = ssaomodulate->getTechnique(0)->getPass(0);

		Ogre::ManualObject::ManualObjectSection * objsection = m_pScreenQuadObj->getSection(0);

		sceneMgr->manualRender( objsection->getRenderOperation(), 
								ssaoPass, 
								0, 
								Ogre::Matrix4::IDENTITY,
								Ogre::Matrix4::IDENTITY, 
								Ogre::Matrix4::IDENTITY
							  ) ;

		//sceneMgr->manualRender(objsection , ssaoPass, 0 , Ogre::Matrix4::IDENTITY , Ogre::Matrix4::IDENTITY);
	}
	
}
//==========================================================================================================
void DeferredRendFrameWork::_rendScene(Ogre::RenderWindow * RendWindow)
{
	//to do deal with rend windows changed, ogre has bug in release render target so not deal currently
	int currWidth  = RendWindow->getWidth();
	int currheight = RendWindow->getHeight();
	if(currWidth != m_RendWindowWidth || currheight != m_RendWindowHeight)
	{	
		m_RendWindowWidth = currWidth;
		m_RendWindowHeight = currheight;
		RenderWindowSizeChanged(RendWindow , m_RendWindowWidth , m_RendWindowHeight);
	}
	//
	if (m_UseCustomRendStage)
	{
		//custom stage 1
		for (int c = 0; c < m_Listeners.size(); c++)
		{
			m_Listeners[c]->BeforeRendStage(DeferredRendSceneManager::Stage_CustomFirst);
		}
		m_pSceneManager->SetCurrentRendStage(DeferredRendSceneManager::Stage_CustomFirst);
		m_Mrt->update();
		for (int c = 0; c < m_Listeners.size(); c++)
		{
			m_Listeners[c]->AfterRendStage(DeferredRendSceneManager::Stage_CustomFirst);
		}

		//custom stage2
		for (int c = 0; c < m_Listeners.size(); c++)
		{
			m_Listeners[c]->BeforeRendStage(DeferredRendSceneManager::Stage_CustomMiddle);
		}

		m_pSceneManager->SetCurrentRendStage(DeferredRendSceneManager::Stage_CustomMiddle);
		int numViewPorts = m_Mrt->getNumViewports();
		m_Mrt->getViewport(0)->setClearEveryFrame(false);
		m_Mrt->update();
		m_Mrt->getViewport(0)->setClearEveryFrame(true, Ogre::FBT_DEPTH | Ogre::FBT_COLOUR);
		for (int c = 0; c < m_Listeners.size(); c++)
		{
			m_Listeners[c]->AfterRendStage(DeferredRendSceneManager::Stage_CustomMiddle);
		}

		//custom stage3
		for (int c = 0; c < m_Listeners.size(); c++)
		{
			m_Listeners[c]->BeforeRendStage(DeferredRendSceneManager::Stage_CustomFinal);
		}
		m_pSceneManager->SetCurrentRendStage(DeferredRendSceneManager::Stage_CustomFinal);

		if (m_PostProcessor.size() <= 1)//no post process directly out put to rend window
		{
			RendSceneOnViewPort(m_pSceneViewPortOnRendWindow);
		}
		else
		{
			m_FinalSceneTarget->DrawExternalScene();

			//now call any post effect 
			for (size_t c = 0; c < m_PostProcessor.size(); c++)
			{
				m_PostProcessor[c]->SceneAllStageFinish(RendWindow, m_FinalSceneTarget, this);
			}

			//hard code composite bloom with final scene
			m_pSceneViewPortOnRendWindow->setCamera(m_pScreenQuadCamera);
			m_pScreenQuadObj->setMaterialName(0, "Bloom/Composite", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

			Ogre::RenderTarget * rt = m_pSceneViewPortOnRendWindow->getTarget();
			rt->_beginUpdate();
			rt->_updateViewport(m_pSceneViewPortOnRendWindow, true);
			rt->_endUpdate();
			rt->swapBuffers();
		}
		for (int c = 0; c < m_Listeners.size(); c++)
		{
			m_Listeners[c]->AfterRendStage(DeferredRendSceneManager::Stage_CustomFinal);
		}
		return;
	}

#if(USEMRT)
	for (int c = 0; c < m_Listeners.size(); c++)
	{
		m_Listeners[c]->BeforeRendStage(DeferredRendSceneManager::Stage_SolidLit);
	}
	m_pSceneManager->SetCurrentRendStage(DeferredRendSceneManager::Stage_SolidLit);
	m_Mrt->update();
	for (int c = 0; c < m_Listeners.size(); c++)
	{
		m_Listeners[c]->AfterRendStage(DeferredRendSceneManager::Stage_SolidLit);
	}
	//m_LightedSceneTarget->m_RendTarget->writeContentsToFile("c:\\lighteddx11.bmp");
	//m_DepthTarget->m_RendTarget->writeContentsToFile("c:\\depthdx11.bmp");
#else
	//@ step 1 : rend depth and camera pos pass
	m_pSceneManager->SetCurrentRendStage(DeferredRendSceneManager::Stage_Depth);
	m_DepthTarget->DrawExternalScene();
	//m_DepthTarget->m_RendTarget->writeContentsToFile("c:\\depthdx11.bmp");
	
	///@ step 2 : rend scene opaque object pass
	m_pSceneManager->SetCurrentRendStage(DeferredRendSceneManager::Stage_SolidLit);
	m_LightedSceneTarget->DrawExternalScene();
	//m_LightedSceneTarget->m_RendTarget->writeContentsToFile("c:\\lighteddx11.bmp");
#endif
	//now call any post effect 
	for(size_t c = 0 ; c < m_PostProcessor.size() ; c++)
	{
		m_PostProcessor[c]->SceneOpaqueStageFinish(RendWindow , this);
	}
	

	///@ step 3 : rend transparent object light pass 
	m_pSceneManager->SetCurrentRendStage(DeferredRendSceneManager::Stage_TransparentLit);
	
	if(m_PostProcessor.size() <= 1)//no post process directly out put to rend window
	{
	   RendSceneOnViewPort(m_pSceneViewPortOnRendWindow);

	}
	else
	{
	   m_FinalSceneTarget->DrawExternalScene();
	   
	   //now call any post effect 
	   for(size_t c = 0 ; c < m_PostProcessor.size() ; c++)
	   {
		   m_PostProcessor[c]->SceneAllStageFinish(RendWindow , m_FinalSceneTarget , this);
	   }

	   //hard code composite bloom with final scene
	   m_pSceneViewPortOnRendWindow->setCamera(m_pScreenQuadCamera);
	   m_pScreenQuadObj->setMaterialName(0 , "Bloom/Composite", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	   Ogre::RenderTarget * rt = m_pSceneViewPortOnRendWindow->getTarget();
	   rt->_beginUpdate();
	   rt->_updateViewport(m_pSceneViewPortOnRendWindow , true);
	   rt->_endUpdate();
	   rt->swapBuffers();
	}
	
}
//=======================================================================================================================
void DeferredRendFrameWork::RendSceneOnViewPort(Ogre::Viewport * vp)
{
	vp->setCamera(m_pSceneCamera);//set scene camera to viewport
	Ogre::RenderTarget * rt = vp->getTarget();
	rt->_beginUpdate();
	rt->_updateViewport(vp , true);
	rt->_endUpdate();
	rt->swapBuffers();
}
//=======================================================================================================================
void DeferredRendFrameWork::RenderWindowSizeChanged(Ogre::RenderWindow * RendWindow , int WinWidth , int WinHeight)
{
#if(USEMRT)
	m_Mrt->unbindSurface(0); //unbind invalid  target
	m_Mrt->unbindSurface(1); //unbind invalid  target
#endif
	m_LightedSceneTarget->ResizeRendTarget(WinWidth , WinHeight);
	m_DepthTarget->ResizeRendTarget(WinWidth , WinHeight);
	
	Ogre::DepthBuffer * depthBuff = RendWindow->getDepthBuffer();
	bool resultA = m_LightedSceneTarget->m_RendTarget->attachDepthBuffer(depthBuff);
	bool resultB = m_DepthTarget->m_RendTarget->attachDepthBuffer(depthBuff);
	
	if(resultA == false || resultB == false)
	{
		OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR , "can not attach depth buffer", "DeferredRendFrameWork::RenderWindowSizeChanged");
	}

	if(m_FinalSceneTarget)
	{
		m_FinalSceneTarget->ResizeRendTarget(WinWidth , WinHeight);
		bool result = m_FinalSceneTarget->m_RendTarget->attachDepthBuffer(depthBuff);
		if(result == false)
		{
		   OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR , "can not attach depth buffer", "DeferredRendFrameWork::RenderWindowSizeChanged");
		}
	}

#if(USEMRT)
	Ogre::RenderTexture * sceneRTTex = dynamic_cast<Ogre::RenderTexture*>(m_LightedSceneTarget->m_RendTarget);

	Ogre::RenderTexture * depthRTTex = dynamic_cast<Ogre::RenderTexture*>(m_DepthTarget->m_RendTarget);

	sceneRTTex->setAutoUpdated(false);//ensure not auto update;

	depthRTTex->setAutoUpdated(false);//ensure not auto update;

	m_Mrt->bindSurface(0 , sceneRTTex);//target0 - scene

	m_Mrt->bindSurface(1 , depthRTTex);//target1 - depth

	m_Mrt->attachDepthBuffer(depthBuff);

	m_Mrt->removeAllViewports();//only allow one view port at a time

	Ogre::Viewport * mrtVp = m_Mrt->addViewport( m_pSceneCamera ); 

	mrtVp->setClearEveryFrame(true , Ogre::FBT_COLOUR|Ogre::FBT_DEPTH);

	mrtVp->setBackgroundColour(Ogre::ColourValue(1,1,1,0));

	mrtVp->setOverlaysEnabled(false);
#else
	if(m_Mrt != 0)
	{
		OGRE_EXCEPT(Ogre::Exception::ERR_INTERNAL_ERROR , "State Error shoud not haveMrt", "DeferredRendFrameWork::RenderWindowSizeChanged");

	}
#endif
	//Screen QuadObject Build
	RebuildScreeQuadVertex(WinWidth , WinHeight);
}
//==============================================================================================================================================================
void DeferredRendFrameWork::writeSceneTexture()
{
	if(m_LightedSceneTarget)
	{
	   m_LightedSceneTarget->m_RendTarget->writeContentsToFile("c:\\SAOScene.bmp");
	}
}
//==============================================================================================================================================================
void DeferredRendFrameWork::writeDepthTexture()
{
	if(m_DepthTarget)
	{
	   m_DepthTarget->m_RendTarget->writeContentsToFile("c:\\SAODepthMap.bmp");
	}
}
