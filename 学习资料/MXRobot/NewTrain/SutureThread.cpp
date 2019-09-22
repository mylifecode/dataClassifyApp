#include "stdafx.h"
#include "SutureThread.h"
#include "MisNewTraining.h"
#include "Dynamic\Constraint\GoPhysSoftBodyDistConstraint.h"
#include "Dynamic\Constraint\GoPhysPBDPresetConstraint.h"
#include "math\GoPhysTransformUtil.h"
#include "Math\GoPhysSIMDMath.h"
#include "Math\GoPhysMathUtil.h"
#include "CustomCollision.h"
#include "MisMedicOrganOrdinary.h"
#include "PhysicsWrapper.h"
#include "TrainingMgr.h"
#include "IObjDefine.h"
#include "MXOgreGraphic.h"
#include "Instruments/Tool.h"
#include "MXEventsDump.h"
#include "MisMedicSutureKnot.h"
#include <QDebug>

#define  USECCDINSELFCOLLISION 1
#define  KNOTRENDER 1
#define  USECCDINRIGIDCOLLISION 1

const Real TESTTHRESHOLD = 0.3333333f;
const int NUM = 3;
const Real SHRINK1 = 5.0f;
const Real SHRINK2 = 0.5f;

const Real SOLVEDAMPINGCOFF = 50.0f;
//Real RScollideMassScale = 0.35f;

//================================================================================================================
STSTCollidePair::STSTCollidePair(SutureThread * ropeA,
	SutureThread * ropeB,
	int SegmentA,
	int SegmentB)
{
	m_RopeA = ropeA;
	m_RopeB = ropeB;
	m_SegmentA = SegmentA;
	m_SegmentB = SegmentB;
}
//================================================================================================================

void STSTCollidePair::CorrectPosition()
{
	GFPhysVector3 S0, S1, L0, L1;

	SutureRopeNode ns0, ns1, nl0, nl1;

	m_RopeA->GetThreadSegmentNode(ns0, ns1, m_SegmentA);
	m_RopeB->GetThreadSegmentNode(nl0, nl1, m_SegmentB);

	m_RopeA->GetThreadSegmentNodePos(S0, S1, m_SegmentA);
	m_RopeB->GetThreadSegmentNodePos(L0, L1, m_SegmentB);

	Real w0 = ns0.GetSolverInvMass();
	Real w1 = ns1.GetSolverInvMass();
	Real w2 = nl0.GetSolverInvMass();
	Real w3 = nl1.GetSolverInvMass();

	GFPhysVector3 PS = S0 * (1 - m_WeightA) + S1 * m_WeightA;
	GFPhysVector3 PL = L0 * (1 - m_WeightB) + L1 * m_WeightB;

	GFPhysVector3 Diff = (PS - PL);

	Real Length = Diff.Length();

	Real correctStiff = 0.85f;

	if (Length > FLT_EPSILON)
	{
		Diff /= Length;

		Real wS0 = (1 - m_WeightA);
		Real wS1 = m_WeightA;

		Real wL0 = (1 - m_WeightB);
		Real wL1 = m_WeightB;


		GFPhysVector3 gradS0 = Diff * wS0;
		GFPhysVector3 gradS1 = Diff * wS1;

		GFPhysVector3 gradL0 = -Diff * wL0;
		GFPhysVector3 gradL1 = -Diff * wL1;

		//weighted by inverse mass 's sum of gradient
		Real sumgrad = gradS0.Length2()*w0 + gradS1.Length2()*w1 + gradL0.Length2()*w2 + gradL1.Length2()*w3;//wS0*wS0 + wS1*wS1 + wL0*wL0 + wL1*wL1;

		Real scale = correctStiff * (-Length) / sumgrad;

		GFPhysVector3 deltaS0 = scale*gradS0*w0;
		GFPhysVector3 deltaS1 = scale*gradS1*w1;

		GFPhysVector3 deltaL0 = scale*gradL0*w2;
		GFPhysVector3 deltaL1 = scale*gradL1*w3;

		m_RopeA->GetThreadNodeRef(m_SegmentA).m_CurrPosition = S0 + deltaS0;
		m_RopeA->GetThreadNodeRef(m_SegmentA + 1).m_CurrPosition = S1 + deltaS1;
		m_RopeB->GetThreadNodeRef(m_SegmentB).m_CurrPosition = L0 + deltaL0;
		m_RopeB->GetThreadNodeRef(m_SegmentB + 1).m_CurrPosition = L1 + deltaL1;
	}
}

class SutureThreadCompoundCallBack : public GFPhysNodeOverlapCallback, public GFPhysDCDInterface::Result
{
public:

    SutureThreadCompoundCallBack(Real margin,
        GFPhysRigidBody * rb,
        GFPhysCollideShape * childshape,
        GFPhysTransform & rbTrans,
        Real collideradius,
        SutureThread * rope,
        GFPhysAlignedVectorObj<TRCollidePair> & collidepair)
        : m_rb(rb),
        m_Rope(rope),
        m_collideRadius(collideradius),
        m_CollidePairs(collidepair)
    {
        m_collideshape = childshape;
        m_threadMarin = margin;//0.02f;
        m_rbTransform = rbTrans;
    }

    virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
    {
        m_SegIndex = (int)UserData;

        SutureRopeNode n0, n1;

        bool succeed = m_Rope->GetThreadSegmentNode(n0, n1, m_SegIndex);

        if (n0.IsAttached() || n1.IsAttached())
        {
            return;
        }

        if (succeed)
        {
            m_SegPos0 = n0.m_LastPosition;
			m_SegPos1 = n1.m_LastPosition;

			m_SegNodeInvM[0] = n0.GetInvMass();
			m_SegNodeInvM[1] = n1.GetInvMass();

            GFPhysLineSegmentShape linesegShape(m_SegPos0, m_SegPos1);
            linesegShape.SetMargin(m_collideRadius + m_threadMarin);

            GFPhysConvexCDShape * convexShape = (GFPhysConvexCDShape*)m_collideshape;

            m_convexMargin = convexShape->GetMargin();

            GFPhysConvexCDShape * min0 = convexShape;
            GFPhysConvexCDShape * min1 = &linesegShape;

            GFPhysGJKCollideDetector::ClosestPointInput input;
            GFPhysVSimplexCloseCalculator VSimplexSolver;
            GFPhysGjkEpaPDCalculor PDSolver;
            GFPhysGJKCollideDetector gjkPairDetector(min0, min1, &VSimplexSolver, &PDSolver);

            gjkPairDetector.SetMinkowskiA(min0);
            gjkPairDetector.SetMinkowskiB(min1);
            input.m_maximumDistanceSquared = min0->GetMargin() + min1->GetMargin();

            input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;

            input.m_transformA = m_rbTransform;//m_rb->GetWorldTransform();
            input.m_transformB.SetIdentity();

            gjkPairDetector.GetClosestPoints(false, input, *this);
        }
    }

    void SetShapeIdentifiers(int partId0, int index0, int partId1, int index1)
    {}
    void AddContactPoint(const GFPhysVector3& normalOnBInWorld, const GFPhysVector3& pointInWorld, Real depth)
    {
        GFPhysVector3 pointOnSegWithOutMargin = pointInWorld - normalOnBInWorld*(m_collideRadius + m_threadMarin);

        Real t0 = (pointOnSegWithOutMargin - m_SegPos0).Dot(m_SegPos1 - m_SegPos0);

        Real t1 = (m_SegPos1 - m_SegPos0).Dot(m_SegPos1 - m_SegPos0);

        if (t1 > FLT_EPSILON)
        {
            Real depthWithOutMargin = depth + (m_collideRadius + m_threadMarin) + m_convexMargin;

            Real segweight = t0 / t1;

            GPClamp(segweight, 0.0f, 1.0f);

            GFPhysVector3 pointOnConvexWithoutMargin = pointOnSegWithOutMargin + normalOnBInWorld * depthWithOutMargin;

			TRCollidePair trPair(m_rb, m_rbTransform, pointOnConvexWithoutMargin, -normalOnBInWorld, segweight, depthWithOutMargin, m_SegIndex);
#if(1)
			GFPhysVector3 segPos[2];
		
			segPos[0] = m_SegPos0;
			segPos[1] = m_SegPos1;

		
			trPair.BuildAdditional(segPos, pointOnConvexWithoutMargin,
				                   m_SegNodeInvM,
								   -normalOnBInWorld,
				                   m_Rope->GetCollideRadius(),
								   m_rb->GetWorldTransform());
#endif

			m_CollidePairs.push_back(trPair);
        }
    }

    virtual ~SutureThreadCompoundCallBack()
    {
    }
    int m_SegIndex;

    GFPhysVector3 m_SegPos0;

    GFPhysVector3 m_SegPos1;

	Real m_SegNodeInvM[2];
    GFPhysRigidBody * m_rb;

    GFPhysCollideShape * m_collideshape;

    SutureThread * m_Rope;
    Real m_collideRadius;

    Real m_threadMarin;

    Real m_convexMargin;

    GFPhysAlignedVectorObj<TRCollidePair>  & m_CollidePairs;
    GFPhysTransform m_rbTransform;
};
//================================================================================================================
class SutureThreadConvexCallBack : public GFPhysNodeOverlapCallback, public GFPhysDCDInterface::Result
{
public:
	SutureThreadConvexCallBack(Real margin,
		GFPhysRigidBody * rb,
		GFPhysTransform & rbTrans,
		Real dt,
		Real collideradius,
		SutureThread * rope,
		GFPhysAlignedVectorObj<TRCollidePair> & collidepair
		) : m_rb(rb),
		m_Rope(rope),
		m_collideRadius(collideradius),
		m_CollidePairs(collidepair)
	{
		m_threadMarin = margin;//0.02f;
		m_rbTransform = rbTrans;
		m_dt = dt;
	}

	virtual ~SutureThreadConvexCallBack()
	{}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
	{
		m_SegIndex = (int)UserData;

		SutureRopeNode n0, n1;

		bool succeed = m_Rope->GetThreadSegmentNode(n0, n1, m_SegIndex);

        if (n0.IsAttached() || n1.IsAttached())
        {
            return;
        }
		if (succeed)
		{
			m_SegPos0 = n0.m_LastPosition;
			m_SegPos1 = n1.m_LastPosition;

			m_SegNodeInvM[0] = n0.GetInvMass();
			m_SegNodeInvM[1] = n1.GetInvMass();
			GFPhysConvexCDShape * convexShape = (GFPhysConvexCDShape*)m_rb->GetCollisionShape();

			m_convexMargin = convexShape->GetMargin();

#if USECCDINRIGIDCOLLISION
						
			GFPhysTransform BodyPredictTrans;

			GFPhysTransformUtil::IntegrateTransform(m_rbTransform,
					m_rb->m_linearVelocity,
					m_rb->m_angularVelocity,
					m_dt,
					BodyPredictTrans);

			GFPhysVector3  pointInConvexWolrd;
			GFPhysVector3  normalInConvexWorld;
			GFPhysVector3  pointInEdgeWorld;
			GFPhysTransform collideTrans;
			Real depth;
			Real collideTime = GFPhysCABasedCCD::GetConvexDeformEdgeContactTime(
					convexShape,
					m_rbTransform,
					BodyPredictTrans, /*BodyPredictTrans,m_rbTransform*/
					n0.m_LastPosition,
					n1.m_LastPosition,
					n0.m_CurrPosition,
					n1.m_CurrPosition,
					m_Rope->m_Margin + m_Rope->GetCollideRadius(),
					0.001f,
					pointInConvexWolrd,
					normalInConvexWorld,
					pointInEdgeWorld, 
					collideTrans,
					depth);

			if (collideTime >= 0.0f && collideTime <= 1.0f)
			{
					Real depthWithOutMargin = m_convexMargin + (pointInEdgeWorld - pointInConvexWolrd).Dot(normalInConvexWorld);

					GFPhysVector3 SegPos0 = n0.m_LastPosition * (1.0f - collideTime) + n0.m_CurrPosition * collideTime;
					GFPhysVector3 SegPos1 = n1.m_LastPosition * (1.0f - collideTime) + n1.m_CurrPosition * collideTime;

					Real t0 = (pointInEdgeWorld - SegPos0).Dot(SegPos1 - SegPos0);

					Real t1 = (SegPos1 - SegPos0).Dot(SegPos1 - SegPos0);

					if (t1 > GP_EPSILON)
					{
						Real segweight = t0 / t1;

						GPClamp(segweight, 0.0f, 1.0f);
	
						GFPhysVector3  pointInConvexWolrdNoMargin = pointInConvexWolrd - normalInConvexWorld * m_convexMargin;

						pointInConvexWolrdNoMargin = m_rbTransform * (collideTrans.Inverse() * pointInConvexWolrdNoMargin);//redundant optimize me ! since in tr pair we also need transform to local point!

						TRCollidePair trpair(m_rb, m_rbTransform, pointInConvexWolrdNoMargin, normalInConvexWorld, segweight, depthWithOutMargin, m_SegIndex);
				#if(1)
						GFPhysVector3 segPos[2];
						Real segNodeInvMass[2];

						segPos[0] = SegPos0;
						segPos[1] = SegPos1;

						segNodeInvMass[0] = n0.GetInvMass();
						segNodeInvMass[1] = n1.GetInvMass();

						trpair.BuildAdditional(segPos, 
							                   pointInConvexWolrd - normalInConvexWorld * m_convexMargin ,
											   segNodeInvMass,
											   normalInConvexWorld,
											   m_Rope->GetCollideRadius(),
											   collideTrans);
				#endif
						m_CollidePairs.push_back(trpair);
					}
			}
#else
            
			GFPhysLineSegmentShape linesegShape(m_SegPos0, m_SegPos1);
			linesegShape.SetMargin(m_collideRadius + m_threadMarin);

            GFPhysConvexCDShape * min0 = convexShape;
            GFPhysConvexCDShape * min1 = &linesegShape;

            GFPhysGJKCollideDetector::ClosestPointInput input;
            GFPhysVSimplexCloseCalculator VSimplexSolver;
            GFPhysGjkEpaPDCalculor PDSolver;
            GFPhysGJKCollideDetector gjkPairDetector(min0, min1, &VSimplexSolver, &PDSolver);

            gjkPairDetector.SetMinkowskiA(min0);
            gjkPairDetector.SetMinkowskiB(min1);
            input.m_maximumDistanceSquared = min0->GetMargin() + min1->GetMargin();

            input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;

            input.m_transformA = m_rbTransform;//m_rb->GetWorldTransform();
            input.m_transformB.SetIdentity();

            gjkPairDetector.GetClosestPoints(false, input, *this);
            
#endif
		}
	}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode)
	{}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB)
	{}


	void SetShapeIdentifiers(int partId0, int index0, int partId1, int index1)
	{

	}

	void AddContactPoint(const GFPhysVector3& normalOnBInWorld, const GFPhysVector3& pointInWorld, Real depth)
	{
		GFPhysVector3 pointOnSegWithOutMargin = pointInWorld - normalOnBInWorld*(m_collideRadius + m_threadMarin);

		Real t0 = (pointOnSegWithOutMargin - m_SegPos0).Dot(m_SegPos1 - m_SegPos0);

		Real t1 = (m_SegPos1 - m_SegPos0).Dot(m_SegPos1 - m_SegPos0);

		if (t1 > FLT_EPSILON)
		{
			Real depthWithOutMargin = depth + (m_collideRadius + m_threadMarin) + m_convexMargin;

			Real segweight = t0 / t1;

			GPClamp(segweight, 0.0f, 1.0f);

			GFPhysVector3 pointOnConvexWithoutMargin = pointOnSegWithOutMargin + normalOnBInWorld * depthWithOutMargin;

			TRCollidePair trPair(m_rb, m_rbTransform, pointOnConvexWithoutMargin, -normalOnBInWorld, segweight, depthWithOutMargin, m_SegIndex);

#if(1)
			GFPhysVector3 segPos[2];
			segPos[0] = m_SegPos0;
			segPos[1] = m_SegPos1;


			trPair.BuildAdditional(segPos, pointOnConvexWithoutMargin,
				                   m_SegNodeInvM,
				                   -normalOnBInWorld,
				                   m_Rope->GetCollideRadius(),
				                   m_rbTransform);
#endif

			m_CollidePairs.push_back(trPair);
		}
	}
	int m_SegIndex;

	GFPhysVector3 m_SegPos0;

	GFPhysVector3 m_SegPos1;

	Real m_SegNodeInvM[2];

	GFPhysRigidBody * m_rb;
	SutureThread * m_Rope;
	Real m_collideRadius;

	Real m_threadMarin;

	Real m_convexMargin;

	GFPhysAlignedVectorObj<TRCollidePair>  & m_CollidePairs;
	GFPhysTransform m_rbTransform;
	Real m_dt;
};
//
//================================================================================================================
class SutureThreadThreadCallBack : public GFPhysNodeOverlapCallback
{
public:
	SutureThreadThreadCallBack(Real margin,
		Real collideradius,
		SutureThread * ropeA,
		SutureThread * ropeB,
		GFPhysAlignedVectorObj<STSTCollidePair> & collidepair) : m_RopeA(ropeA), m_RopeB(ropeB), m_threadRadius(collideradius), m_CollidePairs(collidepair)
	{
		m_threadMarin = margin;//0.02f;
		m_IsSelftCollision = (m_RopeA == m_RopeB ? true : false);
	}

	virtual ~SutureThreadThreadCallBack()
	{}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
	{}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode)
	{}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB)
	{
		int SegIndexA = (int)(dynNodeA->m_UserData);
		int SegIndexB = (int)(dynNodeB->m_UserData);

		if (SegIndexA < SegIndexB)
		{
			if (m_IsSelftCollision && abs(SegIndexA - SegIndexB) <= 2)
				return;

			const SutureRopeNode & A0 = m_RopeA->GetThreadNodeRef(SegIndexA);
			const SutureRopeNode & A1 = m_RopeA->GetThreadNodeRef(SegIndexA + 1);

			const SutureRopeNode & B0 = m_RopeB->GetThreadNodeRef(SegIndexB);
			const SutureRopeNode & B1 = m_RopeB->GetThreadNodeRef(SegIndexB + 1);

			if (A0.GetCanCollideSelf() && A1.GetCanCollideSelf() && B0.GetCanCollideSelf() && B1.GetCanCollideSelf())
			{
#if(USECCDINSELFCOLLISION)

				GFPhysVector3 cpoint0, cpoint1, collideNorm;//on second edge
				bool exceedMaxItor;

				Real sumRadius = (m_threadRadius * 0.25f + m_threadRadius)*2.0f;
				GFPhysVector3 VA0 = A0.m_CurrPosition - A0.m_LastPosition;
				GFPhysVector3 VA1 = A1.m_CurrPosition - A1.m_LastPosition;
				GFPhysVector3 VB0 = B0.m_CurrPosition - B0.m_LastPosition;
				GFPhysVector3 VB1 = B1.m_CurrPosition - B1.m_LastPosition;

				Real collideTime = GFPhysCABasedCCD::GetEdgesWithRadiusCollideTime(A0.m_LastPosition, A1.m_LastPosition,
					B0.m_LastPosition, B1.m_LastPosition,

					VA0, VA1,
					VB0, VB1,

					1.0f,
					sumRadius,
					sumRadius * 0.025f,

					cpoint0, cpoint1, collideNorm, exceedMaxItor);

				if (collideTime >= 0 && (!exceedMaxItor))//collide
				{
					GFPhysVector3 CA0 = A0.m_LastPosition + VA0*collideTime;
					GFPhysVector3 CA1 = A1.m_LastPosition + VA1*collideTime;
					GFPhysVector3 CB0 = B0.m_LastPosition + VB0*collideTime;
					GFPhysVector3 CB1 = B1.m_LastPosition + VB1*collideTime;

					STSTCollidePair ttpair(m_RopeA, m_RopeB, SegIndexA, SegIndexB);
					ttpair.m_WeightA = GPClamped((cpoint0 - CA0).Dot(CA1 - CA0) / (CA1 - CA0).Dot(CA1 - CA0), 0.0f, 1.0f);
					ttpair.m_WeightB = GPClamped((cpoint1 - CB0).Dot(CB1 - CB0) / (CB1 - CB0).Dot(CB1 - CB0), 0.0f, 1.0f);
					ttpair.m_CollideDist = (cpoint0 - cpoint1).Length();
					ttpair.m_NormalOnB = collideNorm;
					m_CollidePairs.push_back(ttpair);
				}

#else
				Real s, t;
				GFPhysVector3 cA, cB;
				Real dist = GPClosestPtSegmentSegment(A0.m_CurrPosition, A1.m_CurrPosition, B0.m_CurrPosition, B1.m_CurrPosition, s, t, cA, cB);

				if (dist < (m_threadMarin + m_threadRadius)*2.0f)//collision
				{
					STSTCollidePair ttpair(m_RopeA, m_RopeB, SegIndexA, SegIndexB);
					ttpair.m_WeightA = s;
					ttpair.m_WeightB = t;
					ttpair.m_CollideDist = dist;
					ttpair.m_NormalOnB = (cA - cB).Normalized();
					m_CollidePairs.push_back(ttpair);
				}
#endif
			}



		}
	}

	void SetShapeIdentifiers(int partId0, int index0, int partId1, int index1)
	{

	}


	SutureThread * m_RopeB;

	SutureThread * m_RopeA;

	bool  m_IsSelftCollision;

	Real m_threadRadius;

	Real m_threadMarin;

	GFPhysAlignedVectorObj<STSTCollidePair>  & m_CollidePairs;
};

//@collision need optimize
//@also check whether SAT method have a lot  jitter effect in contact face(the SAT and closet method should behavier as the same)
class SutureThreadSegSoftFaceCallback : public GFPhysNodeOverlapCallback
{
public:
	SutureThreadSegSoftFaceCallback(Real Margin,
		GFPhysSoftBody * sb,
		Real collideradius,
		SutureThread * rope,
		std::vector<TFCollidePair> & paircd,
		bool useCCD) : m_sb(sb),
		m_collideRadius(collideradius + Margin),
		m_Rope(rope),
		m_CollidePairs(paircd)

