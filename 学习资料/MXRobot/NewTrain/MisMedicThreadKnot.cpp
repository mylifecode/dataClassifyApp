#include "MisMedicThreadKnot.h"
#include "MXOgreGraphic.h"
#include "utility/GoPhysSoftBodyUtil.h"
//======================================================================================
KnotIntervalPair::KnotIntervalPair()
{
	collideSeg_S = -1;
	collideSeg_L = -1;
	m_SInWarpShape = false;
	m_LInWarpShape = false;
	m_OrientRotTheta = 0.0f;
}
//======================================================================================
void KnotIntervalPair::SetPair(int segA , float weightA , int segB , float weightB)
{
	if(segA + weightA < segB + weightB)
	{
		collideSeg_S = segA;
		collideWeight_S = weightA;

		collideSeg_L = segB;
		collideWeight_L = weightB;
	}
	else
	{
		collideSeg_S = segB;
		collideWeight_S = weightB;

		collideSeg_L = segA;
		collideWeight_L = weightA;
	}
}
//======================================================================================
void KnotIntervalPair::CheckInWarpState(MisMedicThreadRope * ThreadObject)
{
	GFPhysVector3 normalOnPair = GetCollideNormal(ThreadObject , true);
	//float d0 = 1;
	//float d1 = 1;

	GFPhysVector3 prevSegPos[2]; 
	
	int NumOppsiteDir_P = 0;
	int NumOppsiteDir_N = 0;
	//for(int c = 1 ; c <= 1 ; c++)
	//{
	
	int PrevSeg = collideSeg_S-1;
	int NextSeg = collideSeg_S+1;

	if(PrevSeg >= 2)
	{
	   ThreadObject->GetThreadSegmentNodePos(prevSegPos[0] , prevSegPos[1] , PrevSeg);
	   GFPhysVector3 direction = (prevSegPos[0]-prevSegPos[1]).Normalized();
	   float dot = direction.Dot(normalOnPair);
	   if(dot < 0)//-0.4f)
	   {
		  NumOppsiteDir_P++;
	   }
	}

	if(NextSeg < ThreadObject->GetNumThreadNodes()-3)
	{
		ThreadObject->GetThreadSegmentNodePos(prevSegPos[0] , prevSegPos[1] , NextSeg);
		GFPhysVector3 direction = (prevSegPos[1]-prevSegPos[0]).Normalized();
		float dot = direction.Dot(normalOnPair);
		if(dot < 0)//-0.4f)
		{
		   NumOppsiteDir_N++;
		}
	}
	//}
	if(NumOppsiteDir_P > 0 && NumOppsiteDir_N > 0)
	{
		m_SInWarpShape = true;
	}
	else
	{
		m_SInWarpShape = false;
	}


	NumOppsiteDir_P = 0;
	NumOppsiteDir_N = 0;
	normalOnPair *= -1.0f;
	//for(int c = 1 ; c <= 1 ; c++)
	//{
	    PrevSeg  = collideSeg_L-1;
	    NextSeg = collideSeg_L+1;
		if(PrevSeg >= 2)
		{
			ThreadObject->GetThreadSegmentNodePos(prevSegPos[0] , prevSegPos[1] , PrevSeg);
			GFPhysVector3 direction = (prevSegPos[0]-prevSegPos[1]).Normalized();
			float dot = direction.Dot(normalOnPair);
			if(dot < 0)//-0.4f)
			{
				NumOppsiteDir_P++;
			}
		}

		if(NextSeg < ThreadObject->GetNumThreadNodes()-3)
		{
			ThreadObject->GetThreadSegmentNodePos(prevSegPos[0] , prevSegPos[1] , NextSeg);
			GFPhysVector3 direction = (prevSegPos[1]-prevSegPos[0]).Normalized();
			float dot = direction.Dot(normalOnPair);
			if(dot < 0)//-0.4f)
			{
				NumOppsiteDir_N++;
			}
		}
	//}
	if(NumOppsiteDir_P > 0 && NumOppsiteDir_N > 0)
	{
		m_LInWarpShape = true;
	}
	else
	{
		m_LInWarpShape = false;
	}
	
}
//=================================================================================================================================
void KnotIntervalPair::CalcKnotPairTetra(MisMedicThreadRope * rope)
{
	/*
	float unitLen = rope->GetUnitLen();

	GFPhysVector3 rotAxis(0,0,1);

	GFPhysVector3 S0(0 , 0 , 0);
	GFPhysVector3 S1(unitLen , 0 , 0);
	GFPhysQuaternion rotQuat(rotAxis , m_collideSegAngular);

	GFPhysVector3 L0(0,0,0);
	GFPhysVector3 L1 = QuatRotate(rotQuat , (S1-S0));

	GFPhysVector3 collidesS = S0 * (1-collideWeight_S) + S1 * collideWeight_S;
	GFPhysVector3 collidesL = L0 * (1-collideWeight_L) + L1 * collideWeight_L;

	GFPhysVector3 delta = collidesS-collidesL;
	L0 = L0 + delta;// + rotAxis * m_collideDist;
	L1 = L1 + delta;// + rotAxis * m_collideDist;

	//tetra's 6 edge
	m_LengthS0S1 = (S1-S0).Length();
	m_LengthS0L0 = (L0-S0).Length();
	m_LengthS0L1 = (L1-S0).Length();

	m_LengthS1L0 = (L0-S1).Length();
	m_LengthS1L1 = (L1-S1).Length();
	m_LengthL0L1 = (L1-L0).Length();

	//tetra's volume
	m_RestVolume = GFPhysSoftBodyUtility::CalcTetraVolume(S0 , S1 , L0 , L1);
	*/
}
//=================================================================================================================================
void KnotIntervalPair::CalcCollideTopology(MisMedicThreadRope * rope)
{
	/*
	GFPhysVector3 S0 , S1 , L0 , L1;

    rope->GetThreadSegmentNodePos(S0 , S1 , collideSeg_S);
	rope->GetThreadSegmentNodePos(L0 , L1 , collideSeg_L);

	float LengthS0S1 = (S1-S0).Length();
	float LengthL0L1 = (L1-L0).Length();

	if(LengthS0S1 > FLT_EPSILON && LengthL0L1 > FLT_EPSILON)
	{
	   GFPhysVector3 es = ((S1-S0) / LengthS0S1);
	   GFPhysVector3 el = ((L1-L0) / LengthL0L1);
	   
	   m_collideSegAngular = es.Angle(el);//radian rotate from (s1,s0)-->(l1,l0)
	   GFPhysVector3 angleAxis = es.Cross(el).Normalized();

	   GFPhysVector3 collides = S0 * (1-collideWeight_S) + S1 * collideWeight_S;
	   GFPhysVector3 collidel = L0 * (1-collideWeight_L) + L1 * collideWeight_L;

	   //m_collideDist = (collidel-collides).Dot(angleAxis);

	}
	CalcKnotPairTetra(rope);
	//m_RestVolume = GFPhysSoftBodyUtility::CalcTetraVolume(S0 , S1 , L0 , L1);
	*/
}
//=================================================================================================================================
void KnotIntervalPair::CorrectPosition(MisMedicThreadRope * rope)
{
	GFPhysVector3 S0 , S1 , L0 , L1;

	ThreadNode ns0 , ns1 , nl0 , nl1;

	rope->GetThreadSegmentNode(ns0 , ns1 , collideSeg_S);
	rope->GetThreadSegmentNode(nl0 , nl1 , collideSeg_L);

	rope->GetThreadSegmentNodePos(S0 , S1 , collideSeg_S);
	rope->GetThreadSegmentNodePos(L0 , L1 , collideSeg_L);

	float w0 = ns0.GetSolverInvMass();
	float w1 = ns1.GetSolverInvMass();
	float w2 = nl0.GetSolverInvMass();
	float w3 = nl1.GetSolverInvMass();

	GFPhysVector3 PS = S0 * (1-collideWeight_S) + S1 * collideWeight_S;
	GFPhysVector3 PL = L0 * (1-collideWeight_L) + L1 * collideWeight_L;

	GFPhysVector3 Diff = (PS-PL);

	Real Length = Diff.Length();

	float correctStiff = 0.85f;
	
	if(Length > FLT_EPSILON)
	{	
		Diff  /= Length;

		float wS0 = (1-collideWeight_S);
		float wS1 = collideWeight_S;
		
		float wL0 = (1-collideWeight_L);
		float wL1 = collideWeight_L;
		
		
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

		rope->GetThreadNodeRef(collideSeg_S).m_CurrPosition   = S0+deltaS0;
		rope->GetThreadNodeRef(collideSeg_S+1).m_CurrPosition = S1+deltaS1;
		rope->GetThreadNodeRef(collideSeg_L).m_CurrPosition   = L0+deltaL0;
		rope->GetThreadNodeRef(collideSeg_L+1).m_CurrPosition = L1+deltaL1;
	}
}
//=================================================================================================================================
/*
float KnotIntervalPair::GetSRotateValue(MisMedicThreadRope * rope) const
{
	GFPhysVector3 segNodePos[4];
	rope->GetThreadSegmentNodePos(segNodePos[1] , segNodePos[2] , collideSeg_S);
	rope->GetThreadSegmentNodePos(segNodePos[0] , segNodePos[1] , collideSeg_S-1);
	rope->GetThreadSegmentNodePos(segNodePos[2] , segNodePos[3] , collideSeg_S+1);

	GFPhysVector3 dir0 = (segNodePos[1]-segNodePos[0]).Normalized();
	GFPhysVector3 dir1 = (segNodePos[3]-segNodePos[2]).Normalized();

	GFPhysVector3 rotateAxis = dir0.Cross(dir1).Normalized();

	GFPhysVector3 refNodePos[2];
	rope->GetThreadSegmentNodePos(refNodePos[0] , refNodePos[1] , collideSeg_L);
	GFPhysVector3 SegAxis = (refNodePos[1]-refNodePos[0]).Normalized();

	return rotateAxis.Dot(SegAxis);
}
//=================================================================================================================================
float KnotIntervalPair::GetLRotateValue(MisMedicThreadRope * rope) const
{
	GFPhysVector3 segNodePos[4];
	rope->GetThreadSegmentNodePos(segNodePos[1] , segNodePos[2] , collideSeg_L);
	rope->GetThreadSegmentNodePos(segNodePos[0] , segNodePos[1] , collideSeg_L-1);
	rope->GetThreadSegmentNodePos(segNodePos[2] , segNodePos[3] , collideSeg_L+1);

	GFPhysVector3 dir0 = (segNodePos[1]-segNodePos[0]).Normalized();
	GFPhysVector3 dir1 = (segNodePos[3]-segNodePos[2]).Normalized();

	GFPhysVector3 rotateAxis = dir0.Cross(dir1).Normalized();

	GFPhysVector3 refNodePos[2];
	rope->GetThreadSegmentNodePos(refNodePos[0] , refNodePos[1] , collideSeg_S);
	GFPhysVector3 SegAxis = (refNodePos[1]-refNodePos[0]).Normalized();

	return rotateAxis.Dot(SegAxis);
}
*/


