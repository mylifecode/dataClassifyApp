#pragma once
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/Constraint/GoPhysPositionConstraint.h"
class SutureThreadV2;
class STVSTCollidePair;
using namespace GoPhys;
//#include "MisMedicSutureKnot.h" // for two function
//============================================================================================
Real CalcAngleDiffV2(GFPhysVector3& v1, GFPhysVector3& v2);//Calculated angle difference
//============================================================================================
Real FindGreatestSumOfSubArrayV2(const GFPhysVectorObj<Real>& pData, int & begin, int & end);
////////////////////////////////////////////////////////////////////////////
class knotpbdV2
{
public:
	knotpbdV2();
	knotpbdV2(SutureThreadV2 * rope, int A, int B);
	knotpbdV2(STVSTCollidePair* pair);
	~knotpbdV2();
	void UpdatePosition();

public:
	int             m_A;
	int             m_B;
	Real            m_weightA;
	Real            m_weightB;
	SutureThreadV2* m_Rope;
	bool            m_bWork;

	
};
//////////////////////////////////////////////////////////////////////////
class KnotInSutureRopeV2
{
public:
	KnotInSutureRopeV2();
	
	KnotInSutureRopeV2(SutureThreadV2 * tube , knotpbdV2& k0, knotpbdV2& k1);
	
	~KnotInSutureRopeV2();

	void SolveContraint();

	void GetMinMaxSegIndex(int & minindex , int &maxindex);

public:

	int m_RopFaceAnchorMinInKnot;
	
	int m_RopFaceAnchorMaxInKnot;

	SutureThreadV2 * m_Tube;
	knotpbdV2 m_knotcon0;
	knotpbdV2 m_knotcon1;

	Real    m_Angle;
	
	bool    m_Clockwise;
	
	bool    m_InShrinkKnotStep;//切换收缩步骤的变量

	float   m_ShrinkRate;
};

//////////////////////////////////////////////////////////////////////////
class SutureKnotV2 : public GoPhys::GFPhysPositionConstraint
{
public:
	SutureKnotV2();
	~SutureKnotV2();
	void Update(Real dt, bool surgenknot);
	bool CheckAnyKnotFormed(bool surgenknot = false);

	void PairsShuffle(const GFPhysAlignedVectorObj<STVSTCollidePair> & pairs);



	int CalcNormality(const STVSTCollidePair& pair);
	GFPhysVector3 CalcLocalNormal(const STVSTCollidePair& pair);
	bool NormalCollision();//避免碰撞失效设置碰撞对间的最小长度

	bool AvoidTwist(int start, int end);//避免同向绕圈

	bool RegularBind(int start, int end);
	void PrepareKnotAnchor(KnotInSutureRopeV2& knot);

	void PrepareSolveConstraint(Real Stiffness, Real TimeStep);
	void SolveConstraint(Real Stiffness, Real TimeStep);
	inline void SetThread(SutureThreadV2* thread){ m_Thread = thread; }

	void GetKnotsSegMinMax(int & segmin, int &segmax);

	GFPhysVectorObj<KnotInSutureRopeV2> & GetDeadKnots();

	void GetAllKnots(GFPhysVectorObj<KnotInSutureRopeV2> & knots);

	void GetAllKnotsRef(GFPhysVectorObj<KnotInSutureRopeV2*> & knots);

	void GetDeadKnots(GFPhysVectorObj<KnotInSutureRopeV2> & knots);

	void GetDeadKnotsRef(GFPhysVectorObj<KnotInSutureRopeV2*> & knots);

	KnotInSutureRopeV2 & GetCurrKnot(){ return m_currentKnot; }
public:
	bool m_bHasKnot;
	KnotInSutureRopeV2 m_currentKnot;
//private:
	GFPhysVectorObj<STVSTCollidePair> m_collidePairs;

	SutureThreadV2* m_Thread;

	//KnotInSutureRopeV2 m_currentKnot;

	GFPhysVectorObj<KnotInSutureRopeV2> m_deadKnots;

	int m_IterorCount;
};