	{
		m_UseCCD = useCCD;
	}
	virtual ~SutureThreadSegSoftFaceCallback()
	{}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
	{}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode)
	{}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB)
	{
		GFPhysSoftBodyFace * softFace = (GFPhysSoftBodyFace*)dynNodeA->m_UserData;
		int SegIndex = (int)dynNodeB->m_UserData;

		GFPhysVector3 SoftFaceVerts[3];
		GFPhysVector3 RopeMovePath[3];

		SoftFaceVerts[0] = softFace->m_Nodes[0]->m_CurrPosition;
		SoftFaceVerts[1] = softFace->m_Nodes[1]->m_CurrPosition;
		SoftFaceVerts[2] = softFace->m_Nodes[2]->m_CurrPosition;

		SutureRopeNode n0, n1;

		bool succedc = m_Rope->GetThreadSegmentNode(n0, n1, SegIndex);

		if (m_UseCCD)
		{
			GFPhysVector3 edgeVerts[2];

			GFPhysVector3 triVelocity[3];

			GFPhysVector3 edgeVelocity[2];

			edgeVerts[0] = n0.m_LastPosition;

			edgeVerts[1] = n1.m_LastPosition;

			triVelocity[0] = softFace->m_Nodes[0]->m_Velocity;

			triVelocity[1] = softFace->m_Nodes[1]->m_Velocity; 
			
			triVelocity[2] = softFace->m_Nodes[2]->m_Velocity; //GFPhysVector3(0, 0, 0);//temp static mesh

			edgeVelocity[0] = n0.m_CurrPosition - n0.m_LastPosition;

			edgeVelocity[1] = n1.m_CurrPosition - n1.m_LastPosition;

			Real sumRadius = m_collideRadius;//0.25 is margin

			GFPhysVector3 cp_Tri, cp_Edge, collideNorm;
			bool exceedMaxItor;
			Real collideTime = GFPhysCABasedCCD::GetFaceEdgeWithRadiusCollideTime(SoftFaceVerts, edgeVerts,
				triVelocity, edgeVelocity,
				1.0f, sumRadius, sumRadius*0.02f,
				cp_Tri, cp_Edge, collideNorm, exceedMaxItor);

			if (collideTime >= 0 && (!exceedMaxItor))
			{
				Real FaceWeights[3];

				Real EdgeWeight;

				GFPhysVector3 e0 = edgeVerts[0] + edgeVelocity[0] * collideTime;
				GFPhysVector3 e1 = edgeVerts[1] + edgeVelocity[1] * collideTime;

				GFPhysVector3 t0 = SoftFaceVerts[0] + triVelocity[0] * collideTime;
				GFPhysVector3 t1 = SoftFaceVerts[1] + triVelocity[1] * collideTime;
				GFPhysVector3 t2 = SoftFaceVerts[2] + triVelocity[2] * collideTime;

				CalcBaryCentric(t0, t1, t2, cp_Tri, FaceWeights[0], FaceWeights[1], FaceWeights[2]);

				Real lenSqr = (e1 - e0).Dot(e1 - e0);

				if (lenSqr > 0.0001f)
					EdgeWeight = (cp_Edge - e0).Dot(e1 - e0) / lenSqr;
				else
					EdgeWeight = 0.0f;

				GPClamp(EdgeWeight, 0.0f, 1.0f);
				GPClamp(FaceWeights[0], 0.0f, 1.0f);
				GPClamp(FaceWeights[1], 0.0f, 1.0f);
				GPClamp(FaceWeights[2], 0.0f, 1.0f);

				GFPhysVector3 faceNormal = (t1 - t0).Cross(t2 - t0);
				faceNormal.Normalize();

				TFCollidePair collidePair(m_sb, softFace);
				collidePair.m_FaceWeihts[0] = FaceWeights[0];
				collidePair.m_FaceWeihts[1] = FaceWeights[1];
				collidePair.m_FaceWeihts[2] = FaceWeights[2];
				collidePair.m_ThreadWeigths[0] = 1 - EdgeWeight;
				collidePair.m_ThreadWeigths[1] = EdgeWeight;

				int collideNormalSign = (collideNorm.Dot(faceNormal) > 0 ? 1 : -1);

				collidePair.SetEF_Collision(SegIndex, GPVec3ToOgre(collideNorm), GPVec3ToOgre(faceNormal));
				if (collideNormalSign == 1)
					m_CollidePairs.push_back(collidePair);//for edge - edge collision eliminate normal opposite to face
				//else
				//	m_InvCollidePairs.push_back(collidePair);

			}
		}
		else
		{
			GFPhysTransform identyTrans;

			identyTrans.SetIdentity();

			GFPhysVector3 closetPointFace;

			GFPhysVector3 closetPointSeg;

			GFPhysGJKCollideDetector::ClosestPointInput input;

			GFPhysVSimplexCloseCalculator VSimplexSolver;

			GFPhysGjkEpaPDCalculor PDSolver;

			GFPhysTriangleShape triShape(SoftFaceVerts[0], SoftFaceVerts[1], SoftFaceVerts[2]);
			triShape.SetMargin(0.0f);

			GFPhysLineSegmentShape segShape(n0.m_CurrPosition, n1.m_CurrPosition);
			segShape.SetMargin(m_collideRadius);

			GFPhysGJKCollideDetector gjkPairDetector(&segShape, &triShape, &VSimplexSolver, &PDSolver);

			gjkPairDetector.SetMinkowskiA(&segShape);

			gjkPairDetector.SetMinkowskiB(&triShape);

			input.m_maximumDistanceSquared = segShape.GetMargin() + triShape.GetMargin() + 0.04f;

			input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;

			input.m_transformA.SetIdentity();

			input.m_transformB.SetIdentity();

			MyTriSegClosetResult cdResult;

			gjkPairDetector.GetClosestPoints(false, input, cdResult);

			if (cdResult.m_Valid && cdResult.m_Depth < 0)
			{
				GFPhysVector3 pointOnFaceNoMargin = cdResult.m_PointOnB - cdResult.m_NormalOnB * triShape.GetMargin();

				GFPhysVector3 pointOnSegNoMargin = cdResult.m_PointOnB + cdResult.m_NormalOnB * (cdResult.m_Depth + segShape.GetMargin());

				Real faceWeights[3];

				CalcBaryCentric(SoftFaceVerts[0],
					SoftFaceVerts[1],
					SoftFaceVerts[2],
					pointOnFaceNoMargin,
					faceWeights[0],
					faceWeights[1],
					faceWeights[2]);

				GPClamp(faceWeights[0], 0.0f, 1.0f);
				GPClamp(faceWeights[1], 0.0f, 1.0f);
				GPClamp(faceWeights[2], 0.0f, 1.0f);

				Real sum = faceWeights[0] + faceWeights[1] + faceWeights[2];
				if (sum >FLT_EPSILON)
				{
					faceWeights[0] /= sum;
					faceWeights[1] /= sum;
					faceWeights[2] /= sum;
				}

				//extract thread collision point weight
				Real ThreadWeights[2];
				GFPhysVector3 dn = n1.m_CurrPosition - n0.m_CurrPosition;
				Real dnLen2 = dn.Dot(dn);
				if (dnLen2 > FLT_EPSILON)
				{
					ThreadWeights[1] = (pointOnSegNoMargin - n0.m_CurrPosition).Dot(dn) / dnLen2;
				}
				else
				{
					ThreadWeights[1] = 0;
				}
				GPClamp(ThreadWeights[1], 0.0f, 1.0f);

				ThreadWeights[0] = 1 - ThreadWeights[1];

				//
				GFPhysVector3 faceNormal = (SoftFaceVerts[1] - SoftFaceVerts[0]).Cross(SoftFaceVerts[2] - SoftFaceVerts[0]);
				faceNormal.Normalize();

				TFCollidePair collidePair(m_sb, softFace);
				collidePair.m_FaceWeihts[0] = faceWeights[0];
				collidePair.m_FaceWeihts[1] = faceWeights[1];
				collidePair.m_FaceWeihts[2] = faceWeights[2];
				collidePair.m_ThreadWeigths[0] = ThreadWeights[0];
				collidePair.m_ThreadWeigths[1] = ThreadWeights[1];

				int collideNormalSign = (cdResult.m_NormalOnB.Dot(faceNormal) > 0 ? 1 : -1);

				collidePair.SetEF_Collision(SegIndex, GPVec3ToOgre(cdResult.m_NormalOnB), GPVec3ToOgre(faceNormal));
				//if (collideNormalSign == 1)
					m_CollidePairs.push_back(collidePair);//for edge - edge collision eliminate normal opposite to face
				//else
					//m_InvCollidePairs.push_back(collidePair);
			}
			return;
#if(0)
			Real s, t;
			GFPhysVector3 closetPtThread[5];
			GFPhysVector3 closetPtFace[5];

			closetPtFace[0] = ClosestPtPointTriangle(n0.m_CurrPosition, SoftFaceVerts[0], SoftFaceVerts[1], SoftFaceVerts[2]);
			closetPtThread[0] = n0.m_CurrPosition;

			closetPtFace[1] = ClosestPtPointTriangle(n1.m_CurrPosition, SoftFaceVerts[0], SoftFaceVerts[1], SoftFaceVerts[2]);
			closetPtThread[1] = n1.m_CurrPosition;

			GPClosestPtSegmentSegment(SoftFaceVerts[0],
				SoftFaceVerts[1],
				n0.m_CurrPosition,
				n1.m_CurrPosition,
				s, t,
				closetPtFace[2], closetPtThread[2]);

			GPClosestPtSegmentSegment(SoftFaceVerts[1],
				SoftFaceVerts[2],
				n0.m_CurrPosition,
				n1.m_CurrPosition,
				s, t,
				closetPtFace[3], closetPtThread[3]);

			GPClosestPtSegmentSegment(SoftFaceVerts[2],
				SoftFaceVerts[0],
				n0.m_CurrPosition,
				n1.m_CurrPosition,
				s, t,
				closetPtFace[4], closetPtThread[4]);

			Real intersectWeight;
			GFPhysVector3 intersectpt;
			Real triangleWeight[3];
			bool intersectTri = LineIntersectTriangle(SoftFaceVerts[0],
				SoftFaceVerts[1],
				SoftFaceVerts[2],
				n0.m_CurrPosition,
				n1.m_CurrPosition,
				intersectWeight,
				intersectpt, triangleWeight);

			GFPhysVector3 faceNormal = (SoftFaceVerts[1] - SoftFaceVerts[0]).Cross(SoftFaceVerts[2] - SoftFaceVerts[0]);
			faceNormal.Normalize();

			if (intersectTri == true && intersectWeight > 0 && intersectWeight < 1)
			{
				return;
				//TFCollidePair collidePair(softFace);
				//collidePair.SEtVF_Collision(SegIndex);
				//m_CollidePairs.push_back(collidePair);

				//collidePair.SEtVF_Collision(SegIndex+1);
				//m_CollidePairs.push_back(collidePair);
				//return;
			}

			Real minDist = FLT_MAX;

			int minIndex = -1;

			for (int c = 0; c < 5; c++)
			{
				Real dist = (closetPtThread[c] - closetPtFace[c]).Length();

				if (dist < minDist)
				{
					minIndex = c;
					minDist = dist;
				}
			}

			if (minDist < m_collideRadius)
			{
				TFCollidePair::TFCollideType collidetype;

				int e1, e2, e3, e4;
				switch (minIndex)
				{
				case 0:
					collidetype = TFCollidePair::TFCD_VF;
					e1 = SegIndex;
					break;

				case 1:
					collidetype = TFCollidePair::TFCD_VF;
					e1 = SegIndex + 1;
					break;

				case 2:
					collidetype = TFCollidePair::TFCD_EE;
					e1 = 0;
					e2 = 1;
					break;

				case 3:
					collidetype = TFCollidePair::TFCD_EE;
					e1 = 1;
					e2 = 2;
					break;

				case 4:
					collidetype = TFCollidePair::TFCD_EE;
					e1 = 2;
					e2 = 0;
					break;
				}

				GFPhysVector3 collideNorm = (closetPtThread[minIndex] - closetPtFace[minIndex]).Normalized();

				//extract face weights and thread weights;
				Real faceWeights[3];
				Real ThreadWeights[2];
				CalcBaryCentric(SoftFaceVerts[0],
					SoftFaceVerts[1],
					SoftFaceVerts[2],
					closetPtFace[minIndex],
					faceWeights[0],
					faceWeights[1],
					faceWeights[2]);
				GPClamp(faceWeights[0], 0.0f, 1.0f);
				GPClamp(faceWeights[1], 0.0f, 1.0f);
				GPClamp(faceWeights[2], 0.0f, 1.0f);

				Real sum = faceWeights[0] + faceWeights[1] + faceWeights[2];
				if (sum >FLT_EPSILON)
				{
					faceWeights[0] /= sum;
					faceWeights[1] /= sum;
					faceWeights[2] /= sum;
				}

				//extract thread collision point weight
				GFPhysVector3 dn = n1.m_CurrPosition - n0.m_CurrPosition;
				Real dnLen2 = dn.Dot(dn);
				if (dnLen2 > FLT_EPSILON)
				{
					ThreadWeights[1] = (closetPtThread[minIndex] - n0.m_CurrPosition).Dot(dn) / dnLen2;
				}
				else
				{
					ThreadWeights[1] = 0;
				}
				GPClamp(ThreadWeights[1], 0.0f, 1.0f);

				ThreadWeights[0] = 1 - ThreadWeights[1];

				//
				TFCollidePair collidePair(m_sb, softFace);

				int collideNormalSign = (collideNorm.Dot(faceNormal) > 0 ? 1 : -1);

				if (collidetype == TFCollidePair::TFCD_VF)//thread vertex - soft body face collide
				{
					collidePair.SetVF_Collision(SegIndex, GPVec3ToOgre(collideNorm), GPVec3ToOgre(faceNormal));
					if (collideNormalSign == 1)
						m_CollidePairs.push_back(collidePair);
					else
						m_InvCollidePairs.push_back(collidePair);//for vertex - face collision add normal opposite to face collision to vector for further check

					collidePair.SetVF_Collision(SegIndex + 1, GPVec3ToOgre(collideNorm), GPVec3ToOgre(faceNormal));
					if (collideNormalSign == 1)
						m_CollidePairs.push_back(collidePair);
					else
						m_InvCollidePairs.push_back(collidePair);//for vertex - face collision add normal opposite to face collision to vector for further check
				}

				else if (collidetype == TFCollidePair::TFCD_EE)//thread segment - soft body face edge collide
				{
					e3 = SegIndex;
					e4 = SegIndex + 1;
					GFPhysVector3 EENorm = (SoftFaceVerts[e1], SoftFaceVerts[e2]).Cross(n0.m_CurrPosition - n1.m_CurrPosition).Normalized();
					if (EENorm.Dot(collideNorm) < 0)
						std::swap(e1, e2);
					collidePair.SetEE_Collision(e1, e2, e3, e4, GPVec3ToOgre(collideNorm), GPVec3ToOgre(faceNormal));
					if (collideNormalSign == 1)
						m_CollidePairs.push_back(collidePair);//for edge - edge collision eliminate normal opposite to face
					else
						m_InvCollidePairs.push_back(collidePair);
				}
			}
#endif
		}
	}
	GFPhysSoftBody * m_sb;
	SutureThread * m_Rope;
	Real m_collideRadius;

	std::vector<TFCollidePair> & m_CollidePairs;
	std::vector<TFCollidePair> m_InvCollidePairs;
	bool m_UseCCD;
};
int SutureThread::globalid = 100;
//===================================================================================================
SutureThread::SutureThread(Ogre::SceneManager * sceneMgr, MisNewTraining * ownertrain)
	: /*m_ToolKernalNode(0),*/ m_AttachedFace(0), m_ownertrain(ownertrain), m_IsCutAfterBound(false)
{
	static int s_RopeId = 0;
	s_RopeId++;    
	Ogre::String strSutureThreadName = "SutureThreadObject" + Ogre::StringConverter::toString(s_RopeId);
	m_RendObject.CreateRendPart(strSutureThreadName, sceneMgr);

	s_RopeId++;
	strSutureThreadName = "SutureThreadObject" + Ogre::StringConverter::toString(s_RopeId);
	m_RendObject1.CreateRendPart(strSutureThreadName, MXOgre_SCENEMANAGER);

	s_RopeId++;
	strSutureThreadName = "SutureThreadObject" + Ogre::StringConverter::toString(s_RopeId);
	m_RendObject2.CreateRendPart(strSutureThreadName, MXOgre_SCENEMANAGER);

    //m_RendObjects.push_back(m_RendObject);

	m_Rest_Length = 0.18f;//unit length in rope
	m_RopeCollideRadius = 0.05f;//rope physics radius
	m_RopeRendRadius = 0.03f;//rope rend radius
	m_Margin = 0.06f;//collision margin
	m_RopeFriction = 0.6f;//rope vs soft collision coefficients
	m_RopeRSFriction = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.05f);//rope vs rigid coefficients
	m_UseCCD = true;

	m_Gravity = GFPhysVector3(0, 0, -6.0f);

	m_DampingRate = 1.0f;

	m_TotalItorNum = 14;

	m_TimesPerItor = m_TotalItorNum / GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverItorNum;

	if (m_TimesPerItor < 1)
		m_TimesPerItor = 1;

	m_SingleItorNum = m_TotalItorNum - m_TimesPerItor*GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverItorNum;

	if (m_SingleItorNum < 0)
		m_SingleItorNum = 0;

	m_RopeBendStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.85);//0.85

	m_RopeStrechStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(1.0f);

	m_SolveMassScale = 1.0f;

	m_Catergory = OPC_THREADUNIVERSAL;

	m_EnableSelfCollision = true;

	m_bAttached = false;

	m_RendSegementTag = false;

	m_UseBendForce = true;

	m_NeedRend = true;

	m_InvMassArray = 0;

	m_CurrPosArray = 0;

	m_UndeformPosArray = 0;

	m_NumSMNode = 0;

	m_RigidForceRange = 4;
	m_RigidForceMagnitude = 0.7f;

	//m_RFInvMass.resize(m_RigidForceRange);
	//m_RFCurrPos.resize(m_RigidForceRange);
	//m_RFUndeformPos.resize(m_RigidForceRange);
	//m_RFMomentMat.resize(m_RigidForceRange);
	//m_RFUndeformedMomentMat.resize(m_RigidForceRange);
	m_SimFrameID = 0;

	m_RopeAnchorIndexMin = INT_MAX;
	m_RopeAnchorIndexMax = INT_MIN;

	m_KnotIndexMin = INT_MIN;
	m_KnotIndexMax = INT_MAX;

	m_KnotsInThread = 0;
    m_RopeAnchorIndexVec.clear();
    m_ClampSegIndexVector.clear();
    m_islock = false;    
}
//===================================================================================================
SutureThread::~SutureThread()
{
	DestoryRope();
}
//===================================================================================================
void SutureThread::SetRigidForceRange(int range)
{
	m_RigidForceRange = range;

	//m_RFInvMass.resize(m_RigidForceRange);
	//m_RFCurrPos.resize(m_RigidForceRange);
	//m_RFUndeformPos.resize(m_RigidForceRange);
	//m_RFMomentMat.resize(m_RigidForceRange);
	///m_RFUndeformedMomentMat.resize(m_RigidForceRange);
}
//===================================================================================================
void SutureThread::SetStretchStiffness(Real set)
{
	m_RopeStrechStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(set);

}
//===================================================================================================
void SutureThread::SetBendingStiffness(Real set)
{
	m_RopeBendStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(set);
	m_RigidForceMagnitude = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(set);
}
//===================================================================================================
void SutureThread::SetGravity(const GFPhysVector3 & gravity)
{
	m_Gravity = gravity;
}
//===================================================================================================

