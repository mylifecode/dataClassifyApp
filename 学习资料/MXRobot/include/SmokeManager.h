/*********************************************
ÉÕ°×Ã°ÑÌ
*********************************************/
#pragma once
#include "MXOgreWrapper.h"
#include "Ogre.h"
#include <list>
#include "shadowmap.h"



struct SmokeParticle
{
	//Ogre::Vector3 m_source;
	Ogre::Vector3 m_pos;
	Ogre::Vector3 m_velocity;
	Ogre::Vector3 m_across;
	Ogre::Vector3 m_up;
	//Ogre::Vector3 m_forward;
	float m_width;
	float m_height;
	float m_dwidth;
	float m_dheight;
	float m_time;
	float m_rand;
	float m_rand2;
	float m_rand_length;
	float m_a1;
	float m_a2;
	float m_a3;
	//bool  m_valid;
	SmokeParticle(const Ogre::Vector3 & across, const Ogre::Vector3 & up , const Ogre::Vector3 & pos,const Ogre::Vector3 & velocity, float width, float height, float dw, float dh,float a1 ,float a2 ,float a3);
	
	void Reset(const Ogre::Vector3 & across, const Ogre::Vector3 & up , const Ogre::Vector3 & pos,const Ogre::Vector3 & velocity, float width, float height, float dw, float dh,float a1 ,float a2 ,float a3);

	void update(const Ogre::Vector3 &destpos,float dt);

};

class SmokeManager : public CShadowMap::ShadowRendListener
{
public:
	SmokeManager(float emitWidth = 1,float emitHeight = 1);
	SmokeManager(Ogre::Camera *camera,float emitWidth,float emitHeight);
	~SmokeManager(void);
	//@overridden shadow map listener
	virtual void preRenderShadowDepth();
	virtual void postRenderShadowDepth();

	void addSmoke(const Ogre::Vector3 &pos,float width,float height,int num);
	void update(float dt);
	void updateSmoke(const Ogre::Vector3& dest, float dt);
	void updateManual();
	void setLifetime(float lifetime);
	void setInterval(float interval);
	void setAcceleration(float a1 , float a2 , float a3);
private:
	Ogre::ManualObject *m_manual;
	Ogre::Camera *m_camera;
	std::list<SmokeParticle*> m_smoke_list;
	std::vector<SmokeParticle*> m_DeadList;
	float m_emit_width;
	float m_emit_height;
	float m_last;
	float m_now;
	float m_lifetime;
	float m_interval;
	float m_acceleration1;
	float m_acceleration2;
	float m_acceleration3;
	int	  m_valid_num;
	
};

