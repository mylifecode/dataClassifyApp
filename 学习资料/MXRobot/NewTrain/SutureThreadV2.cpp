#include "SutureThreadV2.h"
#include "MisNewTraining.h"
#include "Dynamic\Constraint\GoPhysSoftBodyDistConstraint.h"
#include "math\GoPhysTransformUtil.h"
#include "Math\GoPhysSIMDMath.h"
#include "Math\GoPhysMathUtil.h"

#include "TrainingMgr.h"
#include "MisMedicSutureKnotV2.h"
//#include <QDebug>

//float RScollideMassScale = 0.35f;

//#define NODECOLLIDE 1
//#define SEGCOLLIDE  2
//#define COLLIDETYPE SEGCOLLIDE

#define KNOTRENDER 1


//const Real TESTTHRESHOLD = 0.3333333f;
//const int NUM = 1;
//const Real SHRINK1 = 5.0f;
//const Real SHRINK2 = 0.5f;

SutureThreadNodeV2::SutureThreadNodeV2(const GFPhysVector3 & restPos)
{
	m_UnDeformedPos = m_CurrPosition = m_LastPosition = restPos;
	m_Velocity = GFPhysVector3(0, 0, 0);
	m_InvMass = 1.0f;
	m_EnableCollide = true;
	m_bVisual = true;
}
//===================================================================================================================================
SutureThreadNodeV2::SutureThreadNodeV2()
{
	m_UnDeformedPos = m_CurrPosition = m_LastPosition = GFPhysVector3(0, 0, 0);
	m_Velocity = GFPhysVector3(0, 0, 0);
	m_InvMass = 1.0f;
	m_EnableCollide = true;
}
//===================================================================================================================================
SutureThreadNodeV2::~SutureThreadNodeV2()
{

}

int SutureThreadV2::globalid = 100;
//=========================================================================
SutureThreadV2::SutureThreadV2(Ogre::SceneManager * sceneMgr, MisNewTraining * ownertrain, Ogre::String matRendObj) : m_ownertrain(ownertrain)
{
	m_dis = 0.0f;
	static int s_RopeId = 0;
	s_RopeId++;

	Ogre::String strSutureThreadName = "SutureThreadV2" + Ogre::StringConverter::toString(s_RopeId);
	m_RendObject.CreateRendPart(strSutureThreadName, sceneMgr);

	s_RopeId++;
	strSutureThreadName = "SutureThreadObject" + Ogre::StringConverter::toString(s_RopeId);
	m_RendObject1.CreateRendPart(strSutureThreadName, MXOgre_SCENEMANAGER);

	s_RopeId++;
	strSutureThreadName = "SutureThreadObject" + Ogre::StringConverter::toString(s_RopeId);
	m_RendObject2.CreateRendPart(strSutureThreadName, MXOgre_SCENEMANAGER);

	//m_UnitLen = 0.18f;//unit length in rope
	//m_RopeCollideRadius = 0.05f;//rope physics radius
	SetCollisionRadius(0.05f);

	m_RopeCollideMargin = 0.06f;// *0.25f;

	m_RopeRendRadius = 0.03f;//rope rend radius

	m_RopeFriction = 0.6f;//rope vs soft collision coefficients
	m_RopeRSFriction = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.2f);//rope vs rigid coefficients
	m_UseCCD = true;

	m_Gravity = GFPhysVector3(0, -18.0, 0);

	m_NodeDamping = 5.0f;

	m_AngularDamping = GFPhysVector3(3.0f,3.0f,3.0f);

	m_UseBendForce = true;

	m_NeedRend = true;

	//m_CasColDetector = new WireArteryCascadedCollideDetector();

	m_TwistValue = 0;
	m_FaceRopeAnchors.clear();

	m_NullNode = SutureThreadNodeV2();
	m_islock = false;
	m_KnotsInThread = 0;

	m_RopeAnchorIndexMin = INT_MAX;
	m_RopeAnchorIndexMax = INT_MIN;
}
//===================================================================================================
SutureThreadV2::~SutureThreadV2()
{
	Destory();
}
void SutureThreadV2::SetCollideRaiuds(float radius)
{
	//m_RopeCollideRadius = radius;//rope physics radius
	GFPhysSoftTube::SetCollisionRadius(radius);
	
	m_RopeCollideMargin = radius;// *0.25f;

	m_RopeRendRadius = radius;//rope rend radius
}
//===================================================================================================
void SutureThreadV2::DisableSegmentCollision(int startSeg, int endSeg)
{
	for (int c = 0, nc = m_Segments.size(); c < nc; c++)
	{
		if (c >= startSeg && c <= endSeg)
			m_Segments[c].DisableCollide();
		else
			m_Segments[c].EnableCollide();
	}
}
//===================================================================================================
void SutureThreadV2::SetStretchShearStiffness(float set)
{
	m_StretchShearInvStiff = (set > 0 ? 1.0f / set : 0.0f);
}
//===================================================================================================
void SutureThreadV2::SetBendingTwistStiffness(float bendStiff, float twistStiff)
{
	m_RopeBendInvStiff = (bendStiff > 0 ? 1.0f / bendStiff : 0.0f);

	m_RopeTwistInvStiff = (twistStiff > 0 ? 1.0f / twistStiff : 0.0f);// GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(set);
}
//===================================================================================================
void SutureThreadV2::SetGravity(const GFPhysVector3 & gravity)
{
	m_Gravity = gravity;
}
//===================================================================================================
void SutureThreadV2::Destory()
{
	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
	m_TubeNodes.clear();

	m_SegmentBVTree.Clear();

	m_TRCollidePair.clear();
	m_TFCollidePair.clear();
	m_TTCollidepair.clear();

	m_Segments.clear();

	if (!m_FaceRopeAnchors.empty())
	{
		for (size_t i = 0, ni = m_FaceRopeAnchors.size(); i < ni; ++i)
		{
			//delete m_FaceRopeAnchors[i];
			DestoryFaceRopeAnchorInternal(m_FaceRopeAnchors[i]);
			m_FaceRopeAnchors[i] = 0;
		}
	}
	m_RopeAnchorIndexVec.clear();
	m_ClampSegIndexVector.clear();
}

