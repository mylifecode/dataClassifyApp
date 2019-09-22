#pragma once

#include <Ogre.h>
#include "Dynamic/GoPhysDynamicLib.h"
#include "Dynamic/PhysicBody/GoPhysSoftTube.h"
#include "PhysicsWrapper.h"
#include "MisMedicTubeShapeRendObject.h"
#include "MisNewTraining.h"
#include "SutureThreadV2Collision.h"
using namespace GoPhys;

class SutureThreadV2;
class SutureNeedleV2;
class SutureKnotV2;
class KnotInSutureRopeV2;
class knotpbdV2;

enum AnchorTypeV2
{
	STATE_NULL,
	STATE_IN,
	STATE_OUT
};
class GFPhysFaceRopeAnchorV2
{
public:
	class GFPhysFRAnchorConstructInFo
	{
	public:
		GFPhysFRAnchorConstructInFo(GFPhysSoftBodyFace * face, float faceWeight[3] , int seg, float segweight, AnchorTypeV2 type)
		{
			m_face = face;
			m_FaceWeight[0] = faceWeight[0];
			m_FaceWeight[1] = faceWeight[1];
			m_FaceWeight[2] = faceWeight[2];

			m_IndexSeg = seg;
			m_WeightSeg = segweight;
			m_type     = type;
		}

		GFPhysSoftBodyFace * m_face;
		float m_FaceWeight[3];

		int   m_IndexSeg;
		float m_WeightSeg;
		AnchorTypeV2 m_type;
	};

	GFPhysFaceRopeAnchorV2(AnchorTypeV2 type, GFPhysSoftBodyFace * face , float faceWeight[3] , SutureThreadV2 * thread, int segIndex, float segWeight);
    
	~GFPhysFaceRopeAnchorV2();
	
	void SetFriction(float friction);
	
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);
    
	void SolveConstraint(Real Stiffniss, Real TimeStep);

	GFPhysSoftBodyFace * GetFace(){return m_Face;}
	
	GFPhysVector3 GetAnchorOnFaceRestPos();

	GFPhysVector3 GetAnchorOnFaceCurrPos();

	void SetAnchorSegAndWeight(int segIndex, Real weight);

	bool IsSlippedOut();

	int  GetSegIndex()
	{ 
		return m_SegIndex;
	}

	float  GetSegWeight()
	{
		return m_WeightSeg;
	}

	float  GetSegPos() const 
	{ 
		return m_SegIndex + m_WeightSeg;
	}
	
	GFPhysVector3 GetAccumPressure() const;
	
	Real					m_weights[3];
   
    SutureThreadV2 *		m_thread;
    
    Real                    m_AccumFriction;
	AnchorTypeV2			m_type;

	Real                    m_lengthEnlarge;
	bool					m_bHaveFixNor;
	GFPhysVector3           m_FixFaceNormal;

	bool                    m_IsSlipDisabled;
	bool                    m_IsDragedTwoEnd;
	float                   m_ToSlipDist;
	float                   m_NbSegStretch;
	bool                    m_AlignedToFace;
private:

	int						m_SegIndex;
	
	float					m_WeightSeg;
	
	GFPhysSoftBodyFace*		m_Face;
	Real					m_Friction;
    GFPhysVector3           m_AccumPressure;
//////////////////////////////////////////////////////////////////////////
	GFPhysVector3			m_Last_Pos;
    GFPhysVector3           m_Tangent;
	int                     m_IterorCount;
};


class SutureThreadNodeV2 :public GFPhysSoftTubeNode
{
public:
	SutureThreadNodeV2();
	SutureThreadNodeV2(const GFPhysVector3 & restPos);
	virtual ~SutureThreadNodeV2();
public:
	//Real m_invmassSclae;
	int m_GlobalId;
	bool m_bVisual;
	//void SetSolverInvMassScaleForSelfCollision(Real scale)
	//{
		//m_invmassSclae = scale;
	//}
	inline void Disappear()
	{
		m_bVisual = false;
	}
	//Real GetSolverInvMassForSelfCollision()
	//{
		//return m_invmassSclae * m_InvMass;
	//}
};


