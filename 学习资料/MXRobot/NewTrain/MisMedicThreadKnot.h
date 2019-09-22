#ifndef _MISMEDICTHREADKNOT_
#define _MISMEDICTHREADKNOT_
#include <vector>
#include "MisMedicThreadRope.h"

class KnotIntervalPair
{
public:
	KnotIntervalPair();
	void SetPair(int segA , float weightA , int segB , float weightB);

	GFPhysVector3 GetCollideNormal(MisMedicThreadRope * rope , bool onsideL = true) const;
	GFPhysVector3 GetSDirection(MisMedicThreadRope * rope) const;
	GFPhysVector3 GetLDirection(MisMedicThreadRope * rope) const;

	void CheckInWarpState(MisMedicThreadRope * rope);

	void CalcKnotPairTetra(MisMedicThreadRope * rope);
	void CalcCollideTopology(MisMedicThreadRope * rope);

	void CorrectPosition(MisMedicThreadRope * rope);
//protected:

	//bool  CanOtherSWarpFormKnotWithMe(MisMedicThreadRope * rope ,  const KnotIntervalPair & other) const;
	
	//bool  CanOtherLWarpFormKnotWithMe(MisMedicThreadRope * rope ,  const KnotIntervalPair & other) const;

	//get 2 S warp pair's stretch direction's opposite value
	float Get_S_WarpStretchDirOppsite(MisMedicThreadRope * rope ,  const KnotIntervalPair & other)  const;

	//get 2 L warp pair's stretch direction's opposite value
	float Get_L_WarpStretchDirOppsite(MisMedicThreadRope * rope ,  const KnotIntervalPair & other)  const;

	float Get_S_NormalOppsiteValue(MisMedicThreadRope * rope ,  const KnotIntervalPair & other)  const;//get 2 warp in "s" segment's collide normal around axis dot value
	float Get_L_NormalOppsiteValue(MisMedicThreadRope * rope ,  const KnotIntervalPair & other)  const;//get 2 warp in "l" segment's collide normal around axis dot value

	//float CalcWarpRotateSign_S(MisMedicThreadRope * rope);//warp in "s-1 , s , s" around l segment > 0 means clock wise < counter clock wise
	//float CalcWarpRotateSign_L(MisMedicThreadRope * rope);//warp in "l-1 , l , l" around l segment 

	//collide segment with small index
	int collideSeg_S;
	float collideWeight_S;

	//collide segment with large index
	int collideSeg_L;
	float collideWeight_L;

	//float m_collideSegAngular;
	//float m_collideDist;//

	bool m_SInWarpShape;
	bool m_LInWarpShape;

	//float m_LengthS0S1;
	//float m_LengthS0L0;
	//float m_LengthS0L1;

	//float m_LengthS1L0;
	//float m_LengthS1L1;
	//float m_LengthL0L1;

	//float m_RestVolume;

	//
	//float m_SWarpRotate;// >0 unti-clock wise else clock wise
	//float m_LWarpRotate;// >0 unti-clock wise else clock wise

	float m_OrientRotTheta;
	//float m_OrientTangent;
};

class KnotInRope
{
public:
	KnotInRope();

	KnotInRope(const TTCollidePair & pairP , const TTCollidePair & pairQ);

	KnotInRope(const KnotIntervalPair & pairP , const KnotIntervalPair & pairQ);

	//return result (-2Pi 2PI) else means calculate failed
	//return negative meanse rotate clock wise positive meanse anti-clock wise
	static float CalcRelativePairRotate( MisMedicThreadRope * threadobj , 
										 const KnotIntervalPair & pairBase ,
										 const KnotIntervalPair & pairOther ,
										 int WarpType);

	void CalcKnotPairOrient(MisMedicThreadRope * threadobj);

	void BuildKnotPoint(MisMedicThreadRope * threadobj , std::vector<Ogre::Vector3> & rendPoints);

	void SolveKnotConstraint(MisMedicThreadRope * rope);

	int m_KnotType;//0 -- S wrapped 1-- L wrapped

	bool m_IsClockWise;//

	std::vector<KnotIntervalPair> m_WarpPairs;//if it is s warp

	//float m_RefTanValue;
};

class MisMedicThreadKnot : public GoPhys::GFPhysSoftBodyConstraint
{
public:
	MisMedicThreadKnot();
	~MisMedicThreadKnot();
	void CheckAnyKnotFormed();

	void CheckWarpsInKnot(KnotInRope & NewInterval , float collideRadius , std::vector<KnotIntervalPair> & PairsInWarp , int WarpType);

	void UpdateKnot();


	void UpdateThreadMesh();

	void Update(float dt);
	void SolveKnotConstraint();

	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	void SolveConstraint(Real globalstiffness,Real TimeStep);

	void BuildBendingSection();

	MisMedicThreadRope * m_ThreadObject;
	std::vector<KnotInRope> m_Knots;

	//KnotInterval m_KnotInProcess;
	bool m_HasKnot;
};


#endif