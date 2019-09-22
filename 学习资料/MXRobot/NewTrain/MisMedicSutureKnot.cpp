#include "MisMedicSutureKnot.h"
#include "SutureNeedle.h"
#include <algorithm>
#include <numeric>
//#include <QDebug>

void solveCenterPointOfCircle(const GFPhysVector3 pd[3], GFPhysVector3& centerpoint)
{
    Real a1, b1, c1, d1;
    Real a2, b2, c2, d2;
    Real a3, b3, c3, d3;

    Real x1 = pd[0].GetX(), y1 = pd[0].GetY(), z1 = pd[0].GetZ();
    Real x2 = pd[1].GetX(), y2 = pd[1].GetY(), z2 = pd[1].GetZ();
    Real x3 = pd[2].GetX(), y3 = pd[2].GetY(), z3 = pd[2].GetZ();

    a1 = (y1*z2 - y2*z1 - y1*z3 + y3*z1 + y2*z3 - y3*z2);
    b1 = -(x1*z2 - x2*z1 - x1*z3 + x3*z1 + x2*z3 - x3*z2);
    c1 = (x1*y2 - x2*y1 - x1*y3 + x3*y1 + x2*y3 - x3*y2);
    d1 = -(x1*y2*z3 - x1*y3*z2 - x2*y1*z3 + x2*y3*z1 + x3*y1*z2 - x3*y2*z1);

    a2 = 2 * (x2 - x1);
    b2 = 2 * (y2 - y1);
    c2 = 2 * (z2 - z1);
    d2 = x1 * x1 + y1 * y1 + z1 * z1 - x2 * x2 - y2 * y2 - z2 * z2;

    a3 = 2 * (x3 - x1);
    b3 = 2 * (y3 - y1);
    c3 = 2 * (z3 - z1);
    d3 = x1 * x1 + y1 * y1 + z1 * z1 - x3 * x3 - y3 * y3 - z3 * z3;

    centerpoint[0] = -(b1*c2*d3 - b1*c3*d2 - b2*c1*d3 + b2*c3*d1 + b3*c1*d2 - b3*c2*d1)
        / (a1*b2*c3 - a1*b3*c2 - a2*b1*c3 + a2*b3*c1 + a3*b1*c2 - a3*b2*c1);
    centerpoint[1] = (a1*c2*d3 - a1*c3*d2 - a2*c1*d3 + a2*c3*d1 + a3*c1*d2 - a3*c2*d1)
        / (a1*b2*c3 - a1*b3*c2 - a2*b1*c3 + a2*b3*c1 + a3*b1*c2 - a3*b2*c1);
    centerpoint[2] = -(a1*b2*d3 - a1*b3*d2 - a2*b1*d3 + a2*b3*d1 + a3*b1*d2 - a3*b2*d1)
        / (a1*b2*c3 - a1*b3*c2 - a2*b1*c3 + a2*b3*c1 + a3*b1*c2 - a3*b2*c1);

}

Real CalcAngleDiff(GFPhysVector3& v1, GFPhysVector3& v2)//Calculated angle difference
{
    GFPhysQuaternion q = ShortestArcQuatNormalize2(v1, v2);

    return q.GetAngle();
}
//============================================================================================
Real FindGreatestSumOfSubArray(const GFPhysVectorObj<Real>& pData, int & begin, int & end)
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
knotpbd::knotpbd()
{
    m_Rope = 0;
    m_bWork = true;
}
//============================================================================================
knotpbd::knotpbd(SutureThread * rope, int A, int B)
{
	m_A = A;
	m_B = B;
	m_weightA = 0.0f;
	m_weightB = 0.0f;
	m_Rope = rope;
	m_bWork = true;
}

