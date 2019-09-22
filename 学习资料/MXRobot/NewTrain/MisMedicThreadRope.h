#ifndef _MISMEDICTHREADROPE_
#define _MISMEDICTHREADROPE_
#include <Ogre.h>
#include "Dynamic/GoPhysDynamicLib.h"
#include "PhysicsWrapper.h"
#include "MisMedicTubeShapeRendObject.h"
#include "MisMedicOrganInterface.h"
#include "MisCustomSimObj.h"
#include "ThreadCollision.h"
using namespace GoPhys;

class MisMedicOrgan_Ordinary;
class MisNewTraining;
class CTool;
class MisMedicThreadRope;
float SegmentTriangleClosetDist(GFPhysVector3 triVerts[3] , GFPhysVector3 segVerts[2]);
enum  ThreadSegmentState
{
	TST_INKNOT = 1,
	TST_INSELFCOLLISION = 1<<1,
};

enum ThreadEleTag
{
	TET_CANCOLLIDERIGID = 1,
	TET_CANCOLLIDESOFT  = (1 << 1),
	TET_CANCOLLIDESELF  = (1 << 2),
	TET_INKNOT = (1 << 3),
	TET_ATTACHED = (1 << 4),
};

enum ThreadRealTimeState
{
	TRT_COLLIDERIGID  = 1,
	TRT_COLLIDESOFT   = (1 << 1),
	TRT_COLLIDETHREAD = (1 << 2),
	TRT_NEEDSCALESOLVERMASS = (1 << 3),
};


class ThreadNode
{
public:
	ThreadNode(const GFPhysVector3 & position)
	{
		m_UnDeformedPos = m_CurrPosition = m_LastPosition = position;
		m_InvMass = 1.0f;
		m_SolverInvMassScale = 1.0f;
		m_Velocity = GFPhysVector3(0,0,0);
		m_rtState  = 0;
		m_Tag = (TET_CANCOLLIDERIGID | TET_CANCOLLIDESOFT | TET_CANCOLLIDESELF);

		//m_bCollideRigid = m_bCollideSoft = true;
		//m_bCanSelfCollision = true;
		//m_FrameCollideTag = false;
		//m_bAttached = false;
		m_ParticleOrient.SetIdentity();
	}	
	//============================================
	ThreadNode()
	{
		m_InvMass = 1.0f;
		m_SolverInvMassScale = 1.0f;
		m_Velocity = GFPhysVector3(0,0,0);
		m_rtState  = 0;
		m_Tag = (TET_CANCOLLIDERIGID | TET_CANCOLLIDESOFT | TET_CANCOLLIDESELF);
		
		//m_bCollideRigid = m_bCollideSoft = true;

		//m_bCanSelfCollision = true;
		//m_FrameCollideTag = false;
		//m_bAttached = false;
		m_ParticleOrient.SetIdentity();
	}
	//============================================
	inline float GetInvMass()
	{
		return m_InvMass;
	}
	//============================================
	inline void  SetInvMass(float invMass)
	{
		m_InvMass = invMass;
	}
	//============================================
	inline float GetSolverInvMasScale()
	{
		return m_SolverInvMassScale;
	}
	//============================================
	inline void  SetSolverInvMassScale(float scalem)
	{
		m_SolverInvMassScale = scalem;
	}
	//============================================
	inline float GetSolverInvMass()
	{
		return m_InvMass*m_SolverInvMassScale;
	}
	//============================================
	inline GFPhysMatrix3 GetParticleOrient()
	{
		return m_ParticleOrient;
	}
	//============================================
	inline void SetParticleOrient(GFPhysMatrix3 & mat)
	{
		m_ParticleOrient = mat;
	}
	//============================================
	inline void MarkAsCollideRigid()
	{
		m_rtState |= TRT_COLLIDERIGID;
	}
	//============================================
	inline void MarkAsCollideSoft()
	{
		m_rtState |= TRT_COLLIDESOFT;
	}
	//============================================
	inline void MarkAsAttahed(bool set = true)
	{
		if(set)
		   m_Tag |= TET_ATTACHED;
		else
		   m_Tag &= (~TET_ATTACHED);
	}
	//============================================
	inline void MarkAsNeedScaleSolverMass()
	{
		m_rtState |= TRT_NEEDSCALESOLVERMASS;
	}
	//============================================
	inline bool IsAttached()
	{
		return (m_Tag & TET_ATTACHED);
	}
	//===============================================
	inline bool IsCollideRigid()
	{
		return (m_rtState & TRT_COLLIDERIGID);
	}
	//===============================================
	inline bool IsCollideSoft()
	{
		return (m_rtState & TRT_COLLIDESOFT);
	}
	//============================================
	inline void ClearRealTimeState()
	{
		m_rtState = 0;
	}
	//============================================
	inline int GetRealTimeState()
	{
		return m_rtState;
	}
	//============================================
	inline void SetCanCollideRigid(bool cancollide)
	{
		if(cancollide)
		   m_Tag |= TET_CANCOLLIDERIGID;
		else
		   m_Tag &= (~TET_CANCOLLIDERIGID);
	}
	//============================================
	inline void SetCanCollideSoft(bool cancollide)
	{
		if(cancollide)
		   m_Tag |= TET_CANCOLLIDESOFT;
		else
		   m_Tag &= (~TET_CANCOLLIDESOFT);
	}
	//============================================
	inline bool GetCanCollideSoft()
	{
		return (m_Tag & TET_CANCOLLIDESOFT);
	}
	//============================================
	inline bool GetCanCollideRigid()
	{
		return (m_Tag & TET_CANCOLLIDERIGID);
	}
	//============================================
	inline void SetCanCollideSelf(bool cancollide)
	{
		if(cancollide)
		   m_Tag |= TET_CANCOLLIDESELF;
		else
		   m_Tag &= (~TET_CANCOLLIDESELF);
	}
	//============================================
	inline void SetInKnotLoop(bool inloop)
	{
		if(inloop)
		   m_Tag |= TET_INKNOT;
		else
		   m_Tag &= (~TET_INKNOT);
	}
	//============================================
	inline void ClearTag()
	{
		m_Tag = 0;
	}
	//============================================
	inline int GetTag()
	{
		return m_Tag;
	}

