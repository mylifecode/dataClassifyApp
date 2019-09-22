#include "SmokeManager.h"
#include <math.h>
using namespace std;

const float M_PI = 3.14159265358979323846;
float sin_table[361];
float cos_table[361];

inline float frand(float max = 1)
{
	return (float)rand() / RAND_MAX * max;
}

inline float rand_in_range(float start,float end)
{
	return (float)rand() / RAND_MAX * (end-start) + start;
}


inline int irand(int max)
{
	return rand() % max;
}

void build_tables()
{
	static bool first = true;
	if(first)
	{
		float convert_factor = M_PI / 180;
		float theta;
		for (int angle = 0; angle <= 360; angle++)
		{
			theta = angle * convert_factor;
			sin_table[angle] = sin(theta);
			cos_table[angle] = cos(theta);

		}
		first = false;
	}
	
}
/*
float fast_sin(float theta)
{	
	static float convert_factor = 180 / M_PI;
	theta *= convert_factor;
	theta = fmodf(theta,360);
	if (theta < 0) 
		theta+=360.0;
	int theta_int = (int)theta;
	float frac = theta - theta_int;
	return(sin_table[theta_int] + frac * (sin_table[theta_int+1] - sin_table[theta_int]));
}

float fast_cos(float theta)
{
	static float convert_factor = 180 / M_PI;
	theta *= convert_factor;
	theta = fmodf(theta,360);
	if (theta < 0) 
		theta+=360.0;
	int theta_int  = (int)theta;
	float frac = theta - theta_int;
	return(cos_table[theta_int] + frac*(cos_table[theta_int+1] - cos_table[theta_int]));

} 
*/
void fast_sincos(float theta , float & sin_value,float & cos_value)
{
	static float convert_factor = 180 / M_PI;
	theta *= convert_factor;
	theta = fmodf(theta,360);
	if (theta < 0) 
		theta+=360.0;
	int theta_int  = (int)theta;
	//float frac = theta - theta_int;
	sin_value = sin_table[theta_int];// + frac * (sin_table[theta_int+1] - sin_table[theta_int]);
	cos_value = cos_table[theta_int];// + frac * (cos_table[theta_int+1] - cos_table[theta_int]);
}


SmokeParticle::SmokeParticle(const Ogre::Vector3 & across, const Ogre::Vector3 & up, const Ogre::Vector3 & pos,const Ogre::Vector3 & velocity, float width, float height, float dw, float dh ,float a1 ,float a2 ,float a3)
:m_pos(pos),m_velocity(velocity),m_across(across),m_up(up),
m_width(width),m_height(height),m_dwidth(dw),m_dheight(dh),m_time(0),
m_a1(a1),m_a2(a2),m_a3(a3)//,m_valid(true)
{
	m_rand = rand_in_range(-1,1);
	m_rand2 = rand_in_range(-1,1);
}
void SmokeParticle::Reset(const Ogre::Vector3 & across, const Ogre::Vector3 & up , const Ogre::Vector3 & pos,const Ogre::Vector3 & velocity, float width, float height, float dw, float dh,float a1 ,float a2 ,float a3)
{
	m_pos = pos;
	m_velocity = velocity;
	m_across = across;
	m_up = up;
	m_width = width;
	m_height = height;
	m_dwidth = dw;
	m_dheight = dh;
	m_time = 0;
	m_a1 = a1;
	m_a2 = a2;
	m_a3 = a3;
	m_rand = rand_in_range(-1,1);
	m_rand2 = rand_in_range(-1,1);
}

void SmokeParticle::update(const Ogre::Vector3 &destpos,float dt)
{
	Ogre::Vector3 acceleration = destpos - m_pos;
	acceleration.normalise();

	if(m_time < 0.2)
	{
		m_width += dt * m_dwidth;
		m_height += dt * m_dheight;
	}
	else if(m_time < 1)
	{
		float sin_value,cos_value;
		fast_sincos(m_time * 5,sin_value,cos_value);
		acceleration *= m_a1;
		acceleration += (m_rand * m_up * sin_value + m_rand2 * m_across * cos_value) * 10;
	}
	else if(m_time < 2)
	{
		float sin_value,cos_value;
		fast_sincos(m_time * 5,sin_value,cos_value);
		acceleration *= m_a2;
		acceleration += (m_rand * m_up * sin_value + m_rand2 * m_across * cos_value) * 10;
	}
	else
	{
		acceleration *= m_a3;
	}
	m_velocity += acceleration * dt;
	m_pos += m_velocity * dt;
	m_width += dt * m_dwidth;
	m_height += dt * m_dheight;
	m_time += dt;
}


SmokeManager::SmokeManager(float emitWidth,float emitHeight)
:m_manual(NULL),m_camera(NULL),m_emit_width(emitWidth),m_emit_height(emitHeight),
m_lifetime(2.0),m_last(0),m_now(0.6),m_interval(0.2),
m_acceleration1(5),m_acceleration2(15),m_acceleration3(30),
m_valid_num(0)
{
	m_manual =MXOgreWrapper::Get()->GetDefaultSceneManger()->createManualObject();
	m_manual->setDynamic(true);
	m_manual->begin("SmokeParticle");
	m_manual->position(0,0,0);
	m_manual->textureCoord(0,0);
	m_manual->textureCoord(1);
	m_manual->textureCoord(1);
	m_manual->textureCoord(1);

	m_manual->position(0,1,0);
	m_manual->textureCoord(0,1);
	m_manual->textureCoord(1);
	m_manual->textureCoord(1);
	m_manual->textureCoord(1);

	m_manual->position(1,0,0);
	m_manual->textureCoord(1,0);
	m_manual->textureCoord(1);
	m_manual->textureCoord(1);
	m_manual->textureCoord(1);

	m_manual->index(0);
	m_manual->index(2);
	m_manual->index(1);
	m_manual->end();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->attachObject(m_manual);
	//if(MXOgreWrapper::Get()->GetDefaultSceneManger()->getCameras().size() != 0)
	m_camera = (MXOgreWrapper::Get()->GetDefaultSceneManger()->getCameras()).begin()->second;
	build_tables();

	SetShadowMap(CShadowMap::Get());
}