knotpbd::knotpbd(STSTCollidePair* pair)
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
	m_Rope = pair->m_RopeA;
	m_bWork = true;

	m_staticFrict = 0.02f;
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
knotpbd::~knotpbd()
{  
	int k = 1;
}
//============================================================================================
void knotpbd::UpdatePosition()
{
    if (true /*m_bWork*/)
    {
        GFPhysVector3 S0, S1, L0, L1;

        SutureRopeNode ns0, ns1, nl0, nl1;

        m_Rope->GetThreadSegmentNode(ns0, ns1, m_A);
        m_Rope->GetThreadSegmentNode(nl0, nl1, m_B);

        m_Rope->GetThreadSegmentNodePos(S0, S1, m_A);
        m_Rope->GetThreadSegmentNodePos(L0, L1, m_B);

        float w0 = ns0.GetSolverInvMass();
        float w1 = ns1.GetSolverInvMass();
        float w2 = nl0.GetSolverInvMass();
        float w3 = nl1.GetSolverInvMass();

        GFPhysVector3 PS = S0 * (1 - m_weightA) + S1 * m_weightA;
        GFPhysVector3 PL = L0 * (1 - m_weightB) + L1 * m_weightB;

        GFPhysVector3 Diff = (PS - PL);

        Real Length = Diff.Length();

        float correctStiff = 1.0f;//0.85f;

        if (Length > FLT_EPSILON)
        {
            Diff /= Length;

            float wS0 = (1 - m_weightA);
            float wS1 = m_weightA;

            float wL0 = (1 - m_weightB);
            float wL1 = m_weightB;


            GFPhysVector3 gradS0 = Diff * wS0;
            GFPhysVector3 gradS1 = Diff * wS1;

            GFPhysVector3 gradL0 = -Diff * wL0;
            GFPhysVector3 gradL1 = -Diff * wL1;

            //weighted by inverse mass 's sum of gradient
            Real sumgrad = gradS0.Length2()*w0 + gradS1.Length2()*w1 + gradL0.Length2()*w2 + gradL1.Length2()*w3;//wS0*wS0 + wS1*wS1 + wL0*wL0 + wL1*wL1;

            Real scale = (-Length) / sumgrad;

            GFPhysVector3 deltaS0 = scale*gradS0*w0;
            GFPhysVector3 deltaS1 = scale*gradS1*w1;

            GFPhysVector3 deltaL0 = scale*gradL0*w2;
            GFPhysVector3 deltaL1 = scale*gradL1*w3;

            //============================================================================================

            m_Rope->GetThreadNodeRef(m_A).m_CurrPosition = S0 + deltaS0;
            m_Rope->GetThreadNodeRef(m_A + 1).m_CurrPosition = S1 + deltaS1;
            m_Rope->GetThreadNodeRef(m_B).m_CurrPosition = L0 + deltaL0;
            m_Rope->GetThreadNodeRef(m_B + 1).m_CurrPosition = L1 + deltaL1;
            //============================================================================================
        }
    }    
}
//============================================================================================

KnotInSutureRope::KnotInSutureRope()
{    
}
//============================================================================================
KnotInSutureRope::KnotInSutureRope(knotpbd& k0, knotpbd& k1)
{
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
    
    
    m_staticFrict = 0.02f;
    m_slack = true;

    m_Angle = 0.0f;
}

//============================================================================================
KnotInSutureRope::KnotInSutureRope(knotpbd& k1)
{
    m_knotcon1 = k1;
    m_knotcon0 = knotpbd();

    //////////////////////////////////////////////////////////////////////////
    m_knotcon0.m_A = m_knotcon1.m_A - 1;
    m_knotcon0.m_B = m_knotcon1.m_B - 1;
    m_knotcon0.m_weightA = 0.0f;
    m_knotcon0.m_weightB = 0.0f;

    m_knotcon0.m_Rope = k1.m_Rope;
    m_knotcon0.m_bWork = true;

    m_knotcon0.m_staticFrict = 0.02f;
    //////////////////////////////////////////////////////////////////////////
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

    m_staticFrict = 0.02f;
    m_slack = true;

    m_Angle = 0.0f;
}
//============================================================================================
KnotInSutureRope::~KnotInSutureRope()
{    
	int k = 1;
}
//============================================================================================
SutureKnot::SutureKnot()
{
    m_bHasKnot = false;
    m_Thread = 0;
    m_deadKnots.clear();
    PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
}
//============================================================================================
SutureKnot::~SutureKnot()
{
    if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
        PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}