void SutureThread::DestoryRope()
{
	m_SutureThreadNodes.clear();	
	m_FixedNodsInFo.clear();

	//m_AttachedFace = 0;
	m_SegmentBVTree.Clear();

	m_TRCollidePair.clear();
	m_TFCollidePair.clear();
	m_TTCollidepair.clear();

	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);

	m_IsCutAfterBound = false;
    m_RopeAnchorIndexVec.clear();
    m_ClampSegIndexVector.clear();
}
//=============================================================================================
void SutureThread::CreateFreeThread(const GFPhysVector3 & StartFixPoint, const GFPhysVector3 & EndFixPoint, int segmentCount, Real masspernode)
{
	DestoryRope();
    m_SutureThreadNodes.reserve(256);
	GFPhysVector3 dir = Perpendicular(EndFixPoint - StartFixPoint);
	dir.Normalize();
	m_Rest_Length = (StartFixPoint - EndFixPoint).Length() / segmentCount;

	for (int r = 0; r <= segmentCount; r++)
	{
		Real weight = (Real)r / (Real)segmentCount;

		GFPhysVector3 threadPos = StartFixPoint*(1 - weight) + EndFixPoint*weight;

		SutureRopeNode threadnode(threadPos);
		threadnode.SetInvMass(1.0f / masspernode);
        threadnode.m_StretchLen = m_Rest_Length;
		//GFPhysMatrix3 frameMat;
		//threadnode.SetParticleOrient(frameMat);

		threadnode.m_MaterialVector = dir;
        threadnode.m_GlobalId = GetGlobalId();
		m_SutureThreadNodes.push_back(threadnode);
	}
	//////////////////////////////////////////////////////////////////////////	

	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

	m_SegmentState.resize(GetNumSegments());
	for (int s = 0; s < GetNumSegments(); s++)
	{
		m_SegmentState[s] = 0;
	}
    m_InitNodesNum = segmentCount + 1;

    m_NullNode = SutureRopeNode();
	m_TotalRopeAnchorFriction = 0.0f;//0.005f;
	m_move = false;
}
//=========================================================================================================
void SutureThread::BeginSimulateSutureThreadPhysics(Real dt)
{
	m_SimFrameID++;

	int indexBeginCollideRigid = 1;
	for (int n = 0; n < indexBeginCollideRigid; n++)
	{
		SutureRopeNode& refNode = GetThreadNodeRef(n);
		refNode.MarkAsAttached(true);
		//refNode.m_EnableCollide = false;		
	}

	if (m_KnotsInThread)
    {
        GFPhysVectorObj<KnotInSutureRope*> AllKnots;
        m_KnotsInThread->GetAllKnotsRef(AllKnots);
        if (AllKnots.size() == 1 && AllKnots[0]->m_slack)
        {
            GFPhysVectorObj<GFPhysFaceRopeAnchor*> activeRopeAnchors;
            m_NeedleAttchedThread->GetFaceRopeAnchors(activeRopeAnchors);
            
            int max0 = AllKnots[0]->m_knotcon0.m_B;
            int min1 = AllKnots[0]->m_knotcon1.m_A;
            if (!activeRopeAnchors.empty())
            {
                if ((abs(max0 - GetRopeAnchorIndexMax()) <= NUM && abs(min1 - GetRopeAnchorIndexMin()) <= NUM) || abs(max0 - min1) < 6)
                {
                    AllKnots[0]->m_slack = false;
                }
            }
            else
            {
                if (abs(max0 - min1) < 6)
                {
                    AllKnots[0]->m_slack = false;
                }
            }
        }
        if (AllKnots.size() == 2 && AllKnots[1]->m_slack)
        {
            int min0 = AllKnots[0]->m_knotcon0.m_A;
            int max0 = AllKnots[0]->m_knotcon1.m_B;

            int min1 = AllKnots[1]->m_knotcon1.m_A;
            int max1 = AllKnots[1]->m_knotcon0.m_B;

            if ((abs(max0 - max1) <= NUM+1 && abs(min1 - min0) <= NUM+1))
            {
                AllKnots[1]->m_slack = false;                
            }
        }
    }

	//clear all real time state and reset solver scale
	for (size_t n = 0, ni = m_SutureThreadNodes.size(); n < ni; n++)
	{
		SutureRopeNode & tNode = m_SutureThreadNodes[n];
		if (tNode.GetSolverInvMasScale() > 0.0f)
		{
			//reset solver inv mass scale
			tNode.SetSolverInvMassScale(1.0f);
		}
		
	}

	//damping velocity and integrate position
	Real dampingRate = m_DampingRate;//3.0f;

    for (size_t n = 0, ni = m_SutureThreadNodes.size(); n < ni; n++)
	{
		SutureRopeNode & tNode = m_SutureThreadNodes[n];

		Real InvMass = tNode.GetInvMass();

		if (InvMass > FLT_EPSILON)
		{
			tNode.m_Velocity += m_Gravity*dt;

			Real realdamp = GPClamped(1.0f - dt * dampingRate, 0.0f, 1.0f);

			tNode.m_Velocity *= realdamp;

			tNode.m_LastPosition = tNode.m_CurrPosition;

			tNode.m_CurrPosition += tNode.m_Velocity*dt;
		}
	}

	//update tree
	m_SegmentBVTree.Clear();//clear first
    for (int n = 0, ni = (int)m_SutureThreadNodes.size() - 1; n < ni; n++)
	{
		GFPhysVector3 posn0 = m_SutureThreadNodes[n].m_LastPosition;
		GFPhysVector3 posn1 = m_SutureThreadNodes[n + 1].m_LastPosition;

		GFPhysVector3 minPos = posn0;
		GFPhysVector3 maxPos = posn0;

#if(USECCDINSELFCOLLISION)
		{
			GFPhysVector3 posl0 = m_SutureThreadNodes[n].m_CurrPosition;
			GFPhysVector3 posl1 = m_SutureThreadNodes[n + 1].m_CurrPosition;

			minPos.SetMin(posn1);
			minPos.SetMin(posl0);
			minPos.SetMin(posl1);

			maxPos.SetMax(posn1);
			maxPos.SetMax(posl0);
			maxPos.SetMax(posl1);
		}
#else
		{
			minPos.SetMin(posn1);
			maxPos.SetMax(posn1);
		}
#endif
		GFPhysVector3 span(m_RopeCollideRadius + m_Margin, m_RopeCollideRadius + m_Margin, m_RopeCollideRadius + m_Margin);

		GFPhysDBVNode * bvNode = m_SegmentBVTree.InsertAABBNode(minPos - span, maxPos + span);
		bvNode->m_UserData = (void*)n;
	}

	//check collision
	ITraining * itrain = CTrainingMgr::Instance()->GetCurTraining();

	MisNewTraining * newTrain = dynamic_cast<MisNewTraining * >(itrain);

	if (newTrain)
	{
		//check collision with soft body
		m_TFCollidePair.clear();//clear previous frame pair first
		m_TRCollidePair.clear();
		m_TTCollidepair.clear();

		std::vector<MisMedicOrganInterface*> organsInTrain;

		newTrain->GetAllOrgan(organsInTrain);

		//check collision with soft organ
        for (size_t c = 0, ni = organsInTrain.size(); c < ni; c++)
		{
			MisMedicOrganInterface * oif = organsInTrain[c];

			GFPhysSoftBody * physbody = 0;

			MisMedicOrgan_Ordinary * organOrdinary = dynamic_cast<MisMedicOrgan_Ordinary *>(oif);

			if (organOrdinary)
			{
				physbody = organOrdinary->m_physbody;
			}

			if (physbody)
			{
				GFPhysVectorObj<GFPhysDBVTree*> bvTrees = physbody->GetSoftBodyShape().GetFaceBVTrees();

				SutureThreadSegSoftFaceCallback collideCallBack(m_Margin, physbody, m_RopeCollideRadius, this, m_TFCollidePair, m_UseCCD);

                for (size_t t = 0, ni = bvTrees.size(); t < ni; t++)
				{
					GFPhysDBVTree * bvTree = bvTrees[t];
					bvTree->CollideWithDBVTree(m_SegmentBVTree, &collideCallBack);
				}
				{
					/*for (size_t c = 0; c < collideCallBack.m_InvCollidePairs.size(); c++)
					{
						const TFCollidePair & InvPair = collideCallBack.m_InvCollidePairs[c];

						for (size_t t = 0; t < collideCallBack.m_CollidePairs.size(); t++)
						{
							const TFCollidePair & NormPair = collideCallBack.m_CollidePairs[t];
							if (NormPair.GetCollideSegmentIndex() == InvPair.GetCollideSegmentIndex()
								&& NormPair.m_FaceNormal.dotProduct(InvPair.m_FaceNormal) < 0)
							{
								collideCallBack.m_CollidePairs.push_back(InvPair);
								break;
							}
						}
					}*/
				}
			}
		}

		//check collision with rigid body
		const GFPhysCollideObjectArray & collObjects = PhysicsWrapper::GetSingleTon().m_dynamicsWorld->GetCollideObjectArray();

        for (size_t c = 0, ni = collObjects.size(); c < ni; c++)
		{
			GFPhysCollideObject * cdobj = collObjects[c];

			if (cdobj->m_MaskBits & m_Catergory)
			{
				GFPhysRigidBody * rigidbody = GFPhysRigidBody::Upcast(cdobj);

				if (rigidbody)
				{
					GFPhysCollideShape * rigidshape = rigidbody->GetCollisionShape();

                    if (rigidshape->IsConvex())
					{
						GFPhysTransform rbTrans = rigidbody->GetWorldTransform();

						GFPhysConvexCDShape * convexshape = (GFPhysConvexCDShape*)rigidbody->GetCollisionShape();

						GFPhysVector3 Convexaabbmin, Convexaabbmax;

						convexshape->GetAabb(rbTrans, Convexaabbmin, Convexaabbmax);

#if USECCDINRIGIDCOLLISION
						GFPhysVector3 PredictConvexaabbmin, PredictConvexaabbmax;
						GFPhysTransform BodyPredictTrans;

						GFPhysTransformUtil::IntegrateTransform(rbTrans,
							rigidbody->m_linearVelocity,
							rigidbody->m_angularVelocity,
							dt,
							BodyPredictTrans);
						convexshape->GetAabb(BodyPredictTrans, PredictConvexaabbmin, PredictConvexaabbmax);

						Convexaabbmin.SetMin(PredictConvexaabbmin);

						Convexaabbmax.SetMax(PredictConvexaabbmax);
#endif

						SutureThreadConvexCallBack rbconcallback(m_Margin, rigidbody, rbTrans,dt, m_RopeCollideRadius, this, m_TRCollidePair);

						m_SegmentBVTree.TraverseTreeAgainstAABB(&rbconcallback, Convexaabbmin, Convexaabbmax);						
					}
                    else if (rigidshape->IsCompound())
                    {
                        GFPhysCompoundShape* compoundshape = (GFPhysCompoundShape*)rigidshape;
                        GFPhysTransform rbTrans = rigidbody->GetWorldTransform();
                        if (true)
                        {
                            for (int i = 0, ni = compoundshape->GetNumComponent(); i < ni; i++)
                            {
                                GFPhysTransform childTrans = compoundshape->GetComponentTransform(i);

                                GFPhysCollideShape * childshape = compoundshape->GetComponentShape(i);

                                GFPhysVector3 childshapeaabbmin, childshapeaabbmax;

                                GFPhysTransform childworldtrans = rbTrans*childTrans;

                                childshape->GetAabb(childworldtrans, childshapeaabbmin, childshapeaabbmax);

                                SutureThreadCompoundCallBack STCompoundCallback(m_Margin, rigidbody, childshape, childworldtrans, m_RopeCollideRadius, this, m_TRCollidePair);

                                m_SegmentBVTree.TraverseTreeAgainstAABB(&STCompoundCallback, childshapeaabbmin, childshapeaabbmax);
                            }
                        }                                   
                    }
				}
			}
		}

		/*
		CTool * leftTool  = (CTool *)m_ownertrain->m_pToolsMgr->GetLeftTool();

		CTool * rightTool = (CTool *)m_ownertrain->m_pToolsMgr->GetRightTool();

		std::set<GFPhysRigidBody*> ToolsRigids;

		if(leftTool)
		{
		ToolsRigids.insert(leftTool->m_centertoolpartconvex.m_rigidbody);
		ToolsRigids.insert(leftTool->m_lefttoolpartconvex.m_rigidbody);
		ToolsRigids.insert(leftTool->m_righttoolpartconvex.m_rigidbody);
		}

		if(rightTool)
		{
		ToolsRigids.insert(rightTool->m_centertoolpartconvex.m_rigidbody);
		ToolsRigids.insert(rightTool->m_lefttoolpartconvex.m_rigidbody);
		ToolsRigids.insert(rightTool->m_righttoolpartconvex.m_rigidbody);
		}
		*/

		m_ownertrain->onSutureThreadConvexCollided(m_TRCollidePair, this);

		//self collision
		if (m_EnableSelfCollision)
		{
			SutureThreadThreadCallBack ttcallback(m_Margin,
				m_RopeCollideRadius,
				this,
				this,
				m_TTCollidepair);

			m_SegmentBVTree.CollideWithDBVTree(m_SegmentBVTree, &ttcallback);
		}
	}
	UpdateMaterialVector();
    if (m_KnotsInThread && m_KnotsInThread->m_bHasKnot == false)
    {
        DisableCollideSelfFromClampToTail();
    }
}


GFPhysVector3& SutureThread::GetThreadNodePositionByIndex(int index)
{
	return m_SutureThreadNodes[index].m_CurrPosition;
}

//bool SutureThread::IsCollideWithOrgan(Ogre::Plane* plane)
//{
// 	for(size_t t = 0  ; t < m_TFCollidePair.size() ; t++)
//  	{
//  		TFCollidePair & tfPair = m_TFCollidePair[t];
//  		//int segIndex = tfPair.GetCollideSegmentIndex(); 
// 		//if(segIndex >= 0)
// 		{
//			Real mindist = 0.001f;
//			Real dist    = mindist;
//
// 			GFPhysSoftBodyFace * softFace = tfPair.m_SoftFace;
// 			if(softFace)
// 			{
// 				GFPhysSoftBodyNode * pSoftNode = softFace->m_Nodes[0];
// 				if( pSoftNode )
// 				{					
//					Ogre::Vector3 ogrePos(pSoftNode->m_UnDeformedPos.GetX(),pSoftNode->m_UnDeformedPos.GetY(),pSoftNode->m_UnDeformedPos.GetZ());					 
//					{
//						 
//						dist = fabs(plane->getDistance(ogrePos));
//						if( dist<mindist )
//						{
//							mindist = dist;
//							return true;
//						}
//					}
// 				}
// 			}
// 		}
//  	}
//	return false;
//}
//

Ogre::Vector3  SutureThread::GetThreadDir()
{
	return GPVec3ToOgre(m_SutureThreadNodes[m_SutureThreadNodes.size() - 1].m_CurrPosition - m_SutureThreadNodes[m_SutureThreadNodes.size() - 2].m_CurrPosition);
}

