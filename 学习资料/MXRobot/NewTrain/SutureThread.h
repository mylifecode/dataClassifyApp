#pragma once

#include <Ogre.h>
#include "Dynamic/GoPhysDynamicLib.h"
#include "PhysicsWrapper.h"
#include "MisMedicTubeShapeRendObject.h"
#include "MisMedicOrganInterface.h"
#include "MisMedicThreadRope.h"
#include "MisCustomSimObj.h"
#include "ThreadCollision.h"

using namespace GoPhys;

class MisMedicOrgan_Ordinary;
class MisNewTraining;
class CTool;
class SutureThread;
const Real ThreadMass = 0.05f;
const Real rotMassPerSeg = 0.005f;

enum SutureThreadEleTag
{
	STET_CANCOLLIDERIGID = 1,
	STET_CANCOLLIDESOFT  = (1 << 1),
	STET_CANCOLLIDESELF  = (1 << 2),
	STET_INKNOT = (1 << 3),
	STET_ATTACHED = (1 << 4),
	STET_STICKINNEEDLE = (1 << 5),
};

class SutureRopeNode
{
public:
	SutureRopeNode(const GFPhysVector3 & position)
	{
		m_UnDeformedPos = m_CurrPosition = m_LastPosition = position;
        m_MaterialVector = GFPhysVector3(0.0f,0.0f,0.0f);
		m_InvMass = 1.0f;
		m_SolverInvMassScale = 1.0f;
		m_SolverInvMassScaleForSelfCollision = 1.0f;
		m_Velocity = GFPhysVector3(0,0,0);
		
		m_Tag = (STET_CANCOLLIDERIGID | STET_CANCOLLIDESOFT | STET_CANCOLLIDESELF);
        m_EnableCollide = true;
        m_StretchLen = 1.0f;

		m_ParticleOrient.SetIdentity();
        
        m_GlobalId = -1;
        m_bVisual = true;
		m_StretchDampingX = 0.01f;
		m_BendDampingX = 0.01f;
	}	
	//============================================
	SutureRopeNode()
	{
		m_InvMass = 1.0f;
		m_SolverInvMassScale = 1.0f;
		m_SolverInvMassScaleForSelfCollision = 1.0f;
		m_Velocity = GFPhysVector3(0,0,0);
		
		m_Tag = (STET_CANCOLLIDERIGID | STET_CANCOLLIDESOFT | STET_CANCOLLIDESELF);
		m_EnableCollide = true;
        m_StretchLen = 1.0f;

		m_ParticleOrient.SetIdentity();
        
        m_GlobalId = -1;
        m_bVisual = true;
		m_StretchDampingX = 0.01f;
		m_BendDampingX = 0.01f;
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
	inline float GetSolverInvMasScaleForSelfCollision()
	{
		return m_SolverInvMassScaleForSelfCollision;
	}
	//============================================
	inline void  SetSolverInvMassScaleForSelfCollision(float scalem)
	{
		m_SolverInvMassScaleForSelfCollision = scalem;
	}
	//============================================
	inline float GetSolverInvMassForSelfCollision()
	{
		return m_InvMass*m_SolverInvMassScaleForSelfCollision;
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
	inline void MarkAsStickInNeedle(bool set = true)
	{
		if (set)
			m_Tag |= STET_STICKINNEEDLE;
		else
			m_Tag &= (~STET_STICKINNEEDLE);
	}
	//============================================
	inline void MarkAsAttached(bool set = true)
	{
		if(set)
		   m_Tag |= STET_ATTACHED;
		else
		   m_Tag &= (~STET_ATTACHED);
	}
	//============================================
	inline void SetCanCollideRigid(bool cancollide)
	{
		if(cancollide)
		   m_Tag |= STET_CANCOLLIDERIGID;
		else
		   m_Tag &= (~STET_CANCOLLIDERIGID);
	}
	//============================================
	inline void SetCanCollideSoft(bool cancollide)
	{
		if(cancollide)
		   m_Tag |= STET_CANCOLLIDESOFT;
		else
		   m_Tag &= (~STET_CANCOLLIDESOFT);
	}
	//============================================
	inline void SetCanCollideSelf(bool cancollide)
	{
		if (cancollide)
			m_Tag |= STET_CANCOLLIDESELF;
		else
			m_Tag &= (~STET_CANCOLLIDESELF);
	}
	//============================================
	inline void SetInKnotLoop(bool inloop)
	{
		if (inloop)
			m_Tag |= STET_INKNOT;
		else
			m_Tag &= (~STET_INKNOT);
	}
	//============================================
	inline bool IsStickInNeedle()
	{
		return (m_Tag & STET_STICKINNEEDLE);
	}
	//============================================
	inline bool IsAttached()
	{
		return (m_Tag & STET_ATTACHED);
	}
	//============================================
	inline bool GetCanCollideSoft()
	{
		return (m_Tag & STET_CANCOLLIDESOFT) && m_EnableCollide;
	}
	//============================================
	inline bool GetCanCollideRigid()
	{
		return (m_Tag & STET_CANCOLLIDERIGID) && m_EnableCollide;
	}
	//=========================================================
	inline bool GetCanCollideSelf() const
	{
		return (m_Tag & STET_CANCOLLIDESELF);
	}
	//=========================================================
	inline bool GetInKnotLoop() const
	{
		return (m_Tag & STET_INKNOT);
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
    //============================================
    inline void Disappear()
    {
        m_bVisual = false;

        m_EnableCollide = false;
        SetCanCollideRigid(false);
        SetCanCollideSoft(false);
        SetCanCollideSelf(false);
    }
	//position
	GFPhysVector3 m_CurrPosition;
	GFPhysVector3 m_LastPosition;
	GFPhysVector3 m_UnDeformedPos;

    GFPhysVector3 m_MaterialVector;

	//velocity
	GFPhysVector3 m_Velocity;
		
	Real m_LambdaStretch;
	Real m_LambdaBend;
	Real m_BendSolveHardness;

	GFPhysVector3 m_EdgeTan[2];
	
	Real m_TanLambda[2];

	bool m_EnableCollide;

    Real m_StretchLen;

    bool m_bVisual;

    int m_GlobalId;
	Real m_BendDampingX;
	Real m_StretchDampingX;
protected:

	int m_Tag;
	Real m_InvMass;	
	Real m_SolverInvMassScale;
	Real m_SolverInvMassScaleForSelfCollision;
	
	GFPhysMatrix3 m_ParticleOrient;
};

//thread segment vs thread segment collision
class STSTCollidePair
{
public:
	STSTCollidePair(SutureThread * ropeA ,
				  SutureThread * ropeB , 
				  int SegmentA,
				  int SegmentB);
	
	int GetCollideSegmentA()const  { return m_SegmentA; };
	int GetCollideSegmentB()const  { return m_SegmentB; };

	int m_SegmentA;
	int m_SegmentB;

	Real m_CollideDist;
	Real m_WeightA;//collide position is pos0*(1-weight0) + pos * weight
	Real m_WeightB;
	SutureThread * m_RopeA;
	SutureThread * m_RopeB;

	GFPhysVector3 m_NormalOnB;

    void CorrectPosition();//
};
class SutureKnot;
class KnotInSutureRope;
class SutureNeedle;

class SutureThread : public MisCustomSimObj, public GFPhysSoftBodyConstraint//currently only rend part
{
public:
    SutureThread(Ogre::SceneManager * sceneMgr, MisNewTraining * ownertrain);

    ~SutureThread();

    static int globalid;

    void SetRigidForceRange(int range);

    void SetStretchStiffness(float);

    void SetBendingStiffness(float);

    void SetGravity(const GFPhysVector3 & gravity);

    void DestoryRope();

    bool CutThreadByTool(CTool * toolToCut);

    virtual void BeginSimulateSutureThreadPhysics(float dt);

    virtual void EndSimulateThreadPhysics(float dt);

    bool GetThreadSegmentNode(SutureRopeNode & n0, SutureRopeNode & n1, int segIndex);

    bool GetThreadSegmentTangent(GFPhysVector3 & tanget, int segIndex);//First derivative

    bool GetThreadSegmentTangentLerped(GFPhysVector3 & tanget, int segIndex, float weight);

    bool GetThreadSegmentTangentOrder1(GFPhysVector3 & tanget, int segIndex);//First derivative
    bool GetThreadSegmentTangentOrder2(GFPhysVector3 & tanget2, int segIndex);//Second derivative
    bool GetThreadSegmentCurvature(Real & curvature, int segIndex);//curvature in 3d

    bool CurrLen2RelativePos(Real currlen, int& index, Real& weight);
    bool RelativePos2CurrLen(int index, Real weight, Real& currlen);

    bool GetThreadSegmentNodePos(GFPhysVector3 & n0, GFPhysVector3 & n1, int segIndex);

    void SolveAdditionConstraint();

    void UpdateFixedNodes();

    void  SetRestLen(Real len){ m_Rest_Length = len; }

    Real GetRestLen(){return m_Rest_Length;}

	Real GetTotalLen(bool deformed);

	Real GetCustomLen(bool deformed, int i, int j);

    int GetNumThreadNodes();

    int GetNumSegments();

    SutureRopeNode GetThreadNode(int NodeIndex);
    SutureRopeNode & GetThreadNodeRef(int NodeIndex);

    SutureRopeNode & GetThreadNodeGlobalRef(int NodeGlobalIndex, int& index);

    inline Real GetRopeAnchorIndexMin(){ return m_RopeAnchorIndexMin; }
    inline Real GetRopeAnchorIndexMax(){ return m_RopeAnchorIndexMax; }

    inline void SetRopeAnchorIndexMin(Real Pos){ m_RopeAnchorIndexMin = Pos; }
    inline void SetRopeAnchorIndexMax(Real Pos){ m_RopeAnchorIndexMax = Pos; }

    inline Real GetKnotIndexMin(){ return m_KnotIndexMin; }
    inline Real GetKnotIndexMax(){ return m_KnotIndexMax; }

    inline void SetKnotIndexMin(Real NodeIndex){ m_KnotIndexMin = NodeIndex; }
    inline void SetKnotIndexMax(Real NodeIndex){ m_KnotIndexMax = NodeIndex; }

    void InsertOneNode(int i, Real wi);

    const std::vector<TFCollidePair> & GetCollidePairs();

    const GFPhysAlignedVectorObj<TRCollidePair> & GetCollidePairsWithRigid();

    const GFPhysAlignedVectorObj<STSTCollidePair> & GetCollidePairsWithThread();

    bool GetApproximateLoopPlane(GFPhysVector3 & planePoint, GFPhysVector3 & planeNormal);

    /*@@ total free thread*/
    void CreateFreeThread(const GFPhysVector3 & StartFixPoint, const GFPhysVector3 & EndFixPoint, int segmentCount, float masspernode);

    virtual void UpdateMesh();

    void BuildKnotPoint(const KnotInSutureRope& knot, std::vector<Ogre::Vector3> & rendpoints1, std::vector<Ogre::Vector3> & rendpoints2);

    //void UpdateAnchor();

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

    void DisableCollideSelfFromClampToTail();
    GFPhysVector3& GetThreadNodePositionByIndex(int index);
    //bool IsCollideWithOrgan(Ogre::Plane* plane);

    Ogre::Vector3 GetThreadDir();

    const GFPhysDBVTree & GetSegmentBVTree()
    {
        return m_SegmentBVTree;
    }

    void SetRopeAnchorIndex(std::vector<Real> nodeindex);
    std::vector<Real> GetRopeAnchorIndex();
    bool computeMaterialFrame(const GFPhysVector3& p0, const GFPhysVector3& p1, const GFPhysVector3& p2, GFPhysMatrix3& frame);
    void LockRopeNode(int index);
    void UnLockRopeNode(int index);
    int GetGlobalId()
    {
        return globalid++;
    }
    bool m_islock;    

    GFPhysVectorObj<int> m_ClampSegIndexVector;

	bool	m_move;

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

	float m_RopeBendStiffness;

	float m_DampingRate;
	int   m_SolveItorCount;

    //unit length of the thread segment
    float m_Rest_Length;// this length is valid before knot slack。
    SutureKnot * m_KnotsInThread;
    SutureNeedle * m_NeedleAttchedThread;

    int   m_InitNodesNum;//绳子创建时的节点数
    //int   m_IncreaseNum;//打第一个结时增加的节点数

    SutureRopeNode m_NullNode;
    GFPhysAlignedVectorObj<GFPhysVector3> m_Impluse;
	Real m_TotalRopeAnchorFriction;
protected:
	float m_RopeFriction;

    Real m_RopeAnchorIndexMax;
    Real m_RopeAnchorIndexMin;
    GFPhysVectorObj<Real> m_RopeAnchorIndexVec;
    Real m_KnotIndexMax;
    Real m_KnotIndexMin;

	MisNewTraining * m_ownertrain;

    MisMedicTubeShapeRendObject m_RendObject;
	MisMedicTubeShapeRendObject m_RendObject1;
	MisMedicTubeShapeRendObject m_RendObject2;
    //std::vector<MisMedicTubeShapeRendObject> m_RendObjects;

	bool m_UseCCD;
	
	bool m_EnableSelfCollision;//default false

	

	float m_RopeStrechStiffness;

	int  m_SingleItorNum;//this is firs time itor num
	int  m_TotalItorNum;//thread iterator num may large to solver num(typically 14 iterator num)
	int  m_TimesPerItor;

    void UpdateMaterialVector();
	
	void SolveFixOrientation(SutureRopeNode &  va, SutureRopeNode & vb, GFPhysVector3 & orientation, float Stiffness);//orientation为线肉锚点中面的切向(单位化)
	void SolveFixOrientation2(SutureRopeNode &  va, SutureRopeNode & vb, GFPhysVector3 & orientation, float Stiffness);//orientation为线肉锚点中面的切向(单位化)

	//@@ solver function solver thread bend and stretch
	void SolveBend(SutureRopeNode & t0 , SutureRopeNode & t1 , SutureRopeNode & t2 , float Stiffness);
	GFPhysVector3 SolveStretch(SutureRopeNode & t0 , SutureRopeNode & t1 , float Stiffness , int interval);
	
    void SolveStretchX(SutureRopeNode & n0, SutureRopeNode & n1, float & lambda, float InvStiff, float damping, float dt, float RestLen, GFPhysVector3& n0_Update, GFPhysVector3& n1_Update);

	void SolveBendX(SutureRopeNode & va, SutureRopeNode & vb, SutureRopeNode & vc, float & lambda, float InvStiff, float damping, float solvehardness, float dt);

	//@@ solver function solve thread - soft body collision
	GFPhysVector3 SolveEECollide(const GFPhysVector3 & collideNormal , GFPhysSoftBodyNode * e1 , GFPhysSoftBodyNode * e2 , SutureRopeNode & e3 , SutureRopeNode & e4 , const TFCollidePair & cdPair);
	GFPhysVector3 SolveVTCollide(const GFPhysVector3 & collideNormal , GFPhysSoftBodyNode * n1 , GFPhysSoftBodyNode * n2 , GFPhysSoftBodyNode * n3 , SutureRopeNode & tn , const TFCollidePair & cdPair);

	GFPhysVector3 SolveEFCollide(const GFPhysVector3 & collideNormal , 
								 GFPhysSoftBodyNode * n1 , 
								 GFPhysSoftBodyNode * n2 ,
								 GFPhysSoftBodyNode * n3 , 
								 SutureRopeNode & t0 , 
								 SutureRopeNode & t1 ,
								 const TFCollidePair & cdPair);
	//@ solve thread attach face 
	void SolveAttachment(GFPhysSoftBodyFace * attachFace , float weights[3] , float stiffness);
	
	void SolveSoftThreadCollisions();
	
	void SolveRigidThreadCollisions(float dt);

	void SolveThreadThreadCollisions();

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	Ogre::Vector3 GetInterplotPointPos(Ogre::Vector3 P1 , Ogre::Vector3 P2 , Ogre::Vector3 P3 , Ogre::Vector3 P4 , float t , float tao = 0.5f);
	void SlideKnot();
	uint32 m_Catergory;
	GFPhysVector3 m_Gravity;
	
	float m_RopeCollideRadius;
	float m_RopeRendRadius;
	std::vector<TFCollidePair>                  m_TFCollidePair;//thread soft face collide pair
	GFPhysAlignedVectorObj<TRCollidePair>       m_TRCollidePair;//thread rigid collide pair
	GFPhysAlignedVectorObj<SutureRopeNode>      m_SutureThreadNodes;//node information of the thread    
	GFPhysAlignedVectorObj<STSTCollidePair>     m_TTCollidepair;//thread vs thread

	GFPhysAlignedVectorObj<ThreadFixNodeInfo>   m_FixedNodsInFo;
	//std::map<GFPhysRigidBody* , RigidTransFormHistory> m_LastRigidTrans;
	
	SutureRopeNode       m_VirtualStickNode;//virtual node in tool
	Ogre::Vector3    m_LoopAxisLocal;
	GFPhysSoftBodyFace * m_AttachedFace;
	float m_AttachWeight[3];
	GFPhysDBVTree m_SegmentBVTree;

	bool m_IsCutAfterBound;

	float * m_InvMassArray;
	GFPhysVector3 * m_CurrPosArray;
	GFPhysVector3 * m_UndeformPosArray;
	int m_NumSMNode;

	int m_SimFrameID;
    GFPhysVector3       m_KnotImpluse[2];
};
