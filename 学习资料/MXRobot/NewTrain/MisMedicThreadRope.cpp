#include "MisMedicThreadRope.h"
#include "MisNewTraining.h"
#include "Dynamic\Constraint\GoPhysSoftBodyDistConstraint.h"
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
float rscollideMassScale = 0.35f;
//int SolveCubicEquation(float a , float b , float c , float d , float & x1 , float & x2 , float & x3)
//{
//	float A = b*b-3.0f*a*c;
//	float B = b*c-9.0f*a*d;
//	float C = c*c-3.0f*b*d;
//
//	float delta = B*B-4.0f*A*C;
//	if(fabsf(A) < FLT_EPSILON && fabsf(B) < FLT_EPSILON)
//	{
//		x1 = x2 = x3 = -b / (3.0f*a);
//		return 1;
//	}
//	if(fabsf(delta) < FLT_EPSILON)
//	{
//		float K = B / A;
//		x1 = -b / a + K;
//		x2 = x3 = -0.5f*K;
//		return 2;
//	}
//	if(delta > 0)
//	{
//		float rootDelta = sqrtf(delta);
//		float Y1 = A*b + 1.5f*a*((-B+rootDelta));
//		float Y2 = A*b + 1.5f*a*((-B-rootDelta));
//		const float expv = 1.0f / 3.0f;
//		float u = pow (fabsf(Y1) , expv);
//		float v = pow (fabsf(Y2) , expv);
//		if(Y1 <= 0)
//			u *= -1.0f;
//		if(Y2 <= 0)
//			v *= -1.0f;
//
//		float t0 = -b-u-v;
//		x1 = (t0)/(3.0f*a);
//		return 1;
//	}
//	else
//	{
//		if(A > 0)
//		{
//			const float sqrOf3 = 1.73205f;//pow(3,(1/2));
//
//			float sqrRootA = sqrtf(A);
//
//			float T = (2.0f*A*b-3.0f*a*B)/ (2.0f*sqrRootA*sqrRootA*sqrRootA);
//
//			GPClamp(T ,-0.9999999f , 0.9999999f);//clamp for numerica error
//
//			float rad = acos(T);
//
//			float costheta = cos(rad * 0.333333f);
//
//			float sintheta = sin(rad * 0.333333f);
//			
//			float Inv3a = 1.0f / (3.0f*a);
//
//			x1=(-b-2*sqrRootA*costheta) * Inv3a;
//
//			x2=(-b+sqrRootA*(costheta+sqrOf3*sintheta)) * Inv3a;
//
//			x3=(-b+sqrRootA*(costheta-sqrOf3*sintheta)) * Inv3a;
//
//			return 3;
//		}
//		else
//			return 0;//should not move there;
//	}
//
//}

//stupid method need optimize
bool SegmentTriangleIntersected(GFPhysVector3 triVerts[3], GFPhysVector3 segVerts[2])
{
	Real intersectWeight;
	GFPhysVector3 intersectpt;
	Real triangleWeight[3];
	bool intersectTri = LineIntersectTriangle(triVerts[0],
		triVerts[1],
		triVerts[2],
		segVerts[0],
		segVerts[1],
		intersectWeight,
		intersectpt,
		triangleWeight);
	if (intersectTri == true && intersectWeight >= 0 && intersectWeight <= 1.0f)
	{
		return true;
	}
	else
		return false;
}
//================================================================================================================
TTCollidePair::TTCollidePair(MisMedicThreadRope * ropeA,
	MisMedicThreadRope * ropeB,
	int SegmentA,
	int SegmentB)
{
	m_RopeA = ropeA;
	m_RopeB = ropeB;
	m_SegmentA = SegmentA;
	m_SegmentB = SegmentB;
}
//================================================================================================================
int TTCollidePair::GetCollideSegmentA()  const
{
	return m_SegmentA;
}
//================================================================================================================
int TTCollidePair::GetCollideSegmentB()  const
{
	return m_SegmentB;
}
//================================================================================================================
void  ThreadRopeBendSection::BuildSectionNodeRestPos(MisMedicThreadRope * ropeobject)
{
	m_NodeRestPos.clear();

	float xpos = 0;

	m_NodeRestPos.push_back(GFPhysVector3(xpos, 0, 0));

	for (int c = m_StartNodeIndex; c < m_EndNodeIndex; c++)
	{
		GFPhysVector3 pos0, pos1;

		ropeobject->GetThreadSegmentNodePos(pos0, pos1, c);

		float segLength = (pos0 - pos1).Length();

		xpos += segLength;

		m_NodeRestPos.push_back(GFPhysVector3(xpos, 0, 0));
	}
}
//================================================================================================================
class MisThreadConvexCallBack : public GFPhysNodeOverlapCallback, public GFPhysDCDInterface::Result
{
public:
	MisThreadConvexCallBack(float margin,
		GFPhysRigidBody * rb,
		GFPhysTransform & rbTrans,
		float collideradius,
		MisMedicThreadRope * rope,
		GFPhysAlignedVectorObj<TRCollidePair> & collidepair
		) : m_rb(rb),
		m_Rope(rope),
		m_collideRadius(collideradius),
		m_CollidePairs(collidepair)

	{
		m_threadMarin = margin;//0.02f;
		m_rbTransform = rbTrans;
	}

	virtual ~MisThreadConvexCallBack()
	{}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
	{
		m_SegIndex = (int)UserData;

		ThreadNode n0, n1;

		bool succeed = m_Rope->GetThreadSegmentNode(n0, n1, m_SegIndex);

		if (succeed)
		{
			m_SegPos0 = n0.m_LastPosition;
			m_SegPos1 = n1.m_LastPosition;

			GFPhysLineSegmentShape linesegShape(m_SegPos0, m_SegPos1);
			linesegShape.SetMargin(m_collideRadius + m_threadMarin);

			GFPhysConvexCDShape * convexShape = (GFPhysConvexCDShape*)m_rb->GetCollisionShape();

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

		float t0 = (pointOnSegWithOutMargin - m_SegPos0).Dot(m_SegPos1 - m_SegPos0);

		float t1 = (m_SegPos1 - m_SegPos0).Dot(m_SegPos1 - m_SegPos0);

		if (t1 > FLT_EPSILON)
		{
			float depthWithOutMargin = depth + (m_collideRadius + m_threadMarin) + m_convexMargin;

			float segweight = t0 / t1;

			GPClamp(segweight, 0.0f, 1.0f);

			GFPhysVector3 pointOnConvexWithoutMargin = pointOnSegWithOutMargin + normalOnBInWorld * depthWithOutMargin;

			m_CollidePairs.push_back(TRCollidePair(m_rb, m_rbTransform, pointOnConvexWithoutMargin, -normalOnBInWorld, segweight, depthWithOutMargin, m_SegIndex));
		}
	}
	int m_SegIndex;

	GFPhysVector3 m_SegPos0;

	GFPhysVector3 m_SegPos1;

	GFPhysRigidBody * m_rb;
	MisMedicThreadRope * m_Rope;
	float m_collideRadius;

	float m_threadMarin;

	float m_convexMargin;

	GFPhysAlignedVectorObj<TRCollidePair>  & m_CollidePairs;
	GFPhysTransform m_rbTransform;
};
//
//================================================================================================================
class MisThreadThreadCallBack : public GFPhysNodeOverlapCallback
{
public:
	MisThreadThreadCallBack(float margin,
		float collideradius,
		MisMedicThreadRope * ropeA,
		MisMedicThreadRope * ropeB,
		GFPhysAlignedVectorObj<TTCollidePair> & collidepair) : m_RopeA(ropeA), m_RopeB(ropeB), m_threadRadius(collideradius), m_CollidePairs(collidepair)
	{
		m_threadMarin = margin;//0.02f;
		m_IsSelftCollision = (m_RopeA == m_RopeB ? true : false);
	}

	virtual ~MisThreadThreadCallBack()
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

			GFPhysVector3 A0, A1;
			GFPhysVector3 B0, B1;
			bool r0 = m_RopeA->GetThreadSegmentNodePos(A0, A1, SegIndexA);
			bool r1 = m_RopeB->GetThreadSegmentNodePos(B0, B1, SegIndexB);

			bool col0 = (m_RopeA->GetThreadNode(SegIndexA).GetTag() & TET_CANCOLLIDESELF);//m_bCanSelfCollision;
			bool col1 = (m_RopeA->GetThreadNode(SegIndexA + 1).GetTag() & TET_CANCOLLIDESELF);//.m_bCanSelfCollision;
			bool col2 = (m_RopeB->GetThreadNode(SegIndexB).GetTag() & TET_CANCOLLIDESELF);//.m_bCanSelfCollision;
			bool col3 = (m_RopeB->GetThreadNode(SegIndexB + 1).GetTag() & TET_CANCOLLIDESELF);//.m_bCanSelfCollision;

			if (r0 && r1 && col0 && col1 && col2 && col3)
			{
				float s, t;
				GFPhysVector3 cA, cB;
				float dist = GPClosestPtSegmentSegment(A0, A1, B0, B1, s, t, cA, cB);

				if (dist < (m_threadMarin + m_threadRadius)*2.0f)//collision
				{
					TTCollidePair ttpair(m_RopeA, m_RopeB, SegIndexA, SegIndexB);
					ttpair.m_WeightA = s;
					ttpair.m_WeightB = t;
					ttpair.m_CollideDist = dist;
					ttpair.m_NormalOnB = (cA - cB).Normalized();
					m_CollidePairs.push_back(ttpair);
				}
			}
		}
	}

	void SetShapeIdentifiers(int partId0, int index0, int partId1, int index1)
	{

	}


	MisMedicThreadRope * m_RopeB;

	MisMedicThreadRope * m_RopeA;

	bool  m_IsSelftCollision;

	float m_threadRadius;

	float m_threadMarin;

	GFPhysAlignedVectorObj<TTCollidePair>  & m_CollidePairs;
};

//@collision need optimize
//@also check whether SAT method have a lot  jitter effect in contact face(the SAT and closet method should behavier as the same)
class MisThreadSegSoftFaceCallback : public GFPhysNodeOverlapCallback
{
public:
	MisThreadSegSoftFaceCallback(float Margin,
		GFPhysSoftBody * sb,
		float collideradius,
		MisMedicThreadRope * rope,
		std::vector<TFCollidePair> & paircd,
		bool useCCD) : m_sb(sb),
		m_collideRadius(collideradius + Margin),
		m_Rope(rope),
		m_CollidePairs(paircd)

	{
		m_UseCCD = useCCD;
	}
	virtual ~MisThreadSegSoftFaceCallback()
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

		ThreadNode n0, n1;

		bool succedc = m_Rope->GetThreadSegmentNode(n0, n1, SegIndex);

