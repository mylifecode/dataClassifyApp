#ifndef _POSTEFFECTBLOOM_
#define _POSTEFFECTBLOOM_
#include "Ogre.h"
#include "DeferredRendFrameWork.h"

class PostEffect_Bloom : public DeferedPostScreenProcessor
{
public:
	
	PostEffect_Bloom(void);
	
	~PostEffect_Bloom();
	
	void SetBloomAndCCParameter(float HiLightthreshold , 
		                        float contrast   = 1.2f,
		                        float brightness = 1.0f,
		                        float saturate   = 1.0f);

	void Initialize(Ogre::RenderWindow * renderWindow , DeferredRendFrameWork * framwork);
	
	void MainRenderWindowSizeChanged(int width , int height);

	void SceneOpaqueStageFinish(Ogre::RenderWindow * renderWindow , DeferredRendFrameWork * framwork);

	void SceneAllStageFinish(Ogre::RenderWindow * renderWindow  , DeferedRendTarget * finalsceneTarget , DeferredRendFrameWork * framwork);

	void _updateShaderParameter(DeferredRendFrameWork * framwork);

private:
	
	void writeBloomTextureA();

	void writeBloomTextureB();

	int m_CurrWindowWid;
	int m_CurrWindowHei;
	
	DeferedRendTarget * m_BloomA;
	DeferedRendTarget * m_BloomB;

};
#endif