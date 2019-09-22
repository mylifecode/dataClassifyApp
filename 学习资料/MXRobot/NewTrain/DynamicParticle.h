#ifndef _DYNAMICPARTICLE_
#define _DYNAMICPARTICLE_
#include "Ogre.h"
class DynamicParticle
{
public:
	DynamicParticle(Ogre::Vector3 origin ,  float mass , float radius);
	
	virtual void reset();
	//particle data
	float m_timelived;
	float m_maxtimelive;
	float m_radius;
	Ogre::Vector3 m_position;
	Ogre::Vector3 m_velocity;

	void * m_usedata;		//BloodMoveData

	DynamicParticle * m_recyclenext;
	DynamicParticle * m_activednext;
	DynamicParticle * m_activedprev;

// 	void * m_usrdata;
	int m_emitid;
};

class DynamicBloodPointParticle
{
public:
	DynamicBloodPointParticle();
	void Reset();


	Ogre::Vector3 m_position;
	Ogre::Radian m_directionAngle;
	float m_velocity;
	Ogre::Vector3 m_binormal;
	Ogre::Vector3 m_tangent;
	
	float m_curLiveTime;	
	float m_totalLiveTime;
	Ogre::ColourValue m_color;
};
#endif