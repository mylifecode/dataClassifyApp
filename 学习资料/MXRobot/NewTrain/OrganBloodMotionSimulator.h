#ifndef _BLOODPARTILCEUPDATOR_
#define _BLOODPARTILCEUPDATOR_
#include "Ogre.h"
#include "DynamicParticle.h"
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "ParticleOrganFaceCollideAlgorithm.h"
using namespace GoPhys;
class DynamicParticle;
class MisMedicOrgan_Ordinary;
class VeinConnectObject;


class BloodMoveData
{
public:
	BloodMoveData(float slipVelAcc, float maxSlipVel);

	void   reset(float slipVelAcc, float maxSlipVel);

	void   SetStickFaceId(unsigned int sid);

	float  m_positionweights[3];

	float  m_linearvelweights[3];
		
	float  m_ExceedPosWeights[3];

	DynamicParticle * m_paritcle;

	MisMedicOrgan_Ordinary * m_stickedOrgan;//sticked organ object

	unsigned int m_stickedfaceid;//the face id 

	bool m_IsBloodMoveable;//

	bool m_exceededface;
			
	Ogre::Vector3 m_currfacenorm;
			
	bool  m_randtangent;

	float m_SlipAccelerate;

	float m_MaxSlipspeed;

	Ogre::Vector3 m_vectorDir;
};


/*class BloodInVeinConnect
{
public:
	BloodInVeinConnect();

	VeinConnectObject * m_veinconnectobj;
	int   m_clusterID;
	int   m_PairID;
	float m_PositionWeight;
	bool  m_randtangent ;
	bool  m_isactive;
	float m_currslipspeed;
	float m_slipaccelerate;
	float m_maxslipspeed;
};
*/

class DynamicBloodParticle : public DynamicParticle
{
public:
	DynamicBloodParticle(Ogre::Vector3 origin ,  float mass , float radius);

	virtual void reset();
	
	//设置特定的参数，用于器官被剪面片处的动态流血效果
	void setParameterForDynamicInCutFace();	
	//Ogre::Vector2 m_TextureUV;

	float m_QuantityDecPermove;

	float m_CurrQuanity;

	float m_StartQuantity;

	float m_StopQuanity;

	float m_Startradius;

	float m_Endradius;

	float m_fadeoutalpha;

	float m_initalpha;


	MisMedicOrgan_Ordinary * m_Organ;
};

class OrganBloodMotionSimulator
{
public:
	class Listener
	{
	public:
		virtual void onParticleCreated(DynamicBloodParticle * particle) = 0;
		virtual void onParticleRemoved(DynamicBloodParticle * particle) = 0;
		virtual void onSimulatorDestoryed() = 0;
	};
	OrganBloodMotionSimulator();

	~OrganBloodMotionSimulator();

	//gravityFlag = true ---- reset  the organ gravitydir
	void Initialize(MisMedicOrgan_Ordinary * bloodobject , bool gravityFlag = false);

	bool Update(float dt);

	//parameter setter
	void setGravityDir(Ogre::Vector3 gravitydir);

	void setSlipAccelerate(float slipacc);//0.0~0.4f

	void setMaxSpeed(float maxspeed);//0.0~0.8f

	void setMaxParticleNum(int parnum);//0~500

	void GetBloodParticles(std::vector<DynamicBloodParticle*> & particles);
	
	void AddListener(OrganBloodMotionSimulator::Listener * listener);

	void RemoveListener(OrganBloodMotionSimulator::Listener * listener);

	DynamicBloodParticle * createBloodParticle(MisMedicOrgan_Ordinary * organ , float weights[3] ,  
		float mass, int initfaceid, float radius, float BloodSlipAccelrate, float BloodMaxSlipVel , float fDir = 0.0f);
	
	void onRemoveParticle(DynamicBloodParticle * particle);

	MisMedicOrgan_Ordinary * GetHostOrgan()
	{
		return m_HostObject;
	}
	
	int m_maxparticlenum;

	float m_paritcletangentrate;

protected:

	bool MoveToNextFace(BloodMoveData & movedata);

	bool MoveInCurrentFace(BloodMoveData & movedata , float deltatime, Ogre::Vector3 & endspeed, float &usedtime , bool cleartangent);

	Ogre::Vector3 CalculateCurrentPos(BloodMoveData & movedata);

	void removeDeadParticles(int maxparnum);

	bool updateParticlePosition(float timeinterval , int stepCount , BloodMoveData & movedata ,  Ogre::Vector3 & finalresult);

	bool updateParticles(float dt);

	//internal use
	DynamicBloodParticle * allocParticle(const GoPhys::GFPhysVector3 & origin ,  float mass , float radius);

	void RecycleParticle(DynamicBloodParticle * particle);

	void addToActiveList(DynamicBloodParticle * movedparticle);

	DynamicBloodParticle * GenNewParticle(const GoPhys::GFPhysVector3 &origin , 
		                                  float mass , float radius ,
										  float BloodSlipAccelrate, float BloodMaxSlipVel);

	void DestoryAllParticles();
	int m_activeparticlecount;

	float m_AccmulateTime;

	Ogre::Vector3 m_gravitydir;

	std::vector<Listener *> m_listeners;

	MisMedicOrgan_Ordinary * m_HostObject;

	PresetParticleOrganFaceCollideAlogrithm m_collidealgo;

	DynamicBloodParticle * m_activeparticlehead;

	DynamicBloodParticle * m_activeparticleend;

	DynamicBloodParticle * m_recycledhead;

	bool m_bIsTest;
};

#endif