	//position
	GFPhysVector3 m_CurrPosition;
	GFPhysVector3 m_LastPosition;
	GFPhysVector3 m_UnDeformedPos;

	//velocity
	GFPhysVector3 m_Velocity;
	
	//bool  m_FrameCollideTag;//
	
	int m_rtState;
	
	int m_Tag;

	//bool  m_bCollideRigid;

	//bool  m_bCollideSoft;

	//bool  m_bCanSelfCollision;

	//bool  m_bAttached;//是否被夹取，附着

	GFPhysVector3 m_Force0;
	GFPhysVector3 m_Force1;
	float m_LambdaBend;
	float m_LambdaStretch;
protected:
	float m_InvMass;
	
	float m_SolverInvMassScale;
	
	GFPhysMatrix3 m_ParticleOrient;

	
};

//thread segment vs thread segment collision
class TTCollidePair
{
public:
	TTCollidePair(MisMedicThreadRope * ropeA ,
				  MisMedicThreadRope * ropeB , 
				  int SegmentA,
				  int SegmentB);
	
	int GetCollideSegmentA()  const;
	int GetCollideSegmentB()  const;

	int m_SegmentA;
	int m_SegmentB;

	float m_CollideDist;
	float m_WeightA;//collide position is pos0*(1-weight0) + pos * weight
	float m_WeightB;
	MisMedicThreadRope * m_RopeA;
	MisMedicThreadRope * m_RopeB;

	GFPhysVector3 m_NormalOnB;
};

class ThreadRopeBendSection
{
public:
	ThreadRopeBendSection(int s , int e)
	{
		m_StartNodeIndex = s;
		m_EndNodeIndex = e;
		m_UseCustomBendFactor = false;
	}