//================================================================================================
void SutureThread::EndSimulateThreadPhysics(Real dt)
{
	//friction
    for (size_t c = 0, ni = m_TFCollidePair.size(); c < ni; c++)
	{
		const TFCollidePair & cdpair = m_TFCollidePair[c];
		if (cdpair.m_CollideType == TFCollidePair::TFCD_VF)
		{
			GFPhysSoftBodyFace * softFace = cdpair.m_SoftFace;
			SutureRopeNode & tnode = m_SutureThreadNodes[cdpair.m_e1];

			GFPhysVector3 CollideNormal = OgreToGPVec3(cdpair.m_CollideNormal);//(softFace->m_Nodes[1]->m_CurrPosition-softFace->m_Nodes[0]->m_CurrPosition).Cross(softFace->m_Nodes[2]->m_CurrPosition-softFace->m_Nodes[0]->m_CurrPosition);

			Real normLen = CollideNormal.Length2();

			if (normLen > FLT_EPSILON)
			{
				const Real frictionDamp = m_RopeFriction;//0.8f;

				CollideNormal /= normLen;

				GFPhysVector3 NodaVel = tnode.m_CurrPosition - tnode.m_LastPosition;

				GFPhysVector3 NormalVel = CollideNormal*NodaVel.Dot(CollideNormal);

				GFPhysVector3 TangVel = NodaVel - NormalVel;

				TangVel *= 0.9f;//GPClamped(1.0f - dt * frictionDamp , 0.0f, 1.0f);

				tnode.m_CurrPosition = tnode.m_LastPosition + NormalVel + TangVel;

				for (int n = 0; n < 3; n++)
				{
					GFPhysSoftBodyNode * faceNode = softFace->m_Nodes[n];

					GFPhysVector3 NodaVel = faceNode->m_Velocity;

					GFPhysVector3 NormalVel = CollideNormal*NodaVel.Dot(CollideNormal);

					GFPhysVector3 TangVel = NodaVel - NormalVel;

					TangVel *= GPClamped(1.0f - dt * frictionDamp, 0.0f, 1.0f);

					faceNode->m_Velocity = NormalVel + TangVel;
				}
			}
		}
	}

	//update velocity
    for (size_t n = 0, ni = m_SutureThreadNodes.size(); n < ni; n++)
	{
		if (m_SutureThreadNodes[n].GetInvMass() > FLT_EPSILON)
		{
			GFPhysVector3 trans = (m_SutureThreadNodes[n].m_CurrPosition - m_SutureThreadNodes[n].m_LastPosition);
			m_SutureThreadNodes[n].m_Velocity = (m_SutureThreadNodes[n].m_CurrPosition - m_SutureThreadNodes[n].m_LastPosition) / dt;
		}
	}



	//eliminate velocity and position inverse collide normal
    for (size_t c = 0, ni = m_TRCollidePair.size(); c < ni; c++)
	{
		TRCollidePair & trPair = m_TRCollidePair[c];

		SutureRopeNode * tNode[2];
		tNode[0] = &(m_SutureThreadNodes[trPair.m_Segment]);
		tNode[1] = &(m_SutureThreadNodes[trPair.m_Segment + 1]);

		if ((tNode[0]->IsAttached()) || (tNode[1]->IsAttached()))
			continue;

		for (int n = 0; n < 2; n++)
		{
			Real velinNormal = tNode[n]->m_Velocity.Dot(trPair.m_NormalOnRigid);

			if (velinNormal < 0)
				tNode[n]->m_Velocity -= trPair.m_NormalOnRigid*velinNormal;

			Real  s_normdist = GPSIMDVec3Dot(GPSIMDVec3Sub(trPair.m_RigidWorldPoint, tNode[n]->m_CurrPosition), trPair.m_NormalOnRigid);
			s_normdist += m_RopeCollideRadius;

			if (s_normdist > 0 && tNode[n]->GetInvMass() > 0)
			{
				tNode[n]->m_CurrPosition += trPair.m_NormalOnRigid*s_normdist;
			}

			//velinNormal = tNode[n]->m_Velocity.Dot(trPair.m_NormalOnRigid);
			//GFPhysVector3 tanvel = tNode[n]->m_Velocity-trPair.m_NormalOnRigid*velinNormal;
			//tNode[n]->m_Velocity = trPair.m_NormalOnRigid*velinNormal + tanvel * 0.8f;
		}
	}


    for (size_t t = 0, ni = m_TFCollidePair.size(); t <ni ; t++)
	{
		TFCollidePair & tfPair = m_TFCollidePair[t];
		GFPhysSoftBodyFace * softFace = tfPair.m_SoftFace;
		if (softFace)
		{
			softFace->m_Nodes[0]->m_InvM = softFace->m_Nodes[0]->m_OriginInvMass;
			softFace->m_Nodes[1]->m_InvM = softFace->m_Nodes[1]->m_OriginInvMass;
			softFace->m_Nodes[2]->m_InvM = softFace->m_Nodes[2]->m_OriginInvMass;

		}
	}
	//
	//damping
	/*for(size_t n = 0 ; n < m_ThreadNodes.size() ; n++)
	{
	if(m_ThreadNodes[n].m_InvMass > FLT_EPSILON)
	{
	Real realdamp = GPClamped(1.0f - dt * dampingRate , 0.0f, 1.0f);
	m_ThreadNodes[n].m_Velocity *= realdamp;
	}
	}*/

	/*
	std::map<GFPhysRigidBody* , RigidTransFormHistory>::iterator rbtItor = m_LastRigidTrans.begin();
	while(rbtItor != m_LastRigidTrans.end())
	{
	if(rbtItor->second.m_SimFrameID < m_SimFrameID)
	{
	rbtItor = m_LastRigidTrans.erase(rbtItor);
	}
	else
	{
	rbtItor->second.m_LastTrans = rbtItor->first->GetWorldTransform();//update last transform
	rbtItor++;
	}
	}*/

    //for (int n = 0; n < GetNumThreadNodes() - 1; n++)
    //{
    //    SutureRopeNode& refNode = GetThreadNodeRef(n);
    //    refNode.SetCanCollideSelf(true);
    //}

	SlideKnot();
}
//================================================================================================
void SutureThread::SlideKnot()
{
	if (m_KnotsInThread && m_KnotsInThread->m_bHasKnot)
	{
		GFPhysVectorObj<GFPhysFaceRopeAnchor*> activeRopeAnchors;
		m_NeedleAttchedThread->GetFaceRopeAnchors(activeRopeAnchors);

		GFPhysVectorObj<KnotInSutureRope*> AllKnots;
		m_KnotsInThread->GetAllKnotsRef(AllKnots);
		if (AllKnots.size() == 1)
		{
			Real lengthcorrect = (m_KnotImpluse[0] - m_KnotImpluse[1]).Dot(AllKnots[0]->m_KnotDirection);

			lengthcorrect = fabsf(lengthcorrect);
			lengthcorrect = GPClamped(lengthcorrect, 0.0f, 0.5f*m_Rest_Length);

			if (AllKnots[0]->m_slack)
			{
				int max0 = AllKnots[0]->m_knotcon0.m_B;
				int min0 = AllKnots[0]->m_knotcon0.m_A;

				int max1 = AllKnots[0]->m_knotcon1.m_B;
				int min1 = AllKnots[0]->m_knotcon1.m_A;

				if (abs(min1 - GetRopeAnchorIndexMin()) > NUM)
				{
					if (m_SutureThreadNodes[min1].m_StretchLen > m_Rest_Length * TESTTHRESHOLD + GP_EPSILON)
					{
						m_SutureThreadNodes[min1].m_StretchLen -= lengthcorrect*0.5f;

						m_SutureThreadNodes[min0 - 1].m_StretchLen += lengthcorrect*0.5f;

						if (m_SutureThreadNodes[min1].m_StretchLen < GP_EPSILON)
						{
							qDebug() << "Error length";
						}
					}
					else
					{
						Real losslength = m_SutureThreadNodes[min1].m_StretchLen;
						Real enlargelength = m_SutureThreadNodes[min0 - 1].m_StretchLen;

						m_SutureThreadNodes.remove((size_t)min1);
						m_SutureThreadNodes[min1].m_StretchLen += losslength;

						Real w = m_Rest_Length / enlargelength;
						InsertOneNode(min0 - 1, w);

						m_SutureThreadNodes[min0 - 1].m_StretchLen = m_Rest_Length;
						m_SutureThreadNodes[min0].m_StretchLen = enlargelength - m_Rest_Length;


						m_SutureThreadNodes[min0 - 1].m_GlobalId = m_SutureThreadNodes[min0].m_GlobalId;
						m_SutureThreadNodes[min0].m_GlobalId = GetGlobalId();
						//////////////////////////////////////////////////////////////////////////

						AllKnots[0]->m_knotcon0.m_A += 1;
						AllKnots[0]->m_knotcon1.m_A += 1;

						//////////////////////////////////////////////////////////////////////////
						m_SutureThreadNodes[min1 + 1].m_StretchLen -= lengthcorrect*0.5f;
						m_SutureThreadNodes[min0].m_StretchLen += lengthcorrect*0.5f;

					}
				}
				/*****************************************************************************************************************************************/
				if (abs(max0 - GetRopeAnchorIndexMax()) > NUM)
				{
					if (m_SutureThreadNodes[max0 - 1].m_StretchLen > m_Rest_Length * TESTTHRESHOLD + GP_EPSILON)
					{
						m_SutureThreadNodes[max0 - 1].m_StretchLen -= lengthcorrect*0.5f;
						m_SutureThreadNodes[max1].m_StretchLen += lengthcorrect*0.5f;
						if (m_SutureThreadNodes[max0 - 1].m_StretchLen < GP_EPSILON)
						{
							qDebug() << "Error length";
						}
					}
					else
					{
						Real losslength = m_SutureThreadNodes[max0 - 1].m_StretchLen;
						Real enlargelength = m_SutureThreadNodes[max1].m_StretchLen;

						m_SutureThreadNodes.remove((size_t)(max0 - 1));
						m_SutureThreadNodes[max0 - 2].m_StretchLen += losslength;

						Real w = m_Rest_Length / enlargelength;
						InsertOneNode(max1 - 1, w);

						m_SutureThreadNodes[max1 - 1].m_StretchLen = enlargelength - m_Rest_Length;					
						m_SutureThreadNodes[max1].m_StretchLen = m_Rest_Length;

						m_SutureThreadNodes[max1 - 1].m_GlobalId = GetGlobalId();
						m_SutureThreadNodes[max1].m_GlobalId = m_SutureThreadNodes[max1].m_GlobalId;
						//////////////////////////////////////////////////////////////////////////

						AllKnots[0]->m_knotcon0.m_B -= 1;
						AllKnots[0]->m_knotcon1.m_B -= 1;

						//////////////////////////////////////////////////////////////////////////
						m_SutureThreadNodes[max0 - 2].m_StretchLen -= lengthcorrect*0.5f;
						m_SutureThreadNodes[max1 - 1].m_StretchLen += lengthcorrect*0.5f;

					}
				}
			}
			else
			{

				AllKnots[0]->m_knotcon0.m_bWork = false;
				AllKnots[0]->m_knotcon1.m_bWork = false;

				int max = AllKnots[0]->m_knotcon1.m_B;
				int min = AllKnots[0]->m_knotcon0.m_A;

				Real SHRINKCOFF1 = SHRINK1 / (max - min);

				Real coff = ((max - min + 2) * m_Rest_Length - lengthcorrect) / ((max - min + 2) * m_Rest_Length);

				//GPClamp(coff, 0.95f, 1.0f);

				Real lengthLossInKnot = 0.0f;
				for (int n = min; n < max/* + 1*/; n++)
				{
					if (m_SutureThreadNodes[n].m_StretchLen > SHRINKCOFF1*m_Rest_Length)
					{
						AllKnots[0]->m_knotcon0.m_bWork = true;
						AllKnots[0]->m_knotcon1.m_bWork = true;
						break;
					}
				}


				if (AllKnots[0]->m_knotcon0.m_bWork && AllKnots[0]->m_knotcon0.m_bWork)
				{

					Real lengthLossInKnot = 0.0f;
					for (int n = min; n < max/* + 1*/; n++)
					{
						m_SutureThreadNodes[n].m_StretchLen *= coff;
						lengthLossInKnot += m_SutureThreadNodes[n].m_StretchLen*(1.0f - coff);
					}


					if (true)
					{
						m_SutureThreadNodes[min - 1].m_StretchLen += lengthLossInKnot*0.5f;
						m_SutureThreadNodes[max].m_StretchLen += lengthLossInKnot*0.5f;
					}

					if (m_SutureThreadNodes[max].m_StretchLen > m_Rest_Length *2.0f || m_SutureThreadNodes[min - 1].m_StretchLen > m_Rest_Length *2.0f)
					{
						Real enlargelength1 = m_SutureThreadNodes[min - 1].m_StretchLen;

						Real enlargelength2 = m_SutureThreadNodes[max].m_StretchLen;

						Real w = (enlargelength1 - m_Rest_Length) / (enlargelength1);
						//////////////////////////////////////////////////////////////////////////
						InsertOneNode(min - 1, w);

						m_SutureThreadNodes[min - 1].m_StretchLen = m_Rest_Length;
						m_SutureThreadNodes[min].m_StretchLen = enlargelength1 - m_Rest_Length;

						m_SutureThreadNodes[min - 1].m_GlobalId = m_SutureThreadNodes[min].m_GlobalId;
						m_SutureThreadNodes[min].m_GlobalId = GetGlobalId();

						AllKnots[0]->m_knotcon0.m_A += 1;
						AllKnots[0]->m_knotcon0.m_B += 1;
						AllKnots[0]->m_knotcon1.m_A += 1;
						AllKnots[0]->m_knotcon1.m_B += 1;

						for (int i = 0, ni = (int)activeRopeAnchors.size(); i < ni; i++)
						{
							activeRopeAnchors[i]->m_NodeIndex += 1;
						}
						//////////////////////////////////////////////////////////////////////////
						w = (enlargelength2 - m_Rest_Length) / (enlargelength2);
						InsertOneNode(max + 1, w);

						m_SutureThreadNodes[max + 1].m_StretchLen = enlargelength2 - m_Rest_Length;
						m_SutureThreadNodes[max + 2].m_StretchLen = m_Rest_Length;

						m_SutureThreadNodes[max + 1].m_GlobalId = GetGlobalId();
						m_SutureThreadNodes[max + 2].m_GlobalId = m_SutureThreadNodes[max + 1].m_GlobalId;

					}
				}

				Real leng = 0.0f;
				for (int i = 0, ni = (int)m_SutureThreadNodes.size() - 1; i < ni; i++)
				{
					if (m_SutureThreadNodes[i].m_StretchLen < 0.0f)
					{
						qDebug() << "error!  length of thread" << i << ":" << m_SutureThreadNodes[i].m_StretchLen;
					}

					leng += m_SutureThreadNodes[i].m_StretchLen;
					//qDebug() << "step - 1 : length of thread" << i << ":" << m_SutureThreadNodes[i].m_StretchLen;
				}
				//qDebug() << "step - 1 : length of suture thread" << leng;                        
			}
		}
		else if (AllKnots.size() == 2)
		{
			Real lengthcorrect = (m_KnotImpluse[0] - m_KnotImpluse[1]).Dot(AllKnots[1]->m_KnotDirection);
			lengthcorrect = fabsf(lengthcorrect);
			lengthcorrect = GPClamped(lengthcorrect, 0.0f, 0.5f*m_Rest_Length);

			if (AllKnots[1]->m_slack)
			{

				int maxofKnot0 = AllKnots[0]->m_knotcon1.m_B;
				int minofKnot0 = AllKnots[0]->m_knotcon0.m_A;

				int max0 = AllKnots[1]->m_knotcon0.m_B;
				int min0 = AllKnots[1]->m_knotcon0.m_A;
				int max1 = AllKnots[1]->m_knotcon1.m_B;
				int min1 = AllKnots[1]->m_knotcon1.m_A;

				//////////////////////////////////////////////////////////////////////////


				if (abs(min1 - minofKnot0) > NUM)
				{
					if (m_SutureThreadNodes[min1].m_StretchLen > m_Rest_Length * TESTTHRESHOLD + GP_EPSILON)
					{
						m_SutureThreadNodes[min1].m_StretchLen -= lengthcorrect*0.5f;

						m_SutureThreadNodes[min0 - 1].m_StretchLen += lengthcorrect*0.5f;

						if (m_SutureThreadNodes[min1].m_StretchLen < GP_EPSILON)
						{
							qDebug() << "step -2 slack :Error length";
						}
					}
					else
					{
						Real losslength = m_SutureThreadNodes[min1].m_StretchLen;
						Real enlargelength = m_SutureThreadNodes[min0 - 1].m_StretchLen;

						m_SutureThreadNodes.remove((size_t)min1);
						m_SutureThreadNodes[min1].m_StretchLen += losslength;

						Real w = m_Rest_Length / enlargelength;
						InsertOneNode(min0 - 1, w);

						m_SutureThreadNodes[min0 - 1].m_StretchLen = m_Rest_Length;
						m_SutureThreadNodes[min0].m_StretchLen = enlargelength - m_Rest_Length;

						m_SutureThreadNodes[min0 - 1].m_GlobalId = m_SutureThreadNodes[min0].m_GlobalId;
						m_SutureThreadNodes[min0].m_GlobalId = GetGlobalId();


						//////////////////////////////////////////////////////////////////////////

						AllKnots[1]->m_knotcon0.m_A += 1;
						AllKnots[1]->m_knotcon1.m_A += 1;

						//AllKnots[0]->m_knotcon0.m_A += 1;
						//AllKnots[0]->m_knotcon0.m_B += 1; 
						//AllKnots[0]->m_knotcon1.m_A += 1;
						//AllKnots[0]->m_knotcon1.m_B += 1;

						//////////////////////////////////////////////////////////////////////////

						m_SutureThreadNodes[min1 + 1].m_StretchLen -= lengthcorrect*0.5f;
						m_SutureThreadNodes[min0].m_StretchLen += lengthcorrect*0.5f;
						if (m_SutureThreadNodes[min1 + 1].m_StretchLen < GP_EPSILON)
						{
							qDebug() << "step -2 slack :Error length";
						}
					}
				}
				/*****************************************************************************************************************************************/

				if (abs(max0 - maxofKnot0) > NUM)
				{
					if (m_SutureThreadNodes[max0 - 1].m_StretchLen > m_Rest_Length * TESTTHRESHOLD + GP_EPSILON)
					{
						m_SutureThreadNodes[max0 - 1].m_StretchLen -= lengthcorrect*0.5f;
						m_SutureThreadNodes[max1].m_StretchLen += lengthcorrect*0.5f;
						if (m_SutureThreadNodes[max0 - 1].m_StretchLen < GP_EPSILON)
						{
							qDebug() << "step -2 slack :Error length";
						}
					}
					else
					{
						Real losslength = m_SutureThreadNodes[max0 - 1].m_StretchLen;
						Real enlargelength = m_SutureThreadNodes[max1].m_StretchLen;

						m_SutureThreadNodes.remove((size_t)(max0 - 1));
						m_SutureThreadNodes[max0 - 2].m_StretchLen += losslength;

						Real w = m_Rest_Length / enlargelength;
						InsertOneNode(max1 - 1, w);

						m_SutureThreadNodes[max1 - 1].m_StretchLen = enlargelength - m_Rest_Length;
						m_SutureThreadNodes[max1].m_StretchLen = m_Rest_Length;

						m_SutureThreadNodes[max1 - 1].m_GlobalId = GetGlobalId();
						m_SutureThreadNodes[max1].m_GlobalId = m_SutureThreadNodes[max1].m_GlobalId;

						//////////////////////////////////////////////////////////////////////////

						AllKnots[1]->m_knotcon0.m_B -= 1;
						AllKnots[1]->m_knotcon1.m_B -= 1;

						//AllKnots[0]->m_knotcon0.m_A -= 1;
						//AllKnots[0]->m_knotcon0.m_B -= 1;
						//AllKnots[0]->m_knotcon1.m_A -= 1;
						//AllKnots[0]->m_knotcon1.m_B -= 1;
						//////////////////////////////////////////////////////////////////////////
						m_SutureThreadNodes[max0 - 2].m_StretchLen -= lengthcorrect*0.5f;
						m_SutureThreadNodes[max1 - 1].m_StretchLen += lengthcorrect*0.5f;
						if (m_SutureThreadNodes[max0 - 2].m_StretchLen < GP_EPSILON)
						{
							qDebug() << "step -2 slack :Error length";
						}
					}
				}
				Real leng = 0.0f;
				for (int i = 0, ni = (int)m_SutureThreadNodes.size() - 1; i < ni; i++)
				{
					if (m_SutureThreadNodes[i].m_StretchLen < 0.0f)
					{
						qDebug() << "error!  length of thread" << i << ":" << m_SutureThreadNodes[i].m_StretchLen;
					}

					leng += m_SutureThreadNodes[i].m_StretchLen;
					//qDebug() << "step - 2 slack: length of thread" << i << ":" << m_SutureThreadNodes[i].m_StretchLen;
				}
				//qDebug() << "step - 2 slack: length of suture thread" << leng;
			}
			else
			{
				int max0 = AllKnots[0]->m_knotcon1.m_B;
				int min0 = AllKnots[0]->m_knotcon0.m_A;

				int max1 = AllKnots[1]->m_knotcon1.m_B;
				int min1 = AllKnots[1]->m_knotcon0.m_A;

				Real SHRINKCOFF2 = SHRINK2 / (max1 - max0 + min0 - min1);

				AllKnots[1]->m_knotcon0.m_bWork = false;
				AllKnots[1]->m_knotcon1.m_bWork = false;

				//lengthcorrect = (kbegin - kend).Length();

				Real coff = ((max1 - max0 + min0 - min1 + 2) * m_Rest_Length - lengthcorrect) / ((max1 - max0 + min0 - min1 + 2) * m_Rest_Length);

				//GPClamp(coff, 0.95f, 1.0f);

				Real lengthLossInKnot = 0.0f;
				Real lengthLossInKnotMin = 0.0f;
				Real lengthLossInKnotMax = 0.0f;

				for (int n = min1; n < min0/* + 1*/; n++)
				{
					if (m_SutureThreadNodes[n].m_StretchLen > SHRINKCOFF2*m_Rest_Length)
					{
						m_SutureThreadNodes[n].m_StretchLen *= coff;
						lengthLossInKnot += m_SutureThreadNodes[n].m_StretchLen*(1.0f - coff);
						lengthLossInKnotMin += m_SutureThreadNodes[n].m_StretchLen*(1.0f - coff);						
					}
					else
					{
						AllKnots[1]->m_knotcon0.m_bWork = true;
						break;
					}
				}

				for (int n = max0; n < max1/* + 1*/; n++)
				{
					if (m_SutureThreadNodes[n].m_StretchLen > SHRINKCOFF2*m_Rest_Length)
					{
						m_SutureThreadNodes[n].m_StretchLen *= coff;
						lengthLossInKnot += m_SutureThreadNodes[n].m_StretchLen*(1.0f - coff);
						lengthLossInKnotMax += m_SutureThreadNodes[n].m_StretchLen*(1.0f - coff);						
					}
					else
					{
						AllKnots[1]->m_knotcon1.m_bWork = true;
						break;
					}
				}

				if (AllKnots[1]->m_knotcon0.m_bWork && AllKnots[1]->m_knotcon1.m_bWork)
				{
					if (true)
					{
						m_SutureThreadNodes[min1 - 1].m_StretchLen += lengthLossInKnotMin;
						m_SutureThreadNodes[max1].m_StretchLen += lengthLossInKnotMax;
					}
					if (m_SutureThreadNodes[min1 - 1].m_StretchLen > m_Rest_Length * 1.20f)
					{
						Real enlargelength1 = m_SutureThreadNodes[min1 - 1].m_StretchLen;
						Real w = (enlargelength1 - m_Rest_Length) / (enlargelength1);

						InsertOneNode(min1 - 1, w);
						m_SutureThreadNodes[min1 - 1].m_StretchLen = m_Rest_Length;
						m_SutureThreadNodes[min1].m_StretchLen = enlargelength1 - m_Rest_Length;

						m_SutureThreadNodes[min1 - 1].m_GlobalId = m_SutureThreadNodes[min1].m_GlobalId;
						m_SutureThreadNodes[min1].m_GlobalId =  GetGlobalId();

						AllKnots[0]->m_knotcon0.m_A += 1;
						AllKnots[0]->m_knotcon0.m_B += 1;
						AllKnots[0]->m_knotcon1.m_A += 1;
						AllKnots[0]->m_knotcon1.m_B += 1;

						AllKnots[1]->m_knotcon0.m_A += 1;
						AllKnots[1]->m_knotcon0.m_B += 1;
						AllKnots[1]->m_knotcon1.m_A += 1;
						AllKnots[1]->m_knotcon1.m_B += 1;

						for (int i = 0; i < (int)activeRopeAnchors.size(); i++)
						{
							activeRopeAnchors[i]->m_NodeIndex += 1;
						}
					}

					max1 = AllKnots[1]->m_knotcon1.m_B;

					if (m_SutureThreadNodes[max1].m_StretchLen > m_Rest_Length * 1.20f)
					{
						Real enlargelength2 = m_SutureThreadNodes[max1].m_StretchLen;
						Real w = (enlargelength2 - m_Rest_Length) / (enlargelength2);
						InsertOneNode(max1, w);

						m_SutureThreadNodes[max1].m_StretchLen = enlargelength2 - m_Rest_Length;
						m_SutureThreadNodes[max1 + 1].m_StretchLen = m_Rest_Length;

						m_SutureThreadNodes[max1].m_GlobalId = GetGlobalId();
						m_SutureThreadNodes[max1 + 1].m_GlobalId = m_SutureThreadNodes[max1].m_GlobalId;
					}
				}

				Real leng = 0.0f;
				for (int i = 0; i < (int)m_SutureThreadNodes.size() - 1; i++)
				{
					if (m_SutureThreadNodes[i].m_StretchLen < 0.0f)
					{
						qDebug() << "error!  length of thread" << i << ":" << m_SutureThreadNodes[i].m_StretchLen;
					}

					leng += m_SutureThreadNodes[i].m_StretchLen;
					//qDebug() << "step - 2 : length of thread" << i << ":" << m_SutureThreadNodes[i].m_StretchLen;
				}
				//qDebug() << "step - 2 : length of suture thread" << leng;
			}
		}
	}
}
//================================================================================================
bool SutureThread::GetThreadSegmentNode(SutureRopeNode & n0, SutureRopeNode & n1, int segIndex)
{
	if (segIndex >= 0 && segIndex < (int)m_SutureThreadNodes.size() - 1)
	{
		n0 = m_SutureThreadNodes[segIndex];
		n1 = m_SutureThreadNodes[segIndex + 1];
		return true;
	}
	return false;
}

bool SutureThread::GetThreadSegmentTangent(GFPhysVector3 & tanget, int NodeIndex)
{
	if (NodeIndex >= 1 && NodeIndex < (int)m_SutureThreadNodes.size() - 1)
	{
		tanget = m_SutureThreadNodes[NodeIndex + 1].m_CurrPosition
			- m_SutureThreadNodes[NodeIndex - 1].m_CurrPosition;
		tanget.Normalize();
		return true;
	}
	else if (NodeIndex == 0)
	{
		tanget = m_SutureThreadNodes[1].m_CurrPosition
			- m_SutureThreadNodes[0].m_CurrPosition;
		tanget.Normalize();
		return true;
	}
	else if (NodeIndex == (int)m_SutureThreadNodes.size() - 1)
	{
		tanget = m_SutureThreadNodes[(int)m_SutureThreadNodes.size() - 1].m_CurrPosition
			- m_SutureThreadNodes[(int)m_SutureThreadNodes.size() - 2].m_CurrPosition;
		tanget.Normalize();
		return true;
	}
	else
	{
		return false;
	}
}

