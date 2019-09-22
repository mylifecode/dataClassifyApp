#include "BubbleManager.h"
#include "MXOgreGraphic.h"
#include "MXOgreWrapper.h"
#include "Math/GoPhysTransformUtil.h"
using namespace std;


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


float montecarlo_small(float min , float max)
{
	float p1,p2;
	while(true)
	{
		p1 = rand_in_range(min,max);
		p2 = rand_in_range(min,max);
		if(p1 < p2)
			return p1;
	}
}

float montecarlo_big(float min , float max)
{
	float p1,p2;
	while(true)
	{
		p1 = rand_in_range(min,max);
		p2 = rand_in_range(min,max);
		if(p1 > p2)
			return p1;
	}
}




double IntegerNoise (int n)

{

	n = (n >> 13) ^ n;

	int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;

	return 1.0 - ((double)nn / 1073741824.0);

}

inline double lerp(double a,double b, double w) { return a * (1 - w) + b * w;}

double CoherentNoise (double x)

{
	int intX = (int)(floor (x));
	double n0 = IntegerNoise (intX);
	double n1 = IntegerNoise (intX + 1);
	double weight = x - floor (x);
	double noise = lerp (n0, n1, weight);
	return noise;

}




float special_rand_num()
{
	static float numbers[15] = {0,0,0,0,0,1,0,0,0,0,0,0,0,0,0};
	return numbers[rand() % 15];
}

float special_rand_size()
{
	static float sizes[15] =  {0,0,0,0,0,0.02,0,0,0,0,0,0,0,0,0.02};
	return sizes[rand() % 15];
}



CBubble::CBubble(Ogre::Vector3 &center, float expandRate, float exposionSize)
	:m_center(center), m_ExplosionSize(exposionSize), m_ExpandRate(expandRate)
,m_time(0),m_lifetime(rand_in_range(0.2,0.25)),m_valid(true)
{
	m_CurrSize = 0.001f;
}

void CBubble::Reset(GFPhysSoftBodyFace *pFace , float beta , float gamma ,float expandRate,float exposionSize)
{
	m_pAttachFace = pFace;
	m_beta	= beta;
	m_gamma = gamma;
	m_CurrSize = 0.001f;
	m_ExplosionSize = exposionSize;
	m_ExpandRate    = expandRate;// m_size * scale;
	m_time = 0;
	m_lifetime = rand_in_range(0.3,0.5);
	m_valid = true;
}

#define MAXBUBBLEINST 500
BubbleManager::BubbleManager()
	:m_now(0),m_last(0)/*,m_burntime(0),m_in_burn(false)*/,m_valid_num(0)
{
	m_bubble_pool.resize(1000);
	m_invalid_bubbles.reserve(m_bubble_pool.size());
	m_NumOfBubblesUsed = 0;

	m_BubbleInstance = MXOgreWrapper::Get()->GetDefaultSceneManger()->createInstanceManager("InstanceMgrHWBasix", "instancespehere.mesh",
		                                                          Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, 
																  Ogre::InstanceManager::HWInstancingBasic,
																  MAXBUBBLEINST, Ogre::IM_USEALL);



	for (int j = 0; j < MAXBUBBLEINST; ++j)
	{
		//Create the instanced entity
		Ogre::InstancedEntity * ent = m_BubbleInstance->createInstancedEntity("Examples/Instancing/HWBasic/Robot");
		m_InstanceEntity.push_back(ent);

		ent->setOrientation(Ogre::Quaternion::IDENTITY);
		ent->setPosition(Ogre::Vector3(0, 0, 0));
		ent->setScale(Ogre::Vector3(0.01f, 0.01f, 0.01f));
		ent->setInUse(false);
	}
}

BubbleManager::~BubbleManager()
{
	Ogre::SceneManager * sceneMgr = MXOgreWrapper::Get()->GetDefaultSceneManger();
	for (int c = 0; c < (int)m_InstanceEntity.size(); c++)
	{
		Ogre::InstancedEntity * ent = m_InstanceEntity[c];
		if (ent->isInUse())
		    sceneMgr->destroyInstancedEntity(ent);//no need "destroyInstanceManager" will auto delete instance entity
	}
	MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyInstanceManager(m_BubbleInstance);
}


//bool BubbleManager::HasCoolDown()
//{
	//if(m_now-m_last > 0.1/* && m_burntime > 0.5*/) //rand_in_range(0.1,0.2))
	//{
	//	m_last = m_now;
	//	return true;
	//}
	//return false;
//}
bool BubbleManager::BeginRandomAddBubble()
{
	static int seed = 0;

	if (m_now - m_last > 0.1)//check has CoolDown
	{
		m_last = m_now;
		
		srand(seed);
		
		seed++;
		
		return true;
	}
	else
	{
		return false;
	}
}
void BubbleManager::addBubblesForClamp(GFPhysSoftBodyFace *pFace , Ogre::Vector2 faceVertices2D[3] , BubbleControlInfoForClamp & controlInfo ,int num)
{
	const int threshold = 20;
	int count = 0 ;
	int addNum = 0;
	int bubble_index = -1;
	
	while(addNum < num)
	{
		if(count > threshold)
		   break;
		count++;
		float beta  = frand(1);
		
		float gamma = frand(1 - beta);
		
		if((beta + gamma) > 1.0)
		{
			beta  = 1 - beta;
			gamma = 1 - gamma;
		}

		Ogre::Vector2 p = faceVertices2D[0] * (1 - beta - gamma) +
			              faceVertices2D[1] * beta +
			              faceVertices2D[2] * gamma;

		bool isused = false;

		if(p.y < controlInfo.Top && p.y > controlInfo.Bottom)
		{
			if(abs(p.x - controlInfo.Left) <= controlInfo.XAllowRange || abs(p.x - controlInfo.Right) <= controlInfo.XAllowRange)
			   isused = true;
		}
		else if((controlInfo.Top < p.y && p.y < (controlInfo.Top + controlInfo.YAllowRange)) || (controlInfo.Bottom > p.y && p.y > (controlInfo.Bottom - controlInfo.YAllowRange)))
		{
			if((p.x < controlInfo.Right + controlInfo.XAllowRange) && p.x > (controlInfo.Left - controlInfo.XAllowRange))
			    isused = true;
		}
		if(isused)
		{
			float explosionsize = rand_in_range(controlInfo.MinExplosionSize, controlInfo.MaxExplosionSize);
			bubble_index = getNewBubble();
			m_bubble_pool[bubble_index].Reset(pFace, beta, gamma, controlInfo.ExpandRate , explosionsize);
			m_bubbles.push_front(bubble_index);
			addNum++;
		}
	}
}