		if (true)
		{
			GFPhysVector3 edgeVerts[2];

			GFPhysVector3 triVelocity[3];

			GFPhysVector3 edgeVelocity[2];

			edgeVerts[0] = n0.m_LastPosition;

			edgeVerts[1] = n1.m_LastPosition;

			triVelocity[0] = triVelocity[1] = triVelocity[2] = GFPhysVector3(0, 0, 0);//temp static mesh

			edgeVelocity[0] = n0.m_CurrPosition - n0.m_LastPosition;

			edgeVelocity[1] = n1.m_CurrPosition - n1.m_LastPosition;

			float sumRadius = m_collideRadius;//0.25 is margin

			GFPhysVector3 cp_Tri, cp_Edge, collideNorm;
			bool exceedMaxItor;
			float collideTime = GFPhysCABasedCCD::GetFaceEdgeWithRadiusCollideTime(SoftFaceVerts, edgeVerts,
				triVelocity, edgeVelocity,
				1.0f, sumRadius, sumRadius*0.02f,
				cp_Tri, cp_Edge, collideNorm, exceedMaxItor);

			if (collideTime >= 0 && (!exceedMaxItor))
			{
				float FaceWeights[3];

				float EdgeWeight;

				GFPhysVector3 e0 = edgeVerts[0] + edgeVelocity[0] * collideTime;
				GFPhysVector3 e1 = edgeVerts[1] + edgeVelocity[1] * collideTime;

				GFPhysVector3 t0 = SoftFaceVerts[0] + triVelocity[0] * collideTime;
				GFPhysVector3 t1 = SoftFaceVerts[1] + triVelocity[1] * collideTime;
				GFPhysVector3 t2 = SoftFaceVerts[2] + triVelocity[2] * collideTime;

				CalcBaryCentric(t0, t1, t2, cp_Tri, FaceWeights[0], FaceWeights[1], FaceWeights[2]);

				float lenSqr = (e1 - e0).Dot(e1 - e0);

				if (lenSqr > 0.0001f)
					EdgeWeight = (cp_Edge - e0).Dot(e1 - e0) / lenSqr;
				else
					EdgeWeight = 0.0f;

				GPClamp(EdgeWeight, 0.0f, 1.0f);
				GPClamp(FaceWeights[0], 0.0f, 1.0f);
				GPClamp(FaceWeights[1], 0.0f, 1.0f);
				GPClamp(FaceWeights[2], 0.0f, 1.0f);

				GFPhysVector3 faceNormal = (SoftFaceVerts[1] - SoftFaceVerts[0]).Cross(SoftFaceVerts[2] - SoftFaceVerts[0]);
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
				else
					m_InvCollidePairs.push_back(collidePair);

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

				float faceWeights[3];

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

				float sum = faceWeights[0] + faceWeights[1] + faceWeights[2];
				if (sum >FLT_EPSILON)
				{
					faceWeights[0] /= sum;
					faceWeights[1] /= sum;
					faceWeights[2] /= sum;
				}

				//extract thread collision point weight
				float ThreadWeights[2];
				GFPhysVector3 dn = n1.m_CurrPosition - n0.m_CurrPosition;
				float dnLen2 = dn.Dot(dn);
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
				if (collideNormalSign == 1)
					m_CollidePairs.push_back(collidePair);//for edge - edge collision eliminate normal opposite to face
				else
					m_InvCollidePairs.push_back(collidePair);
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

			float minDist = FLT_MAX;

			int minIndex = -1;

			for (int c = 0; c < 5; c++)
			{
				float dist = (closetPtThread[c] - closetPtFace[c]).Length();

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
				float faceWeights[3];
				float ThreadWeights[2];
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

				float sum = faceWeights[0] + faceWeights[1] + faceWeights[2];
				if (sum >FLT_EPSILON)
				{
					faceWeights[0] /= sum;
					faceWeights[1] /= sum;
					faceWeights[2] /= sum;
				}

				//extract thread collision point weight
				GFPhysVector3 dn = n1.m_CurrPosition - n0.m_CurrPosition;
				float dnLen2 = dn.Dot(dn);
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
	MisMedicThreadRope * m_Rope;
	float m_collideRadius;

	std::vector<TFCollidePair> & m_CollidePairs;
	std::vector<TFCollidePair> m_InvCollidePairs;
	bool m_UseCCD;
};

//===================================================================================================
MisMedicThreadRope::MisMedicThreadRope(Ogre::SceneManager * sceneMgr, MisNewTraining * ownertrain)
	: m_ToolKernalNode(0), m_AttachedFace(0), m_ownertrain(ownertrain), m_IsCutAfterBound(false)
{
	static int s_RopeId = 0;
	s_RopeId++;

	Ogre::String strRopeName = "RopeThreadObject" + Ogre::StringConverter::toString(s_RopeId);
	m_RendObject.CreateRendPart(strRopeName, sceneMgr);

	m_TopoType = MisMedicThreadRope::TTT_NONE;
	m_UnitLen = 0.18f;//unit length in rope
	m_RopeCollideRadius = 0.06f;//rope physics radius
	m_RopeRendRadius = 0.04f;//rope rend radius
	m_Margin = 0.08f;//collision margin
	m_RopeFriction = 0.6f;//rope vs soft collision coefficients
	m_RopeRSFriction = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.2f);//rope vs rigid coefficients
	m_UseCCD = false;

	m_Gravity = GFPhysVector3(0, 0, -6.0f);
	m_ForceFeed = GFPhysVector3(0, 0, 0);

	m_DampingRate = 8.0f;

	m_TotalItorNum = 14;

	m_TimesPerItor = m_TotalItorNum / GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverItorNum;

	if (m_TimesPerItor < 1)
		m_TimesPerItor = 1;

	m_SingleItorNum = m_TotalItorNum - m_TimesPerItor*GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverItorNum;

	if (m_SingleItorNum < 0)
		m_SingleItorNum = 0;

	m_RopeBendStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.8f);

	m_RopeStrechStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);

	m_SolveMassScale = 1.0f;

	m_Catergory = OPC_THREADUNIVERSAL;

	m_EnableSelfCollision = true;

	m_bAttached = false;

	m_RendSegementTag = false;

	m_UseBendForce = true;

	m_NeedRend = true;

	//m_InvMassArray = 0;

	//m_CurrPosArray = 0;

	//m_UndeformPosArray = 0;

	//m_NumSMNode = 0;

	m_RigidForceRange = 4;
	m_RigidForceMagnitude = 0.7f;

