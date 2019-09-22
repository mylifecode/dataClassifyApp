#ifndef _THREADCOLLISION_
#define _THREADCOLLISION_
#include <Ogre.h>
#include "Dynamic/GoPhysDynamicLib.h"
#include "PhysicsWrapper.h"


using namespace GoPhys;


//thread segment vs rigid body collision
class TRCollidePair
{
public:
	TRCollidePair(GFPhysRigidBody * rigid,
		          GFPhysTransform & rbTrans,
		          const GFPhysVector3 & rigidWorldPt,
		          const GFPhysVector3 & rigidNormal,
		          float SegmentWeight,
		          float depth,
		          int segIndex
		          );

	void BuildAdditional(const GFPhysVector3 segPosInCollideTime[2],
		                 const GFPhysVector3 & PosInConvexInCollideTime,
		                 float segNodeInvMass[2],
		                 const GFPhysVector3 & NormalInCOnvexInCollideTime,
		                 float segRadius,
		                 GFPhysTransform & rbTransInCollideTime);

	void PrepareForSolveContact(float dt);

	void SolveContact(float dt, GFPhysVector3 & segNodePos0, GFPhysVector3 & segNodePos1);

	GFPhysRigidBody * m_Rigid;

	
	GFPhysVector3 m_LocalAnchorOnRigid;

	GFPhysVector3 m_RigidWorldPoint;

	GFPhysVector3 m_NormalOnRigid;
    
	GFPhysVector3 m_Tangnet[2];

	GFPhysMatrix3 m_ContactImpMatrix;

	GFPhysVector3 m_R;

	//for solve mode1
	GFPhysVector3 m_LocalPtOnRigidWithRadius[2];

	GFPhysVector3 m_RWithRadius[2];

	GFPhysMatrix3 m_KMatrix[2];

	float m_NodeInvMass[2];

	//
	int   m_Mode;

	float m_Depth;
	
	float m_SegWeight;//s[2];//point weights in segment

	int   m_Segment;

	float m_ThreadImpulseDeta[2];

	float m_ImpluseNormalOnRigid;

	float m_w0;
	float m_w1;

	float m_LambdaN;
	float m_LambdaT[2];
};

//thread segment vs soft face collision
class TFCollidePair
{
public:
	enum TFCollideType
	{
		TFCD_EE,//Edge edge
		TFCD_VF,//vertex face
		TFCD_EF,//edge to face
	};
	TFCollidePair(GFPhysSoftBody * sb);
	TFCollidePair(GFPhysSoftBody * sb , GFPhysSoftBodyFace * face);

	int GetCollideSegmentIndex()  const;
	GFPhysSoftBody * GetCollideBody() const;
	GFPhysSoftBodyFace * GetCollideSoftFace()  const;

	//mark as edge-edge collision pair
	void SetEE_Collision(int e1 , int e2 , int e3 , int e4 , const Ogre::Vector3 & collideNormal , const Ogre::Vector3 & FaceNormal);
	
	//mark as node-face collision pair
	void SetVF_Collision(int nt , const Ogre::Vector3 & collideNormal , const Ogre::Vector3 & FaceNormal);

	void SetEF_Collision(int segIndex , const Ogre::Vector3 & collideNormal , const Ogre::Vector3 & FaceNormal);

	TFCollideType m_CollideType;
	int m_e1;//
	int m_e2;//
	int m_e3;//
	int m_e4;//
	Ogre::Vector3 m_ImpluseOnThread;
	GFPhysSoftBody * m_CollideBody;
	GFPhysSoftBodyFace * m_SoftFace;
	Ogre::Vector3 m_FaceNormal;
	Ogre::Vector3 m_CollideNormal;//

	GFPhysSoftBodyNode * m_CollideNode[4];
	int   m_NumCollideNode;
	float m_CollideNodeDepth[4];
	
	float m_FaceWeihts[3];
	float m_ThreadWeigths[2];
};

#endif