bool SutureThread::GetThreadSegmentTangentLerped(GFPhysVector3 & tanget, int segIndex, Real weight)
{
	GFPhysVector3 tan0, tan1;
	bool succeed0 = GetThreadSegmentTangentOrder1(tan0, segIndex);
	bool succeed1 = GetThreadSegmentTangentOrder1(tan1, segIndex+1);
	if (succeed0 && succeed1)
	{
		tanget = (tan0 * (1 - weight) + tan1 * weight).Normalized();
		return true;
	}
	else
		return false;
}
bool SutureThread::GetThreadSegmentTangentOrder1(GFPhysVector3 & tanget, int NodeIndex)
{
	if (NodeIndex >= 1 && NodeIndex < (int)m_SutureThreadNodes.size() - 1)
	{
		tanget = m_SutureThreadNodes[NodeIndex + 1].m_CurrPosition
			- m_SutureThreadNodes[NodeIndex - 1].m_CurrPosition;
		tanget.Normalize();
		return true;
	}
	else if (NodeIndex == 0)
	{
		tanget = m_SutureThreadNodes[1].m_CurrPosition
			- m_SutureThreadNodes[0].m_CurrPosition;
		tanget.Normalize();
		return true;
	}
	else if (NodeIndex == (int)m_SutureThreadNodes.size() - 1)
	{
		tanget = m_SutureThreadNodes[(int)m_SutureThreadNodes.size() - 1].m_CurrPosition
			- m_SutureThreadNodes[(int)m_SutureThreadNodes.size() - 2].m_CurrPosition;
		tanget.Normalize();
		return true;
	}
	else
	{
		return false;
	}
}
bool SutureThread::GetThreadSegmentTangentOrder2(GFPhysVector3 & tanget2, int NodeIndex)
{
	if (NodeIndex >= 1 && NodeIndex < (int)m_SutureThreadNodes.size() - 1)
	{
		GFPhysVector3 tangetstart;
		GFPhysVector3 tangetend;
		if (GetThreadSegmentTangentOrder1(tangetstart, NodeIndex - 1) &&
			GetThreadSegmentTangentOrder1(tangetend, NodeIndex + 1))
		{
			tanget2 = tangetend - tangetstart;
			//tanget2.Normalize();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (NodeIndex == 0)
	{
		GFPhysVector3 tangetstart;
		GFPhysVector3 tangetend;
		if (GetThreadSegmentTangentOrder1(tangetstart, 0) &&
			GetThreadSegmentTangentOrder1(tangetend, 1))
		{
			tanget2 = tangetend - tangetstart;
			//tanget2.Normalize();
			return true;
		}
		else
		{
			return false;
		}
	}
	else if (NodeIndex == (int)m_SutureThreadNodes.size() - 1)
	{
		GFPhysVector3 tangetstart;
		GFPhysVector3 tangetend;
		if (GetThreadSegmentTangentOrder1(tangetstart, (int)m_SutureThreadNodes.size() - 2) &&
			GetThreadSegmentTangentOrder1(tangetend, (int)m_SutureThreadNodes.size() - 1))
		{
			tanget2 = tangetend - tangetstart;
			//tanget2.Normalize();
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}
//============================================================================================
bool SutureThread::GetThreadSegmentCurvature(Real & curvature, int NodeIndex)
{
	//空间曲线的曲率计算方法 张学东 2002
	if (NodeIndex >= 0 && NodeIndex < (int)m_SutureThreadNodes.size())
	{
		GFPhysVector3 tanget;
		GFPhysVector3 tanget2;
		if (GetThreadSegmentTangentOrder1(tanget, NodeIndex) && GetThreadSegmentTangentOrder2(tanget2, NodeIndex))
		{
			Real d = tanget.Dot(tanget2);
			curvature = powf((tanget.Length2() * tanget2.Length2() - d * d), 0.50f);
			curvature /= tanget.Length()*tanget.Length()*tanget.Length();
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}
//============================================================================================
bool SutureThread::GetThreadSegmentNodePos(GFPhysVector3 & n0, GFPhysVector3 & n1, int segIndex)
{
	if (segIndex >= 0 && segIndex < (int)m_SutureThreadNodes.size() - 1)
	{
		n0 = m_SutureThreadNodes[segIndex].m_CurrPosition;
		n1 = m_SutureThreadNodes[segIndex + 1].m_CurrPosition;
		return true;
	}
	return false;
}
//================================================================================================
Real SutureThread::GetTotalLen(bool deformed)
{
	if (deformed == false)
		return m_Rest_Length * (m_InitNodesNum - 1);
	else
	{
		int SegNum = m_SutureThreadNodes.size() - 1;
		Real totalLen = 0;
		for (int n = 0; n < SegNum; n++)
		{
			GFPhysVector3 p0 = m_SutureThreadNodes[n].m_CurrPosition;
			GFPhysVector3 p1 = m_SutureThreadNodes[n + 1].m_CurrPosition;

			Real t = (p0 - p1).Length();
			totalLen += t;
		}
		return totalLen;
	}
}
//================================================================================================
Real SutureThread::GetCustomLen(bool deformed,int i,int j)
{
	int SegNum = m_SutureThreadNodes.size() - 1;

	assert(i >= 0 && i < SegNum + 1);
	assert(j >= 0 && j < SegNum + 1);
	assert(i <= j);
	if (deformed == false)
		return m_Rest_Length * (j - i);
	else
	{		
		Real totalLen = 0.0f;
		for (int n = i; n < j; n++)
		{
			GFPhysVector3 p0 = m_SutureThreadNodes[n].m_CurrPosition;
			GFPhysVector3 p1 = m_SutureThreadNodes[n + 1].m_CurrPosition;

			Real t = (p0 - p1).Length();
			totalLen += t;
		}
		return totalLen;
	}
}
//============================================================================================
bool SutureThread::RelativePos2CurrLen(int index, Real weight, Real& currlen)
{

#if 0
    if (index == 0)
    {
        currlen = (m_SutureThreadNodes[0].m_UnDeformedPos
            - m_SutureThreadNodes[1].m_UnDeformedPos).Length() * (1.0f - weight);
        return true;
    }
    else if (index > (int)m_SutureThreadNodes.size() - 1)
    {
        return false;
    }
    else
    {
        Real totalLen = 0.0f;
        for (int n = 0; n < index; n++)
        {
            GFPhysVector3 p0 = m_SutureThreadNodes[n].m_UnDeformedPos;
            GFPhysVector3 p1 = m_SutureThreadNodes[n + 1].m_UnDeformedPos;

            Real t = (p0 - p1).Length();
            totalLen += t;
        }
        totalLen += (m_SutureThreadNodes[index].m_UnDeformedPos
            - m_SutureThreadNodes[index + 1].m_UnDeformedPos).Length() * (1.0f - weight);

        currlen = totalLen;
        return true;
    }
#else
    if (index == 0)
    {
        currlen = (m_SutureThreadNodes[0].m_CurrPosition
            - m_SutureThreadNodes[1].m_CurrPosition).Length() * (1.0f - weight);
        return true;
    }
    else if (index > (int)m_SutureThreadNodes.size() - 1)
    {
        return false;
    }
    else
    {
        Real totalLen = 0.0f;
        for (int n = 0; n < index; n++)
        {
            GFPhysVector3 p0 = m_SutureThreadNodes[n].m_CurrPosition;
            GFPhysVector3 p1 = m_SutureThreadNodes[n + 1].m_CurrPosition;

            Real t = (p0 - p1).Length();
            totalLen += t;
        }
        totalLen += (m_SutureThreadNodes[index].m_CurrPosition
            - m_SutureThreadNodes[index + 1].m_CurrPosition).Length() * (1.0f - weight);

        currlen = totalLen;
        return true;
    }
#endif	
	return false;
}
//============================================================================================
bool SutureThread::CurrLen2RelativePos(Real currlen, int& index, Real& weight)
{
#if 0
    //input filter missing
    Real length = GetTotalLen(false);
    currlen = GPClamped(currlen, 0.0f, length);

    if (fabsf(currlen) < GP_EPSILON)
    {
        index = 0;
        weight = 1.0f;
        return true;
    }
	if (currlen > length - GP_EPSILON)
	{
		index = GetNumSegments();
		weight = 1.0f;
		return true;
	}

    Real partsum = 0.0f;
    for (int n = 0; n < (int)m_SutureThreadNodes.size() - 1; n++)
    {
        GFPhysVector3 p0 = m_SutureThreadNodes[n].m_UnDeformedPos;
        GFPhysVector3 p1 = m_SutureThreadNodes[n + 1].m_UnDeformedPos;

        Real segLen = (p0 - p1).Length();

        partsum += segLen;

        if (segLen > GP_EPSILON && partsum > currlen)
        {
            index = n;
            weight = GPClamped((partsum - currlen) / segLen, 0.0f, 1.0f);
            return true;

        }

        if (fabsf(partsum - currlen) < GP_EPSILON)
        {
            index = n;
            weight = 0.0f;
            return true;
        }
    }
#else
    //input filter missing
    Real length = GetTotalLen(true);
    currlen = GPClamped(currlen, 0.0f, length);

    if (fabsf(currlen) < GP_EPSILON)
    {
        index = 0;
        weight = 1.0f;
        return true;
    }
	if (currlen > length - GP_EPSILON)
	{
		index = GetNumSegments();
		weight = 1.0f;
		return true;
	}

    Real partsum = 0.0f;
	for (int n = 0, ni = m_SutureThreadNodes.size() - 1; n < ni; ++n)
    {
        GFPhysVector3 p0 = m_SutureThreadNodes[n].m_CurrPosition;
        GFPhysVector3 p1 = m_SutureThreadNodes[n + 1].m_CurrPosition;

        Real segLen = (p0 - p1).Length();

        partsum += segLen;

        if (segLen > GP_EPSILON && partsum > currlen)
        {
            index = n;
            weight = GPClamped((partsum - currlen) / segLen, 0.0f, 1.0f);
            return true;
        }

        if (fabsf(partsum - currlen) < GP_EPSILON)
        {
            index = n;
            weight = 0.0f;
            return true;
        }
    }
#endif
	
	return false;
}
//================================================================================================
int SutureThread::GetNumThreadNodes()
{
	return m_SutureThreadNodes.size();
}
//================================================================================================
int SutureThread::GetNumSegments()
{
	return (int)m_SutureThreadNodes.size() - 1;
}
//================================================================================================
SutureRopeNode SutureThread::GetThreadNode(int NodeIndex)
{	
    if (NodeIndex >= 0 && NodeIndex < (int)m_SutureThreadNodes.size())
    {
        return m_SutureThreadNodes[NodeIndex];
    }
    else
    {
		SY_ASSERT(0 && "GetThreadNode Node Index Not InRange");
        return m_NullNode;
    }
}
//================================================================================================
SutureRopeNode & SutureThread::GetThreadNodeRef(int NodeIndex)
{
    if (NodeIndex >= 0 && NodeIndex < (int)m_SutureThreadNodes.size())
    {
        return m_SutureThreadNodes[NodeIndex];
    }
    else
    {
		SY_ASSERT(0 && "GetThreadNode Node Index Not InRange");
		return m_NullNode;
    }
}
//================================================================================================
SutureRopeNode & SutureThread::GetThreadNodeGlobalRef(int NodeGlobalIndex, int& index)
{
    for (int i = m_SutureThreadNodes.size() - 1; i >= 0; i--)
    {
        if (m_SutureThreadNodes[i].m_GlobalId == NodeGlobalIndex)
        {
            index = i;
            return m_SutureThreadNodes[i];
        }
    }
	//MXASSERT(0 && "GetThreadNode Node Index Not InRange");
	return m_NullNode;
}
//================================================================================================
void SutureThread::InsertOneNode(int i, Real wi)
{    
    if (i > (int)(m_SutureThreadNodes.size() - 2))
    {
        return;
    }
    int j = i + 1;
    Real wj = 1.0f - wi;
    SutureRopeNode & nodei = GetThreadNodeRef(i);
    SutureRopeNode & nodej = GetThreadNodeRef(j);
    GFPhysVector3 pos = nodei.m_CurrPosition*wi + nodej.m_CurrPosition*wj;    

    SutureRopeNode newnode = SutureRopeNode(pos);
    //////////////////////////////////////////////////////////////////////////
    Real invmass = nodei.GetInvMass();
    newnode.SetInvMass(invmass);
        
    newnode.m_Velocity = nodei.m_Velocity * wi + nodej.m_Velocity * wj;
    newnode.m_MaterialVector = nodei.m_MaterialVector * wi + nodej.m_MaterialVector * wj;
    newnode.m_StretchLen = m_Rest_Length;

    newnode.m_BendSolveHardness = 1.0f;

    newnode.m_LambdaStretch = nodei.m_LambdaStretch * wi + nodej.m_LambdaStretch * wj;
    newnode.m_LambdaBend = nodei.m_LambdaBend * wi + nodej.m_LambdaBend * wj;

	//在结的两侧新增节点时，GlobalId分配要采取不同的策略，保证大的GlobalId靠近结
	//newnode.m_GlobalId = nodei.m_GlobalId;

    m_SutureThreadNodes.insert(i,newnode);
}
//================================================================================================
const std::vector<TFCollidePair> & SutureThread::GetCollidePairs()
{
	return m_TFCollidePair;
}
const GFPhysAlignedVectorObj<TRCollidePair> & SutureThread::GetCollidePairsWithRigid()
{
	return m_TRCollidePair;
}
const GFPhysAlignedVectorObj<STSTCollidePair> & SutureThread::GetCollidePairsWithThread()
{
	return m_TTCollidepair;
}
//================================================================================================
Ogre::Vector3 SutureThread::GetInterplotPointPos(Ogre::Vector3 P1, Ogre::Vector3 P2, Ogre::Vector3 P3, Ogre::Vector3 P4, Real t, Real tao)
{
	Real a = -tao*t + 2 * tao*t*t - tao*t*t*t;
	Real b = 1 + (tao - 3)*t*t + (2 - tao)*t*t*t;
	Real c = tao*t + (3 - 2 * tao)*t*t + (tao - 2)*t*t*t;
	Real d = -tao *t*t + tao*t*t*t;
	return P1*a + P2*b + P3*c + P4*d;
}
//================================================================================================
void SutureThread::SetRopeAnchorIndex(GFPhysVectorObj<Real> nodeindex)
{
    m_RopeAnchorIndexVec = nodeindex;
}
//================================================================================================
GFPhysVectorObj<Real> SutureThread::GetRopeAnchorIndex()
{
    return m_RopeAnchorIndexVec;
}
//================================================================================================
void SutureThread::BuildKnotPoint(const KnotInSutureRope& knot, std::vector<Ogre::Vector3> & rendpoints1, std::vector<Ogre::Vector3> & rendpoints2)
{
    //not need new render
    int a0 = knot.m_knotcon0.m_A;
    int a1 = knot.m_knotcon1.m_A;
    //need new render
    int b0 = knot.m_knotcon0.m_B;
    int b1 = knot.m_knotcon1.m_B;

    Real angle = knot.m_Angle;

    GFPhysVector3 S0, S1;
    GetThreadSegmentNodePos(S0, S1, a0 - 1);

    //GFPhysVector3 M0, M1;
    //GetThreadSegmentNodePos(M0, M1, b0 - 1);

    GFPhysVector3 point0 = S1;

    GFPhysVector3 L0, L1;
    GetThreadSegmentNodePos(L0, L1, a1);

    GFPhysVector3 K0, K1;
    GetThreadSegmentNodePos(K0, K1, b1);

    GFPhysVector3 point1 = K0;

    bool clockwise = knot.m_Clockwise;

    GFPhysVector3 InitRotVec = (clockwise ? (S1 - S0).Cross(L1 - L0) : (L1 - L0).Cross(S1 - S0));
    InitRotVec.Normalize();

    GFPhysVector3 InitAxis = S1 - S0;
    InitAxis.Normalize();

    Real num = 10.0f;
    Real expandcoff = 1.2f;
   
    Real k = 0.0f;

    for (Real theta = 0; theta < angle; theta += angle / num )
    {
        GFPhysQuaternion LocalRot(InitAxis, theta);
        GFPhysVector3 CurrRotVec = QuatRotate(LocalRot, InitRotVec);
        GFPhysVector3 offset = CurrRotVec * expandcoff * m_RopeRendRadius;

        GFPhysVector3 rendercenter = point0 * (1.0f - k) + point1 * k;

        rendpoints1.push_back(GPVec3ToOgre(rendercenter - offset));
        rendpoints2.push_back(GPVec3ToOgre(rendercenter + offset));
        
        k += 1.0f / num ;
    }

    //k = 0.0f;
    //for (Real theta = angle / num; theta < angle ; theta += angle / num)
    //{
    //    GFPhysQuaternion LocalRot(InitAxis, theta);
    //    GFPhysVector3 CurrRotVec = QuatRotate(LocalRot, InitRotVec);
    //    GFPhysVector3 offset = CurrRotVec*expandcoff * m_RopeRendRadius;
    //    GFPhysVector3 rendercenter = point0 * (1.0f - k) + point1 * k;
    //    rendpoints2.push_back(GPVec3ToOgre(rendercenter + offset));
    //    k += 1.0f / num;
    //}
}
//===================================================================================================
void SutureThread::UpdateMesh()
{



	GFPhysVectorObj<Ogre::Vector3> RendNodes;
	for (size_t n = 0; n < m_SutureThreadNodes.size(); n++)
	{
		if (m_SutureThreadNodes[n].m_bVisual)
		{
			GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
			RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
		}
	}
	m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);

	return;

    if(m_KnotsInThread)
    {
        GFPhysVectorObj<KnotInSutureRope> allKnots;
        GFPhysVectorObj<KnotInSutureRope> deadKnots;
        m_KnotsInThread->GetAllKnots(allKnots);
        m_KnotsInThread->GetDeadKnots(deadKnots);

        GFPhysVectorObj<GFPhysFaceRopeAnchor*> activeRopeAnchors;
        m_NeedleAttchedThread->GetFaceRopeAnchors(activeRopeAnchors);

#if KNOTRENDER

        if (1 == allKnots.size())
        {
            GFPhysVectorObj<Ogre::Vector3> rendPoints1;
            GFPhysVectorObj<Ogre::Vector3> rendPoints2;
            BuildKnotPoint(allKnots[0], rendPoints1, rendPoints2);

            if (rendPoints1.size() == 0 || rendPoints2.size() == 0)
            {
                return;
            }

            GFPhysVectorObj<Ogre::Vector3> RendNodes;
			GFPhysVectorObj<Ogre::Vector3> RendNodes1;
			GFPhysVectorObj<Ogre::Vector3> RendNodes2;

            for (int n = 0, ni = allKnots[0].m_knotcon0.m_A; n < ni; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }
            for (int n = 0,ni= (int)rendPoints1.size() - 1; n <ni; n++)
            {
                RendNodes.push_back(rendPoints1[n]);
            }
            for (int n = allKnots[0].m_knotcon1.m_A, ni = activeRopeAnchors[activeRopeAnchors.size() - 1]->GetSegIndex() + 1; n < ni /*allKnots[0].m_knotcon0.m_B+1*/; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }                
            }

            GFPhysSoftBodyFace* face = activeRopeAnchors[activeRopeAnchors.size() - 1]->GetFace();
            GFPhysVector3 ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[0]
                + face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[1]
                + face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[2];
            RendNodes.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));

			if (activeRopeAnchors.size() == 4)
			{
				GFPhysSoftBodyFace* face = activeRopeAnchors[1]->GetFace();
				GFPhysVector3 ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[1]->m_weights[0]
					+ face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[1]->m_weights[1]
					+ face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[1]->m_weights[2];
				RendNodes2.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));


				face = activeRopeAnchors[2]->GetFace();
				ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[2]->m_weights[0]
					+ face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[2]->m_weights[1]
					+ face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[2]->m_weights[2];
				RendNodes2.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));
			}

            face = activeRopeAnchors[0]->GetFace();
            ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[0]->m_weights[0]
                + face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[0]->m_weights[1]
                + face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[0]->m_weights[2];
            RendNodes1.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));

            for (int n = activeRopeAnchors[0]->GetSegIndex() + 1, ni = allKnots[0].m_knotcon0.m_B + 1; n <ni; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes1.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }

            for (size_t n = 1, ni = rendPoints2.size(); n <ni; n++)
            {
                RendNodes1.push_back(rendPoints2[n]);
            }
            for (int n = allKnots[0].m_knotcon1.m_B + 1, ni = (int)m_SutureThreadNodes.size(); n < ni; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes1.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }


            if (deadKnots.size() == 1)
            {
                m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius/*, "green"*/);
                m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius/*, "green"*/);
				m_RendObject2.UpdateRendSegment(RendNodes2, m_RopeRendRadius/*, "green"*/);

            }
            else
            {
                if (allKnots[0].m_slack)
                {
                    m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius, "green"); 
                    m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius, "green");
					m_RendObject2.UpdateRendSegment(RendNodes2, m_RopeRendRadius, "green");
                } 
                else
                {
                    m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius, "red");
                    m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius, "red");
					m_RendObject2.UpdateRendSegment(RendNodes2, m_RopeRendRadius, "red");
                }
            }

        }
        else if (2 == allKnots.size())
        {
            GFPhysVectorObj<Ogre::Vector3> rendPoints00;
            GFPhysVectorObj<Ogre::Vector3> rendPoints01;
            BuildKnotPoint(allKnots[0], rendPoints00, rendPoints01);
            if (rendPoints00.size() == 0 || rendPoints01.size() == 0)
            {
                return;
            }
            GFPhysVectorObj<Ogre::Vector3> rendPoints10;
            GFPhysVectorObj<Ogre::Vector3> rendPoints11;
            BuildKnotPoint(allKnots[1], rendPoints10, rendPoints11);
            if (rendPoints10.size() == 0 || rendPoints11.size() == 0)
            {
                return;
            }
            GFPhysVectorObj<Ogre::Vector3> RendNodes;
			GFPhysVectorObj<Ogre::Vector3> RendNodes1;
			GFPhysVectorObj<Ogre::Vector3> RendNodes2;

            for (int n = 0, ni = allKnots[1].m_knotcon0.m_A; n <ni; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }
            for (int n = 0, ni = (int)rendPoints10.size() - 1; n <ni; n++)
            {
                RendNodes.push_back(rendPoints10[n]);
            }
            for (int n = allKnots[1].m_knotcon1.m_A, ni = allKnots[0].m_knotcon0.m_A; n <ni; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }

            for (int n = 0, ni = (int)rendPoints00.size() - 1; n <ni; n++)
            {
                RendNodes.push_back(rendPoints00[n]);
            }

            for (int n = allKnots[0].m_knotcon1.m_A, ni = activeRopeAnchors[activeRopeAnchors.size() - 1]->GetSegIndex() + 1; n <ni; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }

            GFPhysSoftBodyFace* face = activeRopeAnchors[activeRopeAnchors.size() - 1]->GetFace();
            GFPhysVector3 ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[0]
                + face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[1]
                + face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[2];
            RendNodes.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));

			if (activeRopeAnchors.size() == 4)
			{
				GFPhysSoftBodyFace* face = activeRopeAnchors[1]->GetFace();
				GFPhysVector3 ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[1]->m_weights[0]
					+ face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[1]->m_weights[1]
					+ face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[1]->m_weights[2];
				RendNodes2.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));


				face = activeRopeAnchors[2]->GetFace();
				ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[2]->m_weights[0]
					+ face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[2]->m_weights[1]
					+ face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[2]->m_weights[2];
				RendNodes2.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));
			}
            face = activeRopeAnchors[0]->GetFace();
            ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[0]->m_weights[0]
                + face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[0]->m_weights[1]
                + face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[0]->m_weights[2];
            RendNodes1.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));

            for (int n = activeRopeAnchors[0]->GetSegIndex() + 1, ni = allKnots[0].m_knotcon0.m_B + 1; n <ni; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes1.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }
            for (size_t n = 1, ni = rendPoints01.size(); n <ni; n++)
            {
                RendNodes1.push_back(rendPoints01[n]);
            }
            for (int n = allKnots[0].m_knotcon1.m_B + 1, ni = allKnots[1].m_knotcon0.m_B; n <ni; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes1.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }
            for (size_t n = 1, ni = rendPoints11.size(); n <ni; n++)
            {
                RendNodes1.push_back(rendPoints11[n]);
            }
            for (int n = allKnots[1].m_knotcon1.m_B + 1, ni = (int)m_SutureThreadNodes.size(); n < ni; n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes1.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }
//////////////////////////////////////////////////////////////////////////


            if (deadKnots.size() == 2)
            {
                m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius/*, "green"*/);
				m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius/*, "green"*/);
				m_RendObject2.UpdateRendSegment(RendNodes2, m_RopeRendRadius/*, "green"*/);
            }
            else
            {
                if (allKnots[1].m_slack)
                {
                    m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius, "green");
					m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius, "green");
					m_RendObject2.UpdateRendSegment(RendNodes2, m_RopeRendRadius, "green");
                }
                else
                {
                    m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius, "red");
					m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius, "red");
					m_RendObject2.UpdateRendSegment(RendNodes2, m_RopeRendRadius, "red");
                }
            }
        }
        else
        {

            GFPhysVectorObj<GFPhysFaceRopeAnchor*> activeRopeAnchors;
            if (m_NeedleAttchedThread)
            {
                m_NeedleAttchedThread->GetFaceRopeAnchors(activeRopeAnchors);

                if (activeRopeAnchors.size() == 0)
                {
                    GFPhysVectorObj<Ogre::Vector3> RendNodes;
                    for (size_t n = 0, ni = m_SutureThreadNodes.size(); n < ni; n++)
                    {
                        if (m_SutureThreadNodes[n].m_bVisual)
                        {
                            GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                            RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                        }
                    }
                    m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);

                    GFPhysVectorObj<Ogre::Vector3> RendNodes1;
                    m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius);
                }
                else if (activeRopeAnchors.size() == 1)
                {

                    GFPhysVectorObj<Ogre::Vector3> RendNodes;
                    for (size_t n = 0, ni = m_SutureThreadNodes.size(); n <ni; n++)
                    {
                        if (m_SutureThreadNodes[n].m_bVisual)
                        {
                            GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                            RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                        }
                    }
                    m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);
                    GFPhysVectorObj<Ogre::Vector3> RendNodes1;
                    m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius);
                }
                else if (activeRopeAnchors.size() == 2)
                {
                    GFPhysVectorObj<Ogre::Vector3> RendNodes0;

                    GFPhysSoftBodyFace* face = activeRopeAnchors[0]->GetFace();
                    GFPhysVector3 ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[0]->m_weights[0]
                        + face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[0]->m_weights[1]
                        + face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[0]->m_weights[2];
                    RendNodes0.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));

                    for (size_t n = activeRopeAnchors[0]->GetSegIndex() + 1, ni = m_SutureThreadNodes.size(); n <ni; n++)
                    {                        
                        if (m_SutureThreadNodes[n].m_bVisual)
                        {
                            GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                            RendNodes0.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                        }
                    }
                    if (activeRopeAnchors[0]->GetSegIndex() > 0.9f*(int)m_SutureThreadNodes.size())
                    {
                        m_RendObject.UpdateRendSegment(RendNodes0, m_RopeRendRadius, "red");
                    }
                    else
                    {
                        m_RendObject.UpdateRendSegment(RendNodes0, m_RopeRendRadius);
                    }

                    GFPhysVectorObj<Ogre::Vector3> RendNodes1;
                    for (int n = 0, ni = activeRopeAnchors[1]->GetSegIndex() + 1; n <ni; n++)
                    {
                        if (m_SutureThreadNodes[n].m_bVisual)
                        {
                            GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                            RendNodes1.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                        }                        
                    }

                    face = activeRopeAnchors[1]->GetFace();
                    ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[0]
                        + face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[1]
                        + face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[activeRopeAnchors.size() - 1]->m_weights[2];
                    RendNodes1.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));

                    m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius);
                }
                else
                {
                    GFPhysVectorObj<Ogre::Vector3> RendNodes0;
                    for (size_t n = activeRopeAnchors[0]->GetSegIndex() + 1, ni = m_SutureThreadNodes.size(); n <ni; n++)
                    {
                        if (m_SutureThreadNodes[n].m_bVisual)
                        {
                            GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                            RendNodes0.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                        }
                    }
                    

                    if (activeRopeAnchors[0]->GetSegIndex() > 0.9f*(int)m_SutureThreadNodes.size())
                    {
                        m_RendObject.UpdateRendSegment(RendNodes0, m_RopeRendRadius, "red");
                    }
                    else
                    {
                        m_RendObject.UpdateRendSegment(RendNodes0, m_RopeRendRadius);
                    }

                    GFPhysVectorObj<Ogre::Vector3> RendNodes1;
                    for (int n = 0, ni = activeRopeAnchors[0]->GetSegIndex() + 2; n < ni; n++)
                    {
                        if (m_SutureThreadNodes[n].m_bVisual)
                        {
                            GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                            RendNodes1.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                        }
                    }
                    m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius);
                }
			}
        }
   

#else

        if (1 == allKnots.size())
        {
            GFPhysVectorObj<Ogre::Vector3> RendNodes;
            for (size_t n = 0; n < m_SutureThreadNodes.size(); n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }
            m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);

            if (deadKnots.size() == 1)
            {
                m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius, "green");
            }
            else
            {
                if (allKnots[0].m_slack)
                {
                    m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius, "green");
                } 
                else
                {
                    m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius, "red");
                }
            }

        }
        else if (2 == allKnots.size())
        {
            GFPhysVectorObj<Ogre::Vector3> RendNodes;
            for (size_t n = 0; n < m_SutureThreadNodes.size(); n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }
            m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);

            if (deadKnots.size() == 2)
            {
                m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius, "green");
            }
            else
            {
                m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius, "red");
            }
        }
        else
        {
            GFPhysVectorObj<Ogre::Vector3> RendNodes;
            for (size_t n = 0; n < m_SutureThreadNodes.size(); n++)
            {
                if (m_SutureThreadNodes[n].m_bVisual)
                {
                    GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                    RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                }
            }
            m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);
        }