	//m_RFInvMass.resize(m_RigidForceRange);
	//m_RFCurrPos.resize(m_RigidForceRange);
	//m_RFUndeformPos.resize(m_RigidForceRange);
	//m_RFMomentMat.resize(m_RigidForceRange);
	//m_RFUndeformedMomentMat.resize(m_RigidForceRange);
	m_SimFrameID = 0;
}
//===================================================================================================
MisMedicThreadRope::~MisMedicThreadRope()
{
	DestoryRope();
}
//===================================================================================================
void MisMedicThreadRope::SetRigidForceRange(int range)
{
	m_RigidForceRange = range;

	//m_RFInvMass.resize(m_RigidForceRange);
	//m_RFCurrPos.resize(m_RigidForceRange);
	//m_RFUndeformPos.resize(m_RigidForceRange);
	//m_RFMomentMat.resize(m_RigidForceRange);
	///m_RFUndeformedMomentMat.resize(m_RigidForceRange);
}
//===================================================================================================
void MisMedicThreadRope::SetStretchStiffness(float set)
{
	m_RopeStrechStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(set);

}
//===================================================================================================
void MisMedicThreadRope::SetBendingStiffness(float set)
{
	m_RopeBendStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(set);
	m_RigidForceMagnitude = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(set);
}
//===================================================================================================
void MisMedicThreadRope::SetGravity(const GFPhysVector3 & gravity)
{
	m_Gravity = gravity;
}
//===================================================================================================
void MisMedicThreadRope::Shrink(float percent)
{
	if (m_TopoType == TTT_LOOP)//can only shrink in loop state
	    SetUnitLen(GetUnitLen() * percent);
}
//===================================================================================================
#define ADDSOFTTHREADFORCE
GFPhysVector3 MisMedicThreadRope::CalcLoopForceFeedBack()
{
	if (m_TopoType != TTT_LOOP)
		return GFPhysVector3(0, 0, 0);

	GFPhysVector3 TotalExternForce(0, 0, 0);
	const GFPhysAlignedVectorObj<TRCollidePair> & trPairs = GetCollidePairsWithRigid();
	for (size_t c = 0; c < trPairs.size(); c++)
	{
		const TRCollidePair & trpair = trPairs[c];
		GFPhysVector3 trContactImpluse = trpair.m_NormalOnRigid*trpair.m_ImpluseNormalOnRigid;
		TotalExternForce -= trContactImpluse*2.0f;//*0.12f;
	}
#ifdef ADDSOFTTHREADFORCE
	const std::vector<TFCollidePair> & softPairs = GetCollidePairs();
	for (size_t c = 0; c < softPairs.size(); c++)
	{
		const TFCollidePair & trpair = softPairs[c];
		GFPhysVector3 ThreadImpluse = OgreToGPVec3(trpair.m_ImpluseOnThread);
		TotalExternForce += ThreadImpluse*2.0f;//*0.12f;
	}
#endif

	float exterForceMag = TotalExternForce.Length();

	GFPhysVector3 externForceDir = (exterForceMag > FLT_EPSILON ? TotalExternForce / exterForceMag : GFPhysVector3(0, 0, 0));

	//float forceMag = TotalContactForce.Length();
	GFPhysVector3 dir1 = (m_ThreadNodes[2].m_CurrPosition - m_ThreadNodes[1].m_CurrPosition).Normalize();
	GFPhysVector3 dir2 = (m_ThreadNodes[m_ThreadNodes.size() - 3].m_CurrPosition - m_ThreadNodes[m_ThreadNodes.size() - 2].m_CurrPosition).Normalize();
	GFPhysVector3 direction = (dir1 + dir2)*0.5f;

	float originLen = GetTotalLen(false);
	float currLen = GetTotalLen(true);

	int segmentNum = m_ThreadNodes.size() - 1;

	int unitTolerant = m_UnitLen * 0.15;

	float delatLen = (currLen - (originLen + unitTolerant*segmentNum));//give some tolerant
	//if(delatLen <= 0)
	// return GFPhysVector3(0,0,0);
	//else
	//  return -direction*delatLen;

	if (delatLen <= 0)
		delatLen = 0;

	return externForceDir*delatLen*delatLen*5.5f;
	//return TotalExternForce;
	//if(externForceDir.Dot(direction) >= 0 )
	//  return direction*(TotalExternForce.Dot(direction));
	//else
	//  return GFPhysVector3(0,0,0);
}
//=========================================================================================================
void MisMedicThreadRope::DestoryRope()
{
	m_ThreadNodes.clear();
	m_FixedNodsInFo.clear();

	m_AttachedFace = 0;
	m_SegmentBVTree.Clear();

	m_TRCollidePair.clear();
	m_TFCollidePair.clear();

	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);

	m_IsCutAfterBound = false;

}
//=========================================================================================================
void MisMedicThreadRope::CreateLoopedThread(const GFPhysVector3 & circleCenter,
	const GFPhysVector3 & circelAxis,
	const GFPhysVector3 & StartVec,
	Ogre::SceneNode * AttachNode,
	ObjPhysCatogry threadCat)
{
	DestoryRope();

	m_UnitLen = 0.3f;

	m_Catergory = threadCat;

	Ogre::Matrix4 worldTrans = AttachNode->_getFullTransform();

	int ThreadSegNum = 30;//偶数

	GFPhysVector3 StartPos = circleCenter + StartVec;

	GFPhysVector3 RopeDir = -StartVec;
	RopeDir.Normalize();

	m_LoopAxisLocal = Ogre::Vector3(circelAxis.m_x, circelAxis.m_y, circelAxis.m_z);

	//separate  a little
	GFPhysVector3 sepDirection = RopeDir.Cross(OgreToGPVec3(m_LoopAxisLocal));
	sepDirection.Normalize();

	GFPhysVector3 firstNodePos = StartPos + sepDirection * 0.01f;
	GFPhysVector3 lastNodePos = StartPos - sepDirection * 0.04f;
	for (int r = 0; r < ThreadSegNum; r++)
	{
		GFPhysVector3 localPos = firstNodePos + RopeDir * m_UnitLen * r;

		Ogre::Vector3 temp(localPos.x(), localPos.y(), localPos.z());

		temp = worldTrans*temp;

		ThreadNode threadnode(localPos);

		//set current position directly
		threadnode.m_CurrPosition = threadnode.m_LastPosition = GFPhysVector3(temp.x, temp.y, temp.z);

		if (r == 0 || r == ThreadSegNum - 1)
			threadnode.SetInvMass(0);//mark as fix temp
		else if (r < 3 || r > ThreadSegNum - 4)//enforce root node
			threadnode.SetInvMass(200);
		else
			threadnode.SetInvMass(500);

		m_ThreadNodes.push_back(threadnode);
	}

	//m_FixedNodaIndex.push_back(0);
	//m_FixedNodaIndex.push_back(ThreadSegNum-1);

	//m_ThreadNodes[0].m_UnDeformedPos = StartPos+sepDirection*0.02f;
	//m_ThreadNodes[ThreadSegNum-1].m_UnDeformedPos = StartPos-sepDirection*0.02f;

	m_FixedNodsInFo.push_back(ThreadFixNodeInfo(0, firstNodePos));
	m_FixedNodsInFo.push_back(ThreadFixNodeInfo(ThreadSegNum - 1, lastNodePos));

	for (int c = 0; c < ThreadSegNum / 2; c++)
	{
		//m_ThreadNodes[ThreadSegNum-1-c].m_UnDeformedPos = m_ThreadNodes[c].m_UnDeformedPos;
		m_ThreadNodes[ThreadSegNum - 1 - c].m_CurrPosition = m_ThreadNodes[c].m_CurrPosition;
		m_ThreadNodes[ThreadSegNum - 1 - c].m_LastPosition = m_ThreadNodes[c].m_LastPosition;
	}

	m_VirtualStickNode = ThreadNode((firstNodePos + lastNodePos)*0.5f-RopeDir*0.2f);
	m_VirtualStickNode.SetInvMass(0);//m_InvMass = 0;

	m_ToolKernalNode = AttachNode;

	m_TopoType = MisMedicThreadRope::TTT_LOOP;

	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

	//disable some
	for (int i = 0; i < 4; i++)
	{
		GetThreadNodeRef(i).SetCanCollideRigid(false);// = false;
		GetThreadNodeRef(m_ThreadNodes.size() - 1 - i).SetCanCollideRigid(false);//m_bCollideRigid = false;
	}
	//m_BendSection.clear();
	//m_BendSection.push_back(ThreadRopeBendSection(0, m_ThreadNodes.size() - 1));
	SetGravity(GFPhysVector3(0, 0, 0));

	//SetRigidForceRange(GetNumThreadNodes()/5);

}
//=========================================================================================================
void MisMedicThreadRope::CreateOnePointFixThread(int ThreadSegNum,
	const GFPhysVector3 & StartFixPoint,
	const GFPhysVector3 & circelAxis,
	const GFPhysVector3 & direction,
	Ogre::SceneNode * AttachNode,
	ObjPhysCatogry threadCat)
{
	DestoryRope();

	m_Catergory = threadCat;

	Ogre::Matrix4 worldTrans = AttachNode->_getFullTransform();

	GFPhysVector3 RopeDir = direction;
	RopeDir.Normalize();

	for (int r = 0; r < ThreadSegNum; r++)
	{
		GFPhysVector3 localPos = StartFixPoint + RopeDir * m_UnitLen * r;

		Ogre::Vector3 temp(localPos.x(), localPos.y(), localPos.z());

		temp = worldTrans*temp;

		ThreadNode threadnode(localPos);

		//set current position directly
		threadnode.m_CurrPosition = threadnode.m_LastPosition = GFPhysVector3(temp.x, temp.y, temp.z);

		if (r == 0)
			threadnode.SetInvMass(0);//m_InvMass = 0;//mark as fix temp

		m_ThreadNodes.push_back(threadnode);
	}
	m_FixedNodsInFo.push_back(ThreadFixNodeInfo(0, m_ThreadNodes[0].m_UnDeformedPos));

	m_VirtualStickNode = ThreadNode(StartFixPoint - RopeDir*m_UnitLen);
	m_VirtualStickNode.SetInvMass(0);//m_InvMass = 0;

	m_LoopAxisLocal = Ogre::Vector3(circelAxis.m_x, circelAxis.m_y, circelAxis.m_z);
	m_ToolKernalNode = AttachNode;

	m_TopoType = MisMedicThreadRope::TTT_FIXONEEND;

	for (size_t c = 0; c < m_ThreadNodes.size(); c++)//temp
	{
		m_ThreadNodes[c].SetCanCollideSoft(false);// m_bCollideSoft = false;
	}

	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

	m_SegmentState.resize(GetNumSegments());
	for (int s = 0; s < GetNumSegments(); s++)
	{
		m_SegmentState[s] = 0;
	}
	//m_BendSection.clear();
	//m_BendSection.push_back(ThreadRopeBendSection(0, m_ThreadNodes.size() - 1));
}
//=============================================================================================
void MisMedicThreadRope::CreateFreeThread(const GFPhysVector3 & StartFixPoint, const GFPhysVector3 & EndFixPoint, int segmetnCount, float masspernode)
{
	DestoryRope();

	m_UnitLen = (StartFixPoint - EndFixPoint).Length() / segmetnCount;

	for (int r = 0; r <= segmetnCount; r++)
	{
		float weight = (float)r / (float)segmetnCount;

		GFPhysVector3 threadPos = StartFixPoint*(1 - weight) + EndFixPoint*weight;

		ThreadNode threadnode(threadPos);
		threadnode.SetInvMass(1.0f / masspernode);

		m_ThreadNodes.push_back(threadnode);
	}

	m_TopoType = MisMedicThreadRope::TTT_FREE;

	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

	m_SegmentState.resize(GetNumSegments());
	for (int s = 0; s < GetNumSegments(); s++)
	{
		m_SegmentState[s] = 0;
	}
	//m_BendSection.clear();
	//m_BendSection.push_back(ThreadRopeBendSection(0, m_ThreadNodes.size() - 1));
}
//=============================================================================================
bool MisMedicThreadRope::CutThreadByTool(CTool * ToolObject)
{
	if (m_TopoType == TTT_FIXONEEND)
	{
		GFPhysVector3 cutQuadVert[4];

		ToolObject->GetToolCutPlaneVerts(cutQuadVert);

		bool cutted = false;

		int cutSeg = -1;

		for (int n = 0; n < GetNumThreadNodes() - 1; n++)
		{
			GFPhysVector3 pos0 = GetThreadNodeRef(n).m_CurrPosition;
			GFPhysVector3 pos1 = GetThreadNodeRef(n + 1).m_CurrPosition;

			GFPhysVector3 cutTriVerts[3];
			GFPhysVector3 segVerts[2];

			cutTriVerts[0] = cutQuadVert[0];
			cutTriVerts[1] = cutQuadVert[1];
			cutTriVerts[2] = cutQuadVert[2];

			segVerts[0] = pos0;
			segVerts[1] = pos1;

			bool collidde0 = SegmentTriangleIntersected(cutTriVerts, segVerts);

			cutTriVerts[0] = cutQuadVert[1];
			cutTriVerts[1] = cutQuadVert[2];
			cutTriVerts[2] = cutQuadVert[3];

			bool collidde1 = SegmentTriangleIntersected(cutTriVerts, segVerts);

			float collideradius = GetCollideRadius();

			if (collidde0 || collidde1)
			{
				cutted = true;
				cutSeg = n;
				break;
			}
		}

		if (cutted == true)
		{
			DetachNodePointFromFace();

			m_ThreadNodes.resize(cutSeg+1);

			m_SegmentState.resize(cutSeg);

			m_IsCutAfterBound = true;

			//发送剪断消息
			MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_CutThread, ToolObject, this);
			CMXEventsDump::Instance()->PushEvent(pEvent, true);

			return true;
		}
	}
	return false;
}
//=============================================================================================
void MisMedicThreadRope::AttachNodePointToFace(GFPhysSoftBodyFace * attachFace, float weights[3])
{
	m_AttachedFace = attachFace;
	m_AttachWeight[0] = weights[0];
	m_AttachWeight[1] = weights[1];
	m_AttachWeight[2] = weights[2];
}
void MisMedicThreadRope::DetachNodePointFromFace()
{
	m_AttachedFace = 0;
}
//=========================================================================================================
void MisMedicThreadRope::BeginSimulateThreadPhysics(float dt)
{
	if (m_TopoType == TTT_NONE)
		return;

	m_SimFrameID++;

	UpdateFixedNodes();

	//clear all real time state and reset solver scale
	for (size_t n = 0; n < m_ThreadNodes.size(); n++)
	{
		ThreadNode & tNode = m_ThreadNodes[n];

		//clear real time state mark
		tNode.ClearRealTimeState();

		//reset solver inv mass scale
		tNode.SetSolverInvMassScale(1.0f);
	}

	//damping velocity and integrate position
	float dampingRate = m_DampingRate;//3.0f;

	for (size_t n = 0; n < m_ThreadNodes.size(); n++)
	{
		ThreadNode & tNode = m_ThreadNodes[n];

		float InvMass = tNode.GetInvMass();


		if (InvMass > FLT_EPSILON)
		{
			tNode.m_Velocity += m_Gravity*dt;

			float realdamp = GPClamped(1.0f - dt * dampingRate, 0.0f, 1.0f);

			tNode.m_Velocity *= realdamp;

			tNode.m_LastPosition = tNode.m_CurrPosition;

			tNode.m_CurrPosition += tNode.m_Velocity*dt;
		}
	}

	//update tree
	m_SegmentBVTree.Clear();//clear first
	for (int n = 0; n < m_ThreadNodes.size() - 1; n++)
	{
		const ThreadNode & Node0 = m_ThreadNodes[n];
		const ThreadNode & Node1 = m_ThreadNodes[n + 1];

		GFPhysVector3 minPos = Node0.m_CurrPosition;
		GFPhysVector3 maxPos = Node0.m_CurrPosition;

		minPos.SetMin(Node1.m_CurrPosition);
		minPos.SetMin(Node0.m_LastPosition);
		minPos.SetMin(Node1.m_LastPosition);

		maxPos.SetMax(Node1.m_CurrPosition);
		maxPos.SetMax(Node0.m_LastPosition);
		maxPos.SetMax(Node1.m_LastPosition);

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

		if (m_ThreadNodes.size() < 3)//prevent bv tree empty in nodes num = 1
			return;

		std::vector<MisMedicOrganInterface*> organsInTrain;

		newTrain->GetAllOrgan(organsInTrain);

		//check collision with soft organ
		for (size_t c = 0; c < organsInTrain.size(); c++)
		{
			MisMedicOrganInterface * oif = organsInTrain[c];

			GFPhysSoftBody * physbody = 0;

			MisMedicOrgan_Ordinary * organOrdinary = dynamic_cast<MisMedicOrgan_Ordinary *>(oif);

			//MisMedicOrgan_Tube * organTube = dynamic_cast<MisMedicOrgan_Tube *>(oif);

			if (organOrdinary)
			{
				physbody = organOrdinary->m_physbody;
			}
			//else if (organTube)
			//{
			//	physbody = organTube->m_physbody;
			//}

			if (physbody)
			{
				GFPhysVectorObj<GFPhysDBVTree*> bvTrees = physbody->GetSoftBodyShape().GetFaceBVTrees();

				MisThreadSegSoftFaceCallback collideCallBack(m_Margin, physbody, m_RopeCollideRadius, this, m_TFCollidePair, m_UseCCD);

				for (size_t t = 0; t < bvTrees.size(); t++)
				{
					GFPhysDBVTree * bvTree = bvTrees[t];
					bvTree->CollideWithDBVTree(m_SegmentBVTree, &collideCallBack);
				}
				{
					int numPosCollide = collideCallBack.m_CollidePairs.size();

					for (size_t c = 0; c < collideCallBack.m_InvCollidePairs.size(); c++)
					{
						const TFCollidePair & InvPair = collideCallBack.m_InvCollidePairs[c];

						for (size_t t = 0; t < numPosCollide; t++)
						{
							const TFCollidePair & NormPair = collideCallBack.m_CollidePairs[t];
							
							if (NormPair.GetCollideSegmentIndex() == InvPair.GetCollideSegmentIndex()
							 && NormPair.m_CollideNormal.dotProduct(InvPair.m_CollideNormal) > 0.17)
							{
								collideCallBack.m_CollidePairs.push_back(InvPair);
								break;
							}
						}
					}
				}
			}
		}
		//Set Collision Tag
		for (size_t t = 0; t < m_TFCollidePair.size(); t++)
		{
			TFCollidePair & tfPair = m_TFCollidePair[t];

			int segIndex = tfPair.GetCollideSegmentIndex();

			if (segIndex >= 0)
			{
				m_ThreadNodes[segIndex].MarkAsCollideSoft();// = true;
				m_ThreadNodes[segIndex + 1].MarkAsCollideSoft();// = true;
			}
		}


		//check collision with rigid body
		const GFPhysCollideObjectArray & collObjects = PhysicsWrapper::GetSingleTon().m_dynamicsWorld->GetCollideObjectArray();

		for (size_t c = 0; c < collObjects.size(); c++)
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

						/*std::map<GFPhysRigidBody* , RigidTransFormHistory>::iterator rbtItor = m_LastRigidTrans.find(rigidbody);

						//use last frame trans form so we can calculate velocity between last frame and this frame from their transformation
						if(rbtItor != m_LastRigidTrans.end())
						{
						rbTrans = rbtItor->second.m_LastTrans;
						}
						else
						{
						RigidTransFormHistory tshistory;
						tshistory.m_LastTrans = rbTrans;
						tshistory.m_LinearVel = tshistory.m_AngluarVel = GFPhysVector3(0,0,0);

						m_LastRigidTrans.insert(std::make_pair(rigidbody , tshistory));
						rbtItor = m_LastRigidTrans.find(rigidbody);
						}*/

						GFPhysConvexCDShape * convexshape = (GFPhysConvexCDShape*)rigidbody->GetCollisionShape();

						GFPhysVector3 Convexaabbmin, Convexaabbmax;

						convexshape->GetAabb(rbTrans, Convexaabbmin, Convexaabbmax);

						MisThreadConvexCallBack rbconcallback(m_Margin, rigidbody, rbTrans, m_RopeCollideRadius, this, m_TRCollidePair);

						m_SegmentBVTree.TraverseTreeAgainstAABB(&rbconcallback, Convexaabbmin, Convexaabbmax);

						//now update rigid velocity and angular velocity
						GFPhysVector3 rbLinearVel = (rigidbody->GetWorldTransform().GetOrigin() - rbTrans.GetOrigin()) / dt;

						GFPhysQuaternion currRot = rigidbody->GetWorldTransform().GetRotation();
						GFPhysQuaternion lastRot = rbTrans.GetRotation();

						GFPhysQuaternion deltaRot = currRot*lastRot.Inverse();
						Ogre::Quaternion OgreRot(deltaRot.w(), deltaRot.x(), deltaRot.y(), deltaRot.z());

						Ogre::Radian  rotRadian;
						Ogre::Vector3 rotAxis;
						OgreRot.ToAngleAxis(rotRadian, rotAxis);
						Real rotateRad = rotRadian.valueRadians();

						//use the shortest rotate
						if (rotateRad > Ogre::Math::PI)
						{
							rotAxis *= -1.0f;
							rotateRad = Ogre::Math::TWO_PI - rotateRad;
						}
						//
						rotAxis.normalise();//no need just for ensure

						Ogre::Vector3 temp = (rotAxis*(rotateRad / dt));
						GFPhysVector3 rbAngularVel = OgreToGPVec3(temp);

						//rbtItor->second.m_LinearVel = rbLinearVel;
						//rbtItor->second.m_AngluarVel = rbAngularVel;
						//rbtItor->second.m_SimFrameID = m_SimFrameID;
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

		for (size_t t = 0; t < m_TRCollidePair.size(); t++)
		{
			const TRCollidePair & trPair = m_TRCollidePair[t];

			m_ThreadNodes[trPair.m_Segment].MarkAsCollideRigid();//m_FrameCollideTag = true;

			m_ThreadNodes[trPair.m_Segment + 1].MarkAsCollideRigid();//m_FrameCollideTag = true;

			m_ownertrain->onThreadConvexCollided(trPair.m_Rigid,
				this,
				trPair.m_Segment,
				trPair.m_RigidWorldPoint,
				trPair.m_NormalOnRigid,
				trPair.m_Depth,
				trPair.m_SegWeight
				);
		}

		//self collision
		if (m_EnableSelfCollision)
		{
			MisThreadThreadCallBack ttcallback(m_Margin,
				m_RopeCollideRadius,
				this,
				this,
				m_TTCollidepair);

			m_SegmentBVTree.CollideWithDBVTree(m_SegmentBVTree, &ttcallback);
		}
	}
}