class SutureThreadV2 : public GFPhysSoftTube, public GFPhysPositionConstraint, public MisCustomSimObj
{
public:
	Real m_dis;
	SutureThreadV2(Ogre::SceneManager * sceneMgr, MisNewTraining * ownertrain, Ogre::String matRendObj = "RopeThreadDefaultMat");

	~SutureThreadV2();
	static int globalid;
	int GetGlobalId()
	{
		return globalid++;
	}
	void GenerateCollidePairs(Real dt);

	void FilterInverseCollidePairs();

	void PrepareSolveConstraint(Real Stiffness, Real TimeStep);

	void SolveConstraint(Real Stiffness, Real TimeStep);

	virtual SutureThreadNodeV2 * CreateTubeNode(const GFPhysVector3 & restPos);

	virtual void DestroyTubeNode(SutureThreadNodeV2 * node);

	//void SolveFix();
	void SolveFixOrientation(GFPhysSoftTubeNode* va, GFPhysSoftTubeNode* vb, GFPhysVector3 &rotCentr ,GFPhysVector3& orientation, Real Stiffness);
	//================================================================================
	GFPhysVector3 GetNodeBackTangent(int NodeIndex)
	{
		if (NodeIndex == m_TubeNodes.size() - 1)
			return (m_TubeNodes[m_TubeNodes.size() - 1]->m_CurrPosition - m_TubeNodes[m_TubeNodes.size() - 2]->m_CurrPosition).Normalized();
		else
			return (m_TubeNodes[NodeIndex + 1]->m_CurrPosition - m_TubeNodes[NodeIndex]->m_CurrPosition).Normalized();
	}
	//================================================================================
	GFPhysVector3 GetNodeAvgTangent(int NodeIndex)
	{
		if (NodeIndex == m_TubeNodes.size() - 1)
			return (m_TubeNodes[m_TubeNodes.size() - 1]->m_CurrPosition - m_TubeNodes[m_TubeNodes.size() - 2]->m_CurrPosition).Normalized();
		else if (NodeIndex == 0)
			return (m_TubeNodes[1]->m_CurrPosition - m_TubeNodes[0]->m_CurrPosition).Normalized();
		else
			return (m_TubeNodes[NodeIndex + 1]->m_CurrPosition - m_TubeNodes[NodeIndex - 1]->m_CurrPosition).Normalized();
	}

	void SetSegmentBeClamped(int segIndex);

	void SetSegmentBeReleased(int segIndex);

	void DampingVelocity(Real dt, Real linearDamp, const GFPhysVector3 & angularDamp);

	void DisableSegmentCollision(int startSeg, int endSeg);

	//@ set stretch stiffness for this SutureThreadV2
	void SetStretchShearStiffness(float);

	//@ set bend stiffness for this SutureThreadV2
	void SetBendingTwistStiffness(float, float);

	//@ set Gravity for SutureThreadV2
	void SetGravity(const GFPhysVector3 & gravity);

	/*@@ */
	void Create(const GFPhysVector3 & StartFixPoint,
		const GFPhysVector3 & EndFixPoint,
		int segmetnCount,
		float masspernode,
		float rotMassPerSeg,
		bool rotHead);

	void CreateFromPoints(GFPhysAlignedVectorObj<GFPhysVector3> & NodePos,
		float masspernode,
		float rotMassPerSeg);

	//@ destroy this object
	void Destory();

	bool computeMaterialFrame(const GFPhysVector3& p0, const GFPhysVector3& p1, const GFPhysVector3& p2, GFPhysMatrix3& frame);

	void SetNodeAsFix(int NodeIndex);
	void ReleaseNodeAsFix(int NodeIndex);


	void  AddCollisionPoint(int nodeId,
		int faceId,
		const GFPhysVector3 & triPos0,
		const GFPhysVector3 & triPos1,
		const GFPhysVector3 & triPos2,
		float triWeights[3],
		float edgeWeight,
		const GFPhysVector3 & collideNormal
		);
	//void  SetNodeAsFixed(int nodeIndex);

	//float GetUnitLen();

	float GetThreadRestLen();

	float GetThreadCurrLen();

	//void  GetSegIndexWeightByDist(float distfromhead, int& index, Real& weight);

	//float GetDistBySegIndexWeight(int index, float weight);

	int   GetNumThreadNodes();

	int   GetNumSegments();

	float GetSegmentCurrLen(int segindex);

