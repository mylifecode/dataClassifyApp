#pragma once
#include "Ogre.h"
#include "Singleton.h"
#include "ToolsMgr.h"
#include "MXOgreWrapper.h"
#include "OgreMaxScene.hpp"
#include "MxOgreGraphic.h"
#include "DeferredRendFrameWork.h"

class CSSAO : public DeferedPostScreenProcessor
{
public:
	struct float4
	{	
		float4(){}
		float4(float ia, float ib, float ic, float id)
		{
			a = ia;
			b = ib;
			c = ic;
			d = id;
		}
		float a , b , c , d;
	};

	class SSAORenderListener
	{
		friend class CSSAO;
	public:
		SSAORenderListener();
		virtual ~SSAORenderListener();
		void SetSSAO(CSSAO * ssao);
		virtual void preRenderShadowDepth() = 0;
		virtual void postRenderShadowDepth() = 0;
	protected:
		CSSAO * m_SSAO;
	};
	
	CSSAO(void);
	
	~CSSAO();
	
	/* overridden manual resource loader*/ 
	//virtual void loadResource(Ogre::Resource* resource);

	void GenerateKernel_SSAO(uint kernelSize);

	void AddListener(CSSAO::SSAORenderListener * listener);
	
	void RemoveListener(CSSAO::SSAORenderListener * listener);
	
	void Initialize(Ogre::RenderWindow * renderWindow , DeferredRendFrameWork * framwork);
	
	void destorySSAO();

	void MainRenderWindowSizeChanged(int width , int height);

	void enableShadow(bool shadow )	
	{
		m_bShadow = shadow;
	}
	
	void SceneOpaqueStageFinish(Ogre::RenderWindow * renderWindow , DeferredRendFrameWork * framwork);

	void SceneAllStageFinish(Ogre::RenderWindow * renderWindow  , DeferedRendTarget * finalsceneTarget , DeferredRendFrameWork * framwork);

	void _updateShaderParameter(Ogre::Camera *cam);

	void SetAOLightCorrect(const Ogre::Vector4 & correctParam);
private:
	
	
	
	Ogre::Vector4 m_AOLightCorrect;

	void writeOcclusionTexture();

	void writeFinalOcclusionTexture();

	bool  m_IsDepthRangeSetted;

	float m_DepthMin;
	float m_DepthMax;
	
	int m_CurrWindowWid;
	int m_CurrWindowHei;
	
	//Ogre::TexturePtr m_SceneMap;
	//Ogre::RenderTarget* m_RenderTargrtScene;

	//Ogre::TexturePtr m_DepthMap;
	//Ogre::RenderTarget* m_DepthRenderTargrt;
	
	//Ogre::TexturePtr m_OcclusionMapNoBlur;
	//Ogre::RenderTarget* m_OcclusionRenderTargrtNoBlur;

	//Ogre::TexturePtr m_OcclusionMapFinal;
	//Ogre::RenderTarget* m_OcclusionRenderTargrtFinal;
	//DeferedRendTarget * m_SceneTarget;
	//DeferedRendTarget * m_DepthTarget;
	DeferedRendTarget * m_OcclusionNoBlurTarget;
	DeferedRendTarget * m_OcclusionFinalTarget;

	//Ogre::Camera* m_pCurrCamera;
	
	//Ogre::SceneManager* m_pSceneMgr;

	
	bool m_bShadow;
	
	bool m_bDirty;

	float4 m_kernel[256];

	std::vector<SSAORenderListener*> m_listener;

	
};
