#include "MisMedicSutureKnotV2.h"
#include <algorithm>
#include <numeric>
#include "SutureThreadV2.h"
//#include <QDebug>

//void solveCenterPointOfCircle(const GFPhysVector3 pd[3], GFPhysVector3& centerpoint)
//{
//	Real a1, b1, c1, d1;
//	Real a2, b2, c2, d2;
//	Real a3, b3, c3, d3;
//
//	Real x1 = pd[0].GetX(), y1 = pd[0].GetY(), z1 = pd[0].GetZ();
//	Real x2 = pd[1].GetX(), y2 = pd[1].GetY(), z2 = pd[1].GetZ();
//	Real x3 = pd[2].GetX(), y3 = pd[2].GetY(), z3 = pd[2].GetZ();
//
//	a1 = (y1*z2 - y2*z1 - y1*z3 + y3*z1 + y2*z3 - y3*z2);
//	b1 = -(x1*z2 - x2*z1 - x1*z3 + x3*z1 + x2*z3 - x3*z2);
//	c1 = (x1*y2 - x2*y1 - x1*y3 + x3*y1 + x2*y3 - x3*y2);
//	d1 = -(x1*y2*z3 - x1*y3*z2 - x2*y1*z3 + x2*y3*z1 + x3*y1*z2 - x3*y2*z1);
//
//	a2 = 2 * (x2 - x1);
//	b2 = 2 * (y2 - y1);
//	c2 = 2 * (z2 - z1);
//	d2 = x1 * x1 + y1 * y1 + z1 * z1 - x2 * x2 - y2 * y2 - z2 * z2;
//
//	a3 = 2 * (x3 - x1);
//	b3 = 2 * (y3 - y1);
//	c3 = 2 * (z3 - z1);
//	d3 = x1 * x1 + y1 * y1 + z1 * z1 - x3 * x3 - y3 * y3 - z3 * z3;
//
//	centerpoint[0] = -(b1*c2*d3 - b1*c3*d2 - b2*c1*d3 + b2*c3*d1 + b3*c1*d2 - b3*c2*d1)
//		/ (a1*b2*c3 - a1*b3*c2 - a2*b1*c3 + a2*b3*c1 + a3*b1*c2 - a3*b2*c1);
//	centerpoint[1] = (a1*c2*d3 - a1*c3*d2 - a2*c1*d3 + a2*c3*d1 + a3*c1*d2 - a3*c2*d1)
//		/ (a1*b2*c3 - a1*b3*c2 - a2*b1*c3 + a2*b3*c1 + a3*b1*c2 - a3*b2*c1);
//	centerpoint[2] = -(a1*b2*d3 - a1*b3*d2 - a2*b1*d3 + a2*b3*d1 + a3*b1*d2 - a3*b2*d1)
//		/ (a1*b2*c3 - a1*b3*c2 - a2*b1*c3 + a2*b3*c1 + a3*b1*c2 - a3*b2*c1);
//
//}
//
Real CalcAngleDiffV2(GFPhysVector3& v1, GFPhysVector3& v2)//Calculated angle difference
{
	GFPhysQuaternion q = ShortestArcQuatNormalize2(v1, v2);

	return q.GetAngle();
}
//============================================================================================
Real FindGreatestSumOfSubArrayV2(const GFPhysVectorObj<Real>& pData, int & begin, int & end)
{
	if (pData.size() == 0)
	{
		return 0.0f;
	}
	Real nCurSum = 0.0f;
	Real nGreatestSum = -GP_INFINITY;
	for (size_t i = 0; i < pData.size(); i++)
	{
		if (nCurSum <= 0.0f)
		{
			nCurSum = pData[i];
			begin = i;
		}
		else
		{
			nCurSum += pData[i];
		}

		if (nCurSum >= nGreatestSum)
		{
			nGreatestSum = nCurSum;
			end = i;
		}
	}
	return nGreatestSum;
}

const Real SegmentInOneKnot = 1.5f;
const Real SegmentPerimeter = SegmentInOneKnot + 2.0f;
const Real SegmentBetweenDead = 0.5f;