void BubbleManager::addBubblesOnFace(GFPhysSoftBodyFace *pFace, float weights[3], BubbleControlInfoForClamp & controlInfo, int num)
{
	const int threshold = 20;
	int count = 0;
	int addNum = 0;
	int bubble_index = -1;
	
	GFPhysVector3 tan0 = Perpendicular(pFace->m_RestFaceNormal);
	
	GFPhysVector3 tan1 = pFace->m_RestFaceNormal.Cross(tan0).Normalized();

	GFPhysVector3 center = pFace->m_Nodes[0]->m_UnDeformedPos * weights[0]
		                 + pFace->m_Nodes[1]->m_UnDeformedPos * weights[1]
		                 + pFace->m_Nodes[2]->m_UnDeformedPos * weights[2];

	while (addNum < num)
	{
		if (count > threshold)
			break;
		
		count++;
		
		float theta  = frand(1) * 2 * 3.141592f;

		float radius = frand(1) * controlInfo.MaxDeviate;

		GFPhysVector3 deviateVec = (tan0 * cosf(theta) + tan1 * sinf(theta)) * radius;

		float weights[3];

		CalcBaryCentric(pFace->m_Nodes[0]->m_UnDeformedPos, 
			            pFace->m_Nodes[1]->m_UnDeformedPos,
						pFace->m_Nodes[2]->m_UnDeformedPos,
			            deviateVec + center, 
						weights[0], weights[1], weights[2]);

		
		float explosionsize = rand_in_range(controlInfo.MinExplosionSize, controlInfo.MaxExplosionSize);
		bubble_index = getNewBubble();
		m_bubble_pool[bubble_index].Reset(pFace, weights[1], weights[2], controlInfo.ExpandRate, explosionsize);
		m_bubbles.push_front(bubble_index);
		addNum++;
	}
}
void BubbleManager::updateBubbles(float dt)
{
	for(list<int>::const_iterator it = m_bubbles.begin();it != m_bubbles.end();)
	{
		CBubble & bubble = m_bubble_pool[*it];
		
		if(bubble.m_CurrSize >= bubble.m_ExplosionSize)
		{
			m_invalid_bubbles.push_back(*it);
			it = m_bubbles.erase(it);
		}
		else
		{
			bubble.m_CurrSize += bubble.m_ExpandRate * dt;
			bubble.m_time += dt;
			it++;
		}
	}
}

void BubbleManager::updateRenderPart()
{
	int numBubble = 0;

	for (list<int>::const_iterator it = m_bubbles.begin(); it != m_bubbles.end(); ++it)
	{
		CBubble * p_bubble = &(m_bubble_pool[*it]);

		Ogre::Vector3 position = GPVec3ToOgre(p_bubble->m_pAttachFace->m_Nodes[0]->m_CurrPosition * (1 - p_bubble->m_beta - p_bubble->m_gamma) +
			                                  p_bubble->m_pAttachFace->m_Nodes[1]->m_CurrPosition * p_bubble->m_beta +
			                                  p_bubble->m_pAttachFace->m_Nodes[2]->m_CurrPosition * p_bubble->m_gamma);

		float scale = p_bubble->m_CurrSize;
		
		if (numBubble < (int)m_InstanceEntity.size())
		{
			m_InstanceEntity[numBubble]->setPosition(position);
			m_InstanceEntity[numBubble]->setScale(Ogre::Vector3(scale, scale, scale));
			
			m_InstanceEntity[numBubble]->setInUse(true);
			numBubble++;
		}
		else
		{
			break;
		}
	}

	while (numBubble < (int)m_InstanceEntity.size())
	{
		m_InstanceEntity[numBubble]->setInUse(false);
		numBubble++;
	}
}

void BubbleManager::BeginRendOneFrame(float dt)
{
	m_now += dt;
	//if(m_in_burn)
		//m_burntime += dt;
	//else
	//	m_burntime = 0;
	updateBubbles(dt);
	
	updateRenderPart();
}

int BubbleManager::getNewBubble()
{
	int index = -1;
	CBubble *p_bubble  = NULL;
	if(m_invalid_bubbles.size() != 0)
	{
		index = m_invalid_bubbles.back();
		m_invalid_bubbles.pop_back();
	}
	else
	{
		if (m_NumOfBubblesUsed < (int)m_bubble_pool.size())
		{
			index = m_NumOfBubblesUsed++;
		}
		else
		{
			m_bubble_pool.resize(m_bubble_pool.size() * 2);
			m_invalid_bubbles.reserve(m_bubble_pool.size());
			index = m_NumOfBubblesUsed++;
		}
	}
	return index;
}