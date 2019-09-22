#pragma once
#include "SutureThread.h"

//============================================================================================
Real CalcAngleDiff(GFPhysVector3& v1, GFPhysVector3& v2);//Calculated angle difference
//============================================================================================
Real FindGreatestSumOfSubArray(const GFPhysVectorObj<Real>& pData, int & begin, int & end);
//////////////////////////////////////////////////////////////////////////
class knotpbd
{
public:
    knotpbd();
	knotpbd(SutureThread * rope, int A, int B);
    knotpbd(STSTCollidePair* pair);
    ~knotpbd();
    void UpdatePosition();    

public:
    int             m_A;
    int             m_B;
    Real            m_weightA;
    Real            m_weightB;
    SutureThread *  m_Rope;
    bool            m_bWork;

    Real            m_staticFrict;
};
//////////////////////////////////////////////////////////////////////////
class KnotInSutureRope
{
public:
    KnotInSutureRope();
    KnotInSutureRope(knotpbd& k0, knotpbd& k1);
    KnotInSutureRope(knotpbd& k0);
    ~KnotInSutureRope();

public:
    knotpbd m_knotcon0;
    knotpbd m_knotcon1;    

    Real     m_staticFrict;
    GFPhysVector3 m_KnotDirection;
    Real    m_Angle;
    bool    m_Clockwise;
    bool    m_slack;//切换收缩步骤的变量
};

//////////////////////////////////////////////////////////////////////////
class SutureKnot : public GoPhys::GFPhysPositionConstraint
{
public:
    SutureKnot();
    ~SutureKnot();
    void Update(Real dt,bool surgenknot);
    bool CheckAnyKnotFormed(bool surgenknot = false);

    void PairsShuffle(const GFPhysAlignedVectorObj<STSTCollidePair> & pairs);

    

    int CalcNormality(const STSTCollidePair& pair);
    GFPhysVector3 CalcLocalNormal(const STSTCollidePair& pair);
    bool NormalCollision();//避免碰撞失效设置碰撞对间的最小长度
    
    bool AvoidTwist(int start, int end);//避免同向绕圈

    bool RegularBind(int start,int end);
    void PrepareKnotAnchor(KnotInSutureRope& knot);

    void PrepareSolveConstraint(Real Stiffness,Real TimeStep);
    void SolveConstraint(Real Stiffness, Real TimeStep);
    inline void SetThread(SutureThread* thread){ m_Thread = thread; }

	GFPhysVectorObj<KnotInSutureRope> & GetDeadKnots(){ return m_deadKnots; }
    void GetAllKnots(GFPhysVectorObj<KnotInSutureRope> & knots)
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

    void GetAllKnotsRef(GFPhysVectorObj<KnotInSutureRope*> & knots)
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

    void GetDeadKnots(GFPhysVectorObj<KnotInSutureRope> & knots)
    {
        if (!m_deadKnots.empty())
        {
            for (size_t c = 0; c < m_deadKnots.size(); c++)
            {
                knots.push_back(m_deadKnots[c]);
            }
        }
    }

    void GetDeadKnotsRef(GFPhysVectorObj<KnotInSutureRope*> & knots)
    {
        if (!m_deadKnots.empty())
        {
            for (size_t c = 0; c < m_deadKnots.size(); c++)
            {
                knots.push_back(&m_deadKnots[c]);
            }
        }
    }
	KnotInSutureRope & GetCurrKnot(){ return m_currentKnot; }
public:
    bool m_bHasKnot;
	KnotInSutureRope m_currentKnot;
//private:
    GFPhysVectorObj<STSTCollidePair> m_collidePairs;

    SutureThread* m_Thread;    



    GFPhysVectorObj<KnotInSutureRope> m_deadKnots;

    int m_IterorCount;
};