//=============================================================================================
void SutureThreadV2::Create(const GFPhysVector3 & StartFixPoint,
	const GFPhysVector3 & EndFixPoint,
	int segmetnCount,
	float masspernode,
	float rotMassPerSeg,
	bool rotHead)
{
	GFPhysAlignedVectorObj<GFPhysVector3> restNodePos;

	//create nodes
	//m_UnitLen = (StartFixPoint - EndFixPoint).Length() / segmetnCount;

	for (int r = 0; r <= segmetnCount; r++)
	{
		float weight = (float)r / (float)segmetnCount;

		GFPhysVector3 nodeRestPos = StartFixPoint*(1 - weight) + EndFixPoint*weight;

		restNodePos.push_back(nodeRestPos);
	}

	//
	if (rotHead)
	{
		int headSegStart = 3;
		GFPhysVector3 direction = (StartFixPoint - EndFixPoint).Normalized();
		GFPhysVector3 rotAxis = Perpendicular(direction);

		for (int c = 0; c < headSegStart; c++)
		{
			GFPhysVector3 t = restNodePos[c] - restNodePos[headSegStart];

			GFPhysQuaternion rotQuat(rotAxis, 3.14159f *0.07f * (1 + c));

			restNodePos[c] = restNodePos[headSegStart] + QuatRotate(rotQuat, t*0.35f);
		}
	}
	//test
	CreateFromPoints(restNodePos, masspernode, rotMassPerSeg);

	m_Rest_Length = 0.24f;

	for (int i = 0, ni = m_TubeNodes.size(); i < ni; i++)
	{
		((SutureThreadNodeV2*)m_TubeNodes[i])->m_GlobalId = GetGlobalId();
	}
	m_TotalRopeAnchorFriction = 0.04f;
}
//=============================================================================================
void SutureThreadV2::CreateFromPoints(GFPhysAlignedVectorObj<GFPhysVector3> & NodePos, float masspernode, float rotMassPerSeg)
{
	GFPhysSoftTube::Create(NodePos, masspernode, rotMassPerSeg);

	SetBendingTwistStiffness(0.0f, 0.0f);
	SetStretchShearStiffness(0.0f);
	//m_StretchShearDamping = 0.0025f;
	m_BendTwistSor = GFPhysVector3(0.2f, 0.2f, 0.2f);
}
//=============================================================================================
void SutureThreadV2::BeginSimulatePhysics(float dt)
{
	m_Segments[0].MarkAsAttached(true);//mark first segment is attach to rigid body

	//damping velocity and integrate position
	DampingVelocity(dt, m_NodeDamping, m_AngularDamping);

	//update current position + velocity * dt -> current position
	PredictUnConstraintMotion(dt);

	//update tree and check collision
	m_SegmentBVTree.Clear();//clear first

	float colRaidus = GetCollisionRadius();

	for (size_t n = 0; n < m_TubeNodes.size() - 1; n++)
	{
		GFPhysVector3 posCurr0 = m_TubeNodes[n]->m_CurrPosition;
		GFPhysVector3 posCurr1 = m_TubeNodes[n + 1]->m_CurrPosition;


		GFPhysVector3 poslLast0 = m_TubeNodes[n]->m_LastPosition;
		GFPhysVector3 poslLast1 = m_TubeNodes[n + 1]->m_LastPosition;

		GFPhysVector3 minPos = posCurr0;
		GFPhysVector3 maxPos = posCurr0;

		minPos.SetMin(posCurr1);
		maxPos.SetMax(posCurr1);

		if (m_UseCCD)
		{
			minPos.SetMin(poslLast0);
			minPos.SetMin(poslLast1);

			maxPos.SetMax(poslLast0);
			maxPos.SetMax(poslLast1);
		}

		GFPhysVector3 span(colRaidus + m_RopeCollideMargin, colRaidus + m_RopeCollideMargin, colRaidus + m_RopeCollideMargin);

		GFPhysDBVNode * bvNode = m_SegmentBVTree.InsertAABBNode(minPos - span, maxPos + span);
		bvNode->m_UserData = (void*)n;
	}
	
	//check collision
	GenerateCollidePairs(dt);
	FilterInverseCollidePairs();

	//disable segment collision between (in , out) pair anchor point
	for (int c = 0; c < GetNumSegments(); c++)
	{
		GFPhysSoftTubeSegment& tubeSeg = GetSegment(c);
		tubeSeg.SetCanCollideSoft(true);
	}

	int maxseg = GetNumSegments() - 1;

	AnchorTypeV2 lasttype = STATE_NULL;

	for (int c = (int)m_FaceRopeAnchors.size() - 1; ; c--)
	{
		bool innerfinish = false;
		
		int  minseg = 0;
		
		if (c < 0)
		{
			if (lasttype == STATE_IN)
			{
				minseg = 0;
				innerfinish = true;
			}
		}
		else
		{
			lasttype = m_FaceRopeAnchors[c]->m_type;
			
			int segIndex = m_FaceRopeAnchors[c]->GetSegIndex();
	
			if (lasttype == STATE_OUT)
			{
				minseg = GPClamped(segIndex - 1, 0, GetNumSegments() - 1);
				innerfinish = true;
			}
			else if (lasttype == STATE_IN)
			{
				maxseg = GPClamped(segIndex + 1, 0, GetNumSegments() - 1);
			}
		}
		if (innerfinish)
		{
			for (int i = minseg; i <= maxseg; i++)
			{
				GFPhysSoftTubeSegment& segMent = GetSegment(i);
				segMent.SetCanCollideSoft(false);
			}
		}
		if (c < 0)
		{
			break;
		}
	}


	if (m_KnotsInThread && m_KnotsInThread->m_bHasKnot == false)
	{
		DisableCollideSelfFromClampToTail();
	}

}
//================================================================================================
void SutureThreadV2::EndSimulatePhysics(float dt)
{
	UpdateVelocity(dt);
	SlideKnot();

	m_KnotImpluse[0] = GFPhysVector3(0.0f, 0.0f, 0.0f);
	m_KnotImpluse[1] = GFPhysVector3(0.0f, 0.0f, 0.0f);

	if (m_KnotsInThread)
	{
		GFPhysVectorObj<KnotInSutureRopeV2*> AllKnots;

		m_KnotsInThread->GetAllKnotsRef(AllKnots);

		if (AllKnots.size() > 0)
		{
			int min = AllKnots[AllKnots.size() - 1]->m_knotcon0.m_A;
			int max = AllKnots[AllKnots.size() - 1]->m_knotcon1.m_B;

			Real length = (GetTubeWireSegment(min).m_Node0->m_CurrPosition - GetTubeWireSegment(min).m_Node1->m_CurrPosition).Length();
			Real restlength = GetTubeWireSegment(min).GetRestLen();
			Real offset = length - restlength;
			if (offset > 0.05f)
			{
				m_KnotImpluse[0] = GFPhysVector3(offset, 0, 0);
			}

			length = (GetTubeWireSegment(max).m_Node0->m_CurrPosition - GetTubeWireSegment(max).m_Node1->m_CurrPosition).Length();
			restlength = GetTubeWireSegment(max).GetRestLen();
			offset = length - restlength;
			if (offset > 0.05f)
			{
				m_KnotImpluse[1] = GFPhysVector3(offset, 0, 0);
			}

		}
	}
}
//================================================================================================
void SutureThreadV2::SlideKnotPair(float slipDist , KnotInSutureRopeV2 & knot, int pair, int inValidSegStart, int inValidSegEnd , bool & reachLimit)
{
	 reachLimit = false;

	 int & mainSeg      = (pair == 0 ? knot.m_knotcon0.m_A       : knot.m_knotcon1.m_B);
	 float & mainWeight = (pair == 0 ? knot.m_knotcon0.m_weightA : knot.m_knotcon1.m_weightB);

	 int & followSeg      = (pair == 0 ? knot.m_knotcon1.m_A : knot.m_knotcon0.m_B);
	 float & followWeight = (pair == 0 ? knot.m_knotcon1.m_weightA : knot.m_knotcon0.m_weightB);

	 int  NewMainSegIndex = -1;
	 float NewMainSegWeight = 0.0f;

	 int  NewFollowSegIndex = -1;
	 float NewFollowSegWeight = 0.0f;

	 float currDist = 0.0f;

	 //calculate slip main
	 RelativePos2RestLen(mainSeg, mainWeight, currDist);
	 currDist += GPClamped(slipDist, -FLT_MAX, FLT_MAX);
	 RestLen2RelativePos(currDist, NewMainSegIndex, NewMainSegWeight);

	 //calculate slip follow
	 RelativePos2RestLen(followSeg, followWeight, currDist);
	 currDist += GPClamped(slipDist, -FLT_MAX, FLT_MAX);
	 RestLen2RelativePos(currDist, NewFollowSegIndex, NewFollowSegWeight);

	 if (NewMainSegIndex < 0 || NewMainSegIndex >= GetNumSegments()
	  || (NewMainSegIndex >= inValidSegStart && NewMainSegIndex <= inValidSegEnd))
	 {
		reachLimit = true;
		return;
	 }
		
	 if (NewFollowSegIndex < 0 || NewFollowSegIndex >= GetNumSegments()
	 || (NewFollowSegIndex >= inValidSegStart && NewFollowSegIndex <= inValidSegEnd))
	 {
		reachLimit = true;
		return;
	 }

	 //slip main part
	 //first slip out old segment
	 if (NewMainSegIndex != mainSeg)
	 {
		 GFPhysSoftTubeSegment & oldSeg = GetSegment(mainSeg);
		 oldSeg.SlipAnchorPoint(pair == 0 ? 1.0f : 0.0f);
	 }
	 //slip in to new one
	 GFPhysSoftTubeSegment & newMainSeg = GetSegment(NewMainSegIndex);
	 newMainSeg.SlipAnchorPoint(NewMainSegWeight);
	 mainSeg    = NewMainSegIndex;
	 mainWeight = NewMainSegWeight;

	 //slip follow part
	 //first slip out old segment
	 if (NewFollowSegIndex != followSeg)
	 {
		 GFPhysSoftTubeSegment & oldSeg = GetSegment(followSeg);
		 oldSeg.SlipAnchorPoint(pair == 0 ? 1.0f : 0.0f);
	 }
	 //slip in to new one
	 GFPhysSoftTubeSegment & newFollowSeg = GetSegment(NewFollowSegIndex);
	 newFollowSeg.SlipAnchorPoint(NewFollowSegWeight);//weight in gpsdk and here use different direction...
	 followSeg    = NewFollowSegIndex;
	 followWeight = NewFollowSegWeight;
}
//================================================================================================
void SutureThreadV2::SlideKnot()
{
	if (m_KnotsInThread && m_KnotsInThread->m_bHasKnot)
	{
		//GFPhysVectorObj<GFPhysFaceRopeAnchorV2*> activeRopeAnchors;
		//GetFaceRopeAnchors(activeRopeAnchors);
		KnotInSutureRopeV2 & knot = m_KnotsInThread->GetCurrKnot();

		if (knot.m_InShrinkKnotStep == false)
		{
			int nSegIndex;
			nSegIndex = (knot.m_knotcon0.m_A > 0 ? knot.m_knotcon0.m_A - 1 : knot.m_knotcon0.m_A);
			GFPhysSoftTubeSegment & NSeg_S = GetSegment(nSegIndex);
				
			nSegIndex = (knot.m_knotcon1.m_B < (GetNumSegments() - 1) ? knot.m_knotcon1.m_B + 1 : knot.m_knotcon1.m_B);
			GFPhysSoftTubeSegment & NSeg_L = GetSegment(nSegIndex);

			float totalSlip = 0;

			float maxFriction = 0.05f;

			bool  CanSlip = true;

			for (int c = 0; c < 2; c++)
			{
				 GFPhysSoftTubeSegment & seg = (c == 0 ? NSeg_S : NSeg_L);

				 float restlen = seg.GetRestLen();

				 float currlen = (seg.m_Node0->m_CurrPosition - seg.m_Node1->m_CurrPosition).Length();

				 float stretch = currlen - restlen;

				 if (stretch > maxFriction)
				 {
					 totalSlip += (stretch - maxFriction);
				 }
				 else
				 {
					 CanSlip = false;
					 break;
				 }
			}

			if (CanSlip)
			{
				totalSlip *= 0.5f;
				if (totalSlip > 0.1f)
					totalSlip = 0.1f;

				int minSegInvalid = GetRopeAnchorIndexMin();
				int maxSegInvalid = GetRopeAnchorIndexMax();
				int AnchroSegTotal = (maxSegInvalid - minSegInvalid + 1);

				int redundant = AnchroSegTotal - (abs(knot.m_knotcon1.m_A - knot.m_knotcon0.m_A) + 1);
				if (redundant < 0)
					redundant = 0;

				minSegInvalid -= int(redundant *0.5f);
				maxSegInvalid += int(redundant *0.5f);

				bool reachlimit0, reachlimit1;
				SlideKnotPair( totalSlip,  knot, 0,  minSegInvalid , maxSegInvalid, reachlimit0);
				SlideKnotPair(-totalSlip,  knot, 1,  minSegInvalid , maxSegInvalid, reachlimit1);
				
				if (reachlimit0 && reachlimit1)
				{
					knot.m_InShrinkKnotStep = true;
				}
			}
		 }
		 else
		 {
			if (knot.m_ShrinkRate > 0.5f)
			{
				knot.m_ShrinkRate -= 0.01f;

				for (int c = knot.m_knotcon0.m_A; c <= knot.m_knotcon1.m_B; c++)//两个节的话这里要修改
				{
					GetSegment(c).SetRestLenScale(knot.m_ShrinkRate);
				}
			}
		 }
	}
}
//================================================================================================
float SutureThreadV2::GetThreadRestLen()
{
	float totalLen = 0;
	
	for (int s = 0; s < GetNumSegments(); s++)
	{
		totalLen += GetSegment(s).GetRestLen();
	}
	
	return totalLen;
}
//================================================================================================
float SutureThreadV2::GetThreadCurrLen()
{
	int SegNum = m_TubeNodes.size() - 1;

	float totalLen = 0;

	for (int s = 0; s < GetNumSegments(); s++)
	{
		GFPhysSoftTubeSegment & segment = GetSegment(s);
		totalLen += (segment.m_Node0->m_CurrPosition - segment.m_Node1->m_CurrPosition).Length();
	}

	return totalLen;
}
//========================================================================================================================================
int SutureThreadV2::GetNumThreadNodes()
{
	return m_TubeNodes.size();
}
//========================================================================================================================================
int SutureThreadV2::GetNumSegments()
{
	return (int)m_Segments.size();
}
float SutureThreadV2::GetSegmentCurrLen(int segindex)
{
	if (segindex >= 0 && segindex < GetNumSegments())
	{
		float  len = (GetSegment(segindex).m_Node0->m_CurrPosition - GetSegment(segindex).m_Node1->m_CurrPosition).Length();
		return len;
	}
	else
	{
		return 0;
	}
}
//==================================================================================================================================================
GFPhysSoftTubeNode SutureThreadV2::GetThreadNode(int NodeIndex)
{
	return (*m_TubeNodes[NodeIndex]);
}
//==================================================================================================================================================
GFPhysSoftTubeNode & SutureThreadV2::GetThreadNodeRef(int NodeIndex)
{
	return (*m_TubeNodes[NodeIndex]);
}
//==================================================================================================================================================
SutureThreadNodeV2 & SutureThreadV2::GetThreadNodeRefReal(int NodeIndex)
{
	return (*(SutureThreadNodeV2*)m_TubeNodes[NodeIndex]);
}
//==================================================================================================================================================
SutureThreadNodeV2 & SutureThreadV2::GetThreadNodeGlobalRef(int NodeGlobalIndex, int& index)
{
	for (int i = m_TubeNodes.size() - 1; i >= 0; i--)
	{
		if (((SutureThreadNodeV2*)m_TubeNodes[i])->m_GlobalId == NodeGlobalIndex)
		{
			index = i;
			return (*(SutureThreadNodeV2*)m_TubeNodes[i]);
		}
	}
	//MXASSERT(0 && "GetThreadNode Node Index Not InRange");
	return m_NullNode;
}
//================================================================================
GFPhysSoftTubeSegment & SutureThreadV2::GetTubeWireSegment(int SegIndex)
{
	return m_Segments[SegIndex];
}
//===================================================================================================
const GFPhysAlignedVectorObj<STVRGCollidePair> & SutureThreadV2::GetCollidePairsWithRigid()
{
	return m_TRCollidePair;
}
//===================================================================================================
static bool SortFaceRopeAnchor(const GFPhysFaceRopeAnchorV2* v1, const GFPhysFaceRopeAnchorV2* v2)
{
	return v1->GetSegPos() < v2->GetSegPos();//降序排列  
}
//===========================================================================================================================
void SutureThreadV2::SetFaceRopeAnchors(GFPhysVectorObj<GFPhysFaceRopeAnchorV2::GFPhysFRAnchorConstructInFo> & anchorInfo)
{
	for (int c = 0; c < (int)anchorInfo.size(); c++)
	{
		GFPhysFaceRopeAnchorV2 * anchor = CreateFaceRopeAnchorInternal(anchorInfo[c]);
		m_FaceRopeAnchors.push_back(anchor);
	}
	//new segment begin in first
	std::sort(m_FaceRopeAnchors.begin(), m_FaceRopeAnchors.end(), SortFaceRopeAnchor);
}
//===========================================================================================================================
GFPhysFaceRopeAnchorV2 * SutureThreadV2::CreateFaceRopeAnchorInternal(GFPhysFaceRopeAnchorV2::GFPhysFRAnchorConstructInFo & cs)
{
	GFPhysFaceRopeAnchorV2 * anchor = new GFPhysFaceRopeAnchorV2(cs.m_type, cs.m_face, cs.m_FaceWeight, this, cs.m_IndexSeg, cs.m_WeightSeg);
	 anchor->m_FixFaceNormal = GFPhysVector3(0, 1, 0);//temp
	 return anchor;
}
//===========================================================================================================================
void SutureThreadV2::DestoryFaceRopeAnchorInternal(GFPhysFaceRopeAnchorV2 * anchor)
{
	 delete anchor;
}
//===========================================================================================================================
void SutureThreadV2::UpdateMesh()
{
	std::vector<Ogre::Vector3> RendNodes;

	for (int s = 0; s < (int)m_Segments.size(); s++)
	{
		if (((SutureThreadNodeV2*)(m_TubeNodes[s]))->m_bVisual)
		{
			RendNodes.push_back(GPVec3ToOgre(m_TubeNodes[s]->m_CurrPosition));

			GFPhysSoftTubeNode * anchor = m_Segments[s].GetAnchorPoint();
			if (anchor && (anchor != m_Segments[s].m_Node0) && (anchor != m_Segments[s].m_Node1))
			{
				RendNodes.push_back(GPVec3ToOgre(m_Segments[s].GetAnchorPoint()->m_CurrPosition));
			}
		}
	}
	RendNodes.push_back(GPVec3ToOgre(m_TubeNodes[m_TubeNodes.size() - 1]->m_CurrPosition));

	m_RendObject.UpdateRendSegment(RendNodes, m_RopeRendRadius);

	GFPhysVectorObj<Ogre::Vector3> RendNodes1;//empty section
	m_RendObject1.UpdateRendSegment(RendNodes1, m_RopeRendRadius);

	return;
}
//===================================================================================================================================
void SutureThreadV2::SolveCollisions(Real TimeStep, bool useFrict)
{
	//求解碰撞
	//solve soft-thread collision
	SolveSoftThreadCollisions();

	//solve rigid-thread collision
	SolveRigidThreadCollisions(TimeStep);

	//solve thread-thread collision
	SolveThreadThreadCollisions();
}
//===================================================================================================================================
void SutureThreadV2::SetSegmentBeClamped(int segIndex)
{
	for (int c = 0; c < (int)m_SegmentsBeClamped.size(); c++)
	{
		if (m_SegmentsBeClamped[c] == segIndex)
			return;
	}
	m_SegmentsBeClamped.push_back(segIndex);
}
//===================================================================================================================================
void SutureThreadV2::SetSegmentBeReleased(int segIndex)
{
	for (int c = 0; c < (int)m_SegmentsBeClamped.size(); c++)
	{
		if (m_SegmentsBeClamped[c] == segIndex)
		{
			m_SegmentsBeClamped.erase(m_SegmentsBeClamped.begin() + c);
			return;
		}
	}
}
//===================================================================================================================================
void SutureThreadV2::DampingVelocity(Real dt, Real linearDamp, const GFPhysVector3 & angularDamp)
{
	for (size_t n = 0; n < m_TubeNodes.size(); n++)
	{
		GFPhysSoftTubeNode * tNode = m_TubeNodes[n];

		float InvMass = tNode->GetInvMass();

		if (InvMass > FLT_EPSILON)
		{
			float realdamp = GPClamped(1.0f - dt * linearDamp, 0.0f, 1.0f);

			tNode->m_Velocity *= realdamp;

			tNode->m_Velocity += m_Gravity*dt;//currently no gravity uncomment this when we need extern forces
		}
	}

	//damping segment angular velocity
	GFPhysVector3 realAdamp;
	realAdamp.m_x = GPClamped(1.0f - dt * angularDamp.m_x, 0.0f, 1.0f);
	realAdamp.m_y = GPClamped(1.0f - dt * angularDamp.m_y, 0.0f, 1.0f);
	realAdamp.m_z = GPClamped(1.0f - dt * angularDamp.m_z, 0.0f, 1.0f);

	for (int c = 0; c < (int)m_Segments.size(); c++)
	{
		if (m_Segments[c].m_qInvMass > 0)
			m_Segments[c].m_AngularVel *= realAdamp;
	}
}
//===================================================================================================================================
SutureThreadNodeV2 * SutureThreadV2::CreateTubeNode(const GFPhysVector3 & restPos)
{
	SutureThreadNodeV2* newnode = new SutureThreadNodeV2(restPos);
	return newnode;
}
//===================================================================================================================================
void SutureThreadV2::DestroyTubeNode(SutureThreadNodeV2 * node)
{
	delete node;
}
//===================================================================================================================================
void SutureThreadV2::PrepareSolveConstraint(Real Stiffness, Real TimeStep)
{
	GFPhysSoftTube::PrepareSolveData(TimeStep, false);

	if (m_KnotsInThread)
	{
		m_KnotsInThread->PrepareSolveConstraint(Stiffness, TimeStep);

	}
	//prepare thread-rigid contact matrix
	for (size_t c = 0; c < m_TRCollidePair.size(); c++)
	{
		STVRGCollidePair & trPair = m_TRCollidePair[c];
        
		trPair.PrepareForSolveContact(TimeStep);
	}

	//face - rope anchor
	for (int i = 0, ni = m_FaceRopeAnchors.size(); i < ni; i++)
	{
		m_FaceRopeAnchors[i]->PrepareSolveConstraint(Stiffness, TimeStep);
	}
	m_SolveItorCount = 0;
}
//===================================================================================================================================
void SutureThreadV2::SolveConstraint(Real Stiffness, Real TimeStep)
{
	//solve rigid ,thread ,soft vs thread collision
	for (int c = 0; c < 2; c++)
	{
		//solve rigid-thread collision
		SolveRigidThreadCollisions(TimeStep);

		GFPhysSoftTube::SolveBendTwist(1.0f, TimeStep, c == 0 ? false : true);

		GFPhysSoftTube::SolveStretch(1.0f, TimeStep, c == 0 ? false : true);

		//solve soft-thread collision
		SolveSoftThreadCollisions();

		//solve thread-thread collision
		SolveThreadThreadCollisions();

		for (int i = 0, ni = m_FaceRopeAnchors.size(); i < ni; i++)
		{
			m_FaceRopeAnchors[i]->SolveConstraint(Stiffness, TimeStep);
		}

		if (m_KnotsInThread)
		{
			m_KnotsInThread->SolveConstraint(Stiffness, TimeStep);

		}
	}

	m_SolveItorCount++;
}
//===================================================================================================================================

