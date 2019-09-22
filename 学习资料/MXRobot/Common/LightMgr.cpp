#include "LightMgr.h"

CLightMgr::CLightMgr()
{

}

CLightMgr::~CLightMgr()
{

}

void CLightMgr::AddLight(CXMLWrapperLight* mLightData,const Ogre::String strLightName)
{
	m_pSceneMgr = MXOgreWrapper::Instance()->GetDefaultSceneManger();
	if (NULL==m_pSceneMgr)
	{
		return;
	}
	
	std::map<Ogre::String,Ogre::Light*>::iterator iter=m_strLightMap.find(strLightName);
	if (iter == m_strLightMap.end())
	{
		Ogre::Light* mLight=m_pSceneMgr->createLight(strLightName);
		Ogre::SceneNode* mLightNode=m_pSceneMgr->getRootSceneNode()->createChildSceneNode(strLightName);
		mLightNode->attachObject(mLight);

		

	    //mLightNode->setPosition(mLightData->m_LightPos);
		//mLightNode->setOrientation(mLightData->m_Rotation);
		
		int type=mLightData->m_LightType;
		Ogre::Light::LightTypes lightType;
		switch(type)
		{
		case Ogre::Light::LT_SPOTLIGHT:
			lightType=Ogre::Light::LT_SPOTLIGHT;
			break;
		case Ogre::Light::LT_DIRECTIONAL:
			lightType=Ogre::Light::LT_DIRECTIONAL;
			break;
		default:
			lightType=Ogre::Light::LT_POINT;
			break;
		}
		
		mLight->setType(lightType);
		mLight->setVisible(mLightData->m_Visible);
		mLight->setCastShadows(mLightData->m_CastShadows);
		mLight->setSpecularColour(mLightData->m_ColorSpecular);
		mLight->setDiffuseColour(mLightData->m_ColorDiffuse/**mLightData->m_Power*/);
		mLight->setSpotlightInnerAngle(Ogre::Radian(mLightData->m_InnerAngle));
		mLight->setSpotlightOuterAngle(Ogre::Radian(mLightData->m_OuterAngle));
		mLight->setSpotlightFalloff(mLightData->m_FallOff);
		mLight->setAttenuation(mLightData->m_Range, mLightData->m_Constant, mLightData->m_Linear, mLightData->m_Quadric);
		
		if(mLightData->m_flag_LightPos)
		   mLight->setPosition(mLightData->m_LightPos);

		if(mLightData->m_flag_LightDir)
		   mLight->setDirection(mLightData->m_LightDir.normalisedCopy());
		else
		   mLight->setDirection(Ogre::Vector3::NEGATIVE_UNIT_Z);

		mLight->setPowerScale(mLightData->m_Power);
		
		m_strLightMap[strLightName]=mLight;
	}
}

void CLightMgr::RemoveLight(const Ogre::String strLightName)
{
	std::map<Ogre::String,Ogre::Light*>::iterator iter=m_strLightMap.find(strLightName);
	if (iter != m_strLightMap.end())
	{

		Ogre::Light* tempLight=dynamic_cast<Ogre::Light*>(iter->second);
		if (tempLight)
		{
			tempLight->setVisible(false);
		}
		m_strLightMap.erase(iter);

	}
}

void CLightMgr::RemoveAllLight()
{
	std::map<Ogre::String,Ogre::Light*>::iterator iter;
	for (iter=m_strLightMap.begin();iter!=m_strLightMap.end();iter++)
	{
		Ogre::Light* tempLight=dynamic_cast<Ogre::Light*>(iter->second);
		if (tempLight)
		{
			tempLight->setVisible(false);
		}
	}
	m_strLightMap.clear();
}

Ogre::Light* CLightMgr::GetLight(const Ogre::String strLightName)
{
	if(m_strLightMap.empty())
		return NULL;

	std::map<Ogre::String,Ogre::Light*>::iterator iter=m_strLightMap.find(strLightName);

	if (strLightName == FIRST_LIGHT)
	{
		iter = m_strLightMap.begin();
	}
	if (iter != m_strLightMap.end())
	{
		Ogre::Light* tempLight=dynamic_cast<Ogre::Light*>(iter->second);
		if (tempLight)
		{
			return tempLight;
		}
	}
	return NULL;
}