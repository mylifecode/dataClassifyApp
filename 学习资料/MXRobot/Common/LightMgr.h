#ifndef __CSLIGHTMGR__
#define __CSLIGHTMGR__

#include "OgreLight.h"
#include "Singleton.h"
#include "MXOgreWrapper.h"
#include "XMLWrapperLight.h"
#include "EffectManager.h"
typedef struct 
{
	Ogre::Vector3 lightPos;
	Ogre::Vector3 lightDir;
	Ogre::Light::LightTypes lightType;
	bool visible;
	bool castShadows;
	//...可以添加灯光衰减参数

}LightData;

#define FIRST_LIGHT "TheFirstLight"

class CLightMgr:public CSingleT<CLightMgr>
{
public:
	CLightMgr();
	~CLightMgr();
	void AddLight(CXMLWrapperLight* mLightData,const Ogre::String strLightName);
	void RemoveLight(const Ogre::String strLightName);
	void RemoveAllLight();
	Ogre::Light* GetLight(const Ogre::String strLightName = FIRST_LIGHT);
	std::map<Ogre::String,Ogre::Light*> GetLightMAP(){return m_strLightMap;}
private:
	std::map<Ogre::String,Ogre::Light*> m_strLightMap;
	Ogre::SceneManager* m_pSceneMgr;
};
#endif