SmokeManager::~SmokeManager(void)
{
	m_manual->detachFromParent();
	MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_manual);
	std::list<SmokeParticle*>::iterator itor = m_smoke_list.begin();
	while(itor != m_smoke_list.end())
	{
		delete (*itor);
		++itor;
	}
	m_smoke_list.clear();

	for(size_t n = 0 ; n < m_DeadList.size(); n++)
	{
		delete m_DeadList[n];
	}
	m_DeadList.clear();
}

void SmokeManager::preRenderShadowDepth()
{
	if(m_manual)
	   m_manual->setVisible(false);
}
void SmokeManager::postRenderShadowDepth()
{
	if(m_manual)
	   m_manual->setVisible(true);
}
void SmokeManager::addSmoke(const Ogre::Vector3 &pos,float width,float height,int num)
{
	if((m_now - m_last) > m_interval)
		m_last = m_now;
	else
		return;

	Ogre::Vector3 across_unit = m_camera->getRealRight();//m_camera->getRealDirection().crossProduct(temp);
	Ogre::Vector3 up_unit = m_camera->getRealUp();//m_camera->getRealDirection().crossProduct(across_unit);

	SmokeParticle *p_smoke = NULL;
	for(int i = 0;i < num; i++)
	{
		if(m_DeadList.size() == 0)
		   p_smoke = new SmokeParticle(across_unit,up_unit , pos , Ogre::Vector3::ZERO ,width ,height ,0.6,0.6 ,m_acceleration1,m_acceleration2,m_acceleration3);
		else
		{
		   p_smoke = m_DeadList[m_DeadList.size()-1];
		   p_smoke->Reset(across_unit,up_unit , pos , Ogre::Vector3::ZERO ,width ,height ,0.6,0.6 ,m_acceleration1,m_acceleration2,m_acceleration3);

		   m_DeadList.resize(m_DeadList.size()-1);
		}
		m_smoke_list.push_front(p_smoke);
		m_valid_num++;
	}

}


void SmokeManager::updateSmoke(const Ogre::Vector3& dest, float dt)
{
	if(m_valid_num)
	{
		std::list<SmokeParticle*>::iterator itor = m_smoke_list.begin();
		while(itor != m_smoke_list.end())
		{
			SmokeParticle * smokeParticle = (*itor);

			//if(smokePar->m_valid)
			//{
				if(smokeParticle->m_time >= m_lifetime)
				{
					//(*itor)->m_valid = false;
					m_DeadList.push_back(smokeParticle);
					m_valid_num--;
					itor = m_smoke_list.erase(itor);
				}
				else
				{
					(*itor)->update(dest,dt);
					itor++;
				}
			//}
			//++itor;
		}
	}
}

void SmokeManager::updateManual()
{
	m_manual->beginUpdate(0);
	std::list<SmokeParticle*>::iterator itor = m_smoke_list.begin();
	SmokeParticle* p_smoke;

	if(m_valid_num)
	{
		while(itor != m_smoke_list.end())
		{
			p_smoke = *itor;
			//if(!(p_smoke->m_valid))
			//{
				//++itor;
				//continue;
			//}

			m_manual->position(p_smoke->m_pos);
			m_manual->textureCoord(0,0);
			m_manual->textureCoord(p_smoke->m_time);
			m_manual->textureCoord(p_smoke->m_width);
			m_manual->textureCoord(p_smoke->m_height);


			m_manual->position(p_smoke->m_pos);
			m_manual->textureCoord(0,1);
			m_manual->textureCoord(p_smoke->m_time);
			m_manual->textureCoord(p_smoke->m_width);
			m_manual->textureCoord(p_smoke->m_height);

			m_manual->position(p_smoke->m_pos);
			m_manual->textureCoord(1,1);
			m_manual->textureCoord(p_smoke->m_time);
			m_manual->textureCoord(p_smoke->m_width);
			m_manual->textureCoord(p_smoke->m_height);

			m_manual->position(p_smoke->m_pos);
			m_manual->textureCoord(1,0);
			m_manual->textureCoord(p_smoke->m_time);
			m_manual->textureCoord(p_smoke->m_width);
			m_manual->textureCoord(p_smoke->m_height); 
			++itor;
		}
		int indices = m_valid_num * 4;//m_smoke_list.size() * 4;
		for(int i = 0; i< indices ; i+=4)
		{
			m_manual->index(i);
			m_manual->index(i+2);
			m_manual->index(i+1);

			m_manual->index(i);
			m_manual->index(i+3);
			m_manual->index(i+2);
		}
	}
	m_manual->end();
}

void SmokeManager::update(float dt)
{
	m_now += dt;
	updateSmoke(m_camera->getRealPosition(),dt);// + 0.2 * m_camera->getRealDirection() * m_camera->getNearClipDistance(),dt);
	updateManual();
}

void SmokeManager::setLifetime(float lifetime)
{
	m_lifetime = lifetime;
}

void SmokeManager::setInterval(float interval)
{
	m_interval = interval;
}


void SmokeManager::setAcceleration(float a1,float a2,float a3)
{
	m_acceleration1 = a1;
	m_acceleration2 = a2;
	m_acceleration3 = a3;
}