//============================================================================================
void SutureKnot::Update(Real dt,bool surgenknot)
{
	GFPhysVectorObj<GFPhysFaceRopeAnchor*> activeRopeAnchors;
	m_Thread->m_NeedleAttchedThread->GetFaceRopeAnchors(activeRopeAnchors);

	if (!m_bHasKnot && activeRopeAnchors.size() > 1 && activeRopeAnchors.size() % 2 == 0)
	{
		CheckAnyKnotFormed(surgenknot);
	}
    else
    {
        m_Thread->SetKnotIndexMax(m_currentKnot.m_knotcon1.m_B + m_currentKnot.m_knotcon1.m_weightB);
        m_Thread->SetKnotIndexMin(m_currentKnot.m_knotcon0.m_A + m_currentKnot.m_knotcon0.m_weightA);

		if (m_bHasKnot)
		{
			for (int n = 0; n < m_Thread->GetNumSegments(); n++)
			{
				SutureRopeNode& refNode = m_Thread->GetThreadNodeRef(n);
				refNode.SetCanCollideSelf(false);
			}		
		}

        if (!(m_currentKnot.m_knotcon0.m_bWork) && !(m_currentKnot.m_knotcon1.m_bWork))
        {
            m_bHasKnot = false;

            m_deadKnots.push_back(m_currentKnot);

            if ((int)m_deadKnots.size() == 1)
            {
                for (int n = 0; n < m_Thread->GetNumThreadNodes(); n++)
                {
                    SutureRopeNode& refNode = m_Thread->GetThreadNodeRef(n);
                    refNode.SetCanCollideSelf(true);
                }

                for (int n = m_deadKnots[0].m_knotcon0.m_A; n < m_deadKnots[0].m_knotcon1.m_B; n++)
                {
                    SutureRopeNode& refNode = m_Thread->GetThreadNodeRef(n);
                    refNode.SetCanCollideSelf(false);
                }
            }
            else if ((int)m_deadKnots.size() == 2)
            {
                for (int n = 0; n < m_Thread->GetNumThreadNodes(); n++)
                {
                    SutureRopeNode& refNode = m_Thread->GetThreadNodeRef(n);
                    refNode.SetCanCollideSelf(true);
                }

                for (int n = m_deadKnots[1].m_knotcon0.m_A; n < m_deadKnots[1].m_knotcon1.m_B; n++)
                {
                    SutureRopeNode& refNode = m_Thread->GetThreadNodeRef(n);
                    refNode.SetCanCollideSelf(false);
                }
            }
            else
            { }
           
            //m_currentKnot = KnotInSutureRope();无需置零
        }
    }
}
//============================================================================================
int SutureKnot::CalcNormality(const STSTCollidePair& pair)
{
    GFPhysVector3 tangetA, tangetB;
    if (pair.m_RopeA->GetThreadSegmentTangent(tangetA, pair.m_SegmentA) && 
        pair.m_RopeB->GetThreadSegmentTangent(tangetB, pair.m_SegmentB))
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
GFPhysVector3 SutureKnot::CalcLocalNormal(const STSTCollidePair& pair)
{
    int k = pair.m_SegmentA;

    GFPhysVector3 n0 = m_Thread->GetThreadNodeRef(k).m_CurrPosition;
    GFPhysVector3 n1 = m_Thread->GetThreadNodeRef(k + 1).m_CurrPosition;
    GFPhysVector3 n0matVec = n0 + m_Thread->GetThreadNodeRef(k).m_MaterialVector;

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
}
//============================================================================================
bool SutureKnot::AvoidTwist(int start , int end)
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
bool SutureKnot::RegularBind(int start, int end)
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
bool SutureKnot::NormalCollision()
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
bool OrderPairByS(const STSTCollidePair & lhs, const STSTCollidePair & rhs)
{
    float lweight = lhs.m_SegmentA + lhs.m_WeightA;
    float rweight = rhs.m_SegmentA + rhs.m_WeightA;

    return lweight < rweight;
}
//============================================================================================

void SutureKnot::PairsShuffle(const GFPhysAlignedVectorObj<STSTCollidePair> & pairs)
{
	GFPhysAlignedVectorObj<STSTCollidePair> fliterpair;
	for (size_t t = 0; t < pairs.size(); t++)
	{
		if (pairs[t].m_SegmentB + pairs[t].m_WeightB >= m_Thread->GetRopeAnchorIndexMax()
			&& pairs[t].m_SegmentA + pairs[t].m_WeightA <= m_Thread->GetRopeAnchorIndexMin())
			
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
bool SutureKnot::CheckAnyKnotFormed(bool surgenknot)
{
	const GFPhysAlignedVectorObj<STSTCollidePair> & OriginPairs = m_Thread->GetCollidePairsWithThread();

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

				Angle_ts.push_back(CalcAngleDiff(normal_0, normal_t));

				int b = -1;
				int e = -1;
				Real sumAngle = FindGreatestSumOfSubArray(Angle_ts, b, e);

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
						//for (int n = 0; n < m_Thread->GetNumSegments(); n++)
						//{
						//	SutureRopeNode& refNode = m_Thread->GetThreadNodeRef(n);
						//	refNode.SetCanCollideSelf(false);
						//}
						m_currentKnot = KnotInSutureRope(knotpbd(&m_collidePairs[b]), knotpbd(&m_collidePairs[e]));
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
void SutureKnot::PrepareKnotAnchor(KnotInSutureRope& knot)
{
    if (knot.m_knotcon0.m_bWork && knot.m_knotcon1.m_bWork)
    {
        SutureRopeNode & At0 = knot.m_knotcon0.m_Rope->GetThreadNodeRef(knot.m_knotcon0.m_A - 1);
        SutureRopeNode & At1 = knot.m_knotcon0.m_Rope->GetThreadNodeRef(knot.m_knotcon0.m_A);

        SutureRopeNode & Bt0 = knot.m_knotcon1.m_Rope->GetThreadNodeRef(knot.m_knotcon1.m_B - 1);
        SutureRopeNode & Bt1 = knot.m_knotcon1.m_Rope->GetThreadNodeRef(knot.m_knotcon1.m_B);

        GFPhysVector3 collpointA = At0.m_CurrPosition * (1 - knot.m_knotcon0.m_weightA) + At1.m_CurrPosition * knot.m_knotcon0.m_weightA;

        GFPhysVector3 collpointB = Bt0.m_CurrPosition * (1 - knot.m_knotcon1.m_weightB) + Bt1.m_CurrPosition * knot.m_knotcon1.m_weightB;

        knot.m_KnotDirection = (collpointA - collpointB).Normalized();

    }
}
//============================================================================================
void SutureKnot::PrepareSolveConstraint( Real Stiffness,Real TimeStep )
{
    if (m_bHasKnot )
    {
        m_IterorCount = 0;

        PrepareKnotAnchor(m_currentKnot);
    }    
}
//============================================================================================
void SutureKnot::SolveConstraint(Real Stiffness, Real TimeStep)
{        
    if (m_bHasKnot)
    {
        m_IterorCount++;

        m_currentKnot.m_knotcon0.UpdatePosition();
        m_currentKnot.m_knotcon1.UpdatePosition();
    }


    for (int i = 0; i < (int)m_deadKnots.size();i++)
    {
        m_deadKnots[i].m_knotcon0.UpdatePosition();
        m_deadKnots[i].m_knotcon1.UpdatePosition();
    }
}
