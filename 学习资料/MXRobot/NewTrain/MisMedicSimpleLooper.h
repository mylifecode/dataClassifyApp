#ifndef _MISMEDICSIMPLELOOPER_
#define _MISMEDICSIMPLELOOPER_
#include <Ogre.h>
#include "Dynamic/GoPhysDynamicLib.h"
#include "PhysicsWrapper.h"
#include "MisMedicThreadRope.h"
#include "MisMedicOrganInterface.h"
using namespace GoPhys;

class MisMedicOrgan_Ordinary;
class MisNewTraining;

//thread segment vs soft face collision
class CollideWithSoftPair
{
public:
	CollideWithSoftPair() : m_CollideBody(0) , m_SoftFace(0)
	{}
	
	CollideWithSoftPair(GFPhysSoftBody * sb , GFPhysSoftBodyFace * face) : m_CollideBody(sb) , m_SoftFace(face)
	{}

	GFPhysSoftBody * m_CollideBody;
	
	GFPhysSoftBodyFace * m_SoftFace;
		
	GFPhysVector3 m_CollideNormalOnFace;//

	GFPhysVector3 m_LocalPos;//on rigid looper

	GFPhysVector3 m_R;//looper rotate * m_LocalPos

	float m_FaceWeights[3];//get point as n0*w0 + n1*w1 + n2* w2
	
	//float m_ThreadWeight;//get point as n0 * (1-w) + n1 * w

	int   m_Segment;
	
	float m_Depth;
};


class FixPointAnchor
{
public:
	FixPointAnchor(const GFPhysVector3 & localRelPos)
	{
		m_LocalPoint = localRelPos;
	}
	void ResetFixPoint(const GFPhysVector3 & localRelPos)
	{
		m_LocalPoint = localRelPos;
	}	
	GFPhysVector3 m_LocalPoint;//local offset to com
	
};
class MisMedicSimpleLooper : public MisMedicThreadRope
{
public:

	MisMedicSimpleLooper(Ogre::SceneManager * sceneMgr , MisNewTraining * ownertrain);
	
	~MisMedicSimpleLooper();

	GFPhysTransform GetWorldTransform()
	{
		return m_LoopRigidTrans;
	}
	virtual void BeginSimulateThreadPhysics(float dt);

	virtual void EndSimuolateThreadPhysics(float dt);

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	void CreateLoopedThread(const GFPhysVector3 & circleCenter,
							const GFPhysVector3 & circelAxis,
							const GFPhysVector3 & StartVec,
							Ogre::SceneNode * AttachNode,
							ObjPhysCatogry threadCat);

	//抽拉的时候 圈套器缩小
	void Shrink(float percent);

	//尝试绑住圈住的对象
	bool TryBind();

	void CutBindedOrgan();

	GFPhysAlignedVectorObj<GFPhysVector3> m_LooperNodeWorldPos;

protected:

	GFPhysMatrix3 CalcLocalInertiaTensorAndCom(float thickness);
	
	void UpdateCollisionData();

	void CheckCollisionWithOrgan();

	void SolveFixPoints(const GFPhysTransform & fixpos , float dt);
	
	void SolveBend(float dt , float stiffness);

	void SolveSoftContact(float dt);

	void SolveConstraintedPairs(float dt);

	GFPhysVector3 CalcLoopForceFeedBack();

	void UpdateMesh();

	void UpdateMeshByCustomedRendNodes(std::vector<Ogre::Vector3> & RendNodes);

	void BuildLoopRestPosition(float radius);

	float m_TorqueStiffness;

	float m_CircleRadius;

	GFPhysVector3 m_LocalFixPoint;

	GFPhysVector3 m_LoopNormalLocal;

	GFPhysVector3 m_LoopDirectionLocal;

	GFPhysMatrix3 m_WorldInvInertial;

	GFPhysMatrix3 m_LocalInvInertial;

	GFPhysVector3 m_LocalLoopCom;

	GFPhysTransform m_LoopRigidTrans;

	float m_LoopMass;

	GFPhysVector3 m_LinearVelocity;

	GFPhysVector3 m_AnularVelocity;

	GFPhysAlignedVectorObj<FixPointAnchor> m_FixPoints;
	
	GFPhysAlignedVectorObj<GFPhysVector3> m_LooperNodeRestPos;
	
	
	
	GFPhysAlignedVectorObj<float> m_LooperNodeMass;

	GFPhysAlignedVectorObj<CollideWithSoftPair> m_CollidePairs;

	GFPhysAlignedVectorObj<CollideWithSoftPair> m_ConstraintedPairs;//pairs be constrainted on the loop

	bool m_IsLoopBinded;
};

#endif