#endif
    }
    else
    {        
        GFPhysVectorObj<GFPhysFaceRopeAnchor*> activeRopeAnchors;
        if (m_NeedleAttchedThread)
        {
            m_NeedleAttchedThread->GetFaceRopeAnchors(activeRopeAnchors);

            if (activeRopeAnchors.size() == 0)
            {
                GFPhysVectorObj<Ogre::Vector3> RendNodes;
                for (size_t n = 0; n < m_SutureThreadNodes.size(); n++)
                {
                    if (m_SutureThreadNodes[n].m_bVisual)
                    {
                        GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                        RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                    }
                }
                m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);

                GFPhysVectorObj<Ogre::Vector3> RendNodes1;
                m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius);
            }
            else if (activeRopeAnchors.size() == 1)
            {

                GFPhysVectorObj<Ogre::Vector3> RendNodes;
                for (size_t n = 0; n < m_SutureThreadNodes.size(); n++)
                {
                    if (m_SutureThreadNodes[n].m_bVisual)
                    {
                        GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                        RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                    }
                }
                m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);
                GFPhysVectorObj<Ogre::Vector3> RendNodes1;
                m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius);
            }
            else if (activeRopeAnchors.size() == 2)
            {
                GFPhysVectorObj<Ogre::Vector3> RendNodes0;

                GFPhysSoftBodyFace* face = activeRopeAnchors[0]->GetFace();
                GFPhysVector3 ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[0]->m_weights[0]
                    + face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[0]->m_weights[1]
                    + face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[0]->m_weights[2];
                RendNodes0.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));

                for (size_t n = activeRopeAnchors[0]->GetSegIndex()+1; n < m_SutureThreadNodes.size(); n++)
                {
                    if (m_SutureThreadNodes[n].m_bVisual)
                    {
                        GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                        RendNodes0.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                    }
                }
                m_RendObject.UpdateRendSegment(RendNodes0, m_RopeRendRadius);

                GFPhysVectorObj<Ogre::Vector3> RendNodes1;
                for (int n = 0; n < activeRopeAnchors[1]->GetSegIndex()+1; n++)
                {
                    if (m_SutureThreadNodes[n].m_bVisual)
                    {
                        GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                        RendNodes1.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                    }
                }

                face = activeRopeAnchors[1]->GetFace();
                ptFace = face->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[1]->m_weights[0]
                    + face->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[1]->m_weights[1]
                    + face->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[1]->m_weights[2];
                RendNodes1.push_back(Ogre::Vector3(ptFace.x(), ptFace.y(), ptFace.z()));

                m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius);
            }
            else
            {
                GFPhysVectorObj<Ogre::Vector3> RendNodes;
                for (size_t n = 0; n < m_SutureThreadNodes.size(); n++)
                {
                    if (m_SutureThreadNodes[n].m_bVisual)
                    {
                        GFPhysVector3 temp = m_SutureThreadNodes[n].m_CurrPosition;
                        RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
                    }
                }
                m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);

                GFPhysVectorObj<Ogre::Vector3> RendNodes1;
                m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius);
            }
        }
        
    }
}
//============================================================================================
void SutureThread::UpdateMeshByCustomedRendNodes(std::vector<Ogre::Vector3> & RendNodes)
{

	//
	int insertPointNum = 2;

	Real delta = 1.0f / Real(insertPointNum + 1);

	std::vector<Ogre::Vector3> RefinedNodes;
	RefinedNodes.reserve(RendNodes.size() + (RendNodes.size() - 1)*insertPointNum);

	int NumSegment = RendNodes.size() - 1;
	for (int s = 0; s < NumSegment; s++)
	{
		RefinedNodes.push_back(RendNodes[s]);

		if (s > 0 && s < NumSegment - 1)
		{
			Ogre::Vector3 P1 = RendNodes[s - 1];
			Ogre::Vector3 P2 = RendNodes[s];
			Ogre::Vector3 P3 = RendNodes[s + 1];
			Ogre::Vector3 P4 = RendNodes[s + 2];

			for (int t = 1; t <= insertPointNum; t++)
			{
				Ogre::Vector3 interplotPt = GetInterplotPointPos(P1, P2, P3, P4, t*delta, 0.5f);
				RefinedNodes.push_back(interplotPt);
			}
		}
	}

	RefinedNodes.push_back(RendNodes[RendNodes.size() - 1]);//last points

	m_RendObject.UpdateRendSegment(RefinedNodes, m_RopeRendRadius);
}
//==============================================================================
void SutureThread::LockRopeNode(int index)
{
    GetThreadNodeRef(index).SetInvMass(0);// = 0.0;
}
//==============================================================================
void SutureThread::UnLockRopeNode(int index)
{
    GetThreadNodeRef(index).SetInvMass(1.0f / ThreadMass);// = 0.0;
}
//==============================================================================
void SutureThread::UpdateMaterialVector()
{
    for (int n = 0; n < (int)m_SutureThreadNodes.size() - 1; n++)
    {
        SutureRopeNode & tNode = GetThreadNodeRef(n);

        SutureRopeNode n0, n1;

        if (GetThreadSegmentNode(n0, n1, n))
        {
            GFPhysVector3 s0 = (n0.m_LastPosition - n1.m_LastPosition).Normalized();
            GFPhysVector3 s1 = (n0.m_CurrPosition - n1.m_CurrPosition).Normalized();

            if ((s1 - s0).Length() > GP_EPSILON)
            {
                GFPhysQuaternion quat = ShortestArcQuatNormalize2(s0, s1);

                tNode.m_MaterialVector = QuatRotate(quat, tNode.m_MaterialVector);
            }
        }
    }
}
//===================================================================
bool SutureThread::computeMaterialFrame(
	const GFPhysVector3& p0,
	const GFPhysVector3& p1,
	const GFPhysVector3& p2,
	GFPhysMatrix3& frame)
{
	GFPhysVector3 d3 = (p1 - p0);
	d3.Normalize();
	frame.SetColumn(2, d3);

	GFPhysVector3 d2 = (frame.GetColumn(2).Cross(p2 - p0));
	d2.Normalize();
	frame.SetColumn(1, d2);

	GFPhysVector3 d1 = frame.GetColumn(1).Cross(frame.GetColumn(2));
	frame.SetColumn(0, d1);
	return true;
}
//===================================================================
void SutureThread::SolveFixOrientation(SutureRopeNode &  va, SutureRopeNode & vb, GFPhysVector3 & orientation, Real Stiffness)
{
    Real wa = va.GetSolverInvMass();

    Real wb = vb.GetSolverInvMass();

    Real w = wa + wb;

    Real lambda = Stiffness * (va.m_CurrPosition - vb.m_CurrPosition).Dot(orientation) / w;

    va.m_CurrPosition += -lambda*wa*orientation;

    vb.m_CurrPosition += lambda*wb*orientation;
}
//===================================================================
void SutureThread::SolveFixOrientation2(SutureRopeNode &  va, SutureRopeNode & vb, GFPhysVector3 & orientation, Real Stiffness)
{	
	//Real wa = va.GetSolverInvMass();

	//Real wb = vb.GetSolverInvMass();

	//Real w = wa + wb;

	//Real lambda = Stiffness * ((va.m_CurrPosition - vb.m_CurrPosition).Dot(orientation) - 1.0f) / w;

	//va.m_CurrPosition += lambda*wa*orientation;

	//vb.m_CurrPosition += -lambda*wb*orientation;


	Real wa = va.GetSolverInvMass();

	Real wb = vb.GetSolverInvMass();

	GFPhysVector3 displace = va.m_CurrPosition - vb.m_CurrPosition;

	Real leng = displace.Length();

	Real c = (displace / leng).Dot(orientation) - 1.0f;

	GFPhysVector3 grad = orientation / leng - (orientation.Dot(displace) / leng / leng / leng)* displace;

	Real lambda = Stiffness * c / (wa*grad.Length2() + wb *grad.Length2());

	va.m_CurrPosition += -lambda*wa*grad;

	vb.m_CurrPosition += lambda*wb*grad;

	//qDebug() << "dleta va " << ":" << -lambda*wa*grad.GetX() << "," << -lambda*wa*grad.GetY() << "," << -lambda*wa*grad.GetZ() << ".";

}
//===================================================================
void SutureThread::SolveBend(SutureRopeNode &  va, SutureRopeNode & vb, SutureRopeNode & vc, Real Stiffness)
{
	if (m_UseBendForce == false)
		return;

	Real wa = va.GetSolverInvMass();//va.GetInvMass() * va.GetSolverInvMasScale();//(va.m_InvMass > FLT_EPSILON ? 1.0f : 0.0f);

	Real wb = vb.GetSolverInvMass();//GetInvMass() * vb.GetSolverInvMasScale();//(vb.m_InvMass > FLT_EPSILON ? 1.0f : 0.0f);

	Real wc = vc.GetSolverInvMass();//GetInvMass() * vc.GetSolverInvMasScale();//(vc.m_InvMass > FLT_EPSILON ? 1.0f : 0.0f);

	//if(va.m_FrameCollideTag)
	//	   wa *= rscollideMassScale;

	//	if(vb.m_FrameCollideTag)
	//	   wb *= rscollideMassScale;

	//	if(vc.m_FrameCollideTag)
	//	   wc *= rscollideMassScale;

	//if(va.GetSolverInvMasScale() < 0.99f || vb.GetSolverInvMasScale() < 0.99f || vc.GetSolverInvMasScale() < 0.99f)
	//{
	//	Stiffness *= 0.3f;
	//}

	GFPhysVector3 vab = va.m_CurrPosition - vb.m_CurrPosition;
	GFPhysVector3 vcb = vc.m_CurrPosition - vb.m_CurrPosition;

	Real lab = vab.Length();
	Real lcb = vcb.Length();

	if (lab * lcb == 0)
		return;

	Real invAB = 1.0f / lab;
	Real invCB = 1.0f / lcb;

	GFPhysVector3 n1 = vab*invAB;
	GFPhysVector3 n2 = vcb*invCB;

	Real d = n1.Dot(n2);

	GPClamp(d, -1.0f, 1.0f);

	Real dd = sqrtf(1 - d*d);

	GFPhysVector3 Col0 = GFPhysVector3(1 - n1.m_x*n1.m_x, -n1.m_y*n1.m_x, -n1.m_z*n1.m_x) * invAB;
	GFPhysVector3 Col1 = GFPhysVector3(-n1.m_x*n1.m_y, 1 - n1.m_y*n1.m_y, -n1.m_z*n1.m_y) * invAB;
	GFPhysVector3 Col2 = GFPhysVector3(-n1.m_x*n1.m_z, -n1.m_y*n1.m_z, 1 - n1.m_z*n1.m_z) * invAB;

	GFPhysVector3 gradVa = GFPhysVector3(Col0.Dot(n2), Col1.Dot(n2), Col2.Dot(n2));

	Col0 = GFPhysVector3(1 - n2.m_x*n2.m_x, -n2.m_y*n2.m_x, -n2.m_z*n2.m_x) * invCB;
	Col1 = GFPhysVector3(-n2.m_x*n2.m_y, 1 - n2.m_y*n2.m_y, -n2.m_z*n2.m_y) * invCB;
	Col2 = GFPhysVector3(-n2.m_x*n2.m_z, -n2.m_y*n2.m_z, 1 - n2.m_z*n2.m_z) * invCB;

	//@note !! in fact gradVc is gradVc * (-1.0f / dd)
	//but 1.0f/dd may be large and the sumgrad is square of (1.0f/dd) may be large than the Real type 
	//precise for numerical robust , we eliminate this factor(-1.0f / dd) with the denomination
	//of sum grad and , finally multiply to result
	GFPhysVector3 gradVc = GFPhysVector3(Col0.Dot(n1), Col1.Dot(n1), Col2.Dot(n1));//gradVa *= arcdx;//gradVc *= arcdx;

	GFPhysVector3 gradVb = -gradVa - gradVc;

	Real sumgrad = wa*gradVa.Length2() + wc*gradVc.Length2() + wb*gradVb.Length2();

	Real c = d+1;//minus PI(180 degree)

	if (fabsf(sumgrad) > FLT_EPSILON)// && (c > FLT_EPSILON || sumgrad > FLT_EPSILON || dd > FLT_EPSILON))//FLT_EPSILON)
	{
		Real s = c  / sumgrad;

		GFPhysVector3 deltaVA = -gradVa*(s*wa);

		GFPhysVector3 deltaVB = -gradVb*(s*wb);

		GFPhysVector3 deltaVC = -gradVc*(s*wc);

		Real detaA = deltaVA.Length();
		Real detaB = deltaVB.Length();
		Real detaC = deltaVC.Length();

		va.m_CurrPosition += deltaVA*Stiffness;
		vb.m_CurrPosition += deltaVB*Stiffness;
		vc.m_CurrPosition += deltaVC*Stiffness;
	}
}
//===================================================================
GFPhysVector3 SutureThread::SolveStretch(SutureRopeNode & Node1, SutureRopeNode & Node2, Real Stiffness, int interval)
{
	Real RestLen = m_Rest_Length*interval;//is not suit after knot slack！！！

	Real w1 = Node1.GetSolverInvMass();//GetInvMass() * Node1.GetSolverInvMasScale();//m_InvMass;

	Real w2 = Node2.GetSolverInvMass();//GetInvMass() * Node2.GetSolverInvMasScale();//m_InvMass;

	Real w = w1 + w2;

	//w1 *= Node1.GetSolverInvMasScale();

	//w2 *= Node2.GetSolverInvMasScale();

	//if(Node1.m_FrameCollideTag)
	// w1 *= rscollideMassScale;

	//if(Node2.m_FrameCollideTag)
	//   w2 *= rscollideMassScale;

	GFPhysVector3 impluseDelta(0, 0, 0);

	if (w > FLT_EPSILON)
	{
		GFPhysVector3 Diff = Node1.m_CurrPosition - Node2.m_CurrPosition;
		Real Length = Diff.Length();

		if (Length > FLT_EPSILON)
		{
			//if(Node1.m_FrameCollideTag && Node2.m_FrameCollideTag)
			//   Stiffness = 0.9f;

			Real InvLength = 1.0f / Length;

			Real Invw1w2 = 1.0f / (w1 + w2);

			Real Temp = (Length - RestLen) * InvLength;///Length;

			GFPhysVector3 Deta1 = -(w1 * Invw1w2) * Temp * Diff;

			GFPhysVector3 Deta2 = (w2 * Invw1w2) * Temp * Diff;

			Node1.m_CurrPosition += Deta1*Stiffness;

			Node2.m_CurrPosition += Deta2*Stiffness;

			if (Length - RestLen > 0)
				impluseDelta = Diff*InvLength*(Length - RestLen)*Stiffness;
		}
	}
	return impluseDelta;
}
//=============================================================================================
//void SutureThread::SolveAttachment(GFPhysSoftBodyFace * attachFace , Real weights[3] , Real stiffness)
//{
//	GFPhysVector3 pointInFace = attachFace->m_Nodes[0]->m_CurrPosition*weights[0]
//							   +attachFace->m_Nodes[1]->m_CurrPosition*weights[1]
//							   +attachFace->m_Nodes[2]->m_CurrPosition*weights[2];
//
//	SutureRopeNode & nodeAttach = m_SutureThreadNodes[m_SutureThreadNodes.size()-1];
//
//	GFPhysVector3 Diff = (pointInFace-nodeAttach.m_CurrPosition);
//
//	Real diffLen = Diff.Length();
//
//	if(diffLen > FLT_EPSILON)
//	{	
//		Diff /= diffLen;
//
//		Real faceInvMass[3];
//		faceInvMass[0] = 1.0f;//attachFace->m_Nodes[0]->m_InvM;
//		faceInvMass[1] = 1.0f;//attachFace->m_Nodes[1]->m_InvM;
//		faceInvMass[2] = 1.0f;//attachFace->m_Nodes[2]->m_InvM;
//
//		Real nodeInvMass =  1.0f;///m_SolveMassScale;//(faceInvMass[0]+faceInvMass[1]+faceInvMass[2]);//*0.33f*0.2f;
//
//		Real sumgrad =  weights[0]*weights[0]*faceInvMass[0]
//						+weights[1]*weights[1]*faceInvMass[1]
//						+weights[2]*weights[2]*faceInvMass[2]
//						+nodeInvMass;
//
//		Real scale = stiffness*(-diffLen) / sumgrad;
//
//		GFPhysVector3 grad00 = Diff*weights[0];
//		GFPhysVector3 grad01 = Diff*weights[1];
//		GFPhysVector3 grad02 = Diff*weights[2];
//
//		GFPhysVector3 gradNoda = -Diff;
//
//		GFPhysVector3 delta00 = scale*grad00*faceInvMass[0];
//		GFPhysVector3 delta01 = scale*grad01*faceInvMass[1];
//		GFPhysVector3 delta02 = scale*grad02*faceInvMass[2];
//		GFPhysVector3 deltaNode = scale*gradNoda*nodeInvMass;
//
//		attachFace->m_Nodes[0]->m_CurrPosition += delta00;
//		attachFace->m_Nodes[1]->m_CurrPosition += delta01;
//		attachFace->m_Nodes[2]->m_CurrPosition += delta02;
//
//		nodeAttach.m_CurrPosition += deltaNode;
//	}
//}
//===================================================================================================
GFPhysVector3 SutureThread::SolveEECollide(const GFPhysVector3 & collideNormal,
	GFPhysSoftBodyNode * e1,
	GFPhysSoftBodyNode * e2,
	SutureRopeNode & e3,
	SutureRopeNode & e4,
	const TFCollidePair & cdPair)
{
	if (e1->m_InvM < GP_EPSILON || e2->m_InvM < GP_EPSILON || e3.GetSolverInvMass()  < GP_EPSILON || e4.GetSolverInvMass() < GP_EPSILON)
	{
		return GFPhysVector3(0, 0, 0);
	}
	Real Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);
	GFPhysVector3 impluse(0, 0, 0);

	SutureRopeNode * tNodes[2];
	GFPhysSoftBodyNode * sNodes[4];
	tNodes[0] = &e3;
	tNodes[1] = &e4;

	sNodes[0] = cdPair.m_CollideNode[0];
	sNodes[1] = cdPair.m_CollideNode[1];
	sNodes[2] = cdPair.m_CollideNode[2];
	sNodes[3] = cdPair.m_CollideNode[3];

	for (int t = 0; t < 2; t++)
	{
		for (int s = 0; s < cdPair.m_NumCollideNode; s++)
		{
			GFPhysVector3 tpos = tNodes[t]->m_CurrPosition;
			GFPhysVector3 spos = sNodes[s]->m_CurrPosition;
			Real distCurr = (tpos - spos).Dot(collideNormal) - m_RopeCollideRadius;
			if (distCurr < 0)//solve constraint when negative
			{
				distCurr *= Stiffness;
				if (distCurr < -0.3f)
					distCurr = -0.3f;

				Real wt = tNodes[t]->GetInvMass() > 0.0f ? 1.0f / m_SolveMassScale : 0.0f;
				Real ws = sNodes[s]->m_InvM > 0.0f ? 1.0f : 0.0f;

				Real sumInvW = wt + ws;

				if (sumInvW > FLT_EPSILON)
				{
					Real moveThread = -(wt / sumInvW)*distCurr;
					Real moveFaceNoda = (ws / sumInvW)*distCurr;

					tNodes[t]->m_CurrPosition += collideNormal*moveThread;
					sNodes[s]->m_CurrPosition += collideNormal*moveFaceNoda;

					impluse += collideNormal*moveThread;
				}
			}
		}
	}
	return impluse;
}
//===================================================================================================
GFPhysVector3 SutureThread::SolveVTCollide(const GFPhysVector3 & collideNormal, GFPhysSoftBodyNode * n0, GFPhysSoftBodyNode * n1, GFPhysSoftBodyNode * n2, SutureRopeNode & tn, const TFCollidePair & cdPair)
{
	if (n0->m_InvM < GP_EPSILON || n1->m_InvM < GP_EPSILON || n2->m_InvM < GP_EPSILON || tn.GetSolverInvMass() < GP_EPSILON)
    {
        return GFPhysVector3(0, 0, 0);
    }

	Real Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);

	GFPhysVector3 impluse(0, 0, 0);

	GFPhysVector3 CollideNodePos[4];
	Real invMassNode[4];
	GFPhysSoftBodyNode * CollideNodes[4];

	for (int s = 0; s < 4; s++)
	{
		CollideNodes[s] = cdPair.m_CollideNode[s];
		CollideNodePos[s] = CollideNodes[s]->m_CurrPosition;
		invMassNode[s] = (CollideNodes[s]->m_InvM > 0.0f ? 1.0f : 0.0f);
	}

	for (int n = 0; n < cdPair.m_NumCollideNode; n++)
	{
		GFPhysVector3 q = tn.m_CurrPosition - CollideNodePos[n];
		Real nDotQ = collideNormal.Dot(q);
		Real distCurr = nDotQ - m_RopeCollideRadius;

		if (distCurr < 0)//solve constraint when negative
		{
			distCurr *= Stiffness;

			if (distCurr < -0.3f)
				distCurr = -0.3f;

			Real InvThreadMass = (tn.GetInvMass() > 0.0f ? 1.0f : 0.0f) / m_SolveMassScale;

			Real SumInvMass = invMassNode[n] + InvThreadMass;

			if (SumInvMass > FLT_EPSILON)
			{
				Real moveThread = -(InvThreadMass / SumInvMass)*distCurr;
				Real moveFaceNoda = (invMassNode[n] / SumInvMass)*distCurr;

				tn.m_CurrPosition += collideNormal*moveThread;
				CollideNodes[n]->m_CurrPosition += collideNormal*moveFaceNoda;

				impluse += collideNormal*moveThread;
			}
		}
	}

	return impluse;
}
//============================================================================================
GFPhysVector3 SutureThread::SolveEFCollide(const GFPhysVector3 & collideNormal,
	GFPhysSoftBodyNode * v0,
	GFPhysSoftBodyNode * v1,
	GFPhysSoftBodyNode * v2,
	SutureRopeNode & t0,
	SutureRopeNode & t1,
	const TFCollidePair & cdPair)
{

	if (v0->m_InvM < GP_EPSILON || v1->m_InvM < GP_EPSILON || v2->m_InvM < GP_EPSILON || t0.GetSolverInvMass() < GP_EPSILON || t1.GetSolverInvMass() < GP_EPSILON)
    {
        return GFPhysVector3(0, 0, 0);
    }
	GFPhysVector3 impluse(0, 0, 0);

	Real faceInvMass = 4.0f;//temp
	Real threadInvMass = 1.0f;//temp

	GFPhysVector3 gradt0 = cdPair.m_ThreadWeigths[0] * collideNormal;
	GFPhysVector3 gradt1 = cdPair.m_ThreadWeigths[1] * collideNormal;

	GFPhysVector3 gradf0 = -cdPair.m_FaceWeihts[0] * collideNormal;
	GFPhysVector3 gradf1 = -cdPair.m_FaceWeihts[1] * collideNormal;
	GFPhysVector3 gradf2 = -cdPair.m_FaceWeihts[2] * collideNormal;

	Real sumGrad = cdPair.m_ThreadWeigths[0] * cdPair.m_ThreadWeigths[0] * threadInvMass
		+ cdPair.m_ThreadWeigths[1] * cdPair.m_ThreadWeigths[1] * threadInvMass
		+ cdPair.m_FaceWeihts[0] * cdPair.m_FaceWeihts[0] * faceInvMass
		+ cdPair.m_FaceWeihts[1] * cdPair.m_FaceWeihts[1] * faceInvMass
		+ cdPair.m_FaceWeihts[2] * cdPair.m_FaceWeihts[2] * faceInvMass;


	GFPhysVector3 ptThread = t0.m_CurrPosition * cdPair.m_ThreadWeigths[0]
		+ t1.m_CurrPosition * cdPair.m_ThreadWeigths[1];

	GFPhysVector3 ptFace = v0->m_CurrPosition * cdPair.m_FaceWeihts[0]
		+ v1->m_CurrPosition * cdPair.m_FaceWeihts[1]
		+ v2->m_CurrPosition * cdPair.m_FaceWeihts[2];

	Real  s_normdist = (ptThread - ptFace).Dot(collideNormal) - m_RopeCollideRadius;

	Real  threadNormalCorrect[2];
	if (sumGrad > FLT_EPSILON && s_normdist < 0)
	{
		Real s = s_normdist / sumGrad;

		s = s * (-1.0f);

		t0.m_CurrPosition += gradt0 * s * threadInvMass;
		t1.m_CurrPosition += gradt1 * s * threadInvMass;

		v0->m_CurrPosition += gradf0 * s * faceInvMass;
		v1->m_CurrPosition += gradf1 * s * faceInvMass;
		v2->m_CurrPosition += gradf2 * s * faceInvMass;

		threadNormalCorrect[0] = (gradt0 * s * threadInvMass).Length();
		threadNormalCorrect[1] = (gradt1 * s * threadInvMass).Length();

		impluse = (gradt0 * s * threadInvMass + gradt1 * s * threadInvMass);
		//tangent damping
		for (int t = 0; t < 2; t++)
		{
			SutureRopeNode & tnode = (t == 0 ? t0 : t1);

			GFPhysVector3 NodaVel = tnode.m_CurrPosition - tnode.m_LastPosition;

			GFPhysVector3 NormalVel = collideNormal*NodaVel.Dot(collideNormal);

			GFPhysVector3 TangVel = NodaVel - NormalVel;

			Real tanLen = TangVel.Length();

			if (tanLen > GP_EPSILON)
			{
				GFPhysVector3 tanVec = TangVel / tanLen;

				Real tanCorrect = threadNormalCorrect[t] * 0.9f;

				tanLen = tanLen - tanCorrect;

				if (tanLen < 0)
					tanLen = 0;

				tnode.m_CurrPosition = tnode.m_LastPosition + NormalVel + tanVec*tanLen;
			}
		}

	}

	Real solverStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.8f);

	return impluse * 0.02f;
}
//===================================================================================================
void SutureThread::SolveSoftThreadCollisions()
{
	Real Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.9f);

	for (size_t c = 0; c < m_TFCollidePair.size(); c++)
	{
		TFCollidePair & cdpair = m_TFCollidePair[c];

		GFPhysSoftBodyFace * softFace = cdpair.m_SoftFace;

        if (softFace->m_Nodes[0]->m_InvM < GP_EPSILON || softFace->m_Nodes[1]->m_InvM < GP_EPSILON || softFace->m_Nodes[2]->m_InvM < GP_EPSILON)
        {
            continue;
        }

		if (cdpair.m_CollideType == TFCollidePair::TFCD_EE)
		{
			if (m_SutureThreadNodes[cdpair.m_e3].GetCanCollideSoft() && m_SutureThreadNodes[cdpair.m_e4].GetCanCollideSoft())
			{
				GFPhysVector3 impluse = SolveEECollide(
					OgreToGPVec3(cdpair.m_CollideNormal),
					softFace->m_Nodes[cdpair.m_e1],
					softFace->m_Nodes[cdpair.m_e2],
					m_SutureThreadNodes[cdpair.m_e3],
					m_SutureThreadNodes[cdpair.m_e4],
					cdpair);

				cdpair.m_ImpluseOnThread += GPVec3ToOgre(impluse);

			}
		}
		else if (cdpair.m_CollideType == TFCollidePair::TFCD_VF)
		{
			if (m_SutureThreadNodes[cdpair.m_e1].GetCanCollideSoft())
			{
				GFPhysVector3 impluse = SolveVTCollide(OgreToGPVec3(cdpair.m_CollideNormal),
					softFace->m_Nodes[0],
					softFace->m_Nodes[1],
					softFace->m_Nodes[2],
					m_SutureThreadNodes[cdpair.m_e1],
					cdpair);

				cdpair.m_ImpluseOnThread += GPVec3ToOgre(impluse);
			}
		}
		else if (cdpair.m_CollideType == TFCollidePair::TFCD_EF)
		{
			if (m_SutureThreadNodes[cdpair.m_e3].GetCanCollideSoft() && m_SutureThreadNodes[cdpair.m_e4].GetCanCollideSoft())
			{
				GFPhysVector3 impluse = SolveEFCollide(
					OgreToGPVec3(cdpair.m_CollideNormal),
					softFace->m_Nodes[0],
					softFace->m_Nodes[1],
					softFace->m_Nodes[2],
					m_SutureThreadNodes[cdpair.m_e3],
					m_SutureThreadNodes[cdpair.m_e4],
					cdpair);

				cdpair.m_ImpluseOnThread += GPVec3ToOgre(impluse);

			}
		}
	}
}
//============================================================================================================================================
void SutureThread::SolveRigidThreadCollisions(Real dt)
{
	for (size_t c = 0; c < m_TRCollidePair.size(); c++)
	{
		 TRCollidePair & trPair = m_TRCollidePair[c];

		 if (trPair.m_w0 + trPair.m_w1 <= 0)
			 continue;

		 SutureRopeNode * tNode[2];
		 tNode[0] = &(m_SutureThreadNodes[trPair.m_Segment]);
		 tNode[1] = &(m_SutureThreadNodes[trPair.m_Segment + 1]);

		 if (tNode[0]->GetSolverInvMass() < GP_EPSILON || tNode[1]->GetSolverInvMass() < GP_EPSILON)
			 continue;

		 GFPhysVector3 tPoint = tNode[0]->m_CurrPosition * trPair.m_w0 + tNode[1]->m_CurrPosition * trPair.m_w1;

		 GFPhysVector3 va = trPair.m_Rigid->GetVelocityInLocalPoint(trPair.m_R) * dt;

		 GFPhysVector3 vr = va + trPair.m_Rigid->GetCenterOfMassPosition() + trPair.m_R - tPoint;
#if 1
		 if (va.Length2() > 0.1f*0.1f)//高速运动时启用新的碰撞
		 {
			 trPair.SolveContact(dt, tNode[0]->m_CurrPosition, tNode[1]->m_CurrPosition);
			 continue;
		 }
#else
		 if (GetCustomLen(true, trPair.m_Segment, trPair.m_Segment+1) < m_Rest_Length * 1.1f)//当绳子被拉长用原来的碰撞解算
		 {
			 trPair.SolveContact(dt, tNode[0]->m_CurrPosition, tNode[1]->m_CurrPosition);
			 continue;
		 }
#endif
		 
		 //solve friction
#if(1)
		 Real frictcoeff = 1.5f;

		 Real maxfriction = frictcoeff * fabsf(trPair.m_LambdaN);
		
		 tPoint = tNode[0]->m_CurrPosition * trPair.m_w0 + tNode[1]->m_CurrPosition * trPair.m_w1;
		 
		 //va = trPair.m_Rigid->GetVelocityInLocalPoint(trPair.m_R) * dt;
		 
		 vr = (/*va +*/ trPair.m_Rigid->GetCenterOfMassPosition() + trPair.m_R - tPoint);

		 GFPhysVector3 segDir = (tNode[0]->m_LastPosition - tNode[1]->m_LastPosition).Normalized();

		 for (int c = 0; c < 2; c++)
		 {
			 Real  vt = vr.Dot(trPair.m_Tangnet[c]);

			 Real t = (trPair.m_ContactImpMatrix * trPair.m_Tangnet[c]).Dot(trPair.m_Tangnet[c]);

			 if (t > FLT_EPSILON)
			 {
				 Real ct = vt / t;

				 Real oldvalue = trPair.m_LambdaT[c];

				 trPair.m_LambdaT[c] = GPClamped(trPair.m_LambdaT[c] + ct, -maxfriction, maxfriction);//trPair.m_LambdaT[c] + ct;// 

				 GFPhysVector3 impluse = trPair.m_Tangnet[c] * (trPair.m_LambdaT[c] - oldvalue);

				 //apply impluse to node
				
				 GFPhysVector3 frictImp0 = (impluse * trPair.m_w0 * tNode[0]->GetSolverInvMass()) * dt;
				 GFPhysVector3 frictImp1 = (impluse * trPair.m_w1 * tNode[1]->GetSolverInvMass()) * dt;

				 frictImp0 = frictImp0 - segDir*frictImp0.Dot(segDir);//only apply friction perpend to axis direction
				 frictImp1 = frictImp1 - segDir*frictImp1.Dot(segDir);

				 tNode[0]->m_CurrPosition += frictImp0;// (impluse * trPair.m_w0 * tNode[0]->GetSolverInvMass()) * dt;
				 tNode[1]->m_CurrPosition += frictImp1;// (impluse * trPair.m_w1 * tNode[1]->GetSolverInvMass()) * dt;

				 //apply negative impluse on rigid
				 trPair.m_Rigid->ApplyImpulse(-impluse, trPair.m_R);

				 trPair.m_ImpluseNormalOnRigid -= impluse.Length();
			 }
		 }
#endif
		 //solve normal impluse
		 tPoint = tNode[0]->m_CurrPosition * trPair.m_w0 + tNode[1]->m_CurrPosition * trPair.m_w1;

		 va = trPair.m_Rigid->GetVelocityInLocalPoint(trPair.m_R) * dt;

		 vr = va + trPair.m_Rigid->GetCenterOfMassPosition() + trPair.m_R - tPoint;
		 

		 Real  vn = m_RopeCollideRadius + vr.Dot(trPair.m_NormalOnRigid);

		 if (vn > 0)
		 {
			 Real t = (trPair.m_ContactImpMatrix * trPair.m_NormalOnRigid).Dot(trPair.m_NormalOnRigid);

			 if (t > FLT_EPSILON)
			 {
				 Real c = vn / t;

				 trPair.m_LambdaN += c;

				 GFPhysVector3 impluse = trPair.m_NormalOnRigid * c;

				 //apply impluse to node
				 tNode[0]->m_CurrPosition += (impluse * trPair.m_w0 * tNode[0]->GetSolverInvMass()) * dt;
				 tNode[1]->m_CurrPosition += (impluse * trPair.m_w1 * tNode[1]->GetSolverInvMass()) * dt;

				 //apply negative impluse on rigid
				 trPair.m_Rigid->ApplyImpulse(-impluse, trPair.m_R);
				 trPair.m_ImpluseNormalOnRigid -= impluse.Length();

			 }
		 }
		//
	}
}
//=================================================================================================================================
void SutureThread::SolveThreadThreadCollisions()
{
#if 1
	for (size_t c = 0; c < m_TTCollidepair.size(); c++)
	{
		STSTCollidePair & ttPair = m_TTCollidepair[c];
		//if (GetKnotIndexMin() > 0)
		//{
		//	if (ttPair.m_SegmentA > GetKnotIndexMin() && ttPair.m_SegmentA + 1 < GetKnotIndexMax())
		//	{
		//		continue;
		//	}
		//	if (ttPair.m_SegmentB > GetKnotIndexMin() && ttPair.m_SegmentB + 1 < GetKnotIndexMax())
		//	{
		//		continue;
		//	}
		//}
		SutureRopeNode * tNodeA[2];
		SutureRopeNode * tNodeB[2];
		tNodeA[0] = &(m_SutureThreadNodes[ttPair.m_SegmentA]);
		tNodeA[1] = &(m_SutureThreadNodes[ttPair.m_SegmentA + 1]);

		tNodeB[0] = &(m_SutureThreadNodes[ttPair.m_SegmentB]);
		tNodeB[1] = &(m_SutureThreadNodes[ttPair.m_SegmentB + 1]);

		Real wA0 = 1.0f - ttPair.m_WeightA;
		Real wA1 = ttPair.m_WeightA;
		Real wB0 = 1.0f - ttPair.m_WeightB;
		Real wB1 = ttPair.m_WeightB;

		Real  invMassA0 = tNodeA[0]->GetSolverInvMassForSelfCollision();		
		Real  invMassA1 = tNodeA[1]->GetSolverInvMassForSelfCollision();
		Real  invMassB0 = tNodeB[0]->GetSolverInvMassForSelfCollision();		
		Real  invMassB1 = tNodeB[1]->GetSolverInvMassForSelfCollision();

		//if(isCollide0 == false)
		//{  w0 = 0; w1 = 1;}

		//if(isCollide1 == false)
		//{  w0 = 1; w1 = 0; }

		GFPhysVector3 PointA = tNodeA[0]->m_CurrPosition * wA0 + tNodeA[1]->m_CurrPosition * wA1;
		GFPhysVector3 PointB = tNodeB[0]->m_CurrPosition * wB0 + tNodeB[1]->m_CurrPosition * wB1;

		GFPhysVector3 GradA0 = wA0 * ttPair.m_NormalOnB;
		GFPhysVector3 GradA1 = wA1 * ttPair.m_NormalOnB;

		GFPhysVector3 GradB0 = -wB0 * ttPair.m_NormalOnB;
		GFPhysVector3 GradB1 = -wB1 * ttPair.m_NormalOnB;

		Real  sumGrad = invMassA0 * wA0 * wA0 + invMassA1 * wA1 * wA1
			          + invMassB0 * wB0 * wB0 + invMassB1 * wB1 * wB1;

		Real  s_normdist = 2 * m_RopeCollideRadius + GPSIMDVec3Dot(GPSIMDVec3Sub(PointB, PointA), ttPair.m_NormalOnB);

		if (sumGrad > FLT_EPSILON && s_normdist > 0)
		{
			Real s = s_normdist / sumGrad;

			tNodeA[0]->m_CurrPosition += invMassA0 * GradA0 * s;
			tNodeA[1]->m_CurrPosition += invMassA1 * GradA1 * s;

			tNodeB[0]->m_CurrPosition += invMassB0 * GradB0 * s;
			tNodeB[1]->m_CurrPosition += invMassB1 * GradB1 * s;
		}
	}
#else
	for (size_t c = 0; c < m_TTCollidepair.size(); c++)
	{
		STSTCollidePair & ttPair = m_TTCollidepair[c];
        if (GetKnotIndexMin() > 0)
        {
            if (ttPair.m_SegmentA > GetKnotIndexMin() && ttPair.m_SegmentA + 1 < GetKnotIndexMax())
            {
				continue;
            }
            if (ttPair.m_SegmentB > GetKnotIndexMin() && ttPair.m_SegmentB + 1 < GetKnotIndexMax())
            {
				continue;
            }
        }
        SutureRopeNode * tNodeA[2];
        SutureRopeNode * tNodeB[2];
		tNodeA[0] = &(m_SutureThreadNodes[ttPair.m_SegmentA]);
		tNodeA[1] = &(m_SutureThreadNodes[ttPair.m_SegmentA + 1]);

		tNodeB[0] = &(m_SutureThreadNodes[ttPair.m_SegmentB]);
		tNodeB[1] = &(m_SutureThreadNodes[ttPair.m_SegmentB + 1]);

		Real wA0 = 1.0f - ttPair.m_WeightA;
		Real wA1 = ttPair.m_WeightA;

		Real wB0 = 1.0f - ttPair.m_WeightB;
		Real wB1 = ttPair.m_WeightB;


		//if(isCollide0 == false)
		//{  w0 = 0; w1 = 1;}

		//if(isCollide1 == false)
		//{  w0 = 1; w1 = 0; }


		GFPhysVector3 PointA = tNodeA[0]->m_CurrPosition * wA0 + tNodeA[1]->m_CurrPosition * wA1;
		GFPhysVector3 PointB = tNodeB[0]->m_CurrPosition * wB0 + tNodeB[1]->m_CurrPosition * wB1;

		GFPhysVector3 GradA0 = wA0 * ttPair.m_NormalOnB;
		GFPhysVector3 GradA1 = wA1 * ttPair.m_NormalOnB;

		GFPhysVector3 GradB0 = -wB0 * ttPair.m_NormalOnB;
		GFPhysVector3 GradB1 = -wB1 * ttPair.m_NormalOnB;

		Real  sumGrad = wA0 * wA0 + wA1 * wA1 + wB0 * wB0 + wB1 * wB1;

		Real  s_normdist = 2 * m_RopeCollideRadius + GPSIMDVec3Dot(GPSIMDVec3Sub(PointB, PointA), ttPair.m_NormalOnB);

		if (sumGrad > FLT_EPSILON && s_normdist > 0)
		{
			Real s = s_normdist / sumGrad;

			tNodeA[0]->m_CurrPosition += GradA0 * s;
			tNodeA[1]->m_CurrPosition += GradA1 * s;

			tNodeB[0]->m_CurrPosition += GradB0 * s;
			tNodeB[1]->m_CurrPosition += GradB1 * s;
		}
	}
#endif
}
//==========================================================================================================================
void SutureThread::SolveStretchX(SutureRopeNode & n0, SutureRopeNode & n1, Real & lambda, Real InvStiff, Real damping, Real dt, Real RestLen, GFPhysVector3& n0_Update, GFPhysVector3& n1_Update)
{
	//Real Damping = 20.0f;
	Real w1 = n0.GetSolverInvMass();

	Real w2 = n1.GetSolverInvMass();

	Real w = w1 + w2;
#if(1)
	{
		GFPhysVector3 dampcorr0;
		GFPhysVector3 dampcorr1;
		//=================================================================================================================
		bool succeed = ApplyEdgeDamping(n0.m_CurrPosition, n1.m_CurrPosition,
			                            n0.m_LastPosition, n1.m_LastPosition,
							            w1, w2, 0.5f,
							            n0.m_TanLambda, n0.m_EdgeTan,
			                            dt,
									    dampcorr0,
									    dampcorr1);
		if (succeed)
		{			
			n0.m_CurrPosition += dampcorr0;
			n1.m_CurrPosition += dampcorr1;

			//n0_Update = dampcorr0;
			n1_Update = dampcorr1;
		}
		else
		{
			//n0_Update = GFPhysVector3(0, 0, 0);
			n1_Update = GFPhysVector3(0, 0, 0);
		}

	}
	//
#endif
	
	Real gamma = /*InvStiff **/ damping / dt;

    InvStiff /= (dt*dt);

    //damping *= (dt*dt);

	

	Real denom = (w > GP_EPSILON ? 1.0f / ((1 + gamma) * w + InvStiff) : 0.0f);

	GFPhysVector3 Grad1 = GPSIMDVec3Sub(n0.m_CurrPosition, n1.m_CurrPosition);

	Real Length = Grad1.Length();

	if (Length > GP_EPSILON)
		Grad1 /= Length;
	else
		Grad1 = GFPhysVector3(0, 0, 0);

	Real dampingPart = gamma * Grad1.Dot((n0.m_CurrPosition - n0.m_LastPosition) - (n1.m_CurrPosition - n1.m_LastPosition));

	Real dLambda = (-(Length - RestLen) - InvStiff * lambda - dampingPart) * denom;//m_Lambda must be set zero before solver first time

	//update lambda
	lambda += dLambda;

    //n0_Update += Grad1 * (w1 * dLambda);
    n1_Update += (-Grad1) * (w2 * dLambda);

    if (/*n0_Update.Length() < 0.05f ||*/ n1_Update.Length() < 0.05f)
    {
        //n0_Update = GFPhysVector3(0.0f, 0.0f, 0.0f);
        n1_Update = GFPhysVector3(0.0f, 0.0f, 0.0f);
    }

	//update position
	n0.m_CurrPosition += Grad1 * (w1 * dLambda);
	n1.m_CurrPosition += (-Grad1) * (w2 * dLambda);
}
//==========================================================================================================================
void SutureThread::SolveBendX(SutureRopeNode & va, SutureRopeNode & vb, SutureRopeNode & vc, Real & lambda, Real InvStiff, Real damping, Real solvehardness, Real dt)
{
	Real gamma = /*InvStiff **/ damping / dt;

	InvStiff /= (dt*dt);

	//damping *= (dt*dt);

	Real wa = va.GetSolverInvMass();

	Real wb = vb.GetSolverInvMass();

	Real wc = vc.GetSolverInvMass();

	GFPhysVector3 vab = va.m_CurrPosition - vb.m_CurrPosition;

	GFPhysVector3 vcb = vc.m_CurrPosition - vb.m_CurrPosition;

	Real lab = vab.Length();
	Real lcb = vcb.Length();

	if (lab * lcb == 0)
		return;

	Real invAB = 1.0f / lab;
	Real invCB = 1.0f / lcb;

	GFPhysVector3 n1 = vab*invAB;
	GFPhysVector3 n2 = vcb*invCB;

	Real d = n1.Dot(n2);

	GPClamp(d, -1.0f, 1.0f);

	Real dd = sqrtf(1 - d*d);

	GFPhysVector3 Col0 = GFPhysVector3(1 - n1.m_x*n1.m_x, -n1.m_y*n1.m_x, -n1.m_z*n1.m_x) * invAB;
	GFPhysVector3 Col1 = GFPhysVector3(-n1.m_x*n1.m_y, 1 - n1.m_y*n1.m_y, -n1.m_z*n1.m_y) * invAB;
	GFPhysVector3 Col2 = GFPhysVector3(-n1.m_x*n1.m_z, -n1.m_y*n1.m_z, 1 - n1.m_z*n1.m_z) * invAB;
	GFPhysVector3 gradVa = GFPhysVector3(Col0.Dot(n2), Col1.Dot(n2), Col2.Dot(n2));

	Col0 = GFPhysVector3(1 - n2.m_x*n2.m_x, -n2.m_y*n2.m_x, -n2.m_z*n2.m_x) * invCB;
	Col1 = GFPhysVector3(-n2.m_x*n2.m_y, 1 - n2.m_y*n2.m_y, -n2.m_z*n2.m_y) * invCB;
	Col2 = GFPhysVector3(-n2.m_x*n2.m_z, -n2.m_y*n2.m_z, 1 - n2.m_z*n2.m_z) * invCB;

	//@note !! in fact gradVc is gradVc * (-1.0f / dd)
	//but 1.0f/dd may be large and the sumgrad is square of (1.0f/dd) may be large than the Real type 
	//precise for numerical robust , we eliminate this factor(-1.0f / dd) with the denomination
	//of sum grad and , finally multiply to result
	GFPhysVector3 gradVc = GFPhysVector3(Col0.Dot(n1), Col1.Dot(n1), Col2.Dot(n1));//gradVa *= arcdx;//gradVc *= arcdx;

	GFPhysVector3 gradVb = -gradVa - gradVc;

	Real sumgrad = (wa*gradVa.Length2() + wc*gradVc.Length2() + wb*gradVb.Length2()) * (1 + gamma) + InvStiff * dd * dd;

	if (fabsf(sumgrad) > FLT_EPSILON)
	{
		Real c = acosf(d) - 3.1415926f;//minus PI(180 degree)

		Real dampcomponent = gradVa.Dot(va.m_CurrPosition - va.m_LastPosition)
			+ gradVb.Dot(vb.m_CurrPosition - vb.m_LastPosition)
			+ gradVc.Dot(vc.m_CurrPosition - vc.m_LastPosition);

		Real dlambdaPrim = ((-c - InvStiff * lambda) * dd + gamma * dampcomponent) / sumgrad;

		lambda += (dlambdaPrim * dd);

		va.m_CurrPosition += -gradVa * (dlambdaPrim*wa) * solvehardness;

		vb.m_CurrPosition += -gradVb * (dlambdaPrim*wb) * solvehardness;

		vc.m_CurrPosition += -gradVc * (dlambdaPrim*wc) * solvehardness;
	}
}
//============================================================================================================================================
void SutureThread::PrepareSolveConstraint(Real Stiffness, Real TimeStep)
{
	m_SolveItorCount = 0;
	for (int n = 0; n < (int)m_SutureThreadNodes.size(); n++)
	{
		m_SutureThreadNodes[n].m_LambdaBend = 0;
		m_SutureThreadNodes[n].m_LambdaStretch = 0;
		m_SutureThreadNodes[n].m_BendSolveHardness = 1.0f;

        if (m_SutureThreadNodes[n].m_StretchLen < GP_EPSILON)
        {
            qDebug() << "Error length";
        }
		//give an edge angular damping force this make the simulation more stable and "viscous"
		if (n < (int)m_SutureThreadNodes.size() - 1)
		{
			SutureRopeNode & n0 = m_SutureThreadNodes[n];
			SutureRopeNode & n1 = m_SutureThreadNodes[n+1];

			GFPhysVector3 direction = (n0.m_LastPosition - n1.m_LastPosition);
			Real nlen = direction.Length();
			if (nlen > 0)
			{
				direction /= nlen;
				n0.m_EdgeTan[0] = Perpendicular(direction).Normalized();
				n0.m_EdgeTan[1] = direction.Cross(n0.m_EdgeTan[0]).Normalized();
			}
			else
			{
				n0.m_EdgeTan[0] = n0.m_EdgeTan[1] = GFPhysVector3(0, 0, 0);
			}
			n0.m_TanLambda[0] = n0.m_TanLambda[1] = 0;
		}
	}


	//prepare thread-rigid contact matrix
	for (size_t c = 0; c < m_TRCollidePair.size(); c++)
	{
		TRCollidePair & trPair = m_TRCollidePair[c];

		SutureRopeNode * tNode[2];
		tNode[0] = &(m_SutureThreadNodes[trPair.m_Segment]);
		tNode[1] = &(m_SutureThreadNodes[trPair.m_Segment + 1]);

		//bacon add
		if (tNode[0]->IsAttached())
		{
			trPair.m_w0 = trPair.m_w1 = 0;
			continue;
		}

		if (tNode[1]->IsAttached())
		{
			trPair.m_w0 = trPair.m_w1 = 0;
			continue;
		}

		//end
		bool isCollide0 = (tNode[0]->GetInvMass() > GP_EPSILON);
		bool isCollide1 = (tNode[1]->GetInvMass() > GP_EPSILON);

		if (isCollide0 == false && isCollide1 == false)
			continue;

		trPair.m_w0 = 1.0f - trPair.m_SegWeight;
		trPair.m_w1 = trPair.m_SegWeight;

		if (isCollide0 == false)
		{
			trPair.m_w0 = 0;
			trPair.m_w1 = 1;
		}

		if (isCollide1 == false)
		{
			trPair.m_w0 = 1;
			trPair.m_w1 = 0;
		}

		//
		Real EdgeInvM = trPair.m_w0 * trPair.m_w0 * tNode[0]->GetSolverInvMass() + trPair.m_w1 * trPair.m_w1 * tNode[1]->GetSolverInvMass();

		//build impluse matrix etc for solve constraints
		trPair.m_R = trPair.m_Rigid->GetWorldTransform().GetBasis() * trPair.m_LocalAnchorOnRigid;
		trPair.m_ContactImpMatrix = GFPhysSoftBodyConstraint::K_DtMatrix(TimeStep,
			EdgeInvM,
			trPair.m_Rigid->GetInvMass(),
			trPair.m_Rigid->GetInvInertiaTensorWorld(),
			trPair.m_R);

		trPair.m_LambdaN = 0;
		trPair.m_LambdaT[0] = 0;
		trPair.m_LambdaT[1] = 0;

		if (trPair.m_Mode == 1)
		{
			//trPair.m_RWithRadius[0] = trPair.m_Rigid->GetWorldTransform().GetBasis() * trPair.m_LocalPtOnRigidWithRadius[0];
			//trPair.m_RWithRadius[1] = trPair.m_Rigid->GetWorldTransform().GetBasis() * trPair.m_LocalPtOnRigidWithRadius[1];

			trPair.PrepareForSolveContact(TimeStep);
		}
	}

	//knot
	if (m_KnotsInThread)
	{
		GFPhysVectorObj<KnotInSutureRope> allKnots;
		m_KnotsInThread->GetAllKnots(allKnots);
		if (allKnots.size() > 0)
		{
			int last = allKnots.size() - 1;
			Real a0 = allKnots[last].m_knotcon0.m_A + allKnots[last].m_knotcon0.m_weightA;
			Real a1 = allKnots[last].m_knotcon1.m_B + allKnots[last].m_knotcon1.m_weightB;

			for (int s = a0; s <= a1; s++)
			{
				m_SutureThreadNodes[s].m_BendSolveHardness = 0.1f;
			}
		}
	}


	Real originLen = GetTotalLen(false);
	Real currLen = GetTotalLen(true);
	m_SolveMassScale = 1.0f;
	if (currLen > originLen)
	{
		Real t = currLen / originLen;
		m_SolveMassScale *= (t*t*t);
	}
    m_KnotImpluse[0] = GFPhysVector3(0.0f, 0.0f, 0.0f);
    m_KnotImpluse[1] = GFPhysVector3(0.0f, 0.0f, 0.0f);

    for (size_t c = 0; c < m_TTCollidepair.size(); c++)
    {
        STSTCollidePair & ttPair = m_TTCollidepair[c];

        SutureRopeNode * tNodeA[2];
        SutureRopeNode * tNodeB[2];

        tNodeA[0] = &(m_SutureThreadNodes[ttPair.m_SegmentA]);
        tNodeA[1] = &(m_SutureThreadNodes[ttPair.m_SegmentA + 1]);

        tNodeB[0] = &(m_SutureThreadNodes[ttPair.m_SegmentB]);
        tNodeB[1] = &(m_SutureThreadNodes[ttPair.m_SegmentB + 1]);

        if (tNodeA[0]->GetSolverInvMasScale() == 1.0f)
        {
            tNodeA[0]->SetSolverInvMassScale(0.5f);
        }
        if (tNodeA[1]->GetSolverInvMasScale() == 1.0f)
        {
            tNodeA[1]->SetSolverInvMassScale(0.5f);
        }
        if (tNodeB[0]->GetSolverInvMasScale() == 1.0f)
        {
            tNodeB[0]->SetSolverInvMassScale(0.5f);
        }
        if (tNodeB[1]->GetSolverInvMasScale() == 1.0f)
        {
            tNodeB[1]->SetSolverInvMassScale(0.5f);
        }
    }

	m_Impluse.clear();
	m_Impluse.reserve(m_SutureThreadNodes.size() - 1);
	for (int c = 0; c < (int)m_SutureThreadNodes.size() - 1; c++)
	{
		m_Impluse.push_back(GFPhysVector3(0, 0, 0));
	}
	return;
}
//============================================================================================
void SutureThread::SolveConstraint(Real Stiffness, Real TimeStep)
{
    m_SolveItorCount++;
	//solve constraint
	Real stretchStiffness = m_RopeStrechStiffness;//1.0f;//GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(1.0f);

	Real bendStiffness = m_RopeBendStiffness;//0.9f;//GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.999f);

	Real stretchStiffnessX = 500000.0f;
	//Real stretchDampingX = 0.01f;
    Real bendStiffnessX = 550000.0f;
	//Real bendDampingX = 0.01f;
    
    GFPhysVector3 n0update = GFPhysVector3(0, 0, 0);
    GFPhysVector3 n1update = GFPhysVector3(0, 0, 0);



	for (int k = 0; k < 2; k++)
	{

#if 0
		//solve stretch constraint
		for(size_t n = 0 ; n < m_SutureThreadNodes.size()-1 ; n++)
		{	
			SolveStretch(m_SutureThreadNodes[n] , m_SutureThreadNodes[n+1] , stretchStiffness , 1);
		}

		//solve bend angle
		for(size_t n = 0 ; n < m_SutureThreadNodes.size()-2 ; n++)
		{	
			SolveBend(m_SutureThreadNodes[n] , m_SutureThreadNodes[n+1] , m_SutureThreadNodes[n+2] ,bendStiffness);
		}
#else
		//solve bend angle and distance
		for (int n = 0, ni = (int)m_SutureThreadNodes.size() - 2; n < ni; n++)
		{
			SolveBendX(m_SutureThreadNodes[n], m_SutureThreadNodes[n + 1], m_SutureThreadNodes[n + 2],
				m_SutureThreadNodes[n + 1].m_LambdaBend, bendStiffnessX > 0 ? 1.0f / bendStiffnessX : 0, m_SutureThreadNodes[n + 1].m_BendDampingX /*bendDampingX*/, m_SutureThreadNodes[n + 1].m_BendSolveHardness, TimeStep);
		}
		for (int n = 0, ni = (int)m_SutureThreadNodes.size() - 1; n < ni; n++)
		{
			SolveStretchX(m_SutureThreadNodes[n], m_SutureThreadNodes[n + 1], m_SutureThreadNodes[n].m_LambdaStretch,
				stretchStiffnessX > 0 ? 1.0f / stretchStiffnessX : 0.0f, m_SutureThreadNodes[n].m_StretchDampingX /*stretchDampingX*/, TimeStep, m_SutureThreadNodes[n].m_StretchLen, n0update, n1update);
			m_Impluse[n] += n1update;
		}
#endif		

		if (m_ClampSegIndexVector.size() > 0)
		{
			int localindex = -1;
			SutureRopeNode & node = GetThreadNodeGlobalRef(m_ClampSegIndexVector[0], localindex);
			if (-1 == localindex)
			{
				continue;
			}
			if (m_Impluse[localindex].Length2() > 1.5f)
			{
				m_move = true;
			}
		}
		

		if (m_KnotsInThread)
		{
			GFPhysVectorObj<KnotInSutureRope*> AllKnots;
			m_KnotsInThread->GetAllKnotsRef(AllKnots);

            if (AllKnots.size() > 0)
			{
				int min = AllKnots[AllKnots.size() - 1]->m_knotcon0.m_A;
				int max = AllKnots[AllKnots.size() - 1]->m_knotcon1.m_B;

				m_KnotImpluse[0] += m_Impluse[min - 1];
				m_KnotImpluse[1] += m_Impluse[max + 1];

                if (true)
                {
                    GFPhysVectorObj<GFPhysFaceRopeAnchor*> activeRopeAnchors;
                    m_NeedleAttchedThread->GetFaceRopeAnchors(activeRopeAnchors);
                    for (int j = 0, nj = (int)activeRopeAnchors.size(); j < nj; j++)
                    {
                        if (j == 0 || j == (int)activeRopeAnchors.size()-1)
                        {
#if 1
							GFPhysVector3 FaceNormal = activeRopeAnchors[j]->GetFace()->m_FaceNormal;

							GFPhysVector3 ori = Perpendicular(FaceNormal).Normalized();

							SutureRopeNode n0, n1;

							if (GetThreadSegmentNode(n0, n1, activeRopeAnchors[j]->m_NodeIndex))
							{
								SolveFixOrientation(n0, n1, ori, 0.98f);
							}
#else
							GFPhysVector3 FaceNormal = activeRopeAnchors[j]->GetFace()->m_FaceNormal;// .Normalized();

							if (activeRopeAnchors[j]->m_type == STATE_IN)
							{
								FaceNormal *= -1.0f;
							}

							SutureRopeNode n0, n1;
							if (GetThreadSegmentNode(n0, n1, activeRopeAnchors[j]->m_NodeIndex))
							{
								SolveFixOrientation2(n0, n1, FaceNormal, 0.98f);
							}						
#endif
                        }                        
                    }
                }
            }
		}

		
		//solve soft-thread collision
		SolveSoftThreadCollisions();

		//solve rigid-thread collision
		SolveRigidThreadCollisions(TimeStep);

		//solve thread-thread collision
		SolveThreadThreadCollisions();
	}
	/*GFPhysVector3 FaceNormal(0, 2, 1);

	FaceNormal.Normalize();

	for (int i = 13; i < 45; i++)
	{
		SutureRopeNode n0, n1;
		if (GetThreadSegmentNode(n0, n1, i))
		{
			SolveFixOrientation2(n0, n1, FaceNormal, 1.0f);
		}
	}*/
}