void SutureThreadV2::SetNodeAsFix(int NodeIndex)
{
	if (true)
	{
#if 0
		SetSegmentAsFix(NodeIndex - 1);
#else
		GFPhysSoftTubeSegment & fixSeg = m_Segments[NodeIndex];
		fixSeg.m_Node0->SetMass(0);
		fixSeg.m_qInvMass = 0.0f;
#endif
		m_islock = true;
	}
}
//===================================================================================================================================

void SutureThreadV2::ReleaseNodeAsFix(int NodeIndex)
{
	if (m_islock)
	{
		GFPhysSoftTubeSegment & fixSeg = m_Segments[NodeIndex];
		fixSeg.m_Node0->SetMass(ThreadMass);
		fixSeg.m_qInvMass = rotMassPerSeg;

		m_islock = false;
	}	
}
//===================================================================================================
void SutureThreadV2::SolveSoftThreadCollisions()
{
	for (size_t c = 0; c < m_TFCollidePair.size(); c++)
	{
		STVSFCollidePair & cdpair = m_TFCollidePair[c];
		cdpair.SolveCollision();
	}
}
//============================================================================================================================================
void SutureThreadV2::SolveRigidThreadCollisions(Real dt)
{
	for (size_t c = 0; c < m_TRCollidePair.size(); c++)
	{
		STVRGCollidePair & trPair = m_TRCollidePair[c];
		trPair.SolveCollision(dt);
	}
}
//=================================================================================================================================
void SutureThreadV2::SolveThreadThreadCollisions()
{
	for (size_t c = 0; c < m_TTCollidepair.size(); c++)
	{
		STVSTCollidePair & ttPair = m_TTCollidepair[c];
		ttPair.SolveCollision();
	}
}
//=================================================================================================================================
void SutureThreadV2::GenerateCollidePairs(Real dt)
{
	ITraining * itrain = CTrainingMgr::Instance()->GetCurTraining();

	MisNewTraining * newTrain = dynamic_cast<MisNewTraining * >(itrain);

	float colradius = GetCollisionRadius();

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

				SutureThreadV2SegSoftFaceCallback collideCallBack(m_RopeCollideMargin, physbody, colradius, this, m_TFCollidePair, m_UseCCD);

				for (size_t t = 0, ni = bvTrees.size(); t < ni; t++)
				{
					GFPhysDBVTree * bvTree = bvTrees[t];
					bvTree->CollideWithDBVTree(m_SegmentBVTree, &collideCallBack);
				}
			}
		}

		//check collision with rigid body
		const GFPhysCollideObjectArray & collObjects = PhysicsWrapper::GetSingleTon().m_dynamicsWorld->GetCollideObjectArray();

		for (size_t c = 0, ni = collObjects.size(); c < ni; c++)
		{
			GFPhysCollideObject * cdobj = collObjects[c];

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

					SutureThreadV2ConvexCallBack rbconcallback(m_RopeCollideMargin, rigidbody, rbTrans, dt, colradius, this, m_TRCollidePair);

					m_SegmentBVTree.TraverseTreeAgainstAABB(&rbconcallback, Convexaabbmin, Convexaabbmax);
				}
				else if (rigidshape->IsCompound())
				{
					GFPhysCompoundShape* compoundshape = (GFPhysCompoundShape*)rigidshape;
					
					GFPhysTransform rbTrans = rigidbody->GetWorldTransform();
					
					for (int i = 0, ni = compoundshape->GetNumComponent(); i < ni; i++)
					{
						 GFPhysTransform childTrans = compoundshape->GetComponentTransform(i);

						 GFPhysCollideShape * childshape = compoundshape->GetComponentShape(i);

						 GFPhysVector3 childshapeaabbmin, childshapeaabbmax;

						 GFPhysTransform childworldtrans = rbTrans*childTrans;

						 childshape->GetAabb(childworldtrans, childshapeaabbmin, childshapeaabbmax);

						 SutureThreadV2CompoundCallBack STCompoundCallback(m_RopeCollideMargin, rigidbody, childshape, childworldtrans, colradius, this, m_TRCollidePair);

						 m_SegmentBVTree.TraverseTreeAgainstAABB(&STCompoundCallback, childshapeaabbmin, childshapeaabbmax);
					}
				}
			}
		}

		m_ownertrain->onSutureThreadConvexCollided(m_TRCollidePair, this);//抓线优化---

		//self collision
		SutureThreadV2ThreadCallBack ttcallback(m_RopeCollideMargin , colradius,
				                               this , this  ,m_TTCollidepair);

		m_SegmentBVTree.CollideWithDBVTree(m_SegmentBVTree, &ttcallback);

	}
}