GFPhysVector3& MisMedicThreadRope::GetThreadNodePositionByIndex(int index)
{
	return m_ThreadNodes[index].m_CurrPosition;
}

bool MisMedicThreadRope::IsCollideWithOrgan(Ogre::Plane* plane)
{
	for (size_t t = 0; t < m_TFCollidePair.size(); t++)
	{
		TFCollidePair & tfPair = m_TFCollidePair[t];
		//int segIndex = tfPair.GetCollideSegmentIndex(); 
		//if(segIndex >= 0)
		{
			Real mindist = 0.001f;
			Real dist = mindist;

			GFPhysSoftBodyFace * softFace = tfPair.m_SoftFace;
			if (softFace)
			{
				GFPhysSoftBodyNode * pSoftNode = softFace->m_Nodes[0];
				if (pSoftNode)
				{
					Ogre::Vector3 ogrePos(pSoftNode->m_UnDeformedPos.GetX(), pSoftNode->m_UnDeformedPos.GetY(), pSoftNode->m_UnDeformedPos.GetZ());
					{

						dist = fabs(plane->getDistance(ogrePos));
						if (dist<mindist)
						{
							mindist = dist;
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

Ogre::Vector3  MisMedicThreadRope::GetThreadDir()
{
	return GPVec3ToOgre(m_ThreadNodes[m_ThreadNodes.size() - 1].m_CurrPosition - m_ThreadNodes[m_ThreadNodes.size() - 2].m_CurrPosition);
}

//================================================================================================
void MisMedicThreadRope::EndSimuolateThreadPhysics(float dt)
{
	if (m_TopoType == TTT_NONE)
		return;

	//friction
	for (size_t c = 0; c < m_TFCollidePair.size(); c++)
	{
		const TFCollidePair & cdpair = m_TFCollidePair[c];
		if (cdpair.m_CollideType == TFCollidePair::TFCD_VF)
		{
			GFPhysSoftBodyFace * softFace = cdpair.m_SoftFace;
			ThreadNode & tnode = m_ThreadNodes[cdpair.m_e1];

			GFPhysVector3 CollideNormal = OgreToGPVec3(cdpair.m_CollideNormal);//(softFace->m_Nodes[1]->m_CurrPosition-softFace->m_Nodes[0]->m_CurrPosition).Cross(softFace->m_Nodes[2]->m_CurrPosition-softFace->m_Nodes[0]->m_CurrPosition);

			Real normLen = CollideNormal.Length();

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
	for (size_t n = 0; n < m_ThreadNodes.size(); n++)
	{
		if (m_ThreadNodes[n].GetInvMass() > FLT_EPSILON)
		{
			GFPhysVector3 trans = (m_ThreadNodes[n].m_CurrPosition - m_ThreadNodes[n].m_LastPosition);
			m_ThreadNodes[n].m_Velocity = (m_ThreadNodes[n].m_CurrPosition - m_ThreadNodes[n].m_LastPosition) / dt;
		}
	}



	//eliminate velocity and position inverse collide normal
	for (size_t c = 0; c < m_TRCollidePair.size(); c++)
	{
		TRCollidePair & trPair = m_TRCollidePair[c];

		ThreadNode * tNode[2];
		tNode[0] = &(m_ThreadNodes[trPair.m_Segment]);
		tNode[1] = &(m_ThreadNodes[trPair.m_Segment + 1]);

		if ((tNode[0]->IsAttached()) || (tNode[1]->IsAttached()))
			continue;

		for (int n = 0; n < 2; n++)
		{
			float velinNormal = tNode[n]->m_Velocity.Dot(trPair.m_NormalOnRigid);

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


	for (size_t t = 0; t < m_TFCollidePair.size(); t++)
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
	float realdamp = GPClamped(1.0f - dt * dampingRate , 0.0f, 1.0f);
	m_ThreadNodes[n].m_Velocity *= realdamp;
	}
	}*/
	m_ForceFeed = GFPhysVector3(0, 0, 0);
	if (m_TopoType == TTT_LOOP)
	{
		//m_ForceFeed += GetThreadNode(0).m_Force1;
		//m_ForceFeed += GetThreadNode(m_ThreadNodes.size()-1).m_Force0;
		//m_ForceFeed *= 0.1f;
	}

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
}

//================================================================================================
void MisMedicThreadRope::SolveAdditionConstraint()
{
	if (m_TopoType == MisMedicThreadRope::TTT_NONE)
		return;

	if (m_TopoType == MisMedicThreadRope::TTT_FREE || m_ToolKernalNode == 0)
		return;

	Ogre::Matrix4 transMat = m_ToolKernalNode->_getFullTransform();

	Ogre::Vector3 temp = transMat.extractQuaternion() * m_LoopAxisLocal;
	temp.normalise();

	GFPhysVector3 loopAxisWorld(temp.x, temp.y, temp.z);

	if (m_TopoType == MisMedicThreadRope::TTT_LOOP)
	{
		//en force virtual bend at root
		int RootBendInterval = 2;
		if ((int)m_ThreadNodes.size() > 2 * (RootBendInterval + 2))
		{
			SolveBend(m_VirtualStickNode, m_ThreadNodes[0], m_ThreadNodes[RootBendInterval], m_RopeBendStiffness*0.5f);
			SolveBend(m_VirtualStickNode, m_ThreadNodes[m_ThreadNodes.size() - 1], m_ThreadNodes[m_ThreadNodes.size() - 1 - RootBendInterval], m_RopeBendStiffness*0.5f);
		}
	}
	else
	{
		int RootBendInterval = 1;

		if ((int)m_ThreadNodes.size() > RootBendInterval + 2)
		{
			//SolveBend(m_VirtualStickNode, m_ThreadNodes[0], m_ThreadNodes[RootBendInterval], 0.3f);
		}
		if (m_AttachedFace)
		{
			SolveAttachment(m_AttachedFace, m_AttachWeight, 1.0f);
		}
	}
}
//================================================================================================
bool MisMedicThreadRope::GetThreadSegmentNode(ThreadNode & n0, ThreadNode & n1, int segIndex)
{
	if (segIndex >= 0 && segIndex < (int)m_ThreadNodes.size() - 1)
	{
		n0 = m_ThreadNodes[segIndex];
		n1 = m_ThreadNodes[segIndex + 1];
		return true;
	}
	return false;
}
bool MisMedicThreadRope::GetThreadSegmentNodePos(GFPhysVector3 & n0, GFPhysVector3 & n1, int segIndex)
{
	if (segIndex >= 0 && segIndex < (int)m_ThreadNodes.size() - 1)
	{
		n0 = m_ThreadNodes[segIndex].m_CurrPosition;
		n1 = m_ThreadNodes[segIndex + 1].m_CurrPosition;
		return true;
	}
	return false;
}
//================================================================================================
void  MisMedicThreadRope::SetUnitLen(float UnitLen)
{
	m_UnitLen = UnitLen;
}
//================================================================================================
float MisMedicThreadRope::GetUnitLen()
{
	return m_UnitLen;
}
//================================================================================================
float MisMedicThreadRope::GetTotalLen(bool deformed)
{
	int SegNum = m_ThreadNodes.size() - 1;

	if (deformed == false)
		return m_UnitLen*SegNum;
	else
	{
		float totalLen = 0;
		for (int n = 0; n < SegNum; n++)
		{
			GFPhysVector3 p0 = m_ThreadNodes[n].m_CurrPosition;
			GFPhysVector3 p1 = m_ThreadNodes[n + 1].m_CurrPosition;

			float t = (p0 - p1).Length();
			totalLen += t;
		}
		return totalLen;
	}
}
//================================================================================================
MisMedicThreadRope::ThreadTopoType MisMedicThreadRope::GetThreadTopoType()
{
	return m_TopoType;
}
//================================================================================================
int MisMedicThreadRope::GetNumThreadNodes()
{
	return m_ThreadNodes.size();
}
//================================================================================================
int MisMedicThreadRope::GetNumSegments()
{
	return (int)m_ThreadNodes.size() - 1;
}
//================================================================================================
ThreadNode MisMedicThreadRope::GetThreadNode(int NodeIndex)
{
	return m_ThreadNodes[NodeIndex];
}
ThreadNode & MisMedicThreadRope::GetThreadNodeRef(int NodeIndex)
{
	return m_ThreadNodes[NodeIndex];
}
//================================================================================================
const std::vector<TFCollidePair> & MisMedicThreadRope::GetCollidePairs()
{
	return m_TFCollidePair;
}
const GFPhysAlignedVectorObj<TRCollidePair> & MisMedicThreadRope::GetCollidePairsWithRigid()
{
	return m_TRCollidePair;
}
const GFPhysAlignedVectorObj<TTCollidePair> & MisMedicThreadRope::GetCollidePairsWithThread()
{
	return m_TTCollidepair;
}
//================================================================================================
bool MisMedicThreadRope::GetApproximateLoopPlane(GFPhysVector3 & planePoint, GFPhysVector3 & planeNormal)
{
	if (m_TopoType != MisMedicThreadRope::TTT_LOOP)
		return false;

	std::vector<int> nodeLeftLoop;

	std::vector<int> nodeRightLoop;

	int NodeNum = (int)m_ThreadNodes.size();

	for (size_t c = 0; c < m_TFCollidePair.size(); c++)
	{
		int nodeIndex;

		if (m_TFCollidePair[c].m_CollideType == TFCollidePair::TFCD_EE)
		{
			nodeIndex = m_TFCollidePair[c].m_e3;
		}
		else
		{
			nodeIndex = m_TFCollidePair[c].m_e1;
		}

		if (nodeIndex < NodeNum / 2 - 1)
		{
			nodeLeftLoop.push_back(nodeIndex);
		}
		else
		{
			nodeRightLoop.push_back(nodeIndex);
		}
	}

	std::sort(nodeLeftLoop.begin(), nodeLeftLoop.end());
	std::sort(nodeRightLoop.begin(), nodeRightLoop.end());

	if (nodeLeftLoop.size() > 6 && nodeRightLoop.size() > 6)
	{
		int selLeft = nodeLeftLoop[nodeLeftLoop.size() / 2];
		int selRight = nodeRightLoop[nodeRightLoop.size() / 2];

		int selectT = nodeLeftLoop[0];

		GFPhysVector3 pos0 = m_ThreadNodes[selLeft].m_CurrPosition;
		GFPhysVector3 pos1 = m_ThreadNodes[selRight].m_CurrPosition;
		GFPhysVector3 pos2 = m_ThreadNodes[selectT].m_CurrPosition;

		planeNormal = (pos1 - pos0).Cross(pos2 - pos0).Normalized();
		planePoint = pos0;
		return true;
	}
	else
	{
		return false;
	}
}
//================================================================================================
void MisMedicThreadRope::UpdateFixedNodes()
{
	if (m_TopoType == MisMedicThreadRope::TTT_NONE)
		return;

	if (m_TopoType == TTT_FREE || m_ToolKernalNode == 0)
		return;

	Ogre::Matrix4 transMat = m_ToolKernalNode->_getFullTransform();

	for (size_t n = 0; n < m_FixedNodsInFo.size(); n++)
	{
		GFPhysVector3 localPos = m_FixedNodsInFo[n].m_UndeformedPos;

		int NodeIndex = m_FixedNodsInFo[n].m_NodeIndex;

		ThreadNode & FixNode = m_ThreadNodes[NodeIndex];

		Ogre::Vector3 temp = Ogre::Vector3(localPos.x(), localPos.y(), localPos.z());
		temp = transMat * temp;

		FixNode.m_CurrPosition = FixNode.m_LastPosition = GFPhysVector3(temp.x, temp.y, temp.z);
	}

	Ogre::Vector3 temp = Ogre::Vector3(m_VirtualStickNode.m_UnDeformedPos.m_x, m_VirtualStickNode.m_UnDeformedPos.m_y, m_VirtualStickNode.m_UnDeformedPos.m_z);
	temp = transMat* temp;

	m_VirtualStickNode.m_CurrPosition = m_VirtualStickNode.m_LastPosition = GFPhysVector3(temp.x, temp.y, temp.z);
}

/*void GetPoints(Ogre::Vector3 P0,Ogre::Vector3 P1,Ogre::Vector3 U,Ogre::Vector3 V,
int count,Ogre::Vector3 *points)
{
float dieta=(float)1/(count+1);
float t=0;
for(int i=0;i<count;i++)
{
t+=dieta;
points[i]=(1-3*t*t+2*t*t*t)*P0+(3*t*t-2*t*t*t)*P1+(t-2*t*t+t*t*t)*U+(-t*t+t*t*t)*V;
}

}*/

Ogre::Vector3 MisMedicThreadRope::GetInterplotPointPos(Ogre::Vector3 P1, Ogre::Vector3 P2, Ogre::Vector3 P3, Ogre::Vector3 P4, float t, float tao)
{
	float a = -tao*t + 2 * tao*t*t - tao*t*t*t;
	float b = 1 + (tao - 3)*t*t + (2 - tao)*t*t*t;
	float c = tao*t + (3 - 2 * tao)*t*t + (tao - 2)*t*t*t;
	float d = -tao *t*t + tao*t*t*t;
	return P1*a + P2*b + P3*c + P4*d;
}

//===================================================================================================
void MisMedicThreadRope::UpdateMesh()
{
	if (m_TopoType == MisMedicThreadRope::TTT_NONE)
		return;

	std::vector<Ogre::Vector3> RendNodes;
	for (size_t n = 0; n < m_ThreadNodes.size(); n++)
	{
		GFPhysVector3 temp = m_ThreadNodes[n].m_CurrPosition;
		RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
	}
	if (RendNodes.size() <= 1)
		return;
	//
	int insertPointNum = 3;

	float delta = 1.0f / float(insertPointNum + 1);

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

	m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);


	if (m_RendSegementTag)
	{
		std::vector<bool> segTags;
		std::vector<Ogre::ColourValue> segColors;
		segTags.resize(GetNumSegments());
		segColors.resize(GetNumSegments());
		for (size_t c = 0; c < segTags.size(); c++)
		{
			if (m_SegmentState[c] & TST_INKNOT)
			{
				segTags[c] = true;
				segColors[c] = Ogre::ColourValue::Green;
			}
			else
				segTags[c] = false;
		}

		m_RendObject.UpdateSegmentTagColor(RendNodes,
			segTags,
			segColors,
			m_RopeRendRadius);
	}

}
void MisMedicThreadRope::UpdateMeshByCustomedRendNodes(std::vector<Ogre::Vector3> & RendNodes)
{
	if (m_TopoType == MisMedicThreadRope::TTT_NONE)
		return;

	//
	int insertPointNum = 2;

	float delta = 1.0f / float(insertPointNum + 1);

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
//===================================================================
void MisMedicThreadRope::SolveBend(ThreadNode &  va, ThreadNode & vb, ThreadNode & vc, float Stiffness)
{
	if (m_UseBendForce == false)
		return;

	float wa = va.GetSolverInvMass();//va.GetInvMass() * va.GetSolverInvMasScale();//(va.m_InvMass > FLT_EPSILON ? 1.0f : 0.0f);

	float wb = vb.GetSolverInvMass();//GetInvMass() * vb.GetSolverInvMasScale();//(vb.m_InvMass > FLT_EPSILON ? 1.0f : 0.0f);

	float wc = vc.GetSolverInvMass();//GetInvMass() * vc.GetSolverInvMasScale();//(vc.m_InvMass > FLT_EPSILON ? 1.0f : 0.0f);

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

	float lab = vab.Length();
	float lcb = vcb.Length();

	if (lab * lcb == 0)
		return;

	float invAB = 1.0f / lab;
	float invCB = 1.0f / lcb;

	GFPhysVector3 n1 = vab*invAB;
	GFPhysVector3 n2 = vcb*invCB;

	float d = n1.Dot(n2);

	GPClamp(d, -1.0f, 1.0f);

	float dd = sqrtf(1 - d*d);

	GFPhysVector3 Col0 = GFPhysVector3(1 - n1.m_x*n1.m_x, -n1.m_y*n1.m_x, -n1.m_z*n1.m_x) * invAB;
	GFPhysVector3 Col1 = GFPhysVector3(-n1.m_x*n1.m_y, 1 - n1.m_y*n1.m_y, -n1.m_z*n1.m_y) * invAB;
	GFPhysVector3 Col2 = GFPhysVector3(-n1.m_x*n1.m_z, -n1.m_y*n1.m_z, 1 - n1.m_z*n1.m_z) * invAB;

	GFPhysVector3 gradVa = GFPhysVector3(Col0.Dot(n2), Col1.Dot(n2), Col2.Dot(n2));

	Col0 = GFPhysVector3(1 - n2.m_x*n2.m_x, -n2.m_y*n2.m_x, -n2.m_z*n2.m_x) * invCB;
	Col1 = GFPhysVector3(-n2.m_x*n2.m_y, 1 - n2.m_y*n2.m_y, -n2.m_z*n2.m_y) * invCB;
	Col2 = GFPhysVector3(-n2.m_x*n2.m_z, -n2.m_y*n2.m_z, 1 - n2.m_z*n2.m_z) * invCB;

	//@note !! in fact gradVc is gradVc * (-1.0f / dd)
	//but 1.0f/dd may be large and the sumgrad is square of (1.0f/dd) may be large than the float type 
	//precise for numerical robust , we eliminate this factor(-1.0f / dd) with the denomination
	//of sum grad and , finally multiply to result
	GFPhysVector3 gradVc = GFPhysVector3(Col0.Dot(n1), Col1.Dot(n1), Col2.Dot(n1));//gradVa *= arcdx;//gradVc *= arcdx;

	GFPhysVector3 gradVb = -gradVa - gradVc;

	float sumgrad = wa*gradVa.Length2() + wc*gradVc.Length2() + wb*gradVb.Length2();

	float c = acosf(d) - 3.1415926f;//minus PI(180 degree)

	if (fabsf(sumgrad) > FLT_EPSILON)// && (c > FLT_EPSILON || sumgrad > FLT_EPSILON || dd > FLT_EPSILON))//FLT_EPSILON)
	{
		float s = c * (-dd) / sumgrad;

		GFPhysVector3 deltaVA = -gradVa*(s*wa);

		GFPhysVector3 deltaVB = -gradVb*(s*wb);

		GFPhysVector3 deltaVC = -gradVc*(s*wc);

		float detaA = deltaVA.Length();
		float detaB = deltaVB.Length();
		float detaC = deltaVC.Length();

		va.m_CurrPosition += deltaVA*Stiffness;
		vb.m_CurrPosition += deltaVB*Stiffness;
		vc.m_CurrPosition += deltaVC*Stiffness;
	}
}
//===================================================================
GFPhysVector3 MisMedicThreadRope::SolveStretch(ThreadNode & Node1, ThreadNode & Node2, float Stiffness, int interval)
{
	Real RestLen = m_UnitLen*interval;

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
void MisMedicThreadRope::SolveAttachment(GFPhysSoftBodyFace * attachFace, float weights[3], float stiffness)
{
	GFPhysVector3 pointInFace = attachFace->m_Nodes[0]->m_CurrPosition*weights[0]
		+ attachFace->m_Nodes[1]->m_CurrPosition*weights[1]
		+ attachFace->m_Nodes[2]->m_CurrPosition*weights[2];

	ThreadNode & nodeAttach = m_ThreadNodes[m_ThreadNodes.size() - 1];

	GFPhysVector3 Diff = (pointInFace - nodeAttach.m_CurrPosition);

	float diffLen = Diff.Length();

	if (diffLen > FLT_EPSILON)
	{
		Diff /= diffLen;

		float faceInvMass[3];
		faceInvMass[0] = 1.0f;//attachFace->m_Nodes[0]->m_InvM;
		faceInvMass[1] = 1.0f;//attachFace->m_Nodes[1]->m_InvM;
		faceInvMass[2] = 1.0f;//attachFace->m_Nodes[2]->m_InvM;

		float nodeInvMass = 1.0f;///m_SolveMassScale;//(faceInvMass[0]+faceInvMass[1]+faceInvMass[2]);//*0.33f*0.2f;

		float sumgrad = weights[0] * weights[0] * faceInvMass[0]
			+ weights[1] * weights[1] * faceInvMass[1]
			+ weights[2] * weights[2] * faceInvMass[2]
			+ nodeInvMass;

		float scale = stiffness*(-diffLen) / sumgrad;

		GFPhysVector3 grad00 = Diff*weights[0];
		GFPhysVector3 grad01 = Diff*weights[1];
		GFPhysVector3 grad02 = Diff*weights[2];

		GFPhysVector3 gradNoda = -Diff;

		GFPhysVector3 delta00 = scale*grad00*faceInvMass[0];
		GFPhysVector3 delta01 = scale*grad01*faceInvMass[1];
		GFPhysVector3 delta02 = scale*grad02*faceInvMass[2];
		GFPhysVector3 deltaNode = scale*gradNoda*nodeInvMass;

		attachFace->m_Nodes[0]->m_CurrPosition += delta00;
		attachFace->m_Nodes[1]->m_CurrPosition += delta01;
		attachFace->m_Nodes[2]->m_CurrPosition += delta02;

		nodeAttach.m_CurrPosition += deltaNode;
	}
}
//===================================================================================================
GFPhysVector3 MisMedicThreadRope::SolveEECollide(const GFPhysVector3 & collideNormal,
	GFPhysSoftBodyNode * e1,
	GFPhysSoftBodyNode * e2,
	ThreadNode & e3,
	ThreadNode & e4,
	const TFCollidePair & cdPair)
{
	float Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);
	GFPhysVector3 impluse(0, 0, 0);

	ThreadNode * tNodes[2];
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
			float distCurr = (tpos - spos).Dot(collideNormal) - m_RopeCollideRadius;
			if (distCurr < 0)//solve constraint when negative
			{
				distCurr *= Stiffness;
				if (distCurr < -0.3f)
					distCurr = -0.3f;

				float wt = tNodes[t]->GetInvMass() > 0.0f ? 1.0f / m_SolveMassScale : 0.0f;
				float ws = sNodes[s]->m_InvM > 0.0f ? 1.0f : 0.0f;

				float sumInvW = wt + ws;

				if (sumInvW > FLT_EPSILON)
				{
					float moveThread = -(wt / sumInvW)*distCurr;
					float moveFaceNoda = (ws / sumInvW)*distCurr;

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
GFPhysVector3 MisMedicThreadRope::SolveVTCollide(const GFPhysVector3 & collideNormal, GFPhysSoftBodyNode * n0, GFPhysSoftBodyNode * n1, GFPhysSoftBodyNode * n2, ThreadNode & tn, const TFCollidePair & cdPair)
{
	float Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);

	GFPhysVector3 impluse(0, 0, 0);

	GFPhysVector3 CollideNodePos[4];
	float invMassNode[4];
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
		float nDotQ = collideNormal.Dot(q);
		float distCurr = nDotQ - m_RopeCollideRadius;

		if (distCurr < 0)//solve constraint when negative
		{
			distCurr *= Stiffness;

			if (distCurr < -0.3f)
				distCurr = -0.3f;

			float InvThreadMass = (tn.GetInvMass() > 0.0f ? 1.0f : 0.0f) / m_SolveMassScale;

			float SumInvMass = invMassNode[n] + InvThreadMass;

			if (SumInvMass > FLT_EPSILON)
			{
				float moveThread = -(InvThreadMass / SumInvMass)*distCurr;
				float moveFaceNoda = (invMassNode[n] / SumInvMass)*distCurr;

				tn.m_CurrPosition += collideNormal*moveThread;
				CollideNodes[n]->m_CurrPosition += collideNormal*moveFaceNoda;

				impluse += collideNormal*moveThread;
			}
		}
	}

	return impluse;
}
GFPhysVector3 MisMedicThreadRope::SolveEFCollide(const GFPhysVector3 & collideNormal,
	                                             GFPhysSoftBodyNode * v0,
	                                             GFPhysSoftBodyNode * v1,
	                                             GFPhysSoftBodyNode * v2,
	                                             ThreadNode & t0,
	                                             ThreadNode & t1,
	                                             const TFCollidePair & cdPair)
{
	GFPhysVector3 impluse(0, 0, 0);

	float faceInvMass = 4.0f;//temp
	float threadInvMass = 1.0f;//temp

	if ((v0->m_StateFlag&EMMP_ClampByTool) || (v1->m_StateFlag&EMMP_ClampByTool) || (v2->m_StateFlag&EMMP_ClampByTool))
		faceInvMass = 0.0f;
	GFPhysVector3 gradt0 = cdPair.m_ThreadWeigths[0] * collideNormal;
	GFPhysVector3 gradt1 = cdPair.m_ThreadWeigths[1] * collideNormal;

	GFPhysVector3 gradf0 = -cdPair.m_FaceWeihts[0] * collideNormal;
	GFPhysVector3 gradf1 = -cdPair.m_FaceWeihts[1] * collideNormal;
	GFPhysVector3 gradf2 = -cdPair.m_FaceWeihts[2] * collideNormal;

	float sumGrad = cdPair.m_ThreadWeigths[0] * cdPair.m_ThreadWeigths[0] * threadInvMass
		+ cdPair.m_ThreadWeigths[1] * cdPair.m_ThreadWeigths[1] * threadInvMass
		+ cdPair.m_FaceWeihts[0] * cdPair.m_FaceWeihts[0] * faceInvMass
		+ cdPair.m_FaceWeihts[1] * cdPair.m_FaceWeihts[1] * faceInvMass
		+ cdPair.m_FaceWeihts[2] * cdPair.m_FaceWeihts[2] * faceInvMass;


	GFPhysVector3 ptThread = t0.m_CurrPosition * cdPair.m_ThreadWeigths[0]
		+ t1.m_CurrPosition * cdPair.m_ThreadWeigths[1];

	GFPhysVector3 ptFace = v0->m_CurrPosition * cdPair.m_FaceWeihts[0]
		+ v1->m_CurrPosition * cdPair.m_FaceWeihts[1]
		+ v2->m_CurrPosition * cdPair.m_FaceWeihts[2];

	float  s_normdist = (ptThread - ptFace).Dot(collideNormal) - m_RopeCollideRadius;

	float  threadNormalCorrect[2];
	if (sumGrad > FLT_EPSILON && s_normdist < 0)
	{
		float s = s_normdist / sumGrad;

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
		float frictcoeff = 0.35f;

		ptThread = t0.m_CurrPosition * cdPair.m_ThreadWeigths[0]
			+ t1.m_CurrPosition * cdPair.m_ThreadWeigths[1];

		ptFace = v0->m_CurrPosition * cdPair.m_FaceWeihts[0]
			+ v1->m_CurrPosition * cdPair.m_FaceWeihts[1]
			+ v2->m_CurrPosition * cdPair.m_FaceWeihts[2];

		GFPhysVector3  TangVec = (ptThread - ptFace) - collideNormal * ((ptThread - ptFace).Dot(collideNormal));

		float tan_Dist = TangVec.Length();

		if (tan_Dist > GP_EPSILON)
		{
			GFPhysVector3 tan_Dir = TangVec / tan_Dist;

			gradt0 = cdPair.m_ThreadWeigths[0] * tan_Dir;

			gradt1 = cdPair.m_ThreadWeigths[1] * tan_Dir;

			sumGrad = cdPair.m_ThreadWeigths[0] * cdPair.m_ThreadWeigths[0] + cdPair.m_ThreadWeigths[1] * cdPair.m_ThreadWeigths[1];

			float tanCorrect = fabsf(s) * frictcoeff * sumGrad;//max tangent increase

			if ((tan_Dist - tanCorrect) < 0)
			{
				tanCorrect = tan_Dist;
				tan_Dist = 0;
			}
			else
			{
				tan_Dist = tan_Dist - tanCorrect;
			}

			float s = (-tanCorrect / sumGrad);

			t0.m_CurrPosition += gradt0 * s;
			t1.m_CurrPosition += gradt1 * s;
		}

		/*
		for(int t = 0 ; t < 2 ; t++)
		{
		ThreadNode & tnode = (t == 0 ? t0 : t1);

		GFPhysVector3 NodaVel  = tnode.m_CurrPosition-tnode.m_LastPosition;

		GFPhysVector3 NormalVel = collideNormal*NodaVel.Dot(collideNormal);

		GFPhysVector3 TangVel = NodaVel-NormalVel;

		float tanLen = TangVel.Length();

		if(tanLen > GP_EPSILON)
		{
		GFPhysVector3 tanVec = TangVel / tanLen;

		float tanCorrect = threadNormalCorrect[t] * 0.9f;

		tanLen = tanLen-tanCorrect;

		if(tanLen < 0)
		tanLen = 0;

		tnode.m_CurrPosition = tnode.m_LastPosition+NormalVel + tanVec*tanLen;
		}
		}*/

	}

	//float solverStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.8f);

	return impluse;// *0.02f;
}

//===================================================================================================
void MisMedicThreadRope::SolveSoftThreadCollisions()
{

	float Stiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.9f);

	for (size_t c = 0; c < m_TFCollidePair.size(); c++)
	{
		TFCollidePair & cdpair = m_TFCollidePair[c];

		GFPhysSoftBodyFace * softFace = cdpair.m_SoftFace;

		if (cdpair.m_CollideType == TFCollidePair::TFCD_EE)
		{
			if (m_ThreadNodes[cdpair.m_e3].GetCanCollideSoft() && m_ThreadNodes[cdpair.m_e4].GetCanCollideSoft())
			{
				GFPhysVector3 impluse = SolveEECollide(
					OgreToGPVec3(cdpair.m_CollideNormal),
					softFace->m_Nodes[cdpair.m_e1],
					softFace->m_Nodes[cdpair.m_e2],
					m_ThreadNodes[cdpair.m_e3],
					m_ThreadNodes[cdpair.m_e4],
					cdpair);

				cdpair.m_ImpluseOnThread += GPVec3ToOgre(impluse);

			}
		}
		else if (cdpair.m_CollideType == TFCollidePair::TFCD_VF)
		{
			if (m_ThreadNodes[cdpair.m_e1].GetCanCollideSoft())
			{
				GFPhysVector3 impluse = SolveVTCollide(OgreToGPVec3(cdpair.m_CollideNormal),
					softFace->m_Nodes[0],
					softFace->m_Nodes[1],
					softFace->m_Nodes[2],
					m_ThreadNodes[cdpair.m_e1],
					cdpair);

				cdpair.m_ImpluseOnThread += GPVec3ToOgre(impluse);
			}
		}
		else if (cdpair.m_CollideType == TFCollidePair::TFCD_EF)
		{
			if (m_ThreadNodes[cdpair.m_e3].GetCanCollideSoft() && m_ThreadNodes[cdpair.m_e4].GetCanCollideSoft())
			{
				GFPhysVector3 impluse = SolveEFCollide(
					OgreToGPVec3(cdpair.m_CollideNormal),
					softFace->m_Nodes[0],
					softFace->m_Nodes[1],
					softFace->m_Nodes[2],
					m_ThreadNodes[cdpair.m_e3],
					m_ThreadNodes[cdpair.m_e4],
					cdpair);

				cdpair.m_ImpluseOnThread += GPVec3ToOgre(impluse);

			}
		}
	}
}
//============================================================================================================================================
void MisMedicThreadRope::SolveRigidThreadCollisions(float dt)
{
	float hardness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.99f);
#if(1)
	//if( m_bAttached ) return;//bacon add for test
	//float hardness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.98f);
	//support rigid is static
	for (size_t c = 0; c < m_TRCollidePair.size(); c++)
	{
		TRCollidePair & trPair = m_TRCollidePair[c];

		ThreadNode * tNode[2];
		tNode[0] = &(m_ThreadNodes[trPair.m_Segment]);
		tNode[1] = &(m_ThreadNodes[trPair.m_Segment + 1]);

		//bacon add
		if (tNode[0]->IsAttached())
			continue;

		if (tNode[1]->IsAttached())
			continue;

		//end
		bool isCollide0 = (tNode[0]->GetInvMass() > FLT_EPSILON && tNode[0]->IsCollideRigid());
		bool isCollide1 = (tNode[1]->GetInvMass() > FLT_EPSILON && tNode[1]->IsCollideRigid());

		if (isCollide0 == false && isCollide1 == false)
			continue;

		float w0 = 1.0f - trPair.m_SegWeight;
		float w1 = trPair.m_SegWeight;

		if (isCollide0 == false)
		{
			w0 = 0; w1 = 1;
		}

		if (isCollide1 == false)
		{
			w0 = 1; w1 = 0;
		}

		GFPhysVector3 tPoint = tNode[0]->m_CurrPosition * w0 + tNode[1]->m_CurrPosition * w1;

		GFPhysVector3 Grad0 = w0 * trPair.m_NormalOnRigid;

		GFPhysVector3 Grad1 = w1 * trPair.m_NormalOnRigid;

		Real  sumGrad = w0 * w0 + w1 * w1;

		Real  s_normdist = m_RopeCollideRadius + GPSIMDVec3Dot(GPSIMDVec3Sub(trPair.m_RigidWorldPoint, tPoint), trPair.m_NormalOnRigid);

		GFPhysVector3  s_TanOffset(0, 0, 0);

		//std::map<GFPhysRigidBody* , RigidTransFormHistory>::iterator rbtItor = m_LastRigidTrans.find(trPair.m_Rigid);
		//if(rbtItor != m_LastRigidTrans.end())
		{
			// RigidTransFormHistory ts = rbtItor->second;
			const GFPhysTransform & worldts = trPair.m_Rigid->GetWorldTransform();

			GFPhysVector3 rel_pos = trPair.m_RigidWorldPoint - worldts.GetOrigin();//ts.m_LastTrans.GetOrigin();

			GFPhysVector3 contactPointVel = trPair.m_Rigid->GetLinearVelocity() + trPair.m_Rigid->GetAngularVelocity().Cross(rel_pos);//trPair.m_Rigid-> GetVelocityInLocalPoint(rel_pos);

			float dn = (contactPointVel * dt).Dot(trPair.m_NormalOnRigid);

			s_normdist += dn;

			s_TanOffset += (contactPointVel * dt) - trPair.m_NormalOnRigid*dn;
		}

		if (sumGrad > FLT_EPSILON && s_normdist > 0)
		{
			float s = s_normdist / sumGrad;

			GFPhysVector3 delta0 = Grad0 * s;
			GFPhysVector3 delta1 = Grad1 * s;

			tNode[0]->m_CurrPosition += delta0;
			tNode[1]->m_CurrPosition += delta1;

			trPair.m_ImpluseNormalOnRigid += (-(delta0.Length() + delta1.Length()));

			GFPhysVector3 lastps = tNode[0]->m_LastPosition * w0 + tNode[1]->m_LastPosition * w1;
			GFPhysVector3 currps = tNode[0]->m_CurrPosition * w0 + tNode[1]->m_CurrPosition * w1;

			const GFPhysVector3	VThread = currps - lastps;//(trPair.m_HasTanOffset ? trPair.m_TanOffsetFromLast : GFPhysVector3(0,0,0));//

			GFPhysVector3 VTangent = GPSIMDVec3Sub(VThread, trPair.m_NormalOnRigid * GPSIMDVec3Dot(VThread, trPair.m_NormalOnRigid));

			VTangent = s_TanOffset - VTangent;
			//friction
			for (int t = 0; t < 2; t++)
			{
				if (tNode[t]->GetInvMass() > 0)
				{
					//const GFPhysVector3	VThread = GPSIMDVec3Sub(tNode[t]->m_CurrPosition , tNode[t]->m_LastPosition);

					//const GFPhysVector3 VTangent = GPSIMDVec3Sub(VThread , trPair.m_NormalOnRigid * GPSIMDVec3Dot(VThread , trPair.m_NormalOnRigid));

					GFPhysVector3 TangentCorrect = VTangent*m_RopeRSFriction;

					tNode[t]->m_CurrPosition += TangentCorrect;
				}
			}
		}
	}
#else
	//support rigid is static
	for (size_t c = 0; c < m_TRCollidePair.size(); c++)
	{
		TRCollidePair & trPair = m_TRCollidePair[c];

		ThreadNode * tNode[2];
		tNode[0] = &(m_ThreadNodes[trPair.m_Segment]);
		tNode[1] = &(m_ThreadNodes[trPair.m_Segment + 1]);

		for (int t = 0; t < 2; t++)
		{
			if (tNode[t]->m_InvMass > FLT_EPSILON && tNode[t]->m_bCollideRigid)
			{
				Real  Normal_Correct = m_RopeCollideRadius + GPSIMDVec3Dot(GPSIMDVec3Sub(trPair.m_RigidWorldPoint, tNode[t]->m_CurrPosition), trPair.m_NormalOnRigid);

				//if(Normal_Correct > 0)
				{
					if (Normal_Correct + trPair.m_ThreadImpulseDeta[t] >= 0)
						trPair.m_ThreadImpulseDeta[t] += Normal_Correct;
					else//clamp to 0
					{
						Normal_Correct = -trPair.m_ThreadImpulseDeta[t];
						trPair.m_ThreadImpulseDeta[t] = 0;
					}

					tNode[t]->m_CurrPosition += trPair.m_NormalOnRigid * Normal_Correct * hardness;

					trPair.m_ImpluseNormalOnRigid += (-Normal_Correct);

					if (Normal_Correct > 0)
					{
						const GFPhysVector3	VThread = GPSIMDVec3Sub(tNode[t]->m_CurrPosition, tNode[t]->m_LastPosition);

						const GFPhysVector3 VTangent = GPSIMDVec3Sub(VThread, trPair.m_NormalOnRigid * GPSIMDVec3Dot(VThread, trPair.m_NormalOnRigid));

						GFPhysVector3 TangentCorrect = VTangent*m_RopeRSFriction;

						tNode[t]->m_CurrPosition -= TangentCorrect;
					}
				}
			}
		}
	}
#endif
}
//=================================================================================================================================
void MisMedicThreadRope::SolveThreadThreadCollisions()
{
	for (size_t c = 0; c < m_TTCollidepair.size(); c++)
	{
		TTCollidePair & ttPair = m_TTCollidepair[c];

		ThreadNode * tNodeA[2];
		ThreadNode * tNodeB[2];

		tNodeA[0] = &(m_ThreadNodes[ttPair.m_SegmentA]);
		tNodeA[1] = &(m_ThreadNodes[ttPair.m_SegmentA + 1]);

		tNodeB[0] = &(m_ThreadNodes[ttPair.m_SegmentB]);
		tNodeB[1] = &(m_ThreadNodes[ttPair.m_SegmentB + 1]);

		float wA0 = 1.0f - ttPair.m_WeightA;
		float wA1 = ttPair.m_WeightA;

		float wB0 = 1.0f - ttPair.m_WeightB;
		float wB1 = ttPair.m_WeightB;


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

		Real  s_normdist = 2.0f * m_RopeCollideRadius + GPSIMDVec3Dot(GPSIMDVec3Sub(PointB, PointA), ttPair.m_NormalOnB);

		if (sumGrad > FLT_EPSILON && s_normdist > 0)
		{
			float s = s_normdist / sumGrad;

			tNodeA[0]->m_CurrPosition += GradA0 * s;
			tNodeA[1]->m_CurrPosition += GradA1 * s;

			tNodeB[0]->m_CurrPosition += GradB0 * s;
			tNodeB[1]->m_CurrPosition += GradB1 * s;
		}
	}
}
//==========================================================================================================================
void MisMedicThreadRope::SolveBendX(ThreadNode & va, ThreadNode & vb, ThreadNode & vc, float & lambda, float InvStiff, float damping, float solvehardness, float dt)
{
	float gamma = InvStiff * damping / dt;

	float wa = va.GetSolverInvMass();

	float wb = vb.GetSolverInvMass();

	float wc = vc.GetSolverInvMass();

	GFPhysVector3 vab = va.m_CurrPosition - vb.m_CurrPosition;

	GFPhysVector3 vcb = vc.m_CurrPosition - vb.m_CurrPosition;

	float lab = vab.Length();
	float lcb = vcb.Length();

	if (lab * lcb == 0)
		return;

	float invAB = 1.0f / lab;
	float invCB = 1.0f / lcb;

	GFPhysVector3 n1 = vab*invAB;
	GFPhysVector3 n2 = vcb*invCB;

	float d = n1.Dot(n2);

	GPClamp(d, -1.0f, 1.0f);

	float dd = sqrtf(1 - d*d);

	GFPhysVector3 Col0 = GFPhysVector3(1 - n1.m_x*n1.m_x, -n1.m_y*n1.m_x, -n1.m_z*n1.m_x) * invAB;
	GFPhysVector3 Col1 = GFPhysVector3(-n1.m_x*n1.m_y, 1 - n1.m_y*n1.m_y, -n1.m_z*n1.m_y) * invAB;
	GFPhysVector3 Col2 = GFPhysVector3(-n1.m_x*n1.m_z, -n1.m_y*n1.m_z, 1 - n1.m_z*n1.m_z) * invAB;
	GFPhysVector3 gradVa = GFPhysVector3(Col0.Dot(n2), Col1.Dot(n2), Col2.Dot(n2));

	Col0 = GFPhysVector3(1 - n2.m_x*n2.m_x, -n2.m_y*n2.m_x, -n2.m_z*n2.m_x) * invCB;
	Col1 = GFPhysVector3(-n2.m_x*n2.m_y, 1 - n2.m_y*n2.m_y, -n2.m_z*n2.m_y) * invCB;
	Col2 = GFPhysVector3(-n2.m_x*n2.m_z, -n2.m_y*n2.m_z, 1 - n2.m_z*n2.m_z) * invCB;

	//@note !! in fact gradVc is gradVc * (-1.0f / dd)
	//but 1.0f/dd may be large and the sumgrad is square of (1.0f/dd) may be large than the float type 
	//precise for numerical robust , we eliminate this factor(-1.0f / dd) with the denomination
	//of sum grad and , finally multiply to result
	GFPhysVector3 gradVc = GFPhysVector3(Col0.Dot(n1), Col1.Dot(n1), Col2.Dot(n1));//gradVa *= arcdx;//gradVc *= arcdx;

	GFPhysVector3 gradVb = -gradVa - gradVc;

	float sumgrad = (wa*gradVa.Length2() + wc*gradVc.Length2() + wb*gradVb.Length2()) * (1 + gamma) + InvStiff * dd * dd;

	if (fabsf(sumgrad) > FLT_EPSILON)
	{
		float c = acosf(d) - 3.1415926f;//minus PI(180 degree)

		float dampcomponent = gradVa.Dot(va.m_CurrPosition - va.m_LastPosition)
			+ gradVb.Dot(vb.m_CurrPosition - vb.m_LastPosition)
			+ gradVc.Dot(vc.m_CurrPosition - vc.m_LastPosition);

		float dlambdaPrim = ((-c - InvStiff * lambda) * dd + gamma * dampcomponent) / sumgrad;

		lambda += (dlambdaPrim * dd);

		va.m_CurrPosition += -gradVa * (dlambdaPrim*wa) * solvehardness;

		vb.m_CurrPosition += -gradVb * (dlambdaPrim*wb) * solvehardness;

		vc.m_CurrPosition += -gradVc * (dlambdaPrim*wc) * solvehardness;
	}

}
//==========================================================================================================================
void MisMedicThreadRope::SolveStretchX(ThreadNode & n0, ThreadNode & n1, float & lambda, float InvStiff, float damping, float dt, float RestLen)
{
	Real gamma = InvStiff * damping / dt;

	Real w1 = n0.GetSolverInvMass();

	Real w2 = n1.GetSolverInvMass();

	Real w = w1 + w2;

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

	//update position
	n0.m_CurrPosition += Grad1 * (w1 * dLambda);
	n1.m_CurrPosition += (-Grad1) * (w2 * dLambda);
}
//============================================================================================================================================
void MisMedicThreadRope::PrepareSolveConstraint(Real Stiffness, Real TimeStep)
{
	m_SolveItorCount = 0;

	for (size_t n = 0; n < m_ThreadNodes.size(); n++)
	{
		m_ThreadNodes[n].m_LambdaBend = 0;
		m_ThreadNodes[n].m_LambdaStretch = 0;
		//m_ThreadNodes[n].m_BendSolveHardness = 1.0f;
	}

	//
	for (size_t t = 0; t < m_TFCollidePair.size(); t++)
	{
		TFCollidePair & tfPair = m_TFCollidePair[t];

		//int segIndex = tfPair.GetCollideSegmentIndex();

		//if(segIndex >= 0)
		//{
		//m_ThreadNodes[segIndex].m_FrameCollideTag = true;
		//m_ThreadNodes[segIndex+1].m_FrameCollideTag = true;
		//}

		GFPhysSoftBodyFace * softFace = tfPair.m_SoftFace;
		if (softFace)
		{
			softFace->m_Nodes[0]->m_InvM = softFace->m_Nodes[0]->m_OriginInvMass*0.2f;
			softFace->m_Nodes[1]->m_InvM = softFace->m_Nodes[1]->m_OriginInvMass*0.2f;
			softFace->m_Nodes[2]->m_InvM = softFace->m_Nodes[2]->m_OriginInvMass*0.2f;
		}
	}

	float originLen = GetTotalLen(false);
	float currLen = GetTotalLen(true);
	m_SolveMassScale = 1.0f;
	if (currLen > originLen)
	{
		float t = currLen / originLen;
		m_SolveMassScale *= (t*t*t);
	}

	for (size_t n = 0; n < m_ThreadNodes.size(); n++)
	{
		ThreadNode & tNode = m_ThreadNodes[n];
		int NodeState = tNode.GetRealTimeState();
		int NodeTag = tNode.GetTag();

		bool inKnotCircle = (NodeTag & TET_INKNOT);

		bool inCollide = (NodeState & (TRT_COLLIDERIGID | TRT_COLLIDESOFT));

		if ((inKnotCircle == false) && inCollide)
		{
			tNode.SetSolverInvMassScale(rscollideMassScale);
		}
	}
	return;
}

#define ADDITIONALBENDSUPPORT 1
void MisMedicThreadRope::SolveConstraint(Real Stiffness, Real TimeStep)
{
	//solve constraint
	float stretchStiffness = m_RopeStrechStiffness;//1.0f;//GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(1.0f);

	float bendStiffness = m_RopeBendStiffness;//0.9f;//GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.999f);

	float physBendStiff = 1.0f / 1000.0f;
	float physStretchStiff = 1.0f / 800.0f;
	//for(int i = 0 ; i < 2 ; i++)
	{
		//solve stretch constraint
		if ((m_SolveItorCount % 2) == 0)
		{
			for (int n = 0; n < m_ThreadNodes.size() - 1; n++)
			{
				//SolveStretch(m_ThreadNodes[n], m_ThreadNodes[n + 1], stretchStiffness, 1);
				SolveStretchX(m_ThreadNodes[n], m_ThreadNodes[n + 1], m_ThreadNodes[n].m_LambdaStretch,
					physStretchStiff, 0, TimeStep, m_UnitLen);


			}
			//solve bend angle
			for (int n = 0; n < m_ThreadNodes.size() - 2; n++)
			{
				//SolveBend(m_ThreadNodes[n], m_ThreadNodes[n + 1], m_ThreadNodes[n + 2], bendStiffness);

				SolveBendX(m_ThreadNodes[n], m_ThreadNodes[n + 1], m_ThreadNodes[n + 2],
					m_ThreadNodes[n + 1].m_LambdaBend, physBendStiff, 5.0f, 1.0f, TimeStep);
			}
		}
		else
		{
			for (int n = m_ThreadNodes.size() - 2; n >= 0; n--)
			{
				//SolveStretch(m_ThreadNodes[n], m_ThreadNodes[n + 1], stretchStiffness, 1);

				SolveStretchX(m_ThreadNodes[n], m_ThreadNodes[n + 1], m_ThreadNodes[n].m_LambdaStretch,
					physStretchStiff, 0, TimeStep, m_UnitLen);
			}

			//solve bend angle
			for (int n = m_ThreadNodes.size() - 3; n >= 0; n--)
			{
				//SolveBend(m_ThreadNodes[n], m_ThreadNodes[n + 1], m_ThreadNodes[n + 2], bendStiffness);

				SolveBendX(m_ThreadNodes[n], m_ThreadNodes[n + 1], m_ThreadNodes[n + 2],
					m_ThreadNodes[n + 1].m_LambdaBend, physBendStiff, 5.0f, 1.0f, TimeStep);
			}
		}

		/*for (int n = 0; n < m_ThreadNodes.size() - 1; n++)
		{
		SolveStretchX(m_ThreadNodes[n], m_ThreadNodes[n + 1], m_ThreadNodes[n].m_LambdaStretch,
		0, 0, TimeStep, m_UnitLen);
		}
		//solve bend angle
		for (int n = 0; n < m_ThreadNodes.size() - 2; n++)
		{
		//SolveBendX(m_ThreadNodes[n], m_ThreadNodes[n + 1], m_ThreadNodes[n + 2],
		/// m_ThreadNodes[n + 1].m_LambdaBend, 0, 0.0f, 0.4f, TimeStep);

		SolveBend(m_ThreadNodes[n], m_ThreadNodes[n + 1], m_ThreadNodes[n + 2], bendStiffness);
		}*/

#if(ADDITIONALBENDSUPPORT)
		for (int n = 0; n < (int)m_ThreadNodes.size() - 4; n++)
		{
			SolveBend(m_ThreadNodes[n], m_ThreadNodes[n + 2], m_ThreadNodes[n + 4], bendStiffness*0.5f);
		}
#endif

		SolveAdditionConstraint();//addition constraint

		//solve soft-thread collision
		SolveSoftThreadCollisions();

		//solve rigid-thread collision
		SolveRigidThreadCollisions(TimeStep);

		//
		SolveThreadThreadCollisions();

		m_SolveItorCount++;
	}

}