void SutureThread::DisableCollideSelfFromClampToTail()
{    
    //考虑左右手器械同时夹线打结
	int num = GetNumThreadNodes();
    if (!m_ClampSegIndexVector.empty())
    {
        std::vector<int> localindexvector;
        for (int i = 0; i < (int)m_ClampSegIndexVector.size();i++)
        {
			int index = -1;
            SutureRopeNode& node = GetThreadNodeGlobalRef(m_ClampSegIndexVector[i], index);
			if (-1 == index)
			{
				continue;
			}
            localindexvector.push_back(index);
        }


        int maxID = 0;
        for (int i = 0; i < (int)localindexvector.size(); i++)
        {
            if (maxID < localindexvector[i])
            {
                maxID = localindexvector[i];
            }
        }

        for (int n = 0; n < maxID; n++)
        {
            SutureRopeNode& refNode = GetThreadNodeRef(n);
            refNode.SetCanCollideSelf(true);
        }
        GFPhysVectorObj<KnotInSutureRope> Knots;
        m_KnotsInThread->GetDeadKnots(Knots);
        if (Knots.size() == 1)
        {
            for (int n = Knots[0].m_knotcon0.m_A; n < Knots[0].m_knotcon1.m_B; n++)
            {
                SutureRopeNode& refNode = GetThreadNodeRef(n);
                refNode.SetCanCollideSelf(false);
            }
        }
		else if (Knots.size() == 2)
		{
			for (int n = Knots[1].m_knotcon0.m_A; n < Knots[1].m_knotcon1.m_B; n++)
			{
				SutureRopeNode& refNode = GetThreadNodeRef(n);
				refNode.SetCanCollideSelf(false);
			}
		}

        for (int n = maxID; n < num; n++)
        {
            SutureRopeNode& refNode = GetThreadNodeRef(n);
            refNode.SetCanCollideSelf(false);
        }
    }
    else
    {
        for (int n = 0; n < num; n++)
        {
            SutureRopeNode& refNode = GetThreadNodeRef(n);
            refNode.SetCanCollideSelf(true);
        }   

		GFPhysVectorObj<KnotInSutureRope> Knots;
		m_KnotsInThread->GetAllKnots(Knots);
		if (Knots.size() == 1)
		{
			for (int n = Knots[0].m_knotcon0.m_A; n < Knots[0].m_knotcon1.m_B; n++)
			{
				SutureRopeNode& refNode = GetThreadNodeRef(n);
				refNode.SetCanCollideSelf(false);
			}
		}
		else if (Knots.size() == 2)
		{
			for (int n = Knots[1].m_knotcon0.m_A; n < Knots[1].m_knotcon1.m_B; n++)
			{
				SutureRopeNode& refNode = GetThreadNodeRef(n);
				refNode.SetCanCollideSelf(false);
			}
		}

    }
}