static int SortCollisionPair(const void * v1, const void * v2)
{
	return ((STVSFCollidePair*)v1)->m_SegIndex - ((STVSFCollidePair*)v2)->m_SegIndex;
}

//========================================================================================================
void SutureThreadV2::FilterInverseCollidePairs()
{
	if (m_TFCollidePair.size() > 0)
	{ 
		std::qsort(&m_TFCollidePair[0], m_TFCollidePair.size(), sizeof(m_TFCollidePair[0]), SortCollisionPair);

		GFPhysAlignedVectorObj<GFPhysVector3> posCollisionNormals;

	    int c = 0;
	    
		int start = 0;
		
		int end = 0;

		bool hasPosNormCol = m_TFCollidePair[0].m_IsPositiveCol;
		
		int  segIndex = m_TFCollidePair[0].m_SegIndex;

		if (m_TFCollidePair[0].m_IsPositiveCol)
		{
			posCollisionNormals.push_back(m_TFCollidePair[c].m_CollideNormal);
		}

		while (true)
		{
			bool intervalEnd = true;
			
			if (c < m_TFCollidePair.size())
			{
				if (m_TFCollidePair[c].m_SegIndex == segIndex)
				{
					if (m_TFCollidePair[c].m_IsPositiveCol)
					{
						//posCollisionNormals.push_back(m_TFCollidePair[c].m_CollideNormal);
						hasPosNormCol = true;
					}
					end = c;
					intervalEnd = false;
				}
			}
			
			if (intervalEnd)
			{
				if (hasPosNormCol == false)
				{
					for (int t = start; t <= end; t++)
					{
						if (m_TFCollidePair[t].m_IsPositiveCol == false)
						{
							/*bool needInverse = true;

							for (int p = 0; p < (int)posCollisionNormals.size(); p++)
							{
								if (m_TFCollidePair[t].m_CollideNormal.Dot(posCollisionNormals[p]) > 0)
								{
									needInverse = false;
									break;
								}
							}

							if (needInverse)*/
								m_TFCollidePair[t].m_CollideNormal *= -1.0f;// m_TFCollidePair[t].m_FaceNormal;// -1.0f;//negative normal
								//m_TFCollidePair[t].m_ColRadius = 1.1f;
						}
					}
				}
				//new interval start
				if (c < m_TFCollidePair.size())
				{
					segIndex = m_TFCollidePair[c].m_SegIndex;
					start = end = c;
					hasPosNormCol = m_TFCollidePair[c].m_IsPositiveCol;
					posCollisionNormals.clear();

					if (m_TFCollidePair[c].m_IsPositiveCol)
					{
						//posCollisionNormals.push_back(m_TFCollidePair[c].m_CollideNormal);
					}
				}
				else
				{
					break;
				}
			}
			c++;
		}
	}
}
void SutureThreadV2::SetRopeAnchorIndex(std::vector<Real> nodeindex)
{
	m_RopeAnchorIndexVec = nodeindex;
}

