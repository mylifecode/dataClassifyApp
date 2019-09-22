#include "PostEffect_Bloom.h"

PostEffect_Bloom::PostEffect_Bloom(void)
{
	m_BloomA = NULL;
	m_BloomB = NULL;
}
//==================================================================================================================================
PostEffect_Bloom::~PostEffect_Bloom()
{
	if(m_BloomA)
	{
	   delete m_BloomA;
	   m_BloomA = 0;
	}

	if(m_BloomB)
	{
	   delete m_BloomB;
	   m_BloomB = 0;
	}
}
//==================================================================================================================================
void PostEffect_Bloom::SetBloomAndCCParameter(float HiLightthreshold ,
	                                          float contrast,
											  float brightness,
											  float saturate)
{
	//@ bloom bloom parameter
	Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().load("Bloom/BrightPass" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).dynamicCast<Ogre::Material>();
	// get the pass
	Ogre::Pass * pass = mat->getTechnique(0)->getPass(0);

	// get the pixel shader parameters
	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
	
	if(params->_findNamedConstantDefinition("HighlightThreshold"))
		params->setNamedConstant("HighlightThreshold", HiLightthreshold);

	
	//@ color conversion parameter
	mat = Ogre::MaterialManager::getSingleton().load("Bloom/Composite", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).dynamicCast<Ogre::Material>();
	// get the pass
	pass = mat->getTechnique(0)->getPass(0);
	// get the pixel shader parameters
	params = pass->getFragmentProgramParameters();

	if (params->_findNamedConstantDefinition("fCC_Contrast"))
		params->setNamedConstant("fCC_Contrast", contrast);

	if (params->_findNamedConstantDefinition("fCC_Brightness"))
		params->setNamedConstant("fCC_Brightness", brightness);

	if (params->_findNamedConstantDefinition("fCC_Saturation"))
		params->setNamedConstant("fCC_Saturation", saturate);

	if (params->_findNamedConstantDefinition("fCC_Hue"))
		params->setNamedConstant("fCC_Hue", 1.0f);
}
//==================================================================================================================================
void PostEffect_Bloom::Initialize(Ogre::RenderWindow * renderWindow , DeferredRendFrameWork * framwork)
{
	m_CurrWindowWid = renderWindow->getWidth();
	m_CurrWindowHei = renderWindow->getHeight();

	m_BloomA = new DeferedRendTarget();
	m_BloomB = new DeferedRendTarget();

	float downsampleScale = 0.5f;
	m_BloomA->CreateRendeTarget("BloomTex_A" , int(m_CurrWindowWid*downsampleScale) , int(m_CurrWindowHei*downsampleScale) , 1.0f , Ogre::PF_R8G8B8A8 , 0);
	m_BloomB->CreateRendeTarget("BloomTex_B" , int(m_CurrWindowWid*downsampleScale) , int(m_CurrWindowHei*downsampleScale) , 1.0f , Ogre::PF_R8G8B8A8 , 0);
	
	//set scene manager to current scene
	
	//RebuildScreeQuadVertex(m_CurrWindowWid , m_CurrWindowHei);
}
//=============================================================================================================
void PostEffect_Bloom::MainRenderWindowSizeChanged(int WinWidth , int WinHeight)
{
	float downsampleScale = 0.5f;
	m_BloomA->ResizeRendTarget(int(WinWidth*downsampleScale) , int(WinHeight*downsampleScale));
	m_BloomB->ResizeRendTarget(int(WinWidth*downsampleScale) , int(WinHeight*downsampleScale));	
}
//=============================================================================================================
void PostEffect_Bloom::_updateShaderParameter(DeferredRendFrameWork * framwork)
{
	//Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().load("Bloom/BloomHighLight" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	//Ogre::TexturePtr sceneTex = framwork->GetFinalSceneTexture();

	//set the texture to bloom material to rend a illuminate texture for blur
	//mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTexture(sceneTex);

	// this is the camera you're using
	/*
	Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().load("SSAO/CrytekSAO" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	
	// get the pass
	Ogre::Pass * pass = mat->getTechnique(0)->getPass(0);

	// get the pixel shader parameters
	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();

	static const Ogre::Matrix4 CLIP_SPACE_TO_IMAGE_SPACE(
		0.5,    0,    0,  0.5,
		0,   -0.5,    0,  0.5,
		0,      0,    1,    0,
		0,      0,    0,    1);

	if (params->_findNamedConstantDefinition("ProjTexMat"))
		params->setNamedConstant("ProjTexMat", CLIP_SPACE_TO_IMAGE_SPACE * cam->getProjectionMatrixWithRSDepth());

	if(params->_findNamedConstantDefinition("kernel"))
	   params->setNamedConstant("kernel", (float*)m_kernel , KERNELSIZE);

	Ogre::Vector4 KernelSize = Ogre::Vector4( KERNELSIZE , 1 , 1 , 1 );
	if(params->_findNamedConstantDefinition("KernelSize"))
	   params->setNamedConstant("KernelSize", KernelSize);
	   */
}
//==================================================================================================================================
void PostEffect_Bloom::SceneOpaqueStageFinish(Ogre::RenderWindow * renderWindow , DeferredRendFrameWork * framwork)
{

}

//==================================================================================================================================
void PostEffect_Bloom::SceneAllStageFinish(Ogre::RenderWindow * renderWindow , DeferedRendTarget * finalsceneTarget , DeferredRendFrameWork * framwork)
{
	//to do deal with rend windows changed, ogre has bug in release render target so not deal currently
	int currWidth  = renderWindow->getWidth();
	int currheight = renderWindow->getHeight();
	if(currWidth != m_CurrWindowWid || currheight != m_CurrWindowHei)
	{	
		m_CurrWindowWid = currWidth;
		m_CurrWindowHei = currheight;
		MainRenderWindowSizeChanged(m_CurrWindowWid , m_CurrWindowHei);
	}

	//
	_updateShaderParameter(framwork);

	//rend high light illumination texture
	m_BloomA->DrawScreenQuad("Bloom/BrightPass");
	m_BloomB->DrawScreenQuad("Bloom/BlurX");
	m_BloomA->DrawScreenQuad("Bloom/BlurY");

	//writeBloomTextureA();
}
//==================================================================================================================================
void PostEffect_Bloom::writeBloomTextureA()
{
	if(m_BloomA)
	{
	   m_BloomA->m_RendTarget->writeContentsToFile("c:\\Bloom TextureA.bmp");
	}
}
//==================================================================================================================================
void PostEffect_Bloom::writeBloomTextureB()
{
	if(m_BloomB)
	{
	   m_BloomB->m_RendTarget->writeContentsToFile("c:\\Bloom TextureB.bmp");
	}
}