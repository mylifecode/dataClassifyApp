#include "math/GophysTransformUtil.h"
#include "VesselBleedEffect.h"
#include "VeinConnectObject.h"
#include "MisMedicOrganOrdinary.h"
#include "DynamicObjectRenderable.h"
#include "IMXDefine.h"
// #include "OgreParticleEmitter.h"
#include "IObjDefine.h"
//====================================================================================================

//==================================================================================================================================
CVesselBleedEffect::CVesselBleedEffect(MisMedicOrganInterface * organif)
{
	m_bFinished = false;
// 	m_bIsActived = false;
	m_bStoped = false;
	m_pSceneNode = NULL;		//bleed drops
	m_pParticleSys = NULL;
	m_OrganIf = organif;
	m_fStartTime = -1;
	m_fEffectLastTime = 0;
	m_pRootNode = organif;
	m_bStopUntilHasAttchment = false;
	SetShadowMap(CShadowMap::Get());
}

void CVesselBleedEffect::initOneBleedPoint(float lasttime, Ogre::SceneManager* sm, GFPhysSoftBodyNode* bleedPointNode, int bleedType, const Ogre::String& templateName)
{
	if (m_pRootNode == NULL)
	{
		return;
	}
	
	if (bleedPointNode == NULL)
	{
		return;
	}

	if (lasttime < 0)
	{
		m_bStopUntilHasAttchment = true;
	}

	m_bleedPointNode = bleedPointNode;
	if (m_pSceneNode == NULL)
	{

		Ogre::SceneNode* nod = m_pRootNode->getSceneNode();

		if (nod == NULL)
		{
			return;
		}

		int organID = m_pRootNode->m_OrganID;

		static int countIndex = 0;
		countIndex++;

		Ogre::String name = "bleedPoint";
		name.append(Ogre::StringConverter::toString(organID));
		name.append("_");
		name.append(Ogre::StringConverter::toString(countIndex));

		switch (bleedType)
		{
		case 1:
			// 	EDOT_BRAVERY_ARTERY:// = 0,	// µ¨ÄÒ¶¯Âö
			//	EDOT_HEPATIC_ARTERY:// = 4,	// ¸Î¶¯Âö
			//	case EDOT_LIVER:// = 5,				// ¸Î
			//	EDOT_VEIN:// = 6,				// µ¨ÄÒÈý½Ç
		{
			m_pParticleSys = MXOgreWrapper::Get()->GetDefaultSceneManger()->createParticleSystem(name,templateName);
			break;
		}
		case 0:
		//	EDOT_COMMON_BILE_DUCT:// = 1,	// ¸Î×Ü¹Ü
		//	EDOT_CYSTIC_DUCT:// = 2,		// µ¨ÄÒ¹Ü
		//	EDOT_GALLBLADDER:// = 3,		// µ¨ÄÒ
		{
			m_pParticleSys = MXOgreWrapper::Get()->GetDefaultSceneManger()->createParticleSystem(name,PT_BLEED_01);
			break;
		}
		default:
			return;
		}

		m_pSceneNode = MXOgreWrapper::Get()->GetDefaultSceneManger()->createSceneNode(name);
		nod->addChild(m_pSceneNode);
		m_pSceneNode->resetOrientation();
		m_pSceneNode->setPosition(bleedPointNode->m_CurrPosition.x(), bleedPointNode->m_CurrPosition.y(), bleedPointNode->m_CurrPosition.z());
		m_pSceneNode->attachObject(m_pParticleSys);
		m_pParticleSys->fastForward(20);

		int emi_num = m_pParticleSys->getNumEmitters();

		m_fStartTime = 0;
		m_fEffectLastTime = lasttime;
	}

}

//==================================================================================================================================
CVesselBleedEffect::~CVesselBleedEffect()
{

	m_pSceneNode->detachObject(m_pParticleSys->getName());

	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();
	pSceneManager->destroyEntity(m_pParticleSys->getName());

	if (m_pRootNode)
	{
		MisMedicOrgan_Ordinary* tempOO = dynamic_cast<MisMedicOrgan_Ordinary*>(m_pRootNode);
		if (tempOO)
		{
			tempOO->RemoveAndDestoryChild(m_pSceneNode->getName());
		}
	}
	

}
void CVesselBleedEffect::preRenderShadowDepth()
{
	if(m_pParticleSys)
	   m_pParticleSys->setVisible(false);
}
void CVesselBleedEffect::postRenderShadowDepth()
{
	if(m_pParticleSys)
	   m_pParticleSys->setVisible(true);
}
bool CVesselBleedEffect::Update(float dt)
{
	// update pos for each particle.

	m_fStartTime +=dt;
	if(m_fEffectLastTime < m_fStartTime || m_bStoped == true)
	{
		int hasattch =	m_OrganIf->getAttchmentFlag();
		if (hasattch == 0 && m_bStopUntilHasAttchment)
		{
			m_fStartTime = 0;
			m_fEffectLastTime = 1;
		}
		else
		{	
			m_pParticleSys->getEmitter(0)->setEnabled(false);
			m_bStoped = true;
		}
	
	}

	if(m_fEffectLastTime + 5 < m_fStartTime && m_bStoped)
	{
		m_bFinished = true;
		return false;
	}

	MisMedicOrgan_Ordinary* tempOO = dynamic_cast<MisMedicOrgan_Ordinary*>(m_pRootNode);
	if (tempOO)
	{
		Ogre::Vector3 vec = Ogre::Vector3(m_bleedPointNode->m_CurrPosition.GetX(), m_bleedPointNode->m_CurrPosition.GetY(), m_bleedPointNode->m_CurrPosition.GetZ());//tempOO->getPointByIndex(1);
		m_pSceneNode->setPosition(vec);
	}

	return true;
}

void CVesselBleedEffect::StopBleed()
{
	if (m_bStoped == false)
	{
		m_bStoped = true;
		m_fStartTime = 0;
		m_fEffectLastTime = 1;
	}
}

void CVesselBleedEffect::ResumeBleed()
{
	if (m_bStoped == true)
	{
		m_bStoped = false;
		m_fStartTime = 0;
		m_fEffectLastTime = 100;
	}
}
//==================================================================	===========================================