	ThreadRopeBendSection(int s , int e , float stiff , int range)
	{
		m_StartNodeIndex = s;
		m_EndNodeIndex = e;
		m_CustomBendRange = range;
		m_CustomBendStiff = stiff;
		m_UseCustomBendFactor = true;
	}

	//only use when apply s.m bend force
	//this function is a tricky since we only need bend effect when use shape match force
	//so in every solve constraint recalculate rest length will preserver stretch state and only affect
	//bend!
	void  BuildSectionNodeRestPos(MisMedicThreadRope * ropeobject);
	int   m_StartNodeIndex;
	int   m_EndNodeIndex;
	bool  m_UseCustomBendFactor;
	int   m_CustomBendRange;
	float m_CustomBendStiff;
	GFPhysAlignedVectorObj<GFPhysVector3> m_NodeRestPos;
};

class ThreadFixNodeInfo
{
public:
	ThreadFixNodeInfo(int index , const GFPhysVector3 & pos)
	{
		m_NodeIndex = index;
		m_UndeformedPos = pos;
	}
	int m_NodeIndex;
	GFPhysVector3 m_UndeformedPos;
};

class RigidTransFormHistory
{
public:
	RigidTransFormHistory()
	{
		m_SimFrameID = -1;
	}
	GFPhysTransform m_LastTrans;
	GFPhysVector3 m_LinearVel;//current transform - last transform
	GFPhysVector3 m_AngluarVel;
	int m_SimFrameID;
};

class MyTriSegClosetResult : public GFPhysDCDInterface::Result
{
public:
	MyTriSegClosetResult()
	{
		m_Valid = false;
	}
	/*overridden*/
	void SetShapeIdentifiers(int partId0,int index0,int partId1,int index1)
	{}
	/*overridden*/
	void AddContactPoint(const GFPhysVector3& normalOnBInWorld,const GFPhysVector3& pointInWorld,Real depth)
	{
		m_Depth = depth;
		m_NormalOnB = normalOnBInWorld;
		m_PointOnB = pointInWorld;
		m_Valid = true;
	}
	bool  m_Valid;
	Real  m_Depth;
	GFPhysVector3 m_NormalOnB;
	GFPhysVector3 m_PointOnB;
};


class MisMedicThreadRope : public MisCustomSimObj , public GFPhysSoftBodyConstraint//currently only rend part
{
public:
	enum ThreadTopoType
	{
		TTT_NONE,
		TTT_LOOP,//线圈
		TTT_FIXONEEND,//单端固定线
		TTT_FREE
	};
	
	MisMedicThreadRope(Ogre::SceneManager * sceneMgr , MisNewTraining * ownertrain);
	
	~MisMedicThreadRope();

	void SetRigidForceRange(int range);

	void SetStretchStiffness(float);

	void SetBendingStiffness(float);

	void SetGravity(const GFPhysVector3 & gravity);
	
	virtual GFPhysVector3 CalcLoopForceFeedBack();

	virtual void Shrink(float percent);//only use in knotter

	void DestoryRope();
	
	bool CutThreadByTool(CTool * toolToCut);

	void AttachNodePointToFace(GFPhysSoftBodyFace * attachFace , float weights[3]);

	void DetachNodePointFromFace();

	virtual void BeginSimulateThreadPhysics(float dt);

	virtual void EndSimuolateThreadPhysics(float dt);

	bool GetThreadSegmentNode(ThreadNode & n0 , ThreadNode & n1 , int segIndex);

	bool GetThreadSegmentNodePos(GFPhysVector3 & n0 , GFPhysVector3 & n1 , int segIndex);

	virtual void SolveAdditionConstraint();

	virtual void UpdateFixedNodes();

	void  SetUnitLen(float);

	float GetUnitLen();

	float GetTotalLen(bool deformed);

	//float GetBindLen();

	ThreadTopoType GetThreadTopoType();