	GFPhysSoftTubeNode GetThreadNode(int NodeIndex);
	SutureThreadNodeV2 & GetThreadNodeRefReal(int NodeIndex);
	GFPhysSoftTubeNode & GetThreadNodeRef(int NodeIndex);
	SutureThreadNodeV2 & GetThreadNodeGlobalRef(int NodeGlobalIndex, int& index);
	
	GFPhysSoftTubeSegment & GetTubeWireSegment(int SegIndex);
	bool GetThreadSegmentNodePos(GFPhysVector3 & n0, GFPhysVector3 & n1, int segIndex);
	const GFPhysAlignedVectorObj<STVRGCollidePair> & GetCollidePairsWithRigid();
	virtual void UpdateMesh();


	//float GetCollideRadius()
	//{
		//return m_RopeCollideRadius;
	//}
	float GetCollideMargin()
	{
		return m_RopeCollideMargin;
	}
	void SetCollideRaiuds(float radius);

	void SetRendRadius(float rendradius)
	{
		m_RopeRendRadius = rendradius;
	}
	void GetFaceRopeAnchors(GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> & ropeanchors)
	{
		for (size_t c = 0; c < m_FaceRopeAnchors.size(); c++)
		{
			ropeanchors.push_back(m_FaceRopeAnchors[c]);
		}
	}
	void SetFaceRopeAnchors(GFPhysVectorObj<GFPhysFaceRopeAnchorV2::GFPhysFRAnchorConstructInFo> & ropeanchors);
	float GetRendRadius()
	{
		return m_RopeRendRadius;
	}
	Real GetRestLen(){ return m_Rest_Length; }
	Real GetCustomLen(bool deformed, int i, int j);
	const GFPhysDBVTree & GetSegmentBVTree()
	{
		return m_SegmentBVTree;
	}
	void SlipInFaceRopeAnchor(AnchorTypeV2 type, GFPhysSoftBodyFace* face, Real * weights, Real offset);
	void RemoveSlipOuttedAnchor();//线尾删除出线约束

	void StandarlizeRopeAnchors();
	//@ 
	virtual void BeginSimulatePhysics(float dt);

	void DisableCollideSelfFromClampToTail();
	void BuildKnotPoint(const KnotInSutureRopeV2& knot, std::vector<Ogre::Vector3> & rendpoints1, std::vector<Ogre::Vector3> & rendpoints2);


	virtual void EndSimulatePhysics(float dt);

	inline void SetRopeAnchorIndexMin(Real Pos){ m_RopeAnchorIndexMin = Pos; }
	inline void SetRopeAnchorIndexMax(Real Pos){ m_RopeAnchorIndexMax = Pos; }
	inline Real GetRopeAnchorIndexMin(){ return m_RopeAnchorIndexMin; }
	inline Real GetRopeAnchorIndexMax(){ return m_RopeAnchorIndexMax; }

	void SetRopeAnchorIndex(std::vector<Real> nodeindex);
	GFPhysVectorObj<Real> GetRopeAnchorIndex();
	//bool GetThreadSegmentTangentLerped(GFPhysVector3 & tanget, int segIndex, float weight);

	//bool GetThreadSegmentTangentOrder1(GFPhysVector3 & tanget, int segIndex);

	Real CalcSlackRatio(Real lengthcorrect, int minseg, int maxseg);

	GFPhysVector3 m_ForceFeed;
	//SutureKnotV2 * m_KnotsInThread;
	GFPhysVectorObj<int> m_ClampSegIndexVector;

	bool  m_NeedRend;

	float m_RopeRSFriction;

	bool  m_UseBendForce;

	float m_TwistValue;
	//int  m_NumsolveItor;
	//virtual void PrepareSolve(Real Stiffness, Real TimeStep);

	//virtual void SolvePhysics(Real Stiffness, Real TimeStep);
	
	bool RestLen2RelativePos(Real currlen, int& index, Real& weight);
	
	bool RelativePos2RestLen(int index, Real weight, Real& currlen);

	float GetRestDistBetweenSegments(int seg0,float weight0 ,int seg1,float weight1);
	
	float GetStretchedDistBetweenSegments(int seg0, float weight0, int seg1, float weight1);