//============================================================================================
knotpbdV2::knotpbdV2()
{
	m_Rope = 0;
	m_bWork = true;
}
//============================================================================================
knotpbdV2::knotpbdV2(SutureThreadV2 * rope,int A,int B)
{
	m_A = A;
	m_B = B;
	m_weightA = 0.0f;
	m_weightB = 0.0f;
	m_Rope = rope;
	m_bWork = true;
}
//============================================================================================
knotpbdV2::knotpbdV2(STVSTCollidePair* pair)
{
	m_A = pair->m_SegmentA;
	m_B = pair->m_SegmentB;

	if (true)
	{
		m_weightA = 0.0f;
		m_weightB = 0.0f;
		//m_weightA = pair->m_WeightA;
		//m_weightB = pair->m_WeightB;
	}
	if (pair->m_WeightA > 0.5f)
	{
		m_A += 1;
	}
	if (pair->m_WeightB > 0.5f)
	{
		m_B += 1;
	}
	m_Rope = (SutureThreadV2*)pair->m_TubeA;
	m_bWork = true;

	
	if (m_B >= m_Rope->GetNumThreadNodes() - 1)
	{
		m_B = m_Rope->GetNumThreadNodes() - 2;
	}

	if (m_A < 1)
	{
		m_A = 1;
	}
}
//============================================================================================
knotpbdV2::~knotpbdV2()
{
}
//============================================================================================
void knotpbdV2::UpdatePosition()
{
	if (true)
	{
		GFPhysSoftTubeNode * anchor0 = m_Rope->GetTubeWireSegment(m_A).GetAnchorPoint();
		GFPhysSoftTubeNode * anchor1 = m_Rope->GetTubeWireSegment(m_B).GetAnchorPoint();


		GFPhysVector3 PS = anchor0->m_CurrPosition;
		GFPhysVector3 PL = anchor1->m_CurrPosition;

		GFPhysVector3 Diff = (PS - PL);

		Real Length = Diff.Length();

		float correctStiff = 1.0f;//0.85f;

		if (Length > FLT_EPSILON)
		{
			anchor0->m_CurrPosition -= Diff * 0.5f;
			anchor1->m_CurrPosition += Diff * 0.5f;
		}
	}
}
//============================================================================================

