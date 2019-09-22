/*********************************************
×ÆÉÕÊ±Ã°ÅÝ
*********************************************/
#pragma once
#include "MXOgreWrapper.h"
#include "Ogre.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include <list>



using namespace GoPhys;


struct CBubble
{
	GFPhysSoftBodyFace *m_pAttachFace;
	float m_beta;
	float m_gamma;
	Ogre::Vector3 m_center;
	float m_CurrSize;
	float m_ExplosionSize;
	float m_ExpandRate;
	float m_time;
	float m_lifetime;
	bool  m_valid;
	//CBubble(Ogre::Vector3 &center,Ogre::Vector3 &across_half,Ogre::Vector3 &up_half, float size,float scale);//:m_center(center),m_across_half(across_half),m_up_half(up_half),m_size(size),m_new_size(size),m_add_size(m_size * (scale - 1)),m_alpha(0.8),m_time(0),m_lifetime(){}
	CBubble::CBubble() {}
	CBubble::CBubble(Ogre::Vector3 &center,float size,float scale);
	void Reset(GFPhysSoftBodyFace *pFace , float beta , float gamma ,float size,float scale);
};

struct BubbleControlInfoForClamp
{
	//the border of the clampplugin
	float Left;
	float Right;
	float Bottom;
	float Top;
	float MaxDeviate;
	float XAllowRange;
	float YAllowRange;
	float MinExplosionSize;
	float MaxExplosionSize;
	float ExpandRate;
};


class BubbleManager
{
public:
	BubbleManager(void);
	
	~BubbleManager(void);

	bool BeginRandomAddBubble();

	void addBubblesForClamp(GFPhysSoftBodyFace *pFace , Ogre::Vector2 faceVertices2D[3]  , BubbleControlInfoForClamp & controlInfo ,int num);

	void addBubblesOnFace(GFPhysSoftBodyFace *pFace, float weights[3], BubbleControlInfoForClamp & controlInfo, int num);
	//bool HasCoolDown();

	void BeginRendOneFrame(float dt);
	//void inBurn() {m_in_burn = true;}
	//void stopBurn() {m_in_burn = false;}
private:
	
	int  getNewBubble();

	void updateBubbles(float dt);
	
	void updateRenderPart();
	//Ogre::ManualObject *m_manual;
	std::list<int> m_bubbles;
	std::vector<int> m_invalid_bubbles;
	std::vector<CBubble> m_bubble_pool;

	Ogre::Camera *m_camera;
	float m_now;
	float m_last;
	//float m_burntime;
	bool m_continue;
	//bool m_in_burn;
	int m_valid_num;
	int m_NumOfBubblesUsed;

	Ogre::InstanceManager * m_BubbleInstance;
	std::vector<Ogre::InstancedEntity*> m_InstanceEntity;
};