	float m_NodeDamping;
	GFPhysVector3 m_AngularDamping;
	Real m_TotalRopeAnchorFriction;

	bool m_islock;

	GFPhysAlignedVectorObj<STVSFCollidePair>		m_TFCollidePair;//thread soft face collide pair
	GFPhysAlignedVectorObj<STVRGCollidePair>		m_TRCollidePair;//thread rigid collide pair	
	GFPhysAlignedVectorObj<STVSTCollidePair>		m_TTCollidepair;//thread vs thread
	GFPhysVectorObj<GFPhysFaceRopeAnchorV2*>		m_FaceRopeAnchors;
	std::vector<int> m_SegmentsBeClamped;

	SutureKnotV2 *		m_KnotsInThread;
	SutureNeedleV2 *	m_NeedleAttchedThread;
	SutureThreadNodeV2	m_NullNode;
protected:

	GFPhysFaceRopeAnchorV2 * CreateFaceRopeAnchorInternal(GFPhysFaceRopeAnchorV2::GFPhysFRAnchorConstructInFo & cs);

	void DestoryFaceRopeAnchorInternal(GFPhysFaceRopeAnchorV2 * );
	//@@ solver function solver thread bend and stretch
	//void SolveBend(TubeWireNode & t0 , TubeWireNode & t1 , TubeWireNode & t2 , float Stiffness);

	//void SolveBendX(TubeWireNode & t0, TubeWireNode & t1, TubeWireNode & t2, float & lambda, float InvStiffness, float damping, float dt);

	//GFPhysVector3 SolveStretch(TubeWireNode & t0 , TubeWireNode & t1 , float Stiffness , int interval);

	//void SolveStretchX(TubeWireNode & t0, TubeWireNode & t1, float & lambda, float Stiffness, float damping, float dt , float restLen);

	void SolveCollisions(Real dt,bool useFrict);

	Ogre::Vector3 GetInterplotPointPos(Ogre::Vector3 P1, Ogre::Vector3 P2, Ogre::Vector3 P3, Ogre::Vector3 P4, float t, float tao = 0.5f);
	void SolveSoftThreadCollisions();
	void SolveRigidThreadCollisions(Real dt);
	void SolveThreadThreadCollisions();
	//GFPhysVector3 SolveEECollide(const GFPhysVector3 & collideNormal, GFPhysSoftBodyNode * e1, GFPhysSoftBodyNode * e2, SutureThreadNodeV2 & e3, SutureThreadNodeV2 & e4, const STVSFCollidePair & cdPair);
	//GFPhysVector3 SolveVTCollide(const GFPhysVector3 & collideNormal, GFPhysSoftBodyNode * n0, GFPhysSoftBodyNode * n1, GFPhysSoftBodyNode * n2, SutureThreadNodeV2 & tn, const STVSFCollidePair & cdPair);
	//GFPhysVector3 SolveEFCollide(const GFPhysVector3 & collideNormal, GFPhysSoftBodyNode * v0, GFPhysSoftBodyNode * v1, GFPhysSoftBodyNode * v2, GFPhysSoftTubeNode & t0, GFPhysSoftTubeNode & t1, const STVSFCollidePair & cdPair);
	void SlideKnot();

	void SlideKnotPair(float slipDist, KnotInSutureRopeV2 & knot, int pair, int inValidSegStart, int inValidSegEnd, bool & reachLimit);
	//WireArteryCascadedCollideDetector * m_CasColDetector;

	GFPhysDBVTree m_SegmentBVTree;
	GFPhysVectorObj<Real> m_RopeAnchorIndexVec;
	GFPhysVector3 m_Gravity;
	Real m_RopeAnchorIndexMax;
	Real m_RopeAnchorIndexMin;
	//float m_UnitLen;
	//float m_RopeCollideRadius;
	float m_RopeCollideMargin;

	float m_RopeRendRadius;
	float m_RopeFriction;

	MisNewTraining *			m_ownertrain;
	MisMedicTubeShapeRendObject m_RendObject;
	MisMedicTubeShapeRendObject m_RendObject1;
	MisMedicTubeShapeRendObject m_RendObject2;

	bool						m_UseCCD;
	int							m_SolveItorCount;
	GFPhysVector3				m_KnotImpluse[2];
	Real						m_Rest_Length;
};