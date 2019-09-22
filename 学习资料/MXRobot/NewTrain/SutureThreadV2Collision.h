#ifndef _SUTURETHREADV2COLLISION_
#define _SUTURETHREADV2COLLISION_
#include <Ogre.h>
#include "Dynamic/GoPhysDynamicLib.h"
#include "Dynamic/PhysicBody/GoPhysSoftTube.h"
#include "PhysicsWrapper.h"

//tube vs soft body face collide pair
class STVSFCollidePair
{
public:
	
	STVSFCollidePair(GFPhysSoftBody * sb , GFPhysSoftTube * tube);
	
	STVSFCollidePair(GFPhysSoftBody * sb , GFPhysSoftTube * tube , GFPhysSoftBodyFace * face);

	int GetCollideSegmentIndex()  const;
	
	GFPhysSoftBody * GetCollideBody() const;
	
	GFPhysSoftBodyFace * GetCollideSoftFace()  const;

	void SetEF_Collision(int segIndex, const GFPhysVector3 & collideNormal, const GFPhysVector3 & FaceNormal);

	GFPhysVector3 SolveCollision();

	int m_SegIndex;//
	GFPhysVector3 m_ImpluseOnThread;
	GFPhysSoftBody * m_CollideBody;
	GFPhysSoftBodyFace * m_SoftFace;
	GFPhysSoftTube * m_Tube;
	GFPhysVector3 m_FaceNormal;
	GFPhysVector3 m_CollideNormal;//

	GFPhysSoftBodyNode * m_CollideNode[4];
	int   m_NumCollideNode;
	float m_CollideNodeDepth[4];

	float m_FaceWeihts[3];
	float m_ThreadWeight;// s[2];

	float m_ColRadius;
	bool  m_IsPositiveCol;

};

//tube segment vs rigid body pair
class STVRGCollidePair
{
public:
	STVRGCollidePair(GFPhysRigidBody * rigid,
		             GFPhysSoftTube  * tube,
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

	void SolveCollision(float dt);// , GFPhysVector3 & segNodePos0, GFPhysVector3 & segNodePos1);

	GFPhysRigidBody * m_Rigid;

	GFPhysSoftTube * m_Tube;

	GFPhysVector3 m_LocalAnchorOnRigid;

	GFPhysVector3 m_RigidWorldPoint;

	GFPhysVector3 m_NormalOnRigid;

	GFPhysVector3 m_Tangnet[2];

	//GFPhysMatrix3 m_ContactImpMatrix;

	//GFPhysVector3 m_R;

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

	//float m_w0;
	//float m_w1;

	//float m_LambdaN;
	//float m_LambdaT[2];
};

//tube segment vs tube segment collison
class STVSTCollidePair
{
public:
	STVSTCollidePair(GFPhysSoftTube * ropeA,
		             GFPhysSoftTube * ropeB,
		             int SegmentA,
		             int SegmentB);

	int GetCollideSegmentA() const
	{ 
		return m_SegmentA; 
	}

	int GetCollideSegmentB() const
	{ 
		return m_SegmentA; 
	}

	int m_SegmentA;
	int m_SegmentB;

	float m_CollideDist;
	float m_WeightA;//collide position is pos0*(1-weight0) + pos * weight
	float m_WeightB;
	
	GFPhysSoftTube * m_TubeA;
	GFPhysSoftTube * m_TubeB;

	GFPhysVector3 m_NormalOnB;
	float m_ColRadiusA;
	float m_ColRadiusB;

	void SolveCollision();//
};

//thread vs compound shape 
class SutureThreadV2CompoundCallBack : public GFPhysNodeOverlapCallback, public GFPhysDCDInterface::Result
{
public:

	SutureThreadV2CompoundCallBack(Real margin,GFPhysRigidBody * rb , GFPhysCollideShape * childshape,
		                           GFPhysTransform & rbTrans,
		                           Real collideradius,GFPhysSoftTube * rope,
		                           GFPhysAlignedVectorObj<STVRGCollidePair> & collidepair);
	virtual ~SutureThreadV2CompoundCallBack();
	
	virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData);

	void SetShapeIdentifiers(int partId0, int index0, int partId1, int index1);

	void AddContactPoint(const GFPhysVector3& normalOnBInWorld, const GFPhysVector3& pointInWorld, Real depth);

	int m_SegIndex;

	GFPhysVector3 m_SegPos0;

	GFPhysVector3 m_SegPos1;

	Real m_SegNodeInvM[2];
	GFPhysRigidBody * m_rb;

	GFPhysCollideShape * m_collideshape;

	GFPhysSoftTube * m_Rope;
	Real m_collideRadius;

	Real m_threadMarin;

	Real m_convexMargin;

	GFPhysAlignedVectorObj<STVRGCollidePair>  & m_CollidePairs;
	GFPhysTransform m_rbTransform;
};

//thread vs convex
class SutureThreadV2ConvexCallBack : public GFPhysNodeOverlapCallback, public GFPhysDCDInterface::Result
{
public:
	SutureThreadV2ConvexCallBack(Real margin, GFPhysRigidBody * rb,
		GFPhysTransform & rbTrans, Real dt, Real collideradius,
		GFPhysSoftTube * rope,
		GFPhysAlignedVectorObj<STVRGCollidePair> & collidepair
		);

	virtual ~SutureThreadV2ConvexCallBack();

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData);

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode);

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB);

	void SetShapeIdentifiers(int partId0, int index0, int partId1, int index1);

	void AddContactPoint(const GFPhysVector3& normalOnBInWorld, const GFPhysVector3& pointInWorld, Real depth);

	int m_SegIndex;

	GFPhysVector3 m_SegPos0;

	GFPhysVector3 m_SegPos1;

	Real m_SegNodeInvM[2];

	GFPhysRigidBody * m_rb;

	GFPhysSoftTube  * m_Rope;

	Real m_collideRadius;

	Real m_threadMarin;

	Real m_convexMargin;

	GFPhysAlignedVectorObj<STVRGCollidePair>  & m_CollidePairs;
	GFPhysTransform m_rbTransform;
	Real m_dt;
};

//thread vs soft body face
class SutureThreadV2SegSoftFaceCallback : public GFPhysNodeOverlapCallback
{
public:
	SutureThreadV2SegSoftFaceCallback(Real Margin , GFPhysSoftBody * sb,
		                              Real collideradius, GFPhysSoftTube * rope,
		                              GFPhysAlignedVectorObj<STVSFCollidePair> & paircd,
		                              bool useCCD);

	virtual ~SutureThreadV2SegSoftFaceCallback();

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData);

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode);

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB);

	GFPhysSoftBody * m_sb;
	GFPhysSoftTube * m_Rope;
	Real m_collideRadius;
	Real m_Margin;
	GFPhysAlignedVectorObj<STVSFCollidePair> & m_CollidePairs;
	std::vector<STVSFCollidePair> m_InvCollidePairs;
	bool m_UseCCD;
};

//thread vs thread
class SutureThreadV2ThreadCallBack : public GFPhysNodeOverlapCallback
{
public:
	SutureThreadV2ThreadCallBack(Real margin , Real collideradius,
		                         GFPhysSoftTube * tubeA,GFPhysSoftTube * tubeB,
		                         GFPhysAlignedVectorObj<STVSTCollidePair> & collidepair);

	virtual ~SutureThreadV2ThreadCallBack();

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData);

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode);

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB);

	void SetShapeIdentifiers(int partId0, int index0, int partId1, int index1);


	GFPhysSoftTube * m_RopeB;

	GFPhysSoftTube * m_RopeA;

	bool  m_IsSelftCollision;

	Real m_threadRadius;

	Real m_threadMarin;

	GFPhysAlignedVectorObj<STVSTCollidePair>  & m_CollidePairs;
};

#endif