//=================================================================================================================================
/*
bool  KnotIntervalPair::CanOtherSWarpFormKnotWithMe(MisMedicThreadRope * rope , const KnotIntervalPair & other)  const
{
	//check three condition
	//float vs = Get_S_WarpStretchDirOppsite(rope , other);
	
	float vn = Get_S_NormalOppsiteValue(rope , other);
	
	float t0 = m_SWarpRotate;//Get_S_RotateAroundLValue(rope);
	
	float t1 = other.m_SWarpRotate;//Get_S_RotateAroundLValue(rope);
	
	if(t0 * t1 > 0 &&  vn < -0.5f)
	   return true;
	else
	   return false;
}
*/
//=================================================================================================================================
/*
bool KnotIntervalPair::CanOtherLWarpFormKnotWithMe(MisMedicThreadRope * rope ,  const KnotIntervalPair & other) const
{
	//check three condition
	///float vs = Get_L_WarpStretchDirOppsite(rope , other);

	float vn = Get_L_NormalOppsiteValue(rope , other);

	float t0 = m_LWarpRotate;//Get_L_RotateAroundSValue(rope);

	float t1 = other.m_LWarpRotate;//Get_L_RotateAroundSValue(rope);

	if(t0 * t1 > 0 &&  vn < -0.5f)
		return true;
	else
		return false;
}
*/
//=================================================================================================================================
float KnotIntervalPair::Get_S_WarpStretchDirOppsite(MisMedicThreadRope * rope ,  const KnotIntervalPair & other) const
{
	float minSegIndex = -1;
	float maxSegIndex = -1;

	float minSegLIndex = -1;
	float maxSegLIndex = -1;

	if(collideSeg_S + collideWeight_S < other.collideSeg_S + other.collideWeight_S)
	{
	   minSegIndex  = collideSeg_S;
	   minSegLIndex = collideSeg_L;

	   maxSegIndex  = other.collideSeg_S;
	   maxSegLIndex = other.collideSeg_L;
	}
	else
	{
	   minSegIndex  = other.collideSeg_S;
	   minSegLIndex = other.collideSeg_L;

	   maxSegIndex  = collideSeg_S;
	   maxSegLIndex = collideSeg_L;
	}

	GFPhysVector3 PrevNodePos[2];
	GFPhysVector3 PostNodePos[2];

	if((minSegIndex-1) < 0 || (maxSegIndex+1) >= rope->GetNumSegments())
	    return 0;
	
	rope->GetThreadSegmentNodePos(PrevNodePos[0] , PrevNodePos[1] , minSegIndex-1);
	rope->GetThreadSegmentNodePos(PostNodePos[0] , PostNodePos[1] , maxSegIndex+1);

	GFPhysVector3 dir0 = (PrevNodePos[0]-PrevNodePos[1]).Normalized();
	GFPhysVector3 dir1 = (PostNodePos[1]-PostNodePos[0]).Normalized();

	GFPhysVector3 LNodePos[2];
	rope->GetThreadSegmentNodePos(LNodePos[0] , LNodePos[1] , minSegLIndex);
	GFPhysVector3 minLDir = (LNodePos[1]-LNodePos[0]).Normalized();

	rope->GetThreadSegmentNodePos(LNodePos[0] , LNodePos[1] , maxSegLIndex);
	GFPhysVector3 maxLDir = (LNodePos[1]-LNodePos[0]).Normalized();


	dir0 = (dir0 - minLDir * dir0.Dot(minLDir)).Normalized();
	dir1 = (dir1 - maxLDir * dir1.Dot(maxLDir)).Normalized();

	return dir0.Dot(dir1);
}	
//=================================================================================================================================
float KnotIntervalPair::Get_L_WarpStretchDirOppsite(MisMedicThreadRope * rope ,  const KnotIntervalPair & other)  const
{
	float minSegIndex = -1;
	float maxSegIndex = -1;

	float minSegSIndex = -1;
	float maxSegSIndex = -1;

	if(collideSeg_L + collideWeight_L < other.collideSeg_L + other.collideWeight_L)
	{
		minSegIndex  = collideSeg_L;
		minSegSIndex = collideSeg_S;

		maxSegIndex  = other.collideSeg_L;
		maxSegSIndex = other.collideSeg_S;
	}
	else
	{
		minSegIndex  = other.collideSeg_L;
		minSegSIndex = other.collideSeg_S;

		maxSegIndex  = collideSeg_L;
		maxSegSIndex = collideSeg_S;
	}

	GFPhysVector3 PrevNodePos[2];
	GFPhysVector3 PostNodePos[2];

	if((minSegIndex-1) < 0 || (maxSegIndex+1) >= rope->GetNumSegments())
		return 0;
	rope->GetThreadSegmentNodePos(PrevNodePos[0] , PrevNodePos[1] , minSegIndex-1);
	rope->GetThreadSegmentNodePos(PostNodePos[0] , PostNodePos[1] , maxSegIndex+1);

	GFPhysVector3 dir0 = (PrevNodePos[0]-PrevNodePos[1]).Normalized();
	GFPhysVector3 dir1 = (PostNodePos[1]-PostNodePos[0]).Normalized();

	GFPhysVector3 SNodePos[2];
	rope->GetThreadSegmentNodePos(SNodePos[0] , SNodePos[1] , minSegSIndex);
	GFPhysVector3 minSDir = (SNodePos[1]-SNodePos[0]).Normalized();

	rope->GetThreadSegmentNodePos(SNodePos[0] , SNodePos[1] , maxSegSIndex);
	GFPhysVector3 maxSDir = (SNodePos[1]-SNodePos[0]).Normalized();


	dir0 = (dir0 - minSDir * dir0.Dot(minSDir)).Normalized();
	dir1 = (dir1 - maxSDir * dir1.Dot(maxSDir)).Normalized();

	return dir0.Dot(dir1);
}
//=================================================================================================================================
float KnotIntervalPair::Get_S_NormalOppsiteValue(MisMedicThreadRope * rope ,  const KnotIntervalPair & other)  const
{
     GFPhysVector3  WarpNormalSelf = GetCollideNormal(rope , true);//normal point from l to s
	 GFPhysVector3  WarpNormalOther = other.GetCollideNormal(rope , true);//normal point from l to s

	 GFPhysVector3 LSegPosMe[2];
	 GFPhysVector3 LSegPosOther[2];
	 rope->GetThreadSegmentNodePos(LSegPosMe[0] , LSegPosMe[1] , collideSeg_L);
	 rope->GetThreadSegmentNodePos(LSegPosOther[0] , LSegPosOther[1] , other.collideSeg_L);

	 GFPhysVector3 L_DirMe = (LSegPosMe[1] - LSegPosMe[0]).Normalized();
	 GFPhysVector3 L_DirOther = (LSegPosOther[1] - LSegPosOther[0]).Normalized();

	 //rotate normal to other segment's frame
	 Ogre::Quaternion rotQuat = GPVec3ToOgre(L_DirMe).getRotationTo(GPVec3ToOgre(L_DirOther));
	 WarpNormalSelf = OgreToGPVec3(rotQuat*GPVec3ToOgre(WarpNormalSelf));

	 return WarpNormalSelf.Dot(WarpNormalOther);
}
//=================================================================================================================================
float KnotIntervalPair::Get_L_NormalOppsiteValue(MisMedicThreadRope * rope ,  const KnotIntervalPair & other)  const
{
	GFPhysVector3  WarpNormalSelf = GetCollideNormal(rope , false);//normal point from s to l
	GFPhysVector3  WarpNormalOther = other.GetCollideNormal(rope , false);//normal point from s to l

	GFPhysVector3 SSegPosMe[2];
	GFPhysVector3 SSegPosOther[2];
	rope->GetThreadSegmentNodePos(SSegPosMe[0] , SSegPosMe[1] , collideSeg_S);
	rope->GetThreadSegmentNodePos(SSegPosOther[0] , SSegPosOther[1] , other.collideSeg_S);

	GFPhysVector3 S_DirMe = (SSegPosMe[1] - SSegPosMe[0]).Normalized();
	GFPhysVector3 S_DirOther = (SSegPosOther[1] - SSegPosOther[0]).Normalized();

	//rotate normal to other segment's frame
	Ogre::Quaternion rotQuat = GPVec3ToOgre(S_DirMe).getRotationTo(GPVec3ToOgre(S_DirOther));
	WarpNormalSelf = OgreToGPVec3(rotQuat*GPVec3ToOgre(WarpNormalSelf));

	return WarpNormalSelf.Dot(WarpNormalOther);
}
//=================================================================================================================================
GFPhysVector3 KnotIntervalPair::GetCollideNormal(MisMedicThreadRope * rope , bool onsideL) const
{
	GFPhysVector3 segNodePos[2];

	//point in small segment part
	rope->GetThreadSegmentNodePos(segNodePos[0] , segNodePos[1] , collideSeg_S);
	GFPhysVector3 posInSmallSeg = segNodePos[0] * (1-collideWeight_S) + segNodePos[1] * collideWeight_S;

	//point in large segment part
	rope->GetThreadSegmentNodePos(segNodePos[0] , segNodePos[1] , collideSeg_L);
	GFPhysVector3 posInLargSeg = segNodePos[0] * (1-collideWeight_L) + segNodePos[1] * collideWeight_L;

	GFPhysVector3 normalVec = (posInSmallSeg-posInLargSeg).Normalized();
	if(onsideL == false)
	   normalVec *= -1.0f;

	return normalVec;
}
//======================================================================================
GFPhysVector3 KnotIntervalPair::GetSDirection(MisMedicThreadRope * rope)  const
{
	GFPhysVector3 segNodePos[2];
	//point in small segment part
	rope->GetThreadSegmentNodePos(segNodePos[0] , segNodePos[1] , collideSeg_S);
	return (segNodePos[1]-segNodePos[0]).Normalized();
}
//======================================================================================
GFPhysVector3 KnotIntervalPair::GetLDirection(MisMedicThreadRope * rope)  const
{
	GFPhysVector3 segNodePos[2];
	//point in small segment part
	rope->GetThreadSegmentNodePos(segNodePos[0] , segNodePos[1] , collideSeg_L);
	return (segNodePos[1]-segNodePos[0]).Normalized();
}
//======================================================================================
KnotInRope::KnotInRope() : m_IsClockWise(false)
{

}
//======================================================================================
KnotInRope::KnotInRope(const TTCollidePair & pairP , const TTCollidePair & pairQ) : m_IsClockWise(false)
{

}
//======================================================================================
KnotInRope::KnotInRope(const KnotIntervalPair & pairP , const KnotIntervalPair & pairQ) : m_IsClockWise(false)
{

}
//======================================================================================
void KnotInRope::SolveKnotConstraint(MisMedicThreadRope * rope)
{
	for(size_t k = 0 ; k < m_WarpPairs.size() ; k++)
	{
		m_WarpPairs[k].CorrectPosition(rope);
	}
}
//======================================================================================
float KnotInRope::CalcRelativePairRotate(MisMedicThreadRope * threadobj , 
										 const KnotIntervalPair & pairBase ,
										 const KnotIntervalPair & pairOther ,
										 int WarpType)
{
	GFPhysVector3 l0 , l1 , s0 , s1;

	threadobj->GetThreadSegmentNodePos(l0 , l1 , pairBase.collideSeg_L);
	threadobj->GetThreadSegmentNodePos(s0 , s1 , pairBase.collideSeg_S);

	//WarpType = 0 "Swarp" else "Lwarp"
	GFPhysVector3 BaseAxis = (WarpType == 0 ? (l1-l0) : (s1-s0));
	BaseAxis.Normalize();

	GFPhysVector3 BaseRotVec = (WarpType == 0 ? (s1-s0).Cross(l1-l0) : (l1-l0).Cross(s1-s0));
	BaseRotVec.Normalize();

	GFPhysVector3 ptOnAxis = (WarpType == 0 ? (l0 * (1-pairBase.collideWeight_L) + l1 * pairBase.collideWeight_L) : (s0 * (1-pairBase.collideWeight_S) + s1 * pairBase.collideWeight_S));
	GFPhysVector3 ptOnWarp = (WarpType == 0 ? (s0 * (1-pairBase.collideWeight_S) + s1 * pairBase.collideWeight_S) : (l0 * (1-pairBase.collideWeight_L) + l1 * pairBase.collideWeight_L));

	GFPhysVector3 BaseOutDir = (ptOnWarp-ptOnAxis).Normalized();

	bool isBaseClockWise = false;
	if(BaseOutDir.Dot( BaseRotVec) < 0)//clock wise
	{
	   isBaseClockWise = true;
	}

	threadobj->GetThreadSegmentNodePos(l0 , l1 , pairOther.collideSeg_L);
	threadobj->GetThreadSegmentNodePos(s0 , s1 , pairOther.collideSeg_S);
	
	GFPhysVector3 CurrAxis = (WarpType == 0 ? (l1-l0) : (s1-s0));
	CurrAxis.Normalize();

	GFPhysVector3 CurrRotVec = (WarpType == 0 ? (s1-s0).Cross(l1-l0) : (l1-l0).Cross(s1-s0));
	CurrRotVec.Normalize();

	ptOnAxis = (WarpType == 0 ? (l0 * (1-pairOther.collideWeight_L) + l1 * pairOther.collideWeight_L) : (s0 * (1-pairOther.collideWeight_S) + s1 * pairOther.collideWeight_S));
	ptOnWarp = (WarpType == 0 ? (s0 * (1-pairOther.collideWeight_S) + s1 * pairOther.collideWeight_S) : (l0 * (1-pairOther.collideWeight_L) + l1 * pairOther.collideWeight_L));

	GFPhysVector3 CurrOutDir = (ptOnWarp-ptOnAxis).Normalized();

	bool isOtherClockWise = false;
	if(CurrOutDir.Dot(CurrRotVec) < 0)//clock wise
	{
	   isOtherClockWise = true;
	}

	//not same direction rotate return failed!!
	if(isBaseClockWise != isOtherClockWise)
	{
	   return FLT_MAX;
	}

	GFPhysQuaternion FrameRotate = ShortestArcQuat(BaseAxis , CurrAxis);

	//transform init rotate vector to current axis frame
	GFPhysVector3 WCSInitOutDir = QuatRotate(FrameRotate , BaseOutDir);

	//calculate rotate between current rotate vector and init rotate vec
	GFPhysVector3 crossVec = WCSInitOutDir.Cross(CurrOutDir);
	crossVec.Normalize();

	float PairRotateTheta = acosf(WCSInitOutDir.Dot(CurrOutDir));

	if(crossVec.Dot(CurrAxis) < 0)
	{
	   PairRotateTheta = GP_2PI - PairRotateTheta;
	}

	//clock wise means rotate negative
	if(isBaseClockWise)
	{
	   PairRotateTheta -= GP_2PI;
	}

	GPClamp(PairRotateTheta , -GP_2PI , GP_2PI);

	return PairRotateTheta;
}
//======================================================================================
void KnotInRope::CalcKnotPairOrient(MisMedicThreadRope * threadobj)
{
		/*GFPhysVector3 l0 , l1 , s0 , s1;

		KnotIntervalPair & refPair = m_WarpPairs[0];

		threadobj->GetThreadSegmentNodePos(l0 , l1 , refPair.collideSeg_L);
		threadobj->GetThreadSegmentNodePos(s0 , s1 , refPair.collideSeg_S);

		GFPhysVector3 InitAxis = (m_KnotType == 0 ? (l1-l0) : (s1-s0));
		InitAxis.Normalize();

		GFPhysVector3 InitRotVec = (m_KnotType == 0 ? (s1-s0).Cross(l1-l0) : (l1-l0).Cross(s1-s0));
		InitRotVec.Normalize();

		bool isClockWise = m_IsClockWise;//(m_KnotType == 0 ? (refPair.m_SWarpRotate < 0) : (refPair.m_LWarpRotate < 0));//whether rotate is clock wise
		if(isClockWise)
		{
		   InitRotVec *= -1.0f;
		}
		refPair.m_OrientRotTheta = 0.0f;

		float prevRotateValue = 0;
		*/
	    KnotIntervalPair & pairHead = m_WarpPairs[0];
		
		for(size_t k = 1 ; k < m_WarpPairs.size() ; k++)
		{
			KnotIntervalPair & srcPair = m_WarpPairs[k];

			/*
			threadobj->GetThreadSegmentNodePos(l0 , l1 , currPair.collideSeg_L);
			threadobj->GetThreadSegmentNodePos(s0 , s1 , currPair.collideSeg_S);

			GFPhysVector3 Axis_Curr = (m_KnotType == 0 ? (l1-l0) : (s1-s0));
			Axis_Curr.Normalize();

			GFPhysVector3 CurRotVec = (m_KnotType == 0 ? (s1-s0).Cross(l1-l0) : (l1-l0).Cross(s1-s0));//(s1-s0).Cross(l1-l0);
			CurRotVec.Normalize();
			
			if(isClockWise)//clock wise
			   CurRotVec *= -1.0f;

			GFPhysQuaternion FrameRotate = ShortestArcQuat(InitAxis, Axis_Curr);
			
			//transform init rotate vector to current axis frame
			GFPhysVector3 WCSInitRotVec  = QuatRotate(FrameRotate , InitRotVec);

			//calculate rotate between current rotate vector and init rotate vec
			GFPhysVector3 crossVec = WCSInitRotVec.Cross(CurRotVec);
			crossVec.Normalize();

			float PairRotateTheta = acosf(WCSInitRotVec.Dot(CurRotVec));
			
			if(crossVec.Dot(Axis_Curr) < 0)
			{
			   PairRotateTheta = GP_2PI - PairRotateTheta;
			}
			*/
			/*
			float PairRotateTheta = CalcRelativePairRotate(threadobj , pairHead , srcPair , m_KnotType);

			if(isClockWise)
			{
				PairRotateTheta -= GP_2PI;
				while(PairRotateTheta > prevRotateValue)
				{
					PairRotateTheta -= GP_2PI;
				}
			}
			else
			{
				while(PairRotateTheta < prevRotateValue)
				{
					PairRotateTheta += GP_2PI;
				}
			}
			
			currPair.m_OrientRotTheta = PairRotateTheta;
			prevRotateValue = PairRotateTheta;*/
		}

}
void KnotInRope::BuildKnotPoint(MisMedicThreadRope * threadobj , std::vector<Ogre::Vector3> & RendNodes)
{
	//temp
	RendNodes.clear();
	for(size_t c = 0 ; c < threadobj->GetNumThreadNodes(); c++)
	{
		GFPhysVector3 temp = threadobj->GetThreadNode(c).m_CurrPosition;
		
		if(c >= m_WarpPairs[0].collideSeg_S && m_KnotType == 0)
		   break;
		else if(c >= m_WarpPairs[0].collideSeg_L && m_KnotType == 1)
		   break;
		else
		   RendNodes.push_back(Ogre::Vector3(temp.x() , temp.y() , temp.z()));
	}	

	//

	float ropecollideradius = threadobj->GetRendRadius();//GetCollideRadius();
	
	GFPhysVector3 Axis0;//to do
	GFPhysVector3 TanVec0;
	
	std::vector<Ogre::Vector3> PointsInKnot;

	GFPhysVector3 l0 , l1 , s0 , s1;
	
	//first point
	KnotIntervalPair & InitPair = m_WarpPairs[0];
	
	threadobj->GetThreadSegmentNodePos(s0 , s1 , InitPair.collideSeg_S);
	threadobj->GetThreadSegmentNodePos(l0 , l1 , InitPair.collideSeg_L);

	GFPhysVector3 InitAxis   = (m_KnotType == 0 ? (l1-l0) : (s1-s0));
	InitAxis.Normalize();


	GFPhysVector3 InitRotVec = (m_KnotType == 0 ? (s1-s0).Cross(l1-l0) : (l1-l0).Cross(s1-s0));
	InitRotVec.Normalize();

	if(m_KnotType == 0)
	{
		GFPhysVector3 p0 , p1;
		threadobj->GetThreadSegmentNodePos(p0 , p1 , InitPair.collideSeg_L-1);
		InitRotVec = (s1-s0).Cross(p1-p0);
		InitRotVec.Normalize();
	}
	else
	{
		GFPhysVector3 p0 , p1;
		threadobj->GetThreadSegmentNodePos(p0 , p1 , InitPair.collideSeg_L-1);
		InitRotVec = (p1-p0).Cross(s1-s0);
		InitRotVec.Normalize();
	}

	bool isClockWise = m_IsClockWise;//(m_KnotType == 0 ? (InitPair.m_SWarpRotate < 0) : (InitPair.m_LWarpRotate < 0));//whether rotate is clock wise
	if(isClockWise)
	{
	   InitRotVec *= -1.0f;
	}

	//GFPhysVector3 WarpPoint = (m_KnotType == 0 ? l0 * (1-InitPair.collideSeg_L) + l1 * InitPair.collideSeg_L : s0 * (1-InitPair.collideSeg_S) + s1 * InitPair.collideSeg_S);
	//WarpPoint = WarpPoint + InitRotVec * ropecollideradius;

	float StartWarpGW = (m_KnotType == 0 ? InitPair.collideSeg_S+InitPair.collideWeight_S : InitPair.collideSeg_L+InitPair.collideWeight_L);

	float StartAxisGW = (m_KnotType == 0 ? InitPair.collideSeg_L+InitPair.collideWeight_L : InitPair.collideSeg_S+InitPair.collideWeight_S);
	
	float prevRotTheta = 0;

	for(size_t k = 1 ; k < m_WarpPairs.size() ; k++)
	{
		KnotIntervalPair & currPair = m_WarpPairs[k];
		
		threadobj->GetThreadSegmentNodePos(s0 , s1 , currPair.collideSeg_S);
		threadobj->GetThreadSegmentNodePos(l0 , l1 , currPair.collideSeg_L);
		
		float EndWarpGW , EndAxisGW;
		GFPhysVector3 Axis_Curr;
		
		if(m_KnotType == 0)
		{
			Axis_Curr = (l1-l0).Normalized();
			EndWarpGW = currPair.collideSeg_S + currPair.collideWeight_S;
			EndAxisGW = currPair.collideSeg_L + currPair.collideWeight_L;
		}
		else
		{
			Axis_Curr = (s1-s0).Normalized();
			EndWarpGW = currPair.collideSeg_L + currPair.collideWeight_L;
			EndAxisGW = currPair.collideSeg_S + currPair.collideWeight_S;
		}

		//interpolated points between warps
		float KnotCurveRoughness = 0.5f;
		
		float s = (k == 1 ? StartWarpGW : StartWarpGW + KnotCurveRoughness);

		if(fabsf(currPair.m_OrientRotTheta) < GP_2PI) //&& currPair.m_OrientRotTheta > 0)
		{
			float extPercent = (GP_2PI - fabsf(currPair.m_OrientRotTheta)) / fabsf(currPair.m_OrientRotTheta);
			EndWarpGW += ((EndWarpGW-StartWarpGW) * extPercent);
		}
		EndWarpGW += 0.2f;

		while(true)
		{
			bool finalPoint = false;
			if((s >= EndWarpGW) && (k == m_WarpPairs.size()-1))
			{
			    s = EndWarpGW;
			    finalPoint = true;
			}
			float percent = (s-StartWarpGW) / (EndWarpGW-StartWarpGW);
			//GPClamp(percent , 0.0f , 1.0f);

			float rotAngle = prevRotTheta * (1-percent) + currPair.m_OrientRotTheta * percent;

			float AxisPtGlobalWeight = StartAxisGW * (1-percent) + EndAxisGW * percent;

			int   AxisPtSeg = (int)(floor(AxisPtGlobalWeight));

			float AxisPtLocalWeight = AxisPtGlobalWeight-AxisPtSeg;

			GFPhysVector3 AxisPtSegPos[2];
			threadobj->GetThreadSegmentNodePos(AxisPtSegPos[0] , AxisPtSegPos[1] , AxisPtSeg);

			GFPhysVector3 StickAxis = AxisPtSegPos[1]-AxisPtSegPos[0];
			StickAxis.Normalize();

			//transform init rotate vector to current axis frame
			GFPhysQuaternion LocalRot(InitAxis , rotAngle);
			GFPhysVector3 CurrRotVec = QuatRotate(LocalRot , InitRotVec);

			//first rotate init axis vector to coincide current axis vector
			GFPhysQuaternion FrameRotate = ShortestArcQuat(InitAxis, StickAxis);
			CurrRotVec = QuatRotate(FrameRotate , CurrRotVec);
		
			/*
			GFPhysVector3 RotVec0 = QuatRotate(FrameRotate , InitRotVec);

			//interpolt rotate angle
			GFPhysQuaternion RotQuat(StickAxis , rotAngle);

			GFPhysVector3 CurrRotVec = QuatRotate(RotQuat , RotVec0);
			*/
			
			GFPhysVector3 AxisStickPos = AxisPtSegPos[0]*(1-AxisPtLocalWeight) + AxisPtSegPos[1]*AxisPtLocalWeight;

			GFPhysVector3 wrapPos = AxisStickPos + CurrRotVec*ropecollideradius*2.0f;
			
			s += KnotCurveRoughness;

			RendNodes.push_back(Ogre::Vector3(wrapPos.x() , wrapPos.y() , wrapPos.z()));

			if(finalPoint)
			   break;
		}

		prevRotTheta = currPair.m_OrientRotTheta;
		StartWarpGW = EndWarpGW;
		StartAxisGW = EndAxisGW;
	}

	//
	for(size_t c = (m_KnotType == 0 ? m_WarpPairs[m_WarpPairs.size()-1].collideSeg_S+2 : m_WarpPairs[m_WarpPairs.size()-1].collideSeg_L+2) ; c < threadobj->GetNumThreadNodes(); c++)
	{
		GFPhysVector3 temp = threadobj->GetThreadNode(c).m_CurrPosition;
		RendNodes.push_back(Ogre::Vector3(temp.x() , temp.y() , temp.z()));
	}	
}
//======================================================================================
bool OrderPairByS(const KnotIntervalPair & lhs, const KnotIntervalPair & rhs)
{
	float lweight = lhs.collideSeg_S + lhs.collideWeight_S;
	float rweight = rhs.collideSeg_S + rhs.collideWeight_S;

	return lweight  < rweight;
}
//======================================================================================
bool OrderPairByL(const KnotIntervalPair & lhs, const KnotIntervalPair & rhs)
{
	float lweight = lhs.collideSeg_L + lhs.collideWeight_L;
	float rweight = rhs.collideSeg_L + rhs.collideWeight_L;

	return lweight  < rweight;
}
//======================================================================================
MisMedicThreadKnot::MisMedicThreadKnot()
{
	m_HasKnot = false;
	PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
}
//=========================================================================================================================
MisMedicThreadKnot::~MisMedicThreadKnot()
{
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}
//=========================================================================================================================
void MisMedicThreadKnot::CheckWarpsInKnot(KnotInRope & NewInterval , float collideRadius , std::vector<KnotIntervalPair> & PairsInWarp , int WarpType)
{
	float MinWarpSeperateDist = 3.1415f * (collideRadius);//min distance between 2 warp
	
	float MaxWarpSeperateDist = 3 * 3.1415f * (collideRadius);//max distance between 2 warp

	float segUnitLen = m_ThreadObject->GetUnitLen();

	int   TotalSegment = m_ThreadObject->GetNumSegments();

	if(WarpType == 0)//S - Warp Order By Contact Global Weight "small to large"
	   std::sort(PairsInWarp.begin() , PairsInWarp.end() , OrderPairByS);//order index from small to large
	else//L - Warp Order By Contact Global Weight "small to large"
	   std::sort(PairsInWarp.begin() , PairsInWarp.end() , OrderPairByL);

	for(size_t t = 0 ; t < PairsInWarp.size() ; t++)
	{
		const KnotIntervalPair & HeadPair = PairsInWarp[t];

		NewInterval.m_WarpPairs.clear();
		
		NewInterval.m_WarpPairs.push_back(HeadPair);
	
		float AxisGlobalW0 = (WarpType == 0 ? HeadPair.collideSeg_L + HeadPair.collideWeight_L : HeadPair.collideSeg_S + HeadPair.collideWeight_S);
		//float l0 = SrcPair.collideSeg_L + SrcPair.collideWeight_L;

		for(size_t s = t+1 ; s < PairsInWarp.size() ; s++)
		{
		   const KnotIntervalPair & DstPair = PairsInWarp[s];
					
		   if(DstPair.collideSeg_S > HeadPair.collideSeg_S && DstPair.collideSeg_S < HeadPair.collideSeg_L// must in the loop form the by (headpair.s headpair.l)
		   && DstPair.collideSeg_L > HeadPair.collideSeg_L//also keep order
		   )
		   {
			   float AxisGlobalW1 = (WarpType == 0 ? DstPair.collideSeg_L + DstPair.collideWeight_L : DstPair.collideSeg_S + DstPair.collideWeight_S);

			   float AxisWeightDist = AxisGlobalW1-AxisGlobalW0;

			   float WarpWeightDist = 0;
			  
			   if(WarpType == 0)
				  WarpWeightDist = fabsf((DstPair.collideSeg_S+DstPair.collideWeight_S) - (HeadPair.collideSeg_S+HeadPair.collideWeight_S));
			   else
			      WarpWeightDist = fabsf((DstPair.collideSeg_L+DstPair.collideWeight_L) - (HeadPair.collideSeg_L+HeadPair.collideWeight_L));

			   float WarpEucleanDist = segUnitLen * WarpWeightDist;

			   // @ !! to form a knot warp must have same order in S and L
			   // @ !! so only collide point in l segment is large than SrcPair can we form a knot
			   // also check the 2 warp point has enough space and not to distant from each other
			   if(AxisWeightDist > 0 && WarpWeightDist > 0)// && fabsf(WarpWeightDist-AxisWeightDist) < (AxisWeightDist*0.2f))
				  // && WarpEucleanDist < MaxWarpSeperateDist && WarpEucleanDist > MinWarpSeperateDist)
			   {
					//if(SrcPair.CanOtherSWarpFormKnotWithMe(m_ThreadObject , DstPair))//now check these 2 warp can really form knot!!
					//{
					//	NewInterval.m_WarpPairs.push_back(DstPair);
					//	break;
					//}
					float rotAngle = KnotInRope::CalcRelativePairRotate(m_ThreadObject , HeadPair , DstPair , WarpType);
					if(fabsf(rotAngle) <= GP_2PI && fabsf(rotAngle) >= GP_2PI * 0.75f)
					{
						NewInterval.m_IsClockWise = (rotAngle < 0 ? true : false);

						KnotIntervalPair NewWarpPair = DstPair;
						if(WarpType == 0)
						{
						   float warpGWeight = HeadPair.collideSeg_S+HeadPair.collideWeight_S+AxisWeightDist;
						   NewWarpPair.collideSeg_S = (int)floor(warpGWeight);
						   NewWarpPair.collideWeight_S = (warpGWeight-NewWarpPair.collideSeg_S);
						}
						else
						{
						   float warpGWeight = HeadPair.collideSeg_L+HeadPair.collideWeight_L+AxisWeightDist;
						   NewWarpPair.collideSeg_L = (int)floor(warpGWeight);
						   NewWarpPair.collideWeight_L = (warpGWeight-NewWarpPair.collideSeg_L);
						}

						NewWarpPair.m_OrientRotTheta = rotAngle;
						
						if(NewWarpPair.collideSeg_S < TotalSegment && NewWarpPair.collideSeg_L < TotalSegment)
						   NewInterval.m_WarpPairs.push_back(NewWarpPair);

						return;
					}
				}
		    }
		}
	}
}
//======================================================================================
void MisMedicThreadKnot::CheckAnyKnotFormed()
{
	if(m_HasKnot == true) //temp
	   return;//

	const GFPhysAlignedVectorObj<TTCollidePair> & OriginPairs = m_ThreadObject->GetCollidePairsWithThread();
	
	std::vector<KnotIntervalPair> SWarpPair;//warp detected in 'S' (small side) segment
	
	std::vector<KnotIntervalPair> LWarpPair;//warp detected in 'L' (large side) segment

	std::vector<KnotInRope> CandidateKnot;

	float collideRadius = m_ThreadObject->GetCollideRadius();

	//bool * SegmentInKnot = new bool[m_ThreadObject->GetNumSegments()];
	//for(int c = 0 ; c < m_ThreadObject->GetNumSegments() ; c++)
	//{
		//SegmentInKnot[c] = false;
	//}

	//detect any warp formed
	for(size_t t = 0 ; t < OriginPairs.size() ; t++)
	{
		const TTCollidePair & ttPair = OriginPairs[t];
		if(ttPair.m_RopeA == ttPair.m_RopeB && ttPair.m_CollideDist < collideRadius * 2.2f
			&& ttPair.m_WeightA > 0.001f && ttPair.m_WeightA < 0.999f
			&& ttPair.m_WeightB > 0.001f && ttPair.m_WeightB < 0.999f)
		{
			KnotIntervalPair contactPair;
			contactPair.SetPair(ttPair.m_SegmentA , ttPair.m_WeightA , ttPair.m_SegmentB , ttPair.m_WeightB);
			contactPair.CheckInWarpState(m_ThreadObject);
			
			if(contactPair.m_SInWarpShape)
			{
			   //contactPair.CalcWarpRotateSign_S(m_ThreadObject);
			   SWarpPair.push_back(contactPair);
			}

			if(contactPair.m_LInWarpShape)
			{
			   //contactPair.CalcWarpRotateSign_L(m_ThreadObject);
			   LWarpPair.push_back(contactPair);
			}
		}
	}

	//check all warp in S part
	int KnotType = -1;

	KnotInRope NewInterval;
	
	if(NewInterval.m_WarpPairs.size() >= 2)//S warp formed an Knot
	{
	   KnotType = 0;
	}
	else//check all warp in L part if no S warps can form a knot
	{
	   NewInterval.m_WarpPairs.clear();
	   CheckWarpsInKnot(NewInterval ,  collideRadius , LWarpPair , 1);
	  
	   if(NewInterval.m_WarpPairs.size() >= 2)
		  KnotType = 1;
	}

	if(KnotType >= 0)//this is a knot
	{
		NewInterval.m_KnotType = KnotType;
		CandidateKnot.push_back(NewInterval);

		int s0 = NewInterval.m_WarpPairs[0].collideSeg_S;
		int l0 = NewInterval.m_WarpPairs[0].collideSeg_L;

		int s1 = NewInterval.m_WarpPairs[NewInterval.m_WarpPairs.size()-1].collideSeg_S;
		int l1 = NewInterval.m_WarpPairs[NewInterval.m_WarpPairs.size()-1].collideSeg_L;

		for(int c = s0 ; c <= s1 ; c++)
		{
			m_ThreadObject->m_SegmentState[c] = true;//mark these region as in knot
		}

		for(int c = l0 ; c <= l1 ; c++)
		{
			m_ThreadObject->m_SegmentState[c] = true;//mark these region as in knot
		}

		for(int c = s0+1 ; c <= s1 ; c++)
		{
			//m_ThreadObject->GetThreadNodeRef(c).m_bCanSelfCollision = false;
		}

		for(int c = l0+1 ; c <= l1 ; c++)
		{
			//m_ThreadObject->GetThreadNodeRef(c).m_bCanSelfCollision = false;
		}
		

		m_ThreadObject->DisableSelfCollision();//test
		m_ThreadObject->m_UseBendForce = true;//test
	}

	if(CandidateKnot.size() > 0)
	{
		for(size_t c = 0 ; c < CandidateKnot.size() ; c++)
		{
			KnotInRope & knotInterVal = CandidateKnot[c];

			//for(size_t p = 0 ; p < knotInterVal.m_WarpPairs.size() ; p++)
			//{
			//	knotInterVal.m_WarpPairs[p].CalcCollideTopology(m_ThreadObject);
			//}

			//knotInterVal.CalcKnotPairOrient(m_ThreadObject);
			m_Knots.push_back(knotInterVal);
		}
		m_HasKnot = true;
	}

	/*
	float MinWarpSeperateDist = 3.1415f * (collideRadius);//*2.0f);//min distance between 2 warp
	
	float MaxWarpSeperateDist = 2 * 3.1415f * (collideRadius);//*2.0f);//max distance between 2 warp

	float segUnitLen = m_ThreadObject->GetUnitLen();

	//check all warp in S part
	std::sort(SWarpPair.begin() , SWarpPair.end() , OrderPairByS);//order index from small to large
	for(size_t t = 0 ; t < SWarpPair.size() ; t++)
	{
		KnotInRope NewInterval;
		
		const KnotIntervalPair & SrcPair = SWarpPair[t];
			
		NewInterval.m_WarpPairs.push_back(SrcPair);
	
		float l0 = SrcPair.collideSeg_L + SrcPair.collideWeight_L;

		for(size_t s = t+1 ; s < SWarpPair.size() ; s++)
		{
			const KnotIntervalPair & DstPair = SWarpPair[s];
					
		   if(DstPair.collideSeg_S > SrcPair.collideSeg_S && DstPair.collideSeg_L > SrcPair.collideSeg_L)
		   {
				float l1 = DstPair.collideSeg_L + DstPair.collideWeight_L;

				float SegmentDist = segUnitLen * fabsf(SrcPair.collideSeg_S+SrcPair.collideWeight_S-(DstPair.collideSeg_S+DstPair.collideWeight_S));

				// @ !! to form a knot warp must have same order in S and L
				// @ !! so only collide point in l segment is large than SrcPair can we form a knot
				// also check the 2 warp point has enough space and not to distant from each other
				if(l1 > l0 && SegmentDist < MaxWarpSeperateDist && SegmentDist > MinWarpSeperateDist)
				{
					//if(SrcPair.CanOtherSWarpFormKnotWithMe(m_ThreadObject , DstPair))//now check these 2 warp can really form knot!!
					//{
					//	NewInterval.m_WarpPairs.push_back(DstPair);
					//	break;
					//}
					float rotAngle = KnotInRope::CalcRelativePairRotate(m_ThreadObject , SrcPair , DstPair , 0);
					if(fabsf(rotAngle) <= GP_2PI && fabsf(rotAngle) >= GP_2PI * 0.75f)
					{
						NewInterval.m_WarpPairs.push_back(DstPair);
						break;
					}
				}
		   }
		}
		if(NewInterval.m_WarpPairs.size() >= 2)//this is a knot
		{
			int s0 = NewInterval.m_WarpPairs[0].collideSeg_S;
			int l0 = NewInterval.m_WarpPairs[0].collideSeg_L;

			int s1 = NewInterval.m_WarpPairs[NewInterval.m_WarpPairs.size()-1].collideSeg_S;
			int l1 = NewInterval.m_WarpPairs[NewInterval.m_WarpPairs.size()-1].collideSeg_L;

			for(int c = s0 ; c <= s1 ; c++)
			{
				m_ThreadObject->m_SegmentState[c] = true;//mark these region as in knot
			}

			for(int c = l0 ; c <= l1 ; c++)
			{
				m_ThreadObject->m_SegmentState[c] = true;//mark these region as in knot
			}

			for(int c = s0+1 ; c <= s1 ; c++)
			{
				m_ThreadObject->GetThreadNodeRef(c).m_bCanSelfCollision = false;
			}

			for(int c = l0+1 ; c <= l1 ; c++)
			{
				m_ThreadObject->GetThreadNodeRef(c).m_bCanSelfCollision = false;
			}
			NewInterval.m_KnotType = 0;
			CandidateKnot.push_back(NewInterval);

			m_ThreadObject->DisableSelfCollision();//test
			m_ThreadObject->m_UseBendForce = true;//test
			break;
		}
	}

	//check all warp in L part if no S warps can form a knot
	if(CandidateKnot.size() == 0)
	{
		std::sort(LWarpPair.begin() , LWarpPair.end() , OrderPairByL);

		for(size_t t = 0 ; t < LWarpPair.size() ; t++)
		{
			KnotInRope NewInterval;

			const KnotIntervalPair & SrcPair = LWarpPair[t];

			NewInterval.m_WarpPairs.push_back(SrcPair);

			float s0 = SrcPair.collideSeg_S + SrcPair.collideWeight_S;

			for(size_t d = t+1 ; d < LWarpPair.size() ; d++)
			{
				const KnotIntervalPair & DstPair = LWarpPair[d];

				if(DstPair.collideSeg_S > SrcPair.collideSeg_S && DstPair.collideSeg_L > SrcPair.collideSeg_L)
				{
					float s1 = DstPair.collideSeg_S + DstPair.collideWeight_S;

					float SegmentDist = segUnitLen * fabsf(SrcPair.collideSeg_L+SrcPair.collideWeight_L-(DstPair.collideSeg_L+DstPair.collideWeight_L));//segUnitLen * (sl - tl);

					if(s0 < s1 && SegmentDist < MaxWarpSeperateDist && SegmentDist > MinWarpSeperateDist)
					{
						if(SrcPair.CanOtherLWarpFormKnotWithMe(m_ThreadObject , DstPair))
						{
							NewInterval.m_WarpPairs.push_back(DstPair);
							break;
						}
					}
				}
			}
			if(NewInterval.m_WarpPairs.size() >= 2)//this is a knot
			{
				int s0 = NewInterval.m_WarpPairs[0].collideSeg_S;
				int l0 = NewInterval.m_WarpPairs[0].collideSeg_L;

				int s1 = NewInterval.m_WarpPairs[NewInterval.m_WarpPairs.size()-1].collideSeg_S;
				int l1 = NewInterval.m_WarpPairs[NewInterval.m_WarpPairs.size()-1].collideSeg_L;

				for(int c = s0 ; c <= s1 ; c++)
				{
					m_ThreadObject->m_SegmentState[c] = true;//mark these region as in knot
				}

				for(int c = l0 ; c <= l1 ; c++)
				{
					m_ThreadObject->m_SegmentState[c] = true;//mark these region as in knot
				}

				for(int c = s0+1 ; c <= s1 ; c++)
				{
					m_ThreadObject->GetThreadNodeRef(c).m_bCanSelfCollision = false;
				}

				for(int c = l0+1 ; c <= l1 ; c++)
				{
					m_ThreadObject->GetThreadNodeRef(c).m_bCanSelfCollision = false;
				}
				NewInterval.m_KnotType = 1;
				CandidateKnot.push_back(NewInterval);

				m_ThreadObject->DisableSelfCollision();
				break;
			}
		}
	}
	*/
	
	/*if(CandidateKnot.size() > 0)
	{
		for(size_t c = 0 ; c < CandidateKnot.size() ; c++)
		{
			KnotInRope & knotInterVal = CandidateKnot[c];

			for(size_t p = 0 ; p < knotInterVal.m_WarpPairs.size() ; p++)
			{
				knotInterVal.m_WarpPairs[p].CalcCollideTopology(m_ThreadObject);

				//int segS = knotInterVal.m_WarpPairs[p].collideSeg_S;

				//int segL = knotInterVal.m_WarpPairs[p].collideSeg_L;

				//m_ThreadObject->GetThreadNodeRef(segS).m_bCanSelfCollision = false;
				//m_ThreadObject->GetThreadNodeRef(segS+1).m_bCanSelfCollision = false;

				//m_ThreadObject->GetThreadNodeRef(segL).m_bCanSelfCollision = false;
				//m_ThreadObject->GetThreadNodeRef(segL+1).m_bCanSelfCollision = false;
			}

			knotInterVal.CalcKnotPairOrient(m_ThreadObject);
			m_Knots.push_back(knotInterVal);
			//m_ThreadObject->m_UseBendForce = true;//temp
		}
		m_HasKnot = true;
	}
	*/
}
//======================================================================================
void CalcCurrAndRestLenBetweenPoint(int segA , float weightA , int segB , float weightB , MisMedicThreadRope * rope , float & restLen , float & currLen)
{
	GFPhysVector3 A0 , A1;
	GFPhysVector3 B0 , B1;
	rope->GetThreadSegmentNodePos(A0 , A1 , segA);
	rope->GetThreadSegmentNodePos(B0 , B1 , segB);

	GFPhysVector3 pA = (A0 * (1-weightA) + A1 * weightA);
	GFPhysVector3 pB = (B0 * (1-weightB) + B1 * weightB);
	currLen = (pA - pB).Length();
	
	float tA = segA + weightA;
	float tB = segB + weightB;
	restLen = fabsf(tA-tB) * rope->GetUnitLen();
}
//======================================================================================
void SlipKnotPointBackWard(float slipDistance , float ropeUnitLen , int & SegIndex , float & SegWeight)
{
	float step = slipDistance / ropeUnitLen;

	float t = SegWeight + step;

	int is = (int)floor(t);

	SegIndex += is;
	
	SegWeight = GPClamped(t - (float)is , 0.0f , 1.0f);
}