GFPhysVectorObj<Real> SutureThreadV2::GetRopeAnchorIndex()
{
	return m_RopeAnchorIndexVec;
}
//============================================================================================
bool SutureThreadV2::RelativePos2RestLen(int index, Real weight, Real& currlen)
{
	GPClamp(index, 0, GetNumSegments() - 1);

	currlen = 0;

	for (int s = 0; s < index; s++)
	{
		currlen += GetSegment(s).GetRestLen();
	}

	currlen += GetSegment(index).GetRestLen() * weight;
	
	return true;
}
//============================================================================================
bool SutureThreadV2::RestLen2RelativePos(Real currlen, int& index, Real& weight)
{
	//input filter missing
	if (currlen <= 0)
	{
		index = 0;
		weight = 0.0f;
		return true;
	}

	float partsum = 0.0f;
	
	for (int s = 0 ; s < GetNumSegments(); s++)
	{
		float segLen = GetSegment(s).GetRestLen();
		
		partsum += segLen;

		if (partsum > currlen)
		{
			index = s;
			weight = GPClamped(1.0f - (partsum - currlen) / segLen, 0.0f, 1.0f);
			return true;
		}
	}

	index  = GetNumSegments()-1;
	weight = 1.0f;
	return true;
}
//==================================================================================================================
float SutureThreadV2::GetRestDistBetweenSegments(int seg0, float weight0, int seg1, float weight1)
{
	if (seg0 + weight0 > seg1 + weight1)
	{
		std::swap(seg0, seg1);
		std::swap(weight0, weight1);
	}

	float dist = 0;
	if (seg0 == seg1)
	{
		dist = GetSegment(seg0).GetRestLen() * (weight1 - weight0);
	}
	else
	{
		dist = GetSegment(seg0).GetRestLen() * (1.0f - weight0);
		for (int c = seg0 + 1; c < seg1; c++)
		{
			dist += GetSegment(c).GetRestLen();
		}
		dist += GetSegment(seg1).GetRestLen() * weight1;
	}

	return dist;
}
//======================================================================================================================================
float SutureThreadV2::GetStretchedDistBetweenSegments(int seg0, float weight0, int seg1, float weight1)
{
	if(seg0 + weight0 > seg1 + weight1)
	{
	   std::swap(seg0, seg1);
	   std::swap(weight0, weight1);
	}

	float distStretch = 0;
	
	///float distCurr = 0;
	float frictionClamp = 0.05f;

	if (seg0 == seg1)
	{
		GFPhysSoftTubeSegment & segMent = GetSegment(seg0);

		float distRest =  segMent.GetRestLen() * (weight1 - weight0);
		
		float distCurr = (segMent.m_Node0->m_CurrPosition - segMent.m_Node1->m_CurrPosition).Length() * (weight1 - weight0);
		
		return distStretch += (distCurr - distRest) < frictionClamp ? 0.0f : (distCurr - distRest - frictionClamp);
	}
	else
	{
		float distRest = GetSegment(seg0).GetRestLen() * (1.0f - weight0);
		
		float distCurr = (GetSegment(seg0).m_Node0->m_CurrPosition - GetSegment(seg0).m_Node1->m_CurrPosition).Length() * (1.0f - weight0);

		distStretch += (distCurr - distRest) < frictionClamp ? 0.0f : (distCurr - distRest - frictionClamp);

		for (int c = seg0 + 1; c < seg1; c++)
		{
			distRest =  GetSegment(c).GetRestLen();
			distCurr = (GetSegment(c).m_Node0->m_CurrPosition - GetSegment(c).m_Node1->m_CurrPosition).Length();

			distStretch += (distCurr - distRest) < frictionClamp ? 0.0f : (distCurr - distRest - frictionClamp);
		}

		 distRest = GetSegment(seg1).GetRestLen() * weight1;
		
		 distCurr = (GetSegment(seg1).m_Node0->m_CurrPosition - GetSegment(seg1).m_Node1->m_CurrPosition).Length() * weight1;

		 distStretch += (distCurr - distRest) < frictionClamp ? 0.0f : (distCurr - distRest - frictionClamp);

		return distStretch;
	}
}
//========================================================================================================================
void SutureThreadV2::SlipInFaceRopeAnchor(AnchorTypeV2 type, GFPhysSoftBodyFace* face, Real * faceweights, Real offset)
{
	 Real tanOffset = offset;

	 tanOffset *= 1.0f;

	 Real TotalThreadLen = GetThreadRestLen();

	 GPClamp(tanOffset, 0.0f, TotalThreadLen * 0.1f);

	 int  segindex  = 0;
	
	 Real segweight = 0.0f;

	 RestLen2RelativePos(tanOffset, segindex, segweight);

	 //check type
	 if (m_FaceRopeAnchors.size() == 0)
		 type = STATE_IN;
	 else
	 {
		 if (m_FaceRopeAnchors[0]->m_type == STATE_IN)
			 type = STATE_OUT;
		 else
			 type = STATE_IN;
	 }
	
	 GFPhysFaceRopeAnchorV2::GFPhysFRAnchorConstructInFo cs(face, faceweights, segindex, segweight, type);

	 GFPhysFaceRopeAnchorV2 * newanchor = CreateFaceRopeAnchorInternal(cs);

	 m_FaceRopeAnchors.resize(m_FaceRopeAnchors.size() + 1);
	
	 for (int c = m_FaceRopeAnchors.size() - 1; c >= 1; c--)
	 {
		m_FaceRopeAnchors[c] = m_FaceRopeAnchors[c - 1];
	 }
	 m_FaceRopeAnchors[0] = newanchor;//newer become first

	 StandarlizeRopeAnchors();
}
//========================================================================================================================
void SutureThreadV2::RemoveSlipOuttedAnchor()
{
	 int startIndex = -1;

	 for (int c = 0; c < (int)m_FaceRopeAnchors.size(); c++)
	 {
		  GFPhysFaceRopeAnchorV2 * ropeAnchor = m_FaceRopeAnchors[c];
		  if (ropeAnchor->IsSlippedOut())
		  {
			  startIndex = c;
			  break;
		  }
	 }

	 if (startIndex >= 0)
	 {
		 for (int c = startIndex; c < (int)m_FaceRopeAnchors.size(); c++)
		 {
			  GFPhysFaceRopeAnchorV2 * invalidAnchor = m_FaceRopeAnchors[c];
			 
			  m_NeedleAttchedThread->notifyRemoveRopeAnchor(invalidAnchor->GetFace(), invalidAnchor->m_weights);

			  DestoryFaceRopeAnchorInternal(invalidAnchor);
		 }
		 m_FaceRopeAnchors.resize(startIndex);
	}
}
//========================================================================================================================
void SutureThreadV2::StandarlizeRopeAnchors()
{
	for (int c = 0; c < (int)m_FaceRopeAnchors.size()-1; c++)
	{
		GFPhysFaceRopeAnchorV2 * A0 = m_FaceRopeAnchors[c];

		GFPhysFaceRopeAnchorV2 * A1 = m_FaceRopeAnchors[c+1];

		if (A0->GetSegIndex() == 0 && A0->GetSegWeight() < 0.5f)
		{
			A0->SetAnchorSegAndWeight(0 , 0.5f);
		}

		if (A0->GetSegIndex() >= A1->GetSegIndex())
		{
			int NewSeg = A0->GetSegIndex() + 1;
			float NewWeight = 0.0f;
			
			if (A1->GetSegIndex() >= GetNumSegments())
			{
				NewSeg = GetNumSegments() - 1;
				NewWeight = 1.0f;
			}
			A1->SetAnchorSegAndWeight(NewSeg, NewWeight);
		}
	}
}
//========================================================================================================================
bool SutureThreadV2::computeMaterialFrame(const GFPhysVector3& p0, const GFPhysVector3& p1, const GFPhysVector3& p2, GFPhysMatrix3& frame)
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
//========================================================================================================================
bool SutureThreadV2::GetThreadSegmentNodePos(GFPhysVector3 & n0, GFPhysVector3 & n1, int segIndex)
{
	if (segIndex >= 0 && segIndex < (int)m_TubeNodes.size() - 1)
	{
		n0 = m_TubeNodes[segIndex]->m_CurrPosition;
		n1 = m_TubeNodes[segIndex + 1]->m_CurrPosition;
		return true;
	}
	return false;
}
//========================================================================================================================
void SutureThreadV2::DisableCollideSelfFromClampToTail()
{	
	//考虑左右手器械同时夹线打结
	int num = GetNumThreadNodes();
	if (!m_ClampSegIndexVector.empty())
	{
		std::vector<int> localindexvector;
		for (int i = 0; i < (int)m_ClampSegIndexVector.size(); i++)
		{
			int index = -1;
			SutureThreadNodeV2& node = GetThreadNodeGlobalRef(m_ClampSegIndexVector[i], index);
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
			GetTubeWireSegment(n).SetCanCollideSelf(true);
		}

		GFPhysVectorObj<KnotInSutureRopeV2> Knots;
		m_KnotsInThread->GetDeadKnots(Knots);
		if (Knots.size() == 1)
		{
			for (int n = Knots[0].m_knotcon0.m_A; n < Knots[0].m_knotcon1.m_B; n++)
			{				
				GetTubeWireSegment(n).SetCanCollideSelf(false);
			}
		}
		else if (Knots.size() == 2)
		{
			for (int n = Knots[1].m_knotcon0.m_A; n < Knots[1].m_knotcon1.m_B; n++)
			{
				GetTubeWireSegment(n).SetCanCollideSelf(false);
			}
		}

		for (int n = maxID; n < num; n++)
		{
			GetTubeWireSegment(n).SetCanCollideSelf(false);
		}
	}
	else
	{
		for (int n = 0; n < num; n++)
		{
			GetTubeWireSegment(n).SetCanCollideSelf(true);
		}

		GFPhysVectorObj<KnotInSutureRopeV2> Knots;
		m_KnotsInThread->GetAllKnots(Knots);
		if (Knots.size() == 1)
		{
			for (int n = Knots[0].m_knotcon0.m_A; n < Knots[0].m_knotcon1.m_B; n++)
			{
				GetTubeWireSegment(n).SetCanCollideSelf(false);
			}
		}
		else if (Knots.size() == 2)
		{
			for (int n = Knots[1].m_knotcon0.m_A; n < Knots[1].m_knotcon1.m_B; n++)
			{
				GetTubeWireSegment(n).SetCanCollideSelf(false);
			}
		}

	}
}
//========================================================================================================================
void SutureThreadV2::BuildKnotPoint(const KnotInSutureRopeV2& knot, std::vector<Ogre::Vector3> & rendpoints1, std::vector<Ogre::Vector3> & rendpoints2)
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

	for (Real theta = 0; theta < angle; theta += angle / num)
	{
		GFPhysQuaternion LocalRot(InitAxis, theta);
		GFPhysVector3 CurrRotVec = QuatRotate(LocalRot, InitRotVec);
		GFPhysVector3 offset = CurrRotVec * expandcoff * m_RopeRendRadius;

		GFPhysVector3 rendercenter = point0 * (1.0f - k) + point1 * k;

		rendpoints1.push_back(GPVec3ToOgre(rendercenter - offset));
		rendpoints2.push_back(GPVec3ToOgre(rendercenter + offset));

		k += 1.0f / num;
	}
}
void SutureThreadV2::SolveFixOrientation(GFPhysSoftTubeNode* va, GFPhysSoftTubeNode* vb, GFPhysVector3 &rotCenter, GFPhysVector3& orientation, Real Stiffness)
{	
	Real wa = va->GetInvMass();

	Real wb = vb->GetInvMass();

	GFPhysVector3 displace = va->m_CurrPosition - vb->m_CurrPosition;



#if(0)
	Ogre::Vector3 v0 = GPVec3ToOgre(displace.Normalized());
	Ogre::Vector3 v1 = GPVec3ToOgre(orientation.Normalized());
	Ogre::Quaternion rot = v0.getRotationTo(v1);
	if (rot.w < 0)
	{
		rot.x = -rot.x;
		rot.y = -rot.y;
		rot.z = -rot.z;
	}

	GFPhysQuaternion gpQuat(rot.x, rot.y, rot.z, rot.w);

	va->m_CurrPosition = (QuatRotate(gpQuat, va->m_CurrPosition - rotCenter) + rotCenter);
	vb->m_CurrPosition = (QuatRotate(gpQuat, vb->m_CurrPosition - rotCenter) + rotCenter);


#else
	Real leng = displace.Length();

	if (leng < GP_EPSILON)
		return;

	Real c = (displace / leng).Dot(orientation) - 1.0f;

	GFPhysVector3 grad = orientation / leng - (orientation.Dot(displace) / leng / leng / leng) * displace;

	Real lambda = Stiffness * c / (wa*grad.Length2() + wb *grad.Length2());

	va->m_CurrPosition += -lambda*wa*grad;

	vb->m_CurrPosition += lambda*wb*grad;
#endif
	//////////////////////////////////////////////////////////////////////////
	//qDebug() << "grad" << ":" << grad.GetX() << "," << grad.GetY() << "," << grad.GetZ() << ".";
	//qDebug() << "displace" << ":" << displace.GetX() << "," << displace.GetY() << "," << displace.GetZ() << ".";
	//qDebug() << "leng " << ":" << leng << ".";
	//qDebug() << "dleta va " << ":" << -lambda*wa*grad.GetX() << "," << -lambda*wa*grad.GetY() << "," << -lambda*wa*grad.GetZ() << ".";

}
#define USESPLITANCHORPOINT 0
//=========================================================================================================================
GFPhysFaceRopeAnchorV2::GFPhysFaceRopeAnchorV2(AnchorTypeV2 type, GFPhysSoftBodyFace* face, float faceWeight[3], SutureThreadV2 * thread, int segindex, float segWeight)
{
	m_type = type;
	m_Face = face;
	m_weights[0] = faceWeight[0];
	m_weights[1] = faceWeight[1];
	m_weights[2] = faceWeight[2];

	m_thread    = thread;
	m_SegIndex  = -1;
	m_WeightSeg = 0.0f;

	m_Friction = 0.0f;

	GFPhysSoftTubeNode& n0 = m_thread->GetThreadNodeRef(m_SegIndex);
	GFPhysSoftTubeNode& n1 = m_thread->GetThreadNodeRef(m_SegIndex + 1);

	m_Last_Pos = n0.m_CurrPosition * (1 - m_WeightSeg) + n1.m_CurrPosition * m_WeightSeg;

	m_lengthEnlarge = 0.0f;

	m_bHaveFixNor = false;
	m_FixFaceNormal = GFPhysVector3(0, 0, 0);

	SetAnchorSegAndWeight(segindex, segWeight);

	m_ToSlipDist = 0;
	m_IsSlipDisabled = false;
	m_AlignedToFace = false;
	m_IsDragedTwoEnd = false;
}
//=========================================================================================================================
GFPhysFaceRopeAnchorV2::~GFPhysFaceRopeAnchorV2()
{
	int numSegment = m_thread->GetNumSegments();
	
	SetAnchorSegAndWeight(numSegment-1 , 1.0f);
}
//=========================================================================================================================
void GFPhysFaceRopeAnchorV2::SetFriction(float friction)
{
	m_Friction = friction;
}
//=========================================================================================================================
void GFPhysFaceRopeAnchorV2::PrepareSolveConstraint(Real Stiffness, Real TimeStep)
{
	m_IterorCount = 0;
	m_AccumFriction = 0.0f;
	m_AccumPressure = GFPhysVector3(0.0f, 0.0f, 0.0f);
}
//=========================================================================================================================
void GFPhysFaceRopeAnchorV2::SolveConstraint(Real Stiffniss, Real TimeStep)
{
	if (m_Face->m_Nodes[0]->m_InvM < GP_EPSILON || m_Face->m_Nodes[1]->m_InvM < GP_EPSILON || m_Face->m_Nodes[2]->m_InvM < GP_EPSILON)
	{
		return;
	}

	if (m_SegIndex < 0 || m_SegIndex >= m_thread->GetNumSegments())
	{
		return;
	}

#if(USESPLITANCHORPOINT)
	GFPhysSoftTubeNode * anchorNode = m_thread->GetSegment(m_SegIndex).GetAnchorPoint();

	GFPhysVector3 ptThread = anchorNode->m_CurrPosition;// t0.m_CurrPosition * m_ThreadWeights[0] + t1.m_CurrPosition * m_ThreadWeights[1];
#else
	GFPhysSoftTubeNode & t0 = m_thread->GetThreadNodeRef(m_SegIndex);
	
	GFPhysSoftTubeNode & t1 = m_thread->GetThreadNodeRef(m_SegIndex + 1);
	
	GFPhysVector3 ptThread = t0.m_CurrPosition * (1 - m_WeightSeg) + t1.m_CurrPosition * m_WeightSeg;
#endif

	GFPhysVector3 ptFace   = m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0]
			               + m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1]
			               + m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2]
						   - m_Face->m_FaceNormal * 0.03f;

	GFPhysVector3 Diff = ptThread - ptFace;

	float deltaLen = Diff.Length();

	float invMassT = 1.0f;

	float invMassF = 0.0f;

	if (m_IsSlipDisabled || m_IsDragedTwoEnd)
	{
		invMassT = 0.5f;
		invMassF = 1.0f;
	}
	if (deltaLen > FLT_EPSILON)
	{
#if(USESPLITANCHORPOINT)
		float w = invMassT + (m_weights[0] * m_weights[0] + m_weights[1] * m_weights[1] + m_weights[2] * m_weights[2]) * invMassF;
		
		anchorNode->m_CurrPosition -= Diff * (invMassT / w);
		
		m_Face->m_Nodes[0]->m_CurrPosition += Diff * (invMassF * m_weights[0] / w);
		m_Face->m_Nodes[1]->m_CurrPosition += Diff * (invMassF * m_weights[1] / w);
		m_Face->m_Nodes[2]->m_CurrPosition += Diff * (invMassF * m_weights[2] / w);
#else
		GFPhysVector3 NormDiff = Diff / deltaLen;
		
		float w = ((1 - m_WeightSeg) * (1 - m_WeightSeg) + m_WeightSeg * m_WeightSeg) * invMassT
			    + (m_weights[0] * m_weights[0] + m_weights[1] * m_weights[1] + m_weights[2] * m_weights[2])*invMassF;

		t0.m_CurrPosition -= NormDiff * deltaLen * (invMassT * (1 - m_WeightSeg) / w);

		t1.m_CurrPosition -= NormDiff * deltaLen * (invMassT *  m_WeightSeg / w);


		m_Face->m_Nodes[0]->m_CurrPosition += NormDiff * deltaLen * (invMassF * m_weights[0] / w);

		m_Face->m_Nodes[1]->m_CurrPosition += NormDiff * deltaLen * (invMassF * m_weights[1] / w);

		m_Face->m_Nodes[2]->m_CurrPosition += NormDiff * deltaLen * (invMassF * m_weights[2] / w);
#endif
		if (m_AlignedToFace)
		{
			//if (m_IterorCount == 0)
			{
				GFPhysVector3 rotCenter = (t0.m_CurrPosition * (1 - m_WeightSeg) + t1.m_CurrPosition * m_WeightSeg);
				if (m_type == STATE_IN)
					m_thread->SolveFixOrientation(&t1, &t0, rotCenter, m_Face->m_FaceNormal, 0.5f);
				else
					m_thread->SolveFixOrientation(&t0, &t1, rotCenter, m_Face->m_FaceNormal, 0.5f);
			}
		}
		m_AccumPressure += Diff;
	}
	m_IterorCount++; //
}
//================================================================================================
GFPhysVector3 GFPhysFaceRopeAnchorV2::GetAnchorOnFaceRestPos()
{
	return m_Face->m_Nodes[0]->m_UnDeformedPos * m_weights[0]
		+ m_Face->m_Nodes[1]->m_UnDeformedPos * m_weights[1]
		+ m_Face->m_Nodes[2]->m_UnDeformedPos * m_weights[2];

}
//================================================================================================
GFPhysVector3 GFPhysFaceRopeAnchorV2::GetAnchorOnFaceCurrPos()
{
	return m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0]
		+ m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1]
		+ m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2];

}
//===============================================================================================
void GFPhysFaceRopeAnchorV2::SetAnchorSegAndWeight(int segIndex, Real weight)
{
	if (segIndex >= m_thread->GetNumSegments())//out limit
	{
		segIndex = m_thread->GetNumSegments() - 1;
		weight = 1.0f;
	}

	if (segIndex < 0)
	{
		segIndex = 0;
		weight = 0.5f;
	}
	weight = GPClamped(weight, 0.0f, 1.0f);

	//out of old one
#if(USESPLITANCHORPOINT)
	if (segIndex != m_SegIndex)
	{
		if (m_SegIndex >= 0 && m_SegIndex < m_thread->GetNumSegments())
		{
			GFPhysSoftTubeSegment & oldSeg = m_thread->GetSegment(m_SegIndex);
			if (segIndex > m_SegIndex)
				oldSeg.SlipAnchorPoint(1.0f);
			else
				oldSeg.SlipAnchorPoint(0.0f);
		}
	}
#endif

	//slip in to new one
	if (segIndex >= 0 && segIndex < m_thread->GetNumSegments())
	{
#if(USESPLITANCHORPOINT)
		GFPhysSoftTubeSegment & newSeg = m_thread->GetSegment(segIndex);
		newSeg.SlipAnchorPoint(weight);
#endif
		m_SegIndex  = segIndex;
		m_WeightSeg = weight;
	}
}
//================================================================================================
bool GFPhysFaceRopeAnchorV2::IsSlippedOut()
{
	if (m_SegIndex >= m_thread->GetNumSegments())
		return true;

	if (m_SegIndex == m_thread->GetNumSegments() - 1 && m_WeightSeg > (1.0f-GP_EPSILON))
		return true;
	else
		return false;
}
//================================================================================================