	int GetNumThreadNodes();

	int GetNumSegments();

	ThreadNode GetThreadNode(int NodeIndex);

	ThreadNode & GetThreadNodeRef(int NodeIndex);

	const std::vector<TFCollidePair> & GetCollidePairs();

	const GFPhysAlignedVectorObj<TRCollidePair> & GetCollidePairsWithRigid();

	const GFPhysAlignedVectorObj<TTCollidePair> & GetCollidePairsWithThread();

	bool GetApproximateLoopPlane(GFPhysVector3 & planePoint , GFPhysVector3 & planeNormal);

	//create function start
	/*@@ loop type thread use in knotter*/
	virtual void CreateLoopedThread(const GFPhysVector3 & circleCenter,
							const GFPhysVector3 & circelAxis,
							const GFPhysVector3 & StartVec,
							Ogre::SceneNode * AttachNode,
							ObjPhysCatogry threadCat);

	/*@@ one fix end type thread use in knotter*/
	void CreateOnePointFixThread(int segmentNum ,
								 const GFPhysVector3 & StartFixPoint ,
								 const GFPhysVector3 & circelAxis ,
								 const GFPhysVector3 & direction ,
								 Ogre::SceneNode * AttachNode,
								 ObjPhysCatogry threadCat);
	
	/*@@ total freed thread*/
	void CreateFreeThread(const GFPhysVector3 & StartFixPoint , const GFPhysVector3 & EndFixPoint , int segmetnCount,float masspernode);

	virtual void UpdateMesh();
	
	virtual void UpdateMeshByCustomedRendNodes(std::vector<Ogre::Vector3> & rendPoints);
	
	float GetCollideRadius()
	{
		return m_RopeCollideRadius;
	}
	
	void SetRendRadius(float rendradius)
	{
		m_RopeRendRadius = rendradius;
	}

	float GetRendRadius()
	{
		return m_RopeRendRadius;
	}

	void EnableSelfCollision()
	{
		m_EnableSelfCollision = true;
	}

	void DisableSelfCollision()
	{
		m_EnableSelfCollision = false;
	}
	GFPhysVector3& GetThreadNodePositionByIndex(int index);
	bool IsCollideWithOrgan(Ogre::Plane* plane);

	Ogre::Vector3 GetThreadDir();

	bool IsCutAfterBound() { return m_TopoType == TTT_FIXONEEND && m_IsCutAfterBound; }
	
	const GFPhysDBVTree & GetSegmentBVTree()
	{
		return m_SegmentBVTree;
	}

	GFPhysVector3 m_ForceFeed;

	bool  m_NeedRend;

	bool  m_bAttached;//是否被夹取，附着

	float m_RopeRSFriction;

	float m_Margin;

	float m_SolveMassScale;

	std::vector<int> m_SegmentState;

	bool m_RendSegementTag;

	bool m_UseBendForce;

	int   m_RigidForceRange;
	float m_RigidForceMagnitude;
	//std::vector<float> m_RFInvMass;// InvMassArray[SMINTERVAL];

	//GFPhysAlignedVectorObj<GFPhysVector3> m_RFCurrPos;//Array[SMINTERVAL];

	//GFPhysAlignedVectorObj<GFPhysVector3> m_RFUndeformPos;//Array[SMINTERVAL];

	//GFPhysAlignedVectorObj<GFPhysMatrix3> m_RFMomentMat;//momentMat[SMINTERVAL];

	//GFPhysAlignedVectorObj<GFPhysMatrix3> m_RFUndeformedMomentMat;//momentMat[SMINTERVAL];

	//std::vector<ThreadRopeBendSection> m_BendSection;

	float m_RopeBendStiffness;

	float m_DampingRate;

	int   m_SolveItorCount;
protected:
	float m_RopeFriction;


	MisNewTraining * m_ownertrain;

	MisMedicTubeShapeRendObject m_RendObject;

	bool m_UseCCD;
	