//======================================================================================
void SlipKnotPointForWard(float slipDistance , float ropeUnitLen , int & SegIndex , float & SegWeight)
{
	float step = slipDistance / ropeUnitLen;

	float t = 1-SegWeight + step;

	int is = (int)floor(t);

	SegIndex -= is;

	float w = (t - (float)is);

	SegWeight = GPClamped(1 - w , 0.0f , 1.0f);
}
void MisMedicThreadKnot::UpdateThreadMesh()
{
	if(m_Knots.size() > 0)
	{
		std::vector<Ogre::Vector3> rendPoints;
		m_Knots[0].BuildKnotPoint(m_ThreadObject , rendPoints);
	    m_ThreadObject->UpdateMeshByCustomedRendNodes(rendPoints);
	}
	else
	{
		m_ThreadObject->UpdateMesh();
	}
}
void MisMedicThreadKnot::Update(float dt)
{
	UpdateKnot();
	CheckAnyKnotFormed();
	//SolveKnotConstraint();
	UpdateThreadMesh();
}
//======================================================================================
void MisMedicThreadKnot::UpdateKnot()
{
	//if(m_ThreadObject)
	//{
		//for(int n = 0; n < m_ThreadObject->GetNumThreadNodes(); n++)
		//{
		//	m_ThreadObject->GetThreadNodeRef(n).m_UseSumCorrect = false;
		//}
	//}

	if(m_HasKnot == false)
	   return;

	KnotInRope & m_KnotInProcess = m_Knots[0];

	int nearstdragindex = -1;
	int farstdratindex = -1;

	int firstWarpIndex = 0;
	int lastWarpIndex  = m_KnotInProcess.m_WarpPairs.size()-1;

	for(int c = m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_S-1 ; c >= 0 ; c--)
	{
		if(m_ThreadObject->GetThreadNodeRef(c).IsAttached() == true)
		{
			nearstdragindex = c;
			break;
		}
	}
	
	for(int c = m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_L+1 ; c < m_ThreadObject->GetNumThreadNodes()-1 ; c++)
	{
		if(m_ThreadObject->GetThreadNodeRef(c).IsAttached() == true)
		{
			farstdratindex = c;
			break;
		}
	}

	float CurrLen , RestLen;

	float ropeUnitLen = m_ThreadObject->GetUnitLen();

	float SWarpsBackSlipDist = 0.0f;
	float LWarpsForwardSlipDist = 0.0f;

	float thresholdPerUnit = ropeUnitLen * 0.15f;

	bool nearestNeedTighten = false;
	bool farestNeedTighten = false;

	if(nearstdragindex >= 0)
	{
		KnotIntervalPair & firstPair = m_KnotInProcess.m_WarpPairs[firstWarpIndex];

		//if interval between P0 and the nearest drag point larger than the rest length for some threshold
		CalcCurrAndRestLenBetweenPoint( firstPair.collideSeg_S , 
										firstPair.collideWeight_S, 
										nearstdragindex , 
										1 , 
										m_ThreadObject , 
										RestLen , 
										CurrLen);

		float detaLen = CurrLen-RestLen;
		float NumUnit = firstPair.collideSeg_S+firstPair.collideWeight_S - (nearstdragindex + 1);

		if(detaLen > thresholdPerUnit * NumUnit)
		{
			nearestNeedTighten = true;

			detaLen -= thresholdPerUnit * NumUnit;
			detaLen *= 0.5f;//to model friction
			if(detaLen > 0.005f)
			   detaLen = 0.005f;
			
			float maxSweight = m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_S
							  +m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideWeight_S;

			float minLweight = m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_L
							  +m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideWeight_L;

			maxSweight += detaLen / ropeUnitLen;

			float deltaWeight = (int)floor(minLweight)-((int)floor(maxSweight)+1);

			float torlerent = m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_L-m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_L+1;
			
			if(torlerent < 2)
			   torlerent = 2;

			if(deltaWeight >= torlerent)
			{
				for(int c = firstWarpIndex  ; c <= lastWarpIndex ; c++)
				{
					SlipKnotPointBackWard(detaLen , ropeUnitLen , m_KnotInProcess.m_WarpPairs[c].collideSeg_S , m_KnotInProcess.m_WarpPairs[c].collideWeight_S);
					m_KnotInProcess.m_WarpPairs[c].CalcKnotPairTetra(m_ThreadObject);
				}
			}	
			else
			{
				int i = 0;
				int j = i+1;
			}
		}
	}

	if(farstdratindex >= 0)
	{
		KnotIntervalPair & lastPair = m_KnotInProcess.m_WarpPairs[lastWarpIndex];

		//inverse visit from end because L need to visit from large to small
		//now check Q1 and the nearest drag point interval
		CalcCurrAndRestLenBetweenPoint( lastPair.collideSeg_L , 
										lastPair.collideWeight_L, 
										farstdratindex ,
										0 , 
										m_ThreadObject , 
										RestLen , 
										CurrLen);

		float detaLen = CurrLen-RestLen;

		float NumUnit = farstdratindex - (lastPair.collideSeg_L + lastPair.collideWeight_L);

		if(detaLen > thresholdPerUnit * NumUnit)
		{
			farestNeedTighten = true;

			detaLen -= thresholdPerUnit * NumUnit;
			detaLen *= 0.5f;//to model friction
			if(detaLen > 0.005f)
			   detaLen = 0.005f;


			float maxSweight = m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_S
							  +m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideWeight_S;

			float minLweight = m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_L
							  +m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideWeight_L;

			minLweight -= detaLen / ropeUnitLen;

			float deltaWeight = (int)floor(minLweight)-((int)floor(maxSweight)+1);

			float torlerent = m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_S-m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_S+1;
			if(torlerent < 2)
			   torlerent = 2;

			if(deltaWeight >= torlerent)
			{
				for(int c = lastWarpIndex  ; c >= firstWarpIndex ; c--)
				{
					SlipKnotPointForWard(detaLen , ropeUnitLen , m_KnotInProcess.m_WarpPairs[c].collideSeg_L , m_KnotInProcess.m_WarpPairs[c].collideWeight_L);
					m_KnotInProcess.m_WarpPairs[c].CalcKnotPairTetra(m_ThreadObject);
				}
			}	
			else
			{
				int i = 0;
				int j = i+1;
			}
		}
	}

	if(farestNeedTighten || nearestNeedTighten)
	{
		float KnotInterval = m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_S+m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideWeight_S
			               -(m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_S+m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideWeight_S);

		if(KnotInterval > 1.5f)
		{
			float TightWeight = KnotInterval-1.5f;
			TightWeight *= 0.001f;

			SlipKnotPointBackWard(TightWeight*ropeUnitLen*0.5f, 
				ropeUnitLen , 
				m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_S ,
				m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideWeight_S);

			SlipKnotPointForWard(TightWeight*ropeUnitLen*0.5f, 
				ropeUnitLen , 
				m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_S ,
				m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideWeight_S);

		}

		KnotInterval = (m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_L+m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideWeight_L)
			          -(m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_L+m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideWeight_L);
		if(KnotInterval > 1.5f)
		{
			float TightWeight = KnotInterval-1.5f;
			TightWeight *= 0.001f;

			SlipKnotPointBackWard(TightWeight*ropeUnitLen*0.5f, 
				ropeUnitLen , 
				m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_L ,
				m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideWeight_L);

			SlipKnotPointForWard(TightWeight*ropeUnitLen*0.5f, 
				ropeUnitLen , 
				m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_L ,
				m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideWeight_L);
		}
	}
	
	int knotSegIntervl_S = m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_S+1;

	int knotSegIntervl_L = m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_L-1;
	
	//if(knotSegIntervl_L-knotSegIntervl_S <= 3)
	//{
		//for(int s = knotSegIntervl_S ; s <= knotSegIntervl_L ; s++)
		//{
		//	m_ThreadObject->GetThreadNodeRef(s).m_UseSumCorrect = true;
		//}
	//}

	BuildBendingSection();

	//Mark loop
	for(int n = 0 ; n < m_ThreadObject->GetNumThreadNodes() ; n++)
	{
		m_ThreadObject->GetThreadNodeRef(n).SetInKnotLoop(false);
	}
	//
	KnotInRope & knot = m_Knots[0];
	
	int s1 = knot.m_WarpPairs[1].collideSeg_S;
	
	int l0 = knot.m_WarpPairs[0].collideSeg_L;
	
	for(int n = s1 ; n <= l0 ; n++)
	{
		m_ThreadObject->GetThreadNodeRef(n).SetInKnotLoop(true);
	}

}
void MisMedicThreadKnot::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{

}
void MisMedicThreadKnot::SolveConstraint(Real globalstiffness,Real TimeStep)
{
	 SolveKnotConstraint();
}

void MisMedicThreadKnot::BuildBendingSection()
{
	if(m_Knots.size() > 0 && m_ThreadObject)
	{
		KnotInRope & knot = m_Knots[0];
		int s0 = knot.m_WarpPairs[0].collideSeg_S;
		int l0 = knot.m_WarpPairs[0].collideSeg_L;

		int s1 = knot.m_WarpPairs[1].collideSeg_S;
		int l1 = knot.m_WarpPairs[1].collideSeg_L;

		int maxNodeindex = m_ThreadObject->GetNumThreadNodes()-1;
	}
	
}
//======================================================================================
void MisMedicThreadKnot::SolveKnotConstraint()
{
	for(size_t k = 0 ; k < m_Knots.size() ; k++)
	{
		m_Knots[k].SolveKnotConstraint(m_ThreadObject);
	}
	/*
	if(m_HasKnot)
	{
		for(int c = 0 ; c < 2 ; c++)
		{
			int sA = (c == 0 ? m_KnotInProcess.m_PairS.collideSeg_S : m_KnotInProcess.m_PairL.collideSeg_S);
			ThreadNode & NodeA0 = m_ThreadObject->GetThreadNodeRef(sA);
			ThreadNode & NodeA1 = m_ThreadObject->GetThreadNodeRef(sA+1);
			float WeightA0 = (c == 0 ? 1-m_KnotInProcess.m_PairS.collideWeight_S : 1-m_KnotInProcess.m_PairL.collideWeight_S);
			float WeightA1 = 1-WeightA0;

			int sB = (c == 0 ? m_KnotInProcess.m_PairS.collideSeg_L : m_KnotInProcess.m_PairL.collideSeg_L);
			ThreadNode & NodeB0 = m_ThreadObject->GetThreadNodeRef(sB);
			ThreadNode & NodeB1 = m_ThreadObject->GetThreadNodeRef(sB+1);
			float WeightB0 = (c == 0 ? 1-m_KnotInProcess.m_PairS.collideWeight_L : 1-m_KnotInProcess.m_PairL.collideWeight_L);
			float WeightB1 = 1-WeightB0;

			GFPhysVector3 pA = NodeA0.m_CurrPosition * WeightA0 + NodeA1.m_CurrPosition * WeightA1;
			GFPhysVector3 pB = NodeB0.m_CurrPosition * WeightB0 + NodeB1.m_CurrPosition * WeightB1;
			
			GFPhysVector3 Diff = pA-pB;

			Real Length = Diff.Length();

			Real diffLen = Length-0.0f;

			if(Length > FLT_EPSILON)
			{	
				Diff /= Length;

				GFPhysVector3 gradA0 = Diff * WeightA0;
				GFPhysVector3 gradA1 = Diff * WeightA1;
				
				GFPhysVector3 gradB0 = -Diff * WeightB0;
				GFPhysVector3 gradB1 = -Diff * WeightB1;

				float SumGrad = WeightA0*WeightA0 + WeightA1*WeightA1 + WeightB0*WeightB0 + WeightB1*WeightB1;

				if(SumGrad > FLT_EPSILON)
				{
					float stiffness = 0.99f;
					Real scale = stiffness*(-diffLen) / SumGrad;

					GFPhysVector3 deltaA0 = scale*gradA0;
					GFPhysVector3 deltaA1 = scale*gradA1;
		
					GFPhysVector3 deltaB0 = scale*gradB0;
					GFPhysVector3 deltaB1 = scale*gradB1;
				
					NodeA0.m_CurrPosition += deltaA0;
					NodeA1.m_CurrPosition += deltaA1;
				
					NodeB0.m_CurrPosition += deltaB0;
					NodeB1.m_CurrPosition += deltaB1;

					 pA = NodeA0.m_CurrPosition * WeightA0 + NodeA1.m_CurrPosition * WeightA1;
					 pB = NodeB0.m_CurrPosition * WeightB0 + NodeB1.m_CurrPosition * WeightB1;

					 Diff = pA-pB;

					 Length = Diff.Length();

					 int t = 0;
					 int j = t+1;
				}
			}
		}
	}
	*/
}


/*
//======================================================================================
void MisMedicThreadKnot::UpdateKnot()
{
if(m_HasKnot == false)
return;

KnotInRope & m_KnotInProcess = m_Knots[0];

int nearstdragindex = -1;
int farstdratindex = -1;

int firstWarpIndex = 0;
int lastWarpIndex  = m_KnotInProcess.m_WarpPairs.size()-1;

for(int c = m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_S-1 ; c >= 0 ; c--)
{
if(m_ThreadObject->GetThreadNodeRef(c).m_bAttached == true)
{
nearstdragindex = c;
break;
}
}

for(int c = m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_L+1 ; c < m_ThreadObject->GetNumThreadNodes()-1 ; c++)
{
if(m_ThreadObject->GetThreadNodeRef(c).m_bAttached == true)
{
farstdratindex = c;
break;
}
}

float CurrLen , RestLen;

float ropeUnitLen = m_ThreadObject->GetUnitLen();

float SWarpsBackSlipDist = 0.0f;
float LWarpsForwardSlipDist = 0.0f;

if(nearstdragindex >= 0)
{
//if interval between P0 and the nearest drag point larger than the rest length for some threshold
CalcCurrAndRestLenBetweenPoint( m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_S , 
m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideWeight_S, 
nearstdragindex , 
1 , 
m_ThreadObject , 
RestLen , 
CurrLen);

SWarpsBackSlipDist = CurrLen-RestLen;

if(SWarpsBackSlipDist > ropeUnitLen * 0.2f)
{
SWarpsBackSlipDist = SWarpsBackSlipDist-ropeUnitLen * 0.2f;//ropeUnitLen * 0.05f;
}
}

if(farstdratindex >= 0)
{
//inverse visit from end because L need to visit from large to small
//now check Q1 and the nearest drag point interval
CalcCurrAndRestLenBetweenPoint( m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_L , 
m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideWeight_L, 
farstdratindex ,
0 , 
m_ThreadObject , 
RestLen , 
CurrLen);

LWarpsForwardSlipDist = CurrLen-RestLen;
if(LWarpsForwardSlipDist > ropeUnitLen * 0.2f)
{
LWarpsForwardSlipDist = SWarpsBackSlipDist-ropeUnitLen * 0.2f;//ropeUnitLen * 0.05f;
}
}

float SrcShrinkLen = fabsf(SWarpsBackSlipDist) + fabsf(LWarpsForwardSlipDist);
float DstShrinkLen = SrcShrinkLen;

float s0 = (m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_S + m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideWeight_S);
float l0 = (m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideSeg_L + m_KnotInProcess.m_WarpPairs[firstWarpIndex].collideWeight_L);

float s1 = (m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_S + m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideWeight_S);
float l1 = (m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideSeg_L + m_KnotInProcess.m_WarpPairs[lastWarpIndex].collideWeight_L);

float remindLen  = (l0-s1)*ropeUnitLen;
float minLoopLen = 3*ropeUnitLen;//temp test //(s1-s0)*m_ThreadObject->GetCollideRadius();

float MaxShrinkLen = remindLen-minLoopLen;

if(MaxShrinkLen < 0)
MaxShrinkLen = 0;

if(DstShrinkLen > MaxShrinkLen)
DstShrinkLen = MaxShrinkLen;

if(DstShrinkLen > 0 && SrcShrinkLen > 0)
{
float scalefactor = DstShrinkLen / SrcShrinkLen;

SWarpsBackSlipDist *= scalefactor;

LWarpsForwardSlipDist *= scalefactor;

if(SWarpsBackSlipDist > FLT_EPSILON)
{
for(int c = firstWarpIndex  ; c <= lastWarpIndex ; c++)
{
SlipKnotPointBackWard(SWarpsBackSlipDist , ropeUnitLen , m_KnotInProcess.m_WarpPairs[c].collideSeg_S , m_KnotInProcess.m_WarpPairs[c].collideWeight_S);
m_KnotInProcess.m_WarpPairs[c].CalcKnotPairTetra(m_ThreadObject);
}
}

if(LWarpsForwardSlipDist > FLT_EPSILON)
{
for(int c = lastWarpIndex  ; c >= firstWarpIndex ; c--)
{
SlipKnotPointForWard(LWarpsForwardSlipDist , ropeUnitLen , m_KnotInProcess.m_WarpPairs[c].collideSeg_L , m_KnotInProcess.m_WarpPairs[c].collideWeight_L);
m_KnotInProcess.m_WarpPairs[c].CalcKnotPairTetra(m_ThreadObject);
}
}
}
}
*/