GoPhys::GFPhysVector3 GFPhysFaceRopeAnchorV2::GetAccumPressure() const
{
	return m_AccumPressure;
}


//================================================================================================
Real SutureThreadV2::GetCustomLen(bool deformed, int i, int j)
{
	int SegNum = m_TubeNodes.size() - 1;

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
			GFPhysVector3 p0 = m_TubeNodes[n]->m_CurrPosition;
			GFPhysVector3 p1 = m_TubeNodes[n + 1]->m_CurrPosition;

			Real t = (p0 - p1).Length();
			totalLen += t;
		}
		return totalLen;
	}
}

//================================================================================================

Real SutureThreadV2::CalcSlackRatio(Real lengthcorrect, int minseg, int maxseg)
{
	Real wmin = 0.5f;
#if 0
	if (minseg > maxseg + 1)
	{
		wmin = 1.0f;
	}
	else if (maxseg > minseg + 1)
	{
		wmin = 0.0f;
	}
#else
	if (minseg > maxseg + 0.5f)
	{
		if (lengthcorrect > m_Rest_Length*(minseg - maxseg))//GetCustomLen(false, GetRopeAnchorIndexMin(), min1) - GetCustomLen(false, max0, GetRopeAnchorIndexMax())
		{
			wmin = (lengthcorrect + m_Rest_Length*(minseg - maxseg))*0.5f / lengthcorrect;
		}
		else
		{
			wmin = 1.0f;
		}
	}
	else if (maxseg > minseg + 0.5f)
	{
		if (lengthcorrect > m_Rest_Length*(maxseg - minseg))
		{
			wmin = (lengthcorrect - m_Rest_Length*(maxseg - minseg))*0.5f / lengthcorrect;
		}
		else
		{
			wmin = 0.0f;
		}
	}
#endif		
	return wmin;
}