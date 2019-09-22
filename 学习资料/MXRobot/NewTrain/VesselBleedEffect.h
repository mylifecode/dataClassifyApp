#ifndef _VESSELBLEED_EFFECT_H_
#define _VESSELBLEED_EFFECT_H_
#include "Ogre.h"
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "OrganBloodMotionSimulator.h"
#include "ShadowMap.h"
class OrganBloodingSystem;
class DynamicBloodParticle;

using namespace GoPhys;

//class OrganSurfaceBloodTrack
//a track represent the move path history of a blood particle
//

class CVesselBleedEffect  : public CShadowMap::ShadowRendListener
{
public:
	CVesselBleedEffect(MisMedicOrganInterface * organif);

	~CVesselBleedEffect();

	//@overridden shadow map listener
	virtual void preRenderShadowDepth();
	virtual void postRenderShadowDepth();

	void initOneBleedPoint(float lasttime, Ogre::SceneManager* sm, GFPhysSoftBodyNode* bleedPointNode, int bleedType, const Ogre::String& templateName);

	OrganBloodMotionSimulator* BleedingOnce();

	bool Update(float dt);
	void StopBleed();
	void ResumeBleed();
	void resetLastTime(int t)
	{
		m_fEffectLastTime = t;
	}


	MisMedicOrganInterface * m_OrganIf;

public:

	MisMedicOrganInterface*		m_pRootNode;

	Ogre::SceneNode				* m_pSceneNode;		//bleed drops	//many times;	// if is a MovableObject, can be a 
	Ogre::ParticleSystem		*m_pParticleSys;		// need attach to m_pSceneNode

	GFPhysSoftBodyNode*			m_bleedPointNode;

	bool m_bStopUntilHasAttchment;
	bool m_bFinished;
	bool m_bStoped;
	float m_fStartTime;
	float m_fEffectLastTime;

};
#endif 		//_VESSELBLEED_EFFECT_H_