	bool m_EnableSelfCollision;//default false

	

	float m_RopeStrechStiffness;

	int  m_SingleItorNum;//this is firs time itor num
	int  m_TotalItorNum;//thread iterator num may large to solver num(typically 14 iterator num)
	int  m_TimesPerItor;
	//@@ solver function solver thread bend and stretch
	void SolveBend(ThreadNode & t0 , ThreadNode & t1 , ThreadNode & t2 , float Stiffness);
	GFPhysVector3 SolveStretch(ThreadNode & t0 , ThreadNode & t1 , float Stiffness , int interval);
	
	void SolveStretchX(ThreadNode & n0, ThreadNode & n1, float & lambda, float InvStiff, float damping, float dt, float RestLen);

	void SolveBendX(ThreadNode & va, ThreadNode & vb, ThreadNode & vc, float & lambda, float InvStiff, float damping, float solvehardness, float dt);

	//@@ solver function solve thread - soft body collision
	GFPhysVector3 SolveEECollide(const GFPhysVector3 & collideNormal , GFPhysSoftBodyNode * e1 , GFPhysSoftBodyNode * e2 , ThreadNode & e3 , ThreadNode & e4 , const TFCollidePair & cdPair);
	GFPhysVector3 SolveVTCollide(const GFPhysVector3 & collideNormal , GFPhysSoftBodyNode * n1 , GFPhysSoftBodyNode * n2 , GFPhysSoftBodyNode * n3 , ThreadNode & tn , const TFCollidePair & cdPair);

	GFPhysVector3 SolveEFCollide(const GFPhysVector3 & collideNormal , 
								 GFPhysSoftBodyNode * n1 , 
								 GFPhysSoftBodyNode * n2 ,
								 GFPhysSoftBodyNode * n3 , 
								 ThreadNode & t0 , 
								 ThreadNode & t1 ,
								 const TFCollidePair & cdPair);
	//@ solve thread attach face 
	void SolveAttachment(GFPhysSoftBodyFace * attachFace , float weights[3] , float stiffness);
	
	void SolveSoftThreadCollisions();
	
	void SolveRigidThreadCollisions(float dt);

	void SolveThreadThreadCollisions();

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	Ogre::Vector3 GetInterplotPointPos(Ogre::Vector3 P1 , Ogre::Vector3 P2 , Ogre::Vector3 P3 , Ogre::Vector3 P4 , float t , float tao = 0.5f);

	//unit length of the thread segment
	uint32 m_Catergory;
	GFPhysVector3 m_Gravity;
	float m_UnitLen;
	float m_RopeCollideRadius;
	float m_RopeRendRadius;
	std::vector<TFCollidePair> m_TFCollidePair;//thread soft face collide pair
	GFPhysAlignedVectorObj<TRCollidePair> m_TRCollidePair;//thread rigid collide pair
	GFPhysAlignedVectorObj<ThreadNode> m_ThreadNodes;//node information of the thread
	GFPhysAlignedVectorObj<TTCollidePair>  m_TTCollidepair;

	GFPhysAlignedVectorObj<ThreadFixNodeInfo> m_FixedNodsInFo;
	//std::map<GFPhysRigidBody* , RigidTransFormHistory> m_LastRigidTrans;

	ThreadTopoType   m_TopoType;//type of the rope
	ThreadNode       m_VirtualStickNode;//virtual node in tool
	Ogre::Vector3    m_LoopAxisLocal;
	GFPhysSoftBodyFace * m_AttachedFace;
	float m_AttachWeight[3];
	GFPhysDBVTree m_SegmentBVTree;


	//rend part start
	Ogre::SceneNode * m_ToolKernalNode;//挂在器械上的节点 仅当线圈模式有效

	bool m_IsCutAfterBound;

	//float * m_InvMassArray;
	//GFPhysVector3 * m_CurrPosArray;
	//GFPhysVector3 * m_UndeformPosArray;
	//int m_NumSMNode;

	int m_SimFrameID;
};

#endif