#pragma once
#include "Ogre.h"
#include "Singleton.h"
#include "ToolsMgr.h"
#include "MXOgreWrapper.h"
#include "OgreMaxScene.hpp"
#include "MxOgreGraphic.h"

//¿ª¹Ø of SSAOÒõÓ°
//#define USE_SSAO
#ifdef USE_SSAO
//#define g_ShadowMap SSAOShadowMap::Instance()
#else
//#define g_ShadowMap CShadowMap::Instance()
#endif

class CShadowMap : public CSingleT<CShadowMap> , public Ogre::RenderTargetListener ,  public Ogre::MaterialManager::Listener
{
public:
	class ShadowRendListener
	{
		friend class CShadowMap;
	public:
		ShadowRendListener();
		virtual ~ShadowRendListener();
		void SetShadowMap(CShadowMap * shadowmap);
		virtual void preRenderShadowDepth() = 0;
		virtual void postRenderShadowDepth() = 0;
	protected:
		CShadowMap * m_ShadowMap;
	};
	
	CShadowMap(void);
	
	~CShadowMap();
	
	void AddListener(CShadowMap::ShadowRendListener * listener);
	
	void RemoveListener(CShadowMap::ShadowRendListener * listener);

	Ogre::Camera * GetShadowCamera(){return m_Camera_Shadow;}

	void GetDepthRange(float & dmin , float & dmax);
	
	void initConfig(Ogre::SceneManager* sceneManager , Ogre::Radian camFov , float camAspectRate);
	
	void createShadowTexture(int width = 1024 , int height = 1024);

	void destoryShadowMap();

	void enableShadow(bool shadow )	
	{
		m_bShadow = shadow;
	}
	
	void _update(Ogre::Light* light , const Ogre::Vector3 & hintUpDir);
	
	Ogre::Vector3 m_ShadowLightDir;
	
	Ogre::Vector3 m_ShadowLightPos;

private:
	
	void setMatrixToShader(Ogre::String materialName, Ogre::String paraName, CG_PROGRAME_TYPE type, Ogre::Matrix4 matrix, int technique=0, int pass=0);

	void SetVector4ToShader(Ogre::String materialName, Ogre::String paraName, CG_PROGRAME_TYPE type, Ogre::Vector4 vec4, int technique=0, int pass=0);

	void updateShadowCamera(Ogre::Light* light  , const Ogre::Vector3 & hintUpDir);

	virtual void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
	
	virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);

	/** @copydoc MaterialManager::Listener::handleSchemeNotFound */
	virtual Ogre::Technique* handleSchemeNotFound(  unsigned short schemeIndex, 
													const Ogre::String& schemeName, Ogre::Material* originalMaterial, unsigned short lodIndex, 
													const Ogre::Renderable* rend);
	void writeDepthTexture();

	bool  m_IsDepthRangeSetted;
	float m_DepthMin;
	float m_DepthMax;
	
	Ogre::TexturePtr m_DepthMap;
	
	Ogre::RenderTarget* m_DepthRendTargrt;
	
	Ogre::Camera* m_Camera_Shadow;
	
	Ogre::SceneManager* m_sceneMgr_Shadow;

	bool m_bShadow;
	
	bool m_bDirty;

	std::vector<ShadowRendListener*> m_listener;
};
