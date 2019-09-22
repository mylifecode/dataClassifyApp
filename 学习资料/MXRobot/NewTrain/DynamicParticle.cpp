#include "DynamicParticle.h"
//================================================================================================================
DynamicParticle::DynamicParticle(Ogre::Vector3 position , float mass , float radius)
{
		m_radius = radius;
		m_position = position;
		m_velocity = Ogre::Vector3(0,0,0);
		m_usedata = 0;
		m_recyclenext = 0;
		m_activednext = 0;
		m_activedprev = 0;
		m_timelived = 0;
		m_maxtimelive = 0.0f;
		m_emitid = -1;

// 		m_usrdata = 0;

}
//================================================================================================================
void DynamicParticle::reset()
{
		m_recyclenext = 0;
		m_activednext = 0;
		m_activedprev = 0;
		m_position = m_velocity = Ogre::Vector3(0,0,0);
		m_radius = 0;
		m_timelived = 0;
}


//================================================================================================================
DynamicBloodPointParticle::DynamicBloodPointParticle()
:m_position(0.f),
m_velocity(0.f),
m_tangent(0.f),
m_binormal(0.f),
m_curLiveTime(0.f),
m_totalLiveTime(0.f),
m_directionAngle(0.f),
m_color()
{
	
}

void DynamicBloodPointParticle::Reset()
{
	m_position = Ogre::Vector3(0.f);
	m_velocity = 0.f;
	m_tangent = Ogre::Vector3(0.f);
	m_binormal = Ogre::Vector3(0.f);
	m_curLiveTime = 0.f;
	m_totalLiveTime = 0.f;
	m_directionAngle = Ogre::Radian(0.f);
	m_color = Ogre::ColourValue();
}