KnotInSutureRopeV2::KnotInSutureRopeV2()
{
	m_RopFaceAnchorMinInKnot = -1;
	m_RopFaceAnchorMaxInKnot = -1;
}
//============================================================================================
KnotInSutureRopeV2::~KnotInSutureRopeV2()
{
	
}
//============================================================================================
KnotInSutureRopeV2::KnotInSutureRopeV2(SutureThreadV2 * tube, knotpbdV2& k0, knotpbdV2& k1)
{
	//now k0.m_SegmentA <= k1.m_SegmentA <= k0.m_SegmentB <= k1.m_SegmentB
	m_Tube = tube;

	m_knotcon0 = k0;
	m_knotcon1 = k1;
	int lengthsmall = (m_knotcon1.m_A - m_knotcon0.m_A);
	int lengthbig = (m_knotcon1.m_B - m_knotcon0.m_B);

	int knotlength;
	if (lengthsmall > lengthbig)
	{
		m_knotcon1.m_A = lengthbig + m_knotcon0.m_A;
		knotlength = lengthbig;
	}
	else
	{
		m_knotcon1.m_B = lengthsmall + m_knotcon0.m_B;
		knotlength = lengthsmall;
	}

	if (knotlength < 2)
	{
		m_knotcon1.m_A = 2 + m_knotcon0.m_A;
		m_knotcon1.m_B = 2 + m_knotcon0.m_B;
	}
	
	m_InShrinkKnotStep =  false;

	m_Angle = 0.0f;

	//m_tempKnotCon = knotpbdV2(k0.m_Rope, k0.m_A + 1, k0.m_B + 1);

	//test
	m_knotcon0.m_weightA = m_knotcon0.m_weightB = 0.5f;
	m_knotcon1.m_weightA = m_knotcon1.m_weightB = 0.5f;
	//test
	m_Tube->GetSegment(m_knotcon0.m_A).SlipAnchorPoint(m_knotcon0.m_weightA);
	m_Tube->GetSegment(m_knotcon0.m_B).SlipAnchorPoint(m_knotcon0.m_weightB);

	m_Tube->GetSegment(m_knotcon1.m_A).SlipAnchorPoint(m_knotcon1.m_weightA);
	m_Tube->GetSegment(m_knotcon1.m_B).SlipAnchorPoint(m_knotcon1.m_weightB);

	//
	int minSeg = 1000;
	int maxSeg = -1000;
	for (int c = 0; c < (int)m_Tube->m_FaceRopeAnchors.size(); c++)
	{
		GFPhysFaceRopeAnchorV2 * faceRopeAnchor = m_Tube->m_FaceRopeAnchors[c];
		
		int segIndex = faceRopeAnchor->GetSegIndex();

		if (segIndex >= m_knotcon0.m_A && segIndex <= m_knotcon1.m_B)
		{
			faceRopeAnchor->m_IsSlipDisabled = true;
			if (segIndex < minSeg)
				minSeg = segIndex;
			else if (segIndex > maxSeg)
				maxSeg = segIndex;
		}
	}
	if (maxSeg >= minSeg)
	{
		m_RopFaceAnchorMinInKnot = minSeg;
		m_RopFaceAnchorMaxInKnot = maxSeg;
	}

	m_ShrinkRate = 1.0f;
}
#define USESPLITANCHORPOINT 0
//============================================================================================
void KnotInSutureRopeV2::SolveContraint()
{
	m_knotcon0.UpdatePosition();
	
	m_knotcon1.UpdatePosition();
	
	if (m_InShrinkKnotStep)
	{
		for (int c = 0; c < (int)m_Tube->m_FaceRopeAnchors.size()-1; c++)
		{
			GFPhysFaceRopeAnchorV2 * a0 = m_Tube->m_FaceRopeAnchors[c];
			GFPhysFaceRopeAnchorV2 * a1 = m_Tube->m_FaceRopeAnchors[c + 1];
			a0->m_AlignedToFace = true;
			a1->m_AlignedToFace = true;

			int seg0 = a0->GetSegIndex();
			int seg1 = a1->GetSegIndex();

			if (seg0 >= m_RopFaceAnchorMinInKnot && seg0 <= m_RopFaceAnchorMaxInKnot
			 && seg1 >= m_RopFaceAnchorMinInKnot && seg1 <= m_RopFaceAnchorMaxInKnot)
			{
				GFPhysSoftBodyFace * f0 = a0->GetFace();
				GFPhysSoftBodyFace * f1 = a0->GetFace();

				float restLen = (a0->GetAnchorOnFaceRestPos()-a1->GetAnchorOnFaceRestPos()).Length()*m_ShrinkRate;// m_Tube->GetRestDistBetweenSegments(a0->GetSegIndex(), a0->GetSegWeight(), a1->GetSegIndex(), a1->GetSegWeight());

#if(USESPLITANCHORPOINT)
				GFPhysSoftTubeNode * node0 = m_Tube->GetSegment(a0->GetSegIndex()).GetAnchorPoint();
				GFPhysSoftTubeNode * node1 = m_Tube->GetSegment(a1->GetSegIndex()).GetAnchorPoint();

				GFPhysVector3 ptThread0 = node0->m_CurrPosition;

				GFPhysVector3 ptThread1 = node1->m_CurrPosition;

				GFPhysVector3 diff = (ptThread0 - ptThread1);
				float Difflen = diff.Length();

				if (Difflen > GP_EPSILON)
				{
					GFPhysVector3 normDiff = diff / Difflen;

					float delta = Difflen - restLen;

					if (delta > 0)
					{
						node0->m_CurrPosition += -delta * normDiff * 0.5f;
						node1->m_CurrPosition +=  delta * normDiff * 0.5f;
					}
				}

#else
				GFPhysSoftTubeNode & t0 = m_Tube->GetThreadNodeRef(a0->GetSegIndex());
				GFPhysSoftTubeNode & t1 = m_Tube->GetThreadNodeRef(a0->GetSegIndex() + 1);
				GFPhysVector3 ptThread0 = t0.m_CurrPosition * (1 - a0->GetSegWeight()) + t1.m_CurrPosition * a0->GetSegWeight();

				GFPhysSoftTubeNode & t2 = m_Tube->GetThreadNodeRef(a1->GetSegIndex());
				GFPhysSoftTubeNode & t3 = m_Tube->GetThreadNodeRef(a1->GetSegIndex() + 1);
				GFPhysVector3 ptThread1 = t2.m_CurrPosition * (1 - a1->GetSegWeight()) + t3.m_CurrPosition * a1->GetSegWeight();

				GFPhysVector3 diff = (ptThread0 - ptThread1);
				float Difflen = diff.Length();

				if (Difflen > GP_EPSILON)
				{
					float w0 = a0->GetSegWeight();
					float w1 = a1->GetSegWeight();

					GFPhysVector3 normDiff = diff / Difflen;
					
					float delta = Difflen - restLen;
					
					if (delta > 0)
					{
						float w = (1 - w0)*(1 - w0) + w0 * w0 + (1 - w1)*(1 - w1) + w1 * w1;

						t0.m_CurrPosition += -delta * normDiff*((1 - w0)/w);
						t1.m_CurrPosition += -delta * normDiff*(w0 / w);

						t2.m_CurrPosition += delta * normDiff *((1 - w1) / w);
						t3.m_CurrPosition += delta * normDiff *(w1 / w);
					}
				}
#endif
			}
		}
	}
}
//============================================================================================
void KnotInSutureRopeV2::GetMinMaxSegIndex(int & minindex, int &maxindex)
{
	minindex = m_knotcon0.m_A;
	maxindex = m_knotcon1.m_B;
}
//============================================================================================
SutureKnotV2::SutureKnotV2()
{
	m_bHasKnot = false;
	m_Thread = 0;
	m_deadKnots.clear();
	//PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
}
//============================================================================================
SutureKnotV2::~SutureKnotV2()
{
	//if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	//	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}
//============================================================================================
void SutureKnotV2::Update(Real dt, bool surgenknot)
{
	GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
	m_Thread->GetFaceRopeAnchors(activeRopeAnchors);

	if (!m_bHasKnot /*&& activeRopeAnchors.size() > 1 && activeRopeAnchors.size() % 2 == 0*/)
	{
		CheckAnyKnotFormed(surgenknot);
	}
	else
	{
		//m_Thread->SetKnotIndexMax(m_currentKnot.m_knotcon1.m_B + m_currentKnot.m_knotcon1.m_weightB);
		//m_Thread->SetKnotIndexMin(m_currentKnot.m_knotcon0.m_A + m_currentKnot.m_knotcon0.m_weightA);

		if (m_bHasKnot)
		{
			for (int n = 0; n < m_Thread->GetNumSegments(); n++)
			{
				m_Thread->GetTubeWireSegment(n).SetCanCollideSelf(false);
			}

			GFPhysVectorObj<KnotInSutureRopeV2*> AllKnots;
			GetAllKnotsRef(AllKnots);
			if (AllKnots.size() == 1)
			{
				int anchorIndex[2];
				anchorIndex[0] = 0;
				anchorIndex[1] = activeRopeAnchors.size() - 1;

				for (int i = 0; i < 2; i++)
				{
					GFPhysFaceRopeAnchorV2* anchor = activeRopeAnchors[anchorIndex[i]];
					if (false == anchor->m_bHaveFixNor)
					{
#if 1
						anchor->m_FixFaceNormal = anchor->GetFace()->m_FaceNormal;

						if (anchor->m_type != STATE_OUT)
						{
							anchor->m_FixFaceNormal *= -1.0f;
						}
#else
						GFPhysVector3 Threaddir = m_Thread->GetTubeWireSegment(anchor->m_NodeIndex).m_Node1->m_CurrPosition -
							m_Thread->GetTubeWireSegment(anchor->m_NodeIndex).m_Node0->m_CurrPosition;

						anchor->m_FixFaceNormal = Threaddir.Normalized();
#endif						
						anchor->m_bHaveFixNor = true;
					}					
				}
			}
		}
	
		if (!(m_currentKnot.m_knotcon0.m_bWork) && !(m_currentKnot.m_knotcon1.m_bWork))
		{
			m_bHasKnot = false;

			m_deadKnots.push_back(m_currentKnot);
			//记录下成死结时刻的锚点间距
			if (m_Thread->m_dis < GP_EPSILON && activeRopeAnchors.size() >= 2)
			{
				int ni = activeRopeAnchors.size() - 1;
				GFPhysVector3 point0 = activeRopeAnchors[0]->GetFace()->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[0]->m_weights[0] +
					activeRopeAnchors[0]->GetFace()->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[0]->m_weights[1] +
					activeRopeAnchors[0]->GetFace()->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[0]->m_weights[2];

				GFPhysVector3 point1 = activeRopeAnchors[ni]->GetFace()->m_Nodes[0]->m_CurrPosition * activeRopeAnchors[ni]->m_weights[0] +
					activeRopeAnchors[ni]->GetFace()->m_Nodes[1]->m_CurrPosition * activeRopeAnchors[ni]->m_weights[1] +
					activeRopeAnchors[ni]->GetFace()->m_Nodes[2]->m_CurrPosition * activeRopeAnchors[ni]->m_weights[2];

				m_Thread->m_dis = (point0 - point1).Length();
			}

			if ((int)m_deadKnots.size() == 1)
			{
				for (int n = 0; n < m_Thread->GetNumSegments(); n++)
				{
					m_Thread->GetTubeWireSegment(n).SetCanCollideSelf(true);
				}

				for (int n = m_deadKnots[0].m_knotcon0.m_A-1; n < m_deadKnots[0].m_knotcon1.m_B+1; n++)
				{
					m_Thread->GetTubeWireSegment(n).SetCanCollideSelf(false);
				}
			}
			else if ((int)m_deadKnots.size() == 2)
			{
				for (int n = 0; n < m_Thread->GetNumSegments(); n++)
				{
					m_Thread->GetTubeWireSegment(n).SetCanCollideSelf(true);
				}

				for (int n = m_deadKnots[1].m_knotcon0.m_A-1; n < m_deadKnots[1].m_knotcon1.m_B+1; n++)
				{
					m_Thread->GetTubeWireSegment(n).SetCanCollideSelf(false);
				}
			}
			else
			{
			}

			//m_currentKnot = KnotInSutureRope();无需置零
		}
	}
}
//============================================================================================
int SutureKnotV2::CalcNormality(const STVSTCollidePair& pair)
{
	GFPhysVector3 tangetA = ((SutureThreadV2*)pair.m_TubeA)->GetNodeAvgTangent(pair.m_SegmentA);
	GFPhysVector3 tangetB = ((SutureThreadV2*)pair.m_TubeB)->GetNodeAvgTangent(pair.m_SegmentB);

	if (true)
	{
		GFPhysVector3 segNodePos[2];
		m_Thread->GetThreadSegmentNodePos(segNodePos[0], segNodePos[1], pair.m_SegmentA);
		GFPhysVector3 posInSmallSeg = segNodePos[0] * (1 - pair.m_WeightA) + segNodePos[1] * pair.m_WeightA;

		//point in large segment part
		m_Thread->GetThreadSegmentNodePos(segNodePos[0], segNodePos[1], pair.m_SegmentB);
		GFPhysVector3 posInLargSeg = segNodePos[0] * (1 - pair.m_WeightB) + segNodePos[1] * pair.m_WeightB;

		GFPhysVector3 CollideDir = (posInSmallSeg - posInLargSeg).Normalized();

		Real a = Triple(tangetA, tangetB, CollideDir);//(tangetA.Cross(tangetB)).Dot(CollideDir);
		if (a > 0.0f)
		{
			return 1;
		}
		else if (a < 0.0f)
		{
			return -1;
		}
	}
	return 0;
}
//============================================================================================
GFPhysVector3 SutureKnotV2::CalcLocalNormal(const STVSTCollidePair& pair)
{
	int k = pair.m_SegmentA;

	GFPhysVector3 n0;
	GFPhysVector3 n1;
	GFPhysVector3 n0matVec;

	m_Thread->GetSegmentMatFrameAxis(k, n0, n1, n0matVec, false);

	GFPhysMatrix3 Frame;
	m_Thread->computeMaterialFrame(n0, n1, n0matVec, Frame);

	//GFPhysVector3 temp = pair.m_NormalOnB;
	//temp *= -1.0f;

	GFPhysVector3 segNodePos[2];
	m_Thread->GetThreadSegmentNodePos(segNodePos[0], segNodePos[1], pair.m_SegmentA);
	GFPhysVector3 posInSmallSeg = segNodePos[0] * (1 - pair.m_WeightA) + segNodePos[1] * pair.m_WeightA;

	//point in large segment part
	m_Thread->GetThreadSegmentNodePos(segNodePos[0], segNodePos[1], pair.m_SegmentB);
	GFPhysVector3 posInLargSeg = segNodePos[0] * (1 - pair.m_WeightB) + segNodePos[1] * pair.m_WeightB;

	return Frame.Inverse() * (posInSmallSeg - posInLargSeg).Normalized();

	return GFPhysVector3(0, 0, 0);
}
//============================================================================================
bool SutureKnotV2::AvoidTwist(int start, int end)
{
	for (int n = start; n < end - 1; n++)
	{
		if (m_collidePairs[n + 1].m_SegmentB < m_collidePairs[n].m_SegmentB)
		{
			return false;
		}
	}
	return true;
}
//============================================================================================
bool SutureKnotV2::RegularBind(int start, int end)
{
	std::vector<Real> m;
	for (int t = start; t < end + 1; t++)
	{
		m.push_back(CalcNormality(m_collidePairs[t]));
	}

	if (std::abs((int)std::accumulate(m.begin(), m.end(), 0)) == (int)m.size())
	{
		return true;
	}
	return false;
}
//============================================================================================
bool SutureKnotV2::NormalCollision()
{
	int Amax = m_collidePairs[0].m_SegmentA;
	int Amin = m_collidePairs[0].m_SegmentA;
	int Bmax = m_collidePairs[0].m_SegmentB;
	int Bmin = m_collidePairs[0].m_SegmentB;

	for (size_t i = 1; i < m_collidePairs.size(); ++i)
	{
		if (m_collidePairs[i].m_SegmentA > Amax)
		{
			Amax = m_collidePairs[i].m_SegmentA;
		}
		if (m_collidePairs[i].m_SegmentA < Amin)
		{
			Amin = m_collidePairs[i].m_SegmentA;
		}
		if (m_collidePairs[i].m_SegmentB > Bmax)
		{
			Bmax = m_collidePairs[i].m_SegmentB;
		}
		if (m_collidePairs[i].m_SegmentB < Bmin)
		{
			Bmin = m_collidePairs[i].m_SegmentB;
		}
	}

	if (Amax - Amin > 0 || Bmax - Bmin > 0)
	{
		return true;
	}
	return false;
}
//============================================================================================
bool OrderPairByS(const STVSTCollidePair & lhs, const STVSTCollidePair & rhs)
{
	float lweight = lhs.m_SegmentA + lhs.m_WeightA;
	float rweight = rhs.m_SegmentA + rhs.m_WeightA;

	return lweight < rweight;
}
//============================================================================================

void SutureKnotV2::PairsShuffle(const GFPhysAlignedVectorObj<STVSTCollidePair> & pairs)
{
	GFPhysAlignedVectorObj<STVSTCollidePair> fliterpair;
	for (size_t t = 0; t < pairs.size(); t++)
	{
		if (/*pairs[t].m_SegmentB + pairs[t].m_WeightB >= m_Thread->GetRopeAnchorIndexMax()
			&& pairs[t].m_SegmentA + pairs[t].m_WeightA <= m_Thread->GetRopeAnchorIndexMin()*/ true)

		{
			fliterpair.push_back(pairs[t]);
		}
	}

	if (0 == fliterpair.size())
	{
		return;
	}

	m_collidePairs.clear();//reentry 
	std::vector<int> segsmall;

	int n0 = fliterpair[0].GetCollideSegmentA();
	int n0_Max_n1 = -100;
	int index = -100;
	for (size_t t = 0; t < fliterpair.size(); t++)
	{
		if (n0 == fliterpair[t].GetCollideSegmentA())
		{
			int b = fliterpair[t].GetCollideSegmentB();
			if (n0_Max_n1 < b)
			{
				n0_Max_n1 = b;
				index = t;
			}
		}
	}

	m_collidePairs.push_back(fliterpair[index]);
	segsmall.push_back(fliterpair[index].GetCollideSegmentA());

	for (size_t t = 1; t < fliterpair.size(); t++)
	{
		std::vector<int>::iterator result = find(segsmall.begin(), segsmall.end(), fliterpair[t].GetCollideSegmentA());
		//必须要优化，这种find的效率很低，至少要换成hashmap.
		if (result == segsmall.end()) //segsmall 里面没有 
		{
			int n0 = fliterpair[t].GetCollideSegmentA();
			int n0_Max_n1 = -100;
			int index = -100;
			for (size_t s = t; s < fliterpair.size(); s++)
			{
				if (n0 == fliterpair[s].GetCollideSegmentA())
				{
					int b = fliterpair[s].GetCollideSegmentB();
					if (n0_Max_n1 < b)
					{
						n0_Max_n1 = b;
						index = s;
					}
				}
			}

			m_collidePairs.push_back(fliterpair[index]);
			segsmall.push_back(fliterpair[index].GetCollideSegmentA());
		}
	}

	std::sort(m_collidePairs.begin(), m_collidePairs.end(), OrderPairByS);

}
//============================================================================================
bool SutureKnotV2::CheckAnyKnotFormed(bool surgenknot)
{
	const GFPhysAlignedVectorObj<STVSTCollidePair> & OriginPairs = m_Thread->m_TTCollidepair;

	GFPhysVectorObj<Real> Angle_ts;

	if (OriginPairs.size() > 1 && m_Thread->m_ClampSegIndexVector.size() > 0)
	{
		PairsShuffle(OriginPairs);

		if (0 == m_collidePairs.size())
		{
			return false;
		}

		if (NormalCollision())
		{
			GFPhysVector3 normal_0 = CalcLocalNormal(m_collidePairs[0]);

			for (size_t t = 1; t < m_collidePairs.size(); t++)
			{
				GFPhysVector3 normal_t = CalcLocalNormal(m_collidePairs[t]);

				Angle_ts.push_back(CalcAngleDiffV2(normal_0, normal_t));

				int b = -1;
				int e = -1;
				Real sumAngle = FindGreatestSumOfSubArrayV2(Angle_ts, b, e);

				Real ratio = fabsf(sumAngle) / GP_PI;
				Ogre::LogManager::getSingleton().logMessage(Ogre::String("check knot form ratio is  ") + Ogre::StringConverter::toString(ratio));

				if (ratio > 2.0f * 0.4f)
				{
					bool b_AnotherKnot = true;
					if (m_deadKnots.size() != 0)
					{
						if (m_collidePairs[b].m_SegmentB < m_deadKnots[m_deadKnots.size() - 1].m_knotcon1.m_B)
						{
							b_AnotherKnot = false;
						}
						if (m_collidePairs[e].m_SegmentA > m_deadKnots[m_deadKnots.size() - 1].m_knotcon0.m_A)
						{
							b_AnotherKnot = false;
						}
					}
					bool CorrectKnot = true;  //make sure knot contain rope anchor
					GFPhysVectorObj<Real> RopeAnchorIndexVec = m_Thread->GetRopeAnchorIndex();
					if (RopeAnchorIndexVec.size() > 0)
					{
						Real ba = m_collidePairs[b].m_SegmentA + m_collidePairs[b].m_WeightA;
						Real bb = m_collidePairs[b].m_SegmentB + m_collidePairs[b].m_WeightB;
						Real ea = m_collidePairs[e].m_SegmentA + m_collidePairs[e].m_WeightA;
						Real eb = m_collidePairs[e].m_SegmentB + m_collidePairs[e].m_WeightB;
						Order4(ba, bb, ea, eb);//small to large
						for (size_t i = 0; i < RopeAnchorIndexVec.size(); i++)
						{
							if (RopeAnchorIndexVec[i] > ea || RopeAnchorIndexVec[i] < bb)
							{
								CorrectKnot = false;
								break;
							}
						}
					}

					if (RegularBind(b, e) && AvoidTwist(b, e) && b_AnotherKnot && CorrectKnot
						&& m_collidePairs[b].m_SegmentB >= m_collidePairs[e].m_SegmentA
						&& m_collidePairs[b].m_SegmentB <= m_collidePairs[e].m_SegmentB
						&& m_collidePairs[e].m_SegmentA >= m_collidePairs[b].m_SegmentA
						&& abs((m_collidePairs[e].m_SegmentB - m_collidePairs[b].m_SegmentB) - (m_collidePairs[e].m_SegmentA - m_collidePairs[b].m_SegmentA)) < 6) //防止成结前一边过大
					{
						m_bHasKnot = true;
						
						//now m_collidePairs[b].m_SegmentA <= m_collidePairs[e].m_SegmentA <= m_collidePairs[b].m_SegmentB <= m_collidePairs[e].m_SegmentB
						
						m_currentKnot = KnotInSutureRopeV2(m_Thread , knotpbdV2(&m_collidePairs[b]), knotpbdV2(&m_collidePairs[e]));
						
						if (surgenknot)
						{
							m_currentKnot.m_Angle = 2.0f*GP_2PI;
						}
						else
						{
							m_currentKnot.m_Angle = GP_2PI;
						}

						if (sumAngle > GP_EPSILON)
						{
							m_currentKnot.m_Clockwise = true;
						}
						else
						{
							m_currentKnot.m_Clockwise = false;
						}

						return true;
					}
					else
					{
						Ogre::LogManager::getSingleton().logMessage(Ogre::String("knot judge is too hard"));
					}
					break;

				}
				normal_0 = normal_t;
			}
		}
	}
	return false;
}
//============================================================================================
void SutureKnotV2::PrepareKnotAnchor(KnotInSutureRopeV2& knot)
{
	if (knot.m_knotcon0.m_bWork && knot.m_knotcon1.m_bWork)
	{
		GFPhysSoftTubeNode & At0 = knot.m_knotcon0.m_Rope->GetThreadNodeRef(knot.m_knotcon0.m_A - 1);
		GFPhysSoftTubeNode & At1 = knot.m_knotcon0.m_Rope->GetThreadNodeRef(knot.m_knotcon0.m_A);

		GFPhysSoftTubeNode & Bt0 = knot.m_knotcon1.m_Rope->GetThreadNodeRef(knot.m_knotcon1.m_B - 1);
		GFPhysSoftTubeNode & Bt1 = knot.m_knotcon1.m_Rope->GetThreadNodeRef(knot.m_knotcon1.m_B);

		GFPhysVector3 collpointA = At0.m_CurrPosition * (1 - knot.m_knotcon0.m_weightA) + At1.m_CurrPosition * knot.m_knotcon0.m_weightA;

		GFPhysVector3 collpointB = Bt0.m_CurrPosition * (1 - knot.m_knotcon1.m_weightB) + Bt1.m_CurrPosition * knot.m_knotcon1.m_weightB;

		//knot.m_KnotDirection = (collpointA - collpointB).Normalized();

	}
}
//============================================================================================
void SutureKnotV2::PrepareSolveConstraint(Real Stiffness, Real TimeStep)
{
	if (m_bHasKnot)
	{
		m_IterorCount = 0;

		PrepareKnotAnchor(m_currentKnot);
	}
}
//============================================================================================
void SutureKnotV2::SolveConstraint(Real Stiffness, Real TimeStep)
{
	if (m_bHasKnot)
	{
		m_IterorCount++;
		m_currentKnot.SolveContraint();
	}


	for (int i = 0; i < (int)m_deadKnots.size(); i++)
	{
		m_deadKnots[i].SolveContraint();
	}
}
//==============================================================================================================================
void SutureKnotV2::GetKnotsSegMinMax(int & segmin, int &segmax)
{
	segmin = 10000;
	segmax = -10000;

	GFPhysVectorObj<KnotInSutureRopeV2> allknots;

	GetAllKnots(allknots);

	for (int c = 0; c < allknots.size(); c++)
	{
		int localmin, localmax;

		allknots[c].GetMinMaxSegIndex(localmin, localmax);

		if (localmin < segmin)
			segmin = localmin;

		if (localmax > segmax)
			segmax = localmax;
	}
}
//==============================================================================================================================
GFPhysVectorObj<KnotInSutureRopeV2> & SutureKnotV2::GetDeadKnots()
{ 
	return m_deadKnots;
}
//==============================================================================================================================
void SutureKnotV2::GetAllKnots(GFPhysVectorObj<KnotInSutureRopeV2> & knots)
{
	if (!m_deadKnots.empty())
	{
		for (size_t c = 0; c < m_deadKnots.size(); c++)
		{
			knots.push_back(m_deadKnots[c]);
		}
	}
	if (m_bHasKnot)
		knots.push_back(m_currentKnot);
}
//==============================================================================================================================
void SutureKnotV2::GetAllKnotsRef(GFPhysVectorObj<KnotInSutureRopeV2*> & knots)
{
	if (!m_deadKnots.empty())
	{
		for (size_t c = 0; c < m_deadKnots.size(); c++)
		{
			knots.push_back(&m_deadKnots[c]);
		}
	}
	if (m_bHasKnot)
		knots.push_back(&m_currentKnot);
}
//==============================================================================================================================
void SutureKnotV2::GetDeadKnots(GFPhysVectorObj<KnotInSutureRopeV2> & knots)
{
	if (!m_deadKnots.empty())
	{
		for (size_t c = 0; c < m_deadKnots.size(); c++)
		{
			knots.push_back(m_deadKnots[c]);
		}
	}
}
//==============================================================================================================================
void SutureKnotV2::GetDeadKnotsRef(GFPhysVectorObj<KnotInSutureRopeV2*> & knots)
{
	if (!m_deadKnots.empty())
	{
		for (size_t c = 0; c < m_deadKnots.size(); c++)
		{
			knots.push_back(&m_deadKnots[c]);
		}
	}
}