#include "SutureThreadV2Collision.h"

#define FACEMASSINANCHORANDCOLLIDE 1.0f
//===================================================================================================================================
STVSFCollidePair::STVSFCollidePair(GFPhysSoftBody * sb, GFPhysSoftTube * tube, GFPhysSoftBodyFace * face)
{
	m_CollideBody = sb;
	m_SoftFace = face;
	m_Tube = tube;

	m_CollideNormal = ((m_SoftFace->m_Nodes[1]->m_CurrPosition - m_SoftFace->m_Nodes[0]->m_CurrPosition).Cross(m_SoftFace->m_Nodes[2]->m_CurrPosition - m_SoftFace->m_Nodes[0]->m_CurrPosition));
	m_CollideNormal.Normalize();
	m_ImpluseOnThread = GFPhysVector3(0, 0, 0);
	m_IsPositiveCol = true;
}
//===================================================================================================================================
STVSFCollidePair::STVSFCollidePair(GFPhysSoftBody * sb ,GFPhysSoftTube * tube)
{
	m_CollideBody = sb;
	m_Tube = tube;
	m_ImpluseOnThread = GFPhysVector3(0, 0, 0);
	m_IsPositiveCol = true;
}
//===================================================================================================================================
void STVSFCollidePair::SetEF_Collision(int segIndex, const GFPhysVector3 & collideNormal, const GFPhysVector3 & FaceNormal)
{
	//m_CollideType = STVSFCollidePair::TFCD_EF;
	m_CollideNormal = collideNormal;
	m_FaceNormal = FaceNormal;
	m_SegIndex = segIndex;
	//m_e4 = segIndex + 1;

	m_CollideNode[0] = m_SoftFace->m_Nodes[0];
	m_CollideNode[1] = m_SoftFace->m_Nodes[1];
	m_CollideNode[2] = m_SoftFace->m_Nodes[2];

	m_CollideNodeDepth[0] = m_CollideNodeDepth[1] = m_CollideNodeDepth[2] = 0;

	GFPhysSoftBodyTetrahedron * tetra = m_SoftFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;
	m_NumCollideNode = 3;
	for (int v = 0; v < 4; v++)
	{
		GFPhysSoftBodyNode * collideNode = tetra->m_TetraNodes[v];
		if ((collideNode != m_CollideNode[0]) && (collideNode != m_CollideNode[1]) && (collideNode != m_CollideNode[2]))
		{
			m_CollideNode[m_NumCollideNode] = collideNode;
			m_CollideNodeDepth[m_NumCollideNode] = (collideNode->m_CurrPosition - m_CollideNode[0]->m_CurrPosition).Dot(m_CollideNormal);
			m_NumCollideNode++;
		}
	}

	m_ColRadius = m_Tube->GetCollisionRadius();
}
//===================================================================================================================================
GFPhysVector3 STVSFCollidePair::SolveCollision()
{
	const GFPhysSoftTubeSegment & segMent = m_Tube->GetSegment(m_SegIndex);

	if (segMent.GetCanCollideSoft())
	{
		GFPhysSoftBodyNode * facen0 = m_SoftFace->m_Nodes[0];

		GFPhysSoftBodyNode * facen1 = m_SoftFace->m_Nodes[1];

		GFPhysSoftBodyNode * facen2 = m_SoftFace->m_Nodes[2];

		GFPhysSoftTubeNode * tNode0 = m_Tube->GetNode(m_SegIndex);
		
		GFPhysSoftTubeNode * tNode1 = m_Tube->GetNode(m_SegIndex + 1);

		if (facen0->m_InvM < GP_EPSILON || facen0->m_InvM < GP_EPSILON || facen0->m_InvM < GP_EPSILON)
		{
			return GFPhysVector3(0, 0, 0);
		}
		
		GFPhysVector3 impluse(0, 0, 0);

		for (int c = 0; c < 2; c++)
		{
			GFPhysSoftTubeNode * tubeNode = (c == 0 ? tNode0 : tNode1);

			Real faceInvMass = FACEMASSINANCHORANDCOLLIDE;//temp

			Real threadInvMass = 1.0f;//temp

			//GFPhysVector3 gradt0 = m_ThreadWeigths[0] * m_CollideNormal;
			//GFPhysVector3 gradt1 = m_ThreadWeigths[1] * m_CollideNormal;
			GFPhysVector3 gradt  = m_CollideNormal;

			GFPhysVector3 gradf0 = -m_FaceWeihts[0] * m_CollideNormal;
			GFPhysVector3 gradf1 = -m_FaceWeihts[1] * m_CollideNormal;
			GFPhysVector3 gradf2 = -m_FaceWeihts[2] * m_CollideNormal;

			Real sumGrad = /*m_ThreadWeigths[0] * m_ThreadWeigths[0] * threadInvMass
				+ m_ThreadWeigths[1] * m_ThreadWeigths[1] * */
				  threadInvMass
				+ m_FaceWeihts[0] * m_FaceWeihts[0] * faceInvMass
				+ m_FaceWeihts[1] * m_FaceWeihts[1] * faceInvMass
				+ m_FaceWeihts[2] * m_FaceWeihts[2] * faceInvMass;


			GFPhysVector3 ptThread = tubeNode->m_CurrPosition;// tNode0->m_CurrPosition * m_ThreadWeigths[0] + tNode1->m_CurrPosition * m_ThreadWeigths[1];

			GFPhysVector3 ptFace = facen0->m_CurrPosition * m_FaceWeihts[0]
			        	+ facen1->m_CurrPosition * m_FaceWeihts[1]
			        	+ facen2->m_CurrPosition * m_FaceWeihts[2];

			Real  s_normdist = (ptThread - ptFace).Dot(m_CollideNormal) - m_ColRadius;

			Real  threadNormalCorrect = 0;// [2];

			if (sumGrad > FLT_EPSILON && s_normdist < 0)
			{
				//if (cdPair.m_e3 < 19 && cdPair.m_e3 > 11)
				//{
				//int k = 1;
				//}

				Real s = -s_normdist / sumGrad;

				//tNode0->m_CurrPosition += gradt0 * s * threadInvMass;
				//tNode1->m_CurrPosition += gradt1 * s * threadInvMass;

				tubeNode->m_CurrPosition += gradt * s * threadInvMass;
				facen0->m_CurrPosition += gradf0 * s * faceInvMass;
				facen1->m_CurrPosition += gradf1 * s * faceInvMass;
				facen2->m_CurrPosition += gradf2 * s * faceInvMass;

				//threadNormalCorrect[0] = (gradt0 * s * threadInvMass).Length();
				//threadNormalCorrect[1] = (gradt1 * s * threadInvMass).Length();
				threadNormalCorrect = (gradt * s * threadInvMass).Length();
				
				impluse += gradt* s * threadInvMass;// (gradt0 * s * threadInvMass + gradt1 * s * threadInvMass);

				//tangent damping
				//for (int t = 0; t < 2; t++)
				//{
				//GFPhysSoftTubeNode * tnode = (t == 0 ? tNode0 : tNode1);
				GFPhysVector3 NodaVel = tubeNode->m_CurrPosition - tubeNode->m_LastPosition;

				GFPhysVector3 NormalVel = m_CollideNormal * NodaVel.Dot(m_CollideNormal);

				GFPhysVector3 TangVel = NodaVel - NormalVel;

				Real tanLen = TangVel.Length();

				if (tanLen > GP_EPSILON)
				{
					GFPhysVector3 tanVec = TangVel / tanLen;

					Real tanCorrect = threadNormalCorrect * 0.9f;

					tanLen = tanLen - tanCorrect;

					if (tanLen < 0)
						tanLen = 0;

					tubeNode->m_CurrPosition = tubeNode->m_LastPosition + NormalVel + tanVec*tanLen;
				}
				//}
			}
		}

		return impluse * 0.02f;
	}

	return GFPhysVector3(0,0,0);
}
//===================================================================================================================================
/*void STVSFCollidePair::SetVF_Collision(int nt, const Ogre::Vector3 & collideNormal, const Ogre::Vector3 & FaceNormal)
{
	m_CollideType = STVSFCollidePair::TFCD_VF;
	m_CollideNormal = collideNormal;
	m_FaceNormal = FaceNormal;
	m_e1 = nt;

	m_CollideNode[0] = m_SoftFace->m_Nodes[0];
	m_CollideNode[1] = m_SoftFace->m_Nodes[1];
	m_CollideNode[2] = m_SoftFace->m_Nodes[2];

	m_CollideNodeDepth[0] = m_CollideNodeDepth[1] = m_CollideNodeDepth[2] = 0;

	GFPhysSoftBodyTetrahedron * tetra = m_SoftFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;
	m_NumCollideNode = 3;
	for (int v = 0; v < 4; v++)
	{
		GFPhysSoftBodyNode * collideNode = tetra->m_TetraNodes[v];
		if ((collideNode != m_CollideNode[0]) && (collideNode != m_CollideNode[1]) && (collideNode != m_CollideNode[2]))
		{
			m_CollideNode[m_NumCollideNode] = collideNode;
			m_CollideNodeDepth[m_NumCollideNode] = (collideNode->m_CurrPosition - m_CollideNode[0]->m_CurrPosition).Dot(OgreToGPVec3(m_CollideNormal));
			m_NumCollideNode++;
		}
	}
}*/
//===================================================================================================================================
/*
void STVSFCollidePair::SetEE_Collision(int e1, int e2, int e3, int e4, const Ogre::Vector3 & collideNormal, const Ogre::Vector3 & FaceNormal)
{
	m_CollideType = STVSFCollidePair::TFCD_EE;
	m_CollideNormal = collideNormal;
	m_FaceNormal = FaceNormal;
	m_e1 = e1;
	m_e2 = e2;
	m_e3 = e3;
	m_e4 = e4;

	m_CollideNode[0] = m_SoftFace->m_Nodes[m_e1];
	m_CollideNode[1] = m_SoftFace->m_Nodes[m_e2];

	m_CollideNodeDepth[0] = 0;
	m_CollideNodeDepth[1] = 0;

	GFPhysSoftBodyTetrahedron * tetra = m_SoftFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;
	m_NumCollideNode = 2;
	for (int v = 0; v < 4; v++)
	{
		GFPhysSoftBodyNode * collideNode = tetra->m_TetraNodes[v];
		if ((collideNode != m_CollideNode[0]) && (collideNode != m_CollideNode[1]))
		{
			m_CollideNode[m_NumCollideNode] = collideNode;
			m_CollideNodeDepth[m_NumCollideNode] = (collideNode->m_CurrPosition - m_CollideNode[0]->m_CurrPosition).Dot(OgreToGPVec3(m_CollideNormal));
			m_NumCollideNode++;
		}
	}
}
*/
//===================================================================================================================================
STVRGCollidePair::STVRGCollidePair(GFPhysRigidBody * rigid, 
	                               GFPhysSoftTube  * tube, 
	                               GFPhysTransform & rbTrans, 
								   const GFPhysVector3 & rigidWorldPt,
								   const GFPhysVector3 & rigidNormal, 
								   float SegmentWeight, float depth, int segIndex)
{
	m_Rigid = rigid;
	m_Tube = tube;

	m_RigidWorldPoint = rigidWorldPt;
	m_NormalOnRigid = rigidNormal;
	m_Tangnet[0] = Perpendicular(m_NormalOnRigid).Normalized();
	m_Tangnet[1] = m_NormalOnRigid.Cross(m_Tangnet[0]).Normalized();

	m_LocalAnchorOnRigid = m_Rigid->GetCenterOfMassTransform().Inverse() * m_RigidWorldPoint;

	m_SegWeight = SegmentWeight;

	m_Depth = depth;
	m_Segment = segIndex;
	m_ThreadImpulseDeta[0] = m_ThreadImpulseDeta[1] = 0;
	m_ImpluseNormalOnRigid = 0;

	m_Mode = 0;
}
//===================================================================================================================================
void STVRGCollidePair::BuildAdditional(const GFPhysVector3 segPosInCollideTime[2], const GFPhysVector3 & PosInConvexInCollideTime, float segNodeInvMass[2], const GFPhysVector3 & NormalInCOnvexInCollideTime, float segRadius, GFPhysTransform & rbTransInCollideTime)
{
	GFPhysTransform invRigidTrans = rbTransInCollideTime.Inverse();

	for (int c = 0; c < 2; c++)
	{
		float nodeDepth = (segPosInCollideTime[c] - PosInConvexInCollideTime).Dot(NormalInCOnvexInCollideTime);

		//if (m_Depth <= 0)
		{
			if (nodeDepth < m_Depth)
				nodeDepth = m_Depth;
		}

		nodeDepth = nodeDepth - segRadius;

		GFPhysVector3 worldpos = segPosInCollideTime[c] - m_NormalOnRigid * nodeDepth;

		//trans form to rigid body frame
		m_LocalPtOnRigidWithRadius[c] = invRigidTrans(worldpos); //m_PointsOnRigidWorld[n] - m_Rigid->GetCenterOfMassPosition();

		m_NodeInvMass[c] = segNodeInvMass[c];
	}
	m_Mode = 1;
}


//===================================================================================================================================
void STVRGCollidePair::PrepareForSolveContact(float dt)
{
	for (int c = 0; c < 2; c++)
	{
		Real invNodeMass = m_NodeInvMass[c];

		m_RWithRadius[c] = m_Rigid->GetWorldTransform().GetBasis() * m_LocalPtOnRigidWithRadius[c];

		//build K_dt matrix
		m_KMatrix[c] = GFPhysSoftBodyConstraint::K_DtMatrix(dt,
			invNodeMass,
			m_Rigid->GetInvMass(),
			m_Rigid->GetInvInertiaTensorWorld(),
			m_RWithRadius[c]);
	}
}
//===================================================================================================================================
void STVRGCollidePair::SolveCollision(float dt)// , GFPhysVector3 & segNodePos0, GFPhysVector3 & segNodePos1)
{
	const GFPhysSoftTubeSegment & segMent = m_Tube->GetSegment(m_Segment);
	
	//if (m_w0 + m_w1 <= 0)
	//	return;

	for (int n = 0; n < 2; n++)
	{
		GFPhysSoftTubeNode * tubeNode = (n == 0 ? segMent.m_Node0 : segMent.m_Node1);
		
		if (tubeNode->GetInvMass() < GP_EPSILON)
			continue;

		GFPhysVector3 va = m_Rigid->GetVelocityInLocalPoint(m_RWithRadius[n]) * dt;

		GFPhysVector3 vr = va + m_Rigid->GetCenterOfMassPosition() + m_RWithRadius[n] - tubeNode->m_CurrPosition;

		float  vn = vr.Dot(m_NormalOnRigid);

		if (vn > 0)
		{
			float t = (m_KMatrix[n] * m_NormalOnRigid).Dot(m_NormalOnRigid);

			if (t > FLT_EPSILON)
			{
				Real c = vn / t;

				GFPhysVector3 impluse = m_NormalOnRigid * c;

				//apply positive impluse on soft
				tubeNode->m_CurrPosition += impluse * dt * m_NodeInvMass[n];

				//apply negative impluse on rigid
				m_Rigid->ApplyImpulse(-impluse, m_RWithRadius[n]);

				m_ImpluseNormalOnRigid -= impluse.Length();
			}
		}

		//friction
		va = m_Rigid->GetVelocityInLocalPoint(m_RWithRadius[n]) * dt;

		vr = va + m_Rigid->GetCenterOfMassPosition() + m_RWithRadius[n] - tubeNode->m_CurrPosition;

		//relative displacement between collide point in tangent direction
		GFPhysVector3 tanDeviate = vr - (m_NormalOnRigid * vr.Dot(m_NormalOnRigid));

		Real vt = tanDeviate.Length();

		float FrictStiff = 0.005f;
		
		//follow is simple equation for test must take accumulate in normal impluse in to account;
		if (vt > 0.001f)
		{
			GFPhysVector3 tanDir = tanDeviate / vt; //normalized tangent direction

			vt = vt * FrictStiff;

			tubeNode->m_CurrPosition += tanDir * vt;
		}
	}
}
//===================================================================================================================================
STVSTCollidePair::STVSTCollidePair(GFPhysSoftTube * ropeA, GFPhysSoftTube * ropeB, int SegmentA, int SegmentB)
{
	m_TubeA = ropeA;
	m_TubeB = ropeB;
	m_SegmentA = SegmentA;
	m_SegmentB = SegmentB;
	m_ColRadiusA = m_TubeA->GetCollisionRadius();
	m_ColRadiusB = m_TubeB->GetCollisionRadius();
}
//===================================================================================================================================
void STVSTCollidePair::SolveCollision()
{
	const GFPhysSoftTubeSegment & SegmentA = m_TubeA->GetSegment(m_SegmentA);

	const GFPhysSoftTubeSegment & SegmentB = m_TubeB->GetSegment(m_SegmentB);

	GFPhysSoftTubeNode * tNodeA[2];

	GFPhysSoftTubeNode * tNodeB[2];
	
	tNodeA[0] = (SegmentA.m_Node0);
	tNodeA[1] = (SegmentA.m_Node1);

	tNodeB[0] = (SegmentB.m_Node0);
	tNodeB[1] = (SegmentB.m_Node1);

	Real wA0 = 1.0f - m_WeightA;
	Real wA1 = m_WeightA;
	Real wB0 = 1.0f - m_WeightB;
	Real wB1 = m_WeightB;

	Real  invMassA0 = tNodeA[0]->GetInvMass();
	Real  invMassA1 = tNodeA[1]->GetInvMass();
	Real  invMassB0 = tNodeB[0]->GetInvMass();
	Real  invMassB1 = tNodeB[1]->GetInvMass();


	GFPhysVector3 PointA = tNodeA[0]->m_CurrPosition * wA0 + tNodeA[1]->m_CurrPosition * wA1;
	GFPhysVector3 PointB = tNodeB[0]->m_CurrPosition * wB0 + tNodeB[1]->m_CurrPosition * wB1;

	GFPhysVector3 GradA0 = wA0 * m_NormalOnB;
	GFPhysVector3 GradA1 = wA1 * m_NormalOnB;

	GFPhysVector3 GradB0 = -wB0 * m_NormalOnB;
	GFPhysVector3 GradB1 = -wB1 * m_NormalOnB;

	Real  sumGrad = invMassA0 * wA0 * wA0 + invMassA1 * wA1 * wA1
		          + invMassB0 * wB0 * wB0 + invMassB1 * wB1 * wB1;

	Real  s_normdist = (m_ColRadiusA + m_ColRadiusB) + GPSIMDVec3Dot(GPSIMDVec3Sub(PointB, PointA), m_NormalOnB);

	if (sumGrad > FLT_EPSILON && s_normdist > 0)
	{
		Real s = s_normdist / sumGrad;

		tNodeA[0]->m_CurrPosition += invMassA0 * GradA0 * s;
		tNodeA[1]->m_CurrPosition += invMassA1 * GradA1 * s;

		tNodeB[0]->m_CurrPosition += invMassB0 * GradB0 * s;
		tNodeB[1]->m_CurrPosition += invMassB1 * GradB1 * s;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//detector
//////////////////////////////////////////////////////////////////////////
#define USECCDINRIGIDCOLLISION 0
#define USECCDINSELFCOLLISION 1

class TubeSegSoftFaceClosetResult : public GFPhysDCDInterface::Result
{
public:
	TubeSegSoftFaceClosetResult()
	{
		m_Valid = false;
	}
	/*overridden*/
	void SetShapeIdentifiers(int partId0, int index0, int partId1, int index1)
	{}
	/*overridden*/
	void AddContactPoint(const GFPhysVector3& normalOnBInWorld, const GFPhysVector3& pointInWorld, Real depth)
	{
		m_Depth = depth;
		m_NormalOnB = normalOnBInWorld;
		m_PointOnB = pointInWorld;
		m_Valid = true;
	}
	bool  m_Valid;
	Real  m_Depth;
	GFPhysVector3 m_NormalOnB;
	GFPhysVector3 m_PointOnB;
};


SutureThreadV2CompoundCallBack::SutureThreadV2CompoundCallBack(
	    Real margin,GFPhysRigidBody * rb,
		GFPhysCollideShape * childshape,GFPhysTransform & rbTrans,
		Real collideradius, GFPhysSoftTube * rope,
		GFPhysAlignedVectorObj<STVRGCollidePair> & collidepair)
		: m_rb(rb),
		m_Rope(rope),
		m_collideRadius(collideradius),
		m_CollidePairs(collidepair)
{
	m_collideshape = childshape;
	m_threadMarin = margin;//0.02f;
	m_rbTransform = rbTrans;
}

void SutureThreadV2CompoundCallBack::ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
{
	 m_SegIndex = (int)UserData;

	 GFPhysSoftTubeSegment & fixSeg = m_Rope->GetSegment(m_SegIndex);
	
	 GFPhysSoftTubeNode * n0 = fixSeg.m_Node0;
	
	 GFPhysSoftTubeNode * n1 = fixSeg.m_Node1;

	// if (fixSeg.IsAttached())
	//	 return;

	if (fixSeg.GetCanCollideRigid())
	{
		m_SegPos0 = n0->m_LastPosition;
		m_SegPos1 = n1->m_LastPosition;

		m_SegNodeInvM[0] = n0->GetInvMass();
		m_SegNodeInvM[1] = n1->GetInvMass();

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

void SutureThreadV2CompoundCallBack::SetShapeIdentifiers(int partId0, int index0, int partId1, int index1)
{}

void SutureThreadV2CompoundCallBack::AddContactPoint(const GFPhysVector3& normalOnBInWorld, const GFPhysVector3& pointInWorld, Real depth)
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

			STVRGCollidePair trPair(m_rb, m_Rope, m_rbTransform, pointOnConvexWithoutMargin, -normalOnBInWorld, segweight, depthWithOutMargin, m_SegIndex);
#if(1)
			GFPhysVector3 segPos[2];

			segPos[0] = m_SegPos0;
			segPos[1] = m_SegPos1;


			trPair.BuildAdditional(segPos, pointOnConvexWithoutMargin,
				m_SegNodeInvM,
				-normalOnBInWorld,
				m_Rope->GetCollisionRadius(),
				m_rb->GetWorldTransform());
#endif

			m_CollidePairs.push_back(trPair);
		}
}

SutureThreadV2CompoundCallBack::~SutureThreadV2CompoundCallBack()
{

}
	
SutureThreadV2ConvexCallBack::SutureThreadV2ConvexCallBack(Real margin,
		GFPhysRigidBody * rb,
		GFPhysTransform & rbTrans,
		Real dt,
		Real collideradius,
		GFPhysSoftTube * rope,
		GFPhysAlignedVectorObj<STVRGCollidePair> & collidepair
		) : m_rb(rb),
		m_Rope(rope),
		m_collideRadius(collideradius),
		m_CollidePairs(collidepair)
{
		m_threadMarin = margin;//0.02f;
		m_rbTransform = rbTrans;
		m_dt = dt;
}

SutureThreadV2ConvexCallBack::~SutureThreadV2ConvexCallBack()
{}

void SutureThreadV2ConvexCallBack::ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
{
	 m_SegIndex = (int)UserData;

	 GFPhysSoftTubeSegment & fixSeg = m_Rope->GetSegment(m_SegIndex);
	
	 GFPhysSoftTubeNode * n0 = fixSeg.m_Node0;
	
	 GFPhysSoftTubeNode * n1 = fixSeg.m_Node1;

	 //if (fixSeg.IsAttached())
	 //{
		//return;
	 //}
	 if (fixSeg.GetCanCollideRigid())
	 {
		m_SegPos0 = n0->m_LastPosition;
		m_SegPos1 = n1->m_LastPosition;

		m_SegNodeInvM[0] = n0->GetInvMass();
		m_SegNodeInvM[1] = n1->GetInvMass();
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
				n0->m_LastPosition,
				n1->m_LastPosition,
				n0->m_CurrPosition,
				n1->m_CurrPosition,
				m_Rope->GetCollideMargin() + m_Rope->GetCollideRadius(),
				0.001f,
				pointInConvexWolrd,
				normalInConvexWorld,
				pointInEdgeWorld,
				collideTrans,
				depth);

			if (collideTime >= 0.0f && collideTime <= 1.0f)
			{
				Real depthWithOutMargin = m_convexMargin + (pointInEdgeWorld - pointInConvexWolrd).Dot(normalInConvexWorld);

				GFPhysVector3 SegPos0 = n0->m_LastPosition * (1.0f - collideTime) + n0->m_CurrPosition * collideTime;
				GFPhysVector3 SegPos1 = n1->m_LastPosition * (1.0f - collideTime) + n1->m_CurrPosition * collideTime;

				Real t0 = (pointInEdgeWorld - SegPos0).Dot(SegPos1 - SegPos0);

				Real t1 = (SegPos1 - SegPos0).Dot(SegPos1 - SegPos0);

				if (t1 > GP_EPSILON)
				{
					Real segweight = t0 / t1;

					GPClamp(segweight, 0.0f, 1.0f);

					GFPhysVector3  pointInConvexWolrdNoMargin = pointInConvexWolrd - normalInConvexWorld * m_convexMargin;

					pointInConvexWolrdNoMargin = m_rbTransform * (collideTrans.Inverse() * pointInConvexWolrdNoMargin);//redundant optimize me ! since in tr pair we also need transform to local point!

					STVRGCollidePair trpair(m_rb, m_rbTransform, pointInConvexWolrdNoMargin, normalInConvexWorld, segweight, depthWithOutMargin, m_SegIndex);
#if(1)
					GFPhysVector3 segPos[2];
					Real segNodeInvMass[2];

					segPos[0] = SegPos0;
					segPos[1] = SegPos1;

					segNodeInvMass[0] = n0->GetInvMass();
					segNodeInvMass[1] = n1->GetInvMass();

					trpair.BuildAdditional(segPos,
						pointInConvexWolrd - normalInConvexWorld * m_convexMargin,
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

void SutureThreadV2ConvexCallBack::ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode)
{

}

void SutureThreadV2ConvexCallBack::ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB)
{

}


void SutureThreadV2ConvexCallBack::SetShapeIdentifiers(int partId0, int index0, int partId1, int index1)
{

}

void SutureThreadV2ConvexCallBack::AddContactPoint(const GFPhysVector3& normalOnBInWorld, const GFPhysVector3& pointInWorld, Real depth)
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

			STVRGCollidePair trPair(m_rb, m_Rope ,m_rbTransform, pointOnConvexWithoutMargin, -normalOnBInWorld, segweight, depthWithOutMargin, m_SegIndex);

#if(1)
			GFPhysVector3 segPos[2];
			segPos[0] = m_SegPos0;
			segPos[1] = m_SegPos1;


			trPair.BuildAdditional(segPos, pointOnConvexWithoutMargin,
				m_SegNodeInvM,
				-normalOnBInWorld,
				m_Rope->GetCollisionRadius(),
				m_rbTransform);
#endif

			m_CollidePairs.push_back(trPair);
		}
}
//================================================================================================================
SutureThreadV2ThreadCallBack::SutureThreadV2ThreadCallBack(Real margin , Real collideradius,
		                                                   GFPhysSoftTube * ropeA,GFPhysSoftTube * ropeB,
		                                                  GFPhysAlignedVectorObj<STVSTCollidePair> & collidepair) 
														  : m_RopeA(ropeA), m_RopeB(ropeB), m_threadRadius(collideradius), m_CollidePairs(collidepair)
{
	m_threadMarin = margin;//0.02f;
	m_IsSelftCollision = (m_RopeA == m_RopeB ? true : false);
}

SutureThreadV2ThreadCallBack::~SutureThreadV2ThreadCallBack()
{}

void SutureThreadV2ThreadCallBack::ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
{}

void SutureThreadV2ThreadCallBack::ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode)
{}

void SutureThreadV2ThreadCallBack::ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB)
{

		int SegIndexA = (int)(dynNodeA->m_UserData);
		int SegIndexB = (int)(dynNodeB->m_UserData);

		if (SegIndexA < SegIndexB)
		{
			if (m_IsSelftCollision && abs(SegIndexA - SegIndexB) <= 2)
				return;

			const GFPhysSoftTubeNode * A0 = m_RopeA->GetNode(SegIndexA);
			const GFPhysSoftTubeNode * A1 = m_RopeA->GetNode(SegIndexA + 1);

			const GFPhysSoftTubeNode * B0 = m_RopeB->GetNode(SegIndexB);
			const GFPhysSoftTubeNode * B1 = m_RopeB->GetNode(SegIndexB + 1);

			if (m_RopeA->GetSegment(SegIndexA).GetCanCollideSelf() 
			 && m_RopeB->GetSegment(SegIndexB).GetCanCollideSelf())
			{
#if(USECCDINSELFCOLLISION)

				GFPhysVector3 cpoint0, cpoint1, collideNorm;//on second edge
				bool exceedMaxItor;

				Real sumRadius = (m_threadRadius * 0.25f + m_threadRadius)*2.0f;
				GFPhysVector3 VA0 = A0->m_CurrPosition - A0->m_LastPosition;
				GFPhysVector3 VA1 = A1->m_CurrPosition - A1->m_LastPosition;
				GFPhysVector3 VB0 = B0->m_CurrPosition - B0->m_LastPosition;
				GFPhysVector3 VB1 = B1->m_CurrPosition - B1->m_LastPosition;

				Real collideTime = GFPhysCABasedCCD::GetEdgesWithRadiusCollideTime(A0->m_LastPosition, A1->m_LastPosition,
					B0->m_LastPosition, B1->m_LastPosition,
					VA0, VA1,
					VB0, VB1,
					1.0f,
					sumRadius,sumRadius * 0.025f,

					cpoint0, cpoint1, collideNorm, exceedMaxItor);

				if (collideTime >= 0 && (!exceedMaxItor))//collide
				{
					GFPhysVector3 CA0 = A0->m_LastPosition + VA0*collideTime;
					GFPhysVector3 CA1 = A1->m_LastPosition + VA1*collideTime;
					GFPhysVector3 CB0 = B0->m_LastPosition + VB0*collideTime;
					GFPhysVector3 CB1 = B1->m_LastPosition + VB1*collideTime;

					STVSTCollidePair ttpair(m_RopeA, m_RopeB, SegIndexA, SegIndexB);
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
					STVSTCollidePair ttpair(m_RopeA, m_RopeB, SegIndexA, SegIndexB);
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

void SutureThreadV2ThreadCallBack::SetShapeIdentifiers(int partId0, int index0, int partId1, int index1)
{

}

SutureThreadV2SegSoftFaceCallback::SutureThreadV2SegSoftFaceCallback(Real Margin,
		GFPhysSoftBody * sb,
		Real collideradius,
		GFPhysSoftTube * rope,
		GFPhysAlignedVectorObj<STVSFCollidePair> & paircd,
		bool useCCD) : m_sb(sb),
		m_collideRadius(collideradius),
		m_Margin(Margin),
		m_Rope(rope),
		m_CollidePairs(paircd)

{
	m_UseCCD = useCCD;
}
	
SutureThreadV2SegSoftFaceCallback::~SutureThreadV2SegSoftFaceCallback()
{}

void SutureThreadV2SegSoftFaceCallback::ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
{}

void SutureThreadV2SegSoftFaceCallback::ProcessOverlappedNodes(GFPhysDBVNode * dynNodes, GFPhysAABBNode * staticnode)
{}

void SutureThreadV2SegSoftFaceCallback::ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA, GFPhysDBVNode * dynNodeB)
{
		GFPhysSoftBodyFace * softFace = (GFPhysSoftBodyFace*)dynNodeA->m_UserData;
		int SegIndex = (int)dynNodeB->m_UserData;

		GFPhysVector3 SoftFaceVerts[3];
		GFPhysVector3 RopeMovePath[3];

		SoftFaceVerts[0] = softFace->m_Nodes[0]->m_CurrPosition;
		SoftFaceVerts[1] = softFace->m_Nodes[1]->m_CurrPosition;
		SoftFaceVerts[2] = softFace->m_Nodes[2]->m_CurrPosition;

		GFPhysSoftTubeNode * n0 = (m_Rope->GetSegment(SegIndex).m_Node0);
		GFPhysSoftTubeNode * n1 = (m_Rope->GetSegment(SegIndex).m_Node1);

		//if (m_Rope->GetTubeWireSegment(SegIndex).GetCanCollideSoft() == false)
		//{
		//	return;
		//}


		if (m_UseCCD)
		{
			GFPhysVector3 edgeVerts[2];

			GFPhysVector3 triVelocity[3];

			GFPhysVector3 edgeVelocity[2];

			edgeVerts[0] = n0->m_LastPosition;

			edgeVerts[1] = n1->m_LastPosition;

			triVelocity[0] = softFace->m_Nodes[0]->m_Velocity;

			triVelocity[1] = softFace->m_Nodes[1]->m_Velocity;

			triVelocity[2] = softFace->m_Nodes[2]->m_Velocity;

			edgeVelocity[0] = n0->m_CurrPosition - n0->m_LastPosition;

			edgeVelocity[1] = n1->m_CurrPosition - n1->m_LastPosition;

			Real sumRadius = m_collideRadius + m_Margin;//0.25 is margin


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

				GFPhysVector3 e0 = n0->m_LastPosition * (1 - collideTime) + n0->m_CurrPosition * collideTime;
				GFPhysVector3 e1 = n1->m_LastPosition * (1 - collideTime) + n1->m_CurrPosition * collideTime;

				GFPhysVector3 t0 = SoftFaceVerts[0] + triVelocity[0] * collideTime;
				GFPhysVector3 t1 = SoftFaceVerts[1] + triVelocity[1] * collideTime;
				GFPhysVector3 t2 = SoftFaceVerts[2] + triVelocity[2] * collideTime;

				GoPhys::CalcBaryCentric(t0, t1, t2, cp_Tri, FaceWeights[0], FaceWeights[1], FaceWeights[2]);

				Real lenSqr = (e1 - e0).Dot(e1 - e0);

				if (lenSqr > 0.0001f)
					EdgeWeight = (cp_Edge - e0).Dot(e1 - e0) / lenSqr;
				else
					EdgeWeight = 0.0f;

				GoPhys::GPClamp(EdgeWeight, 0.0f, 1.0f);
				GoPhys::GPClamp(FaceWeights[0], 0.0f, 1.0f);
				GoPhys::GPClamp(FaceWeights[1], 0.0f, 1.0f);
				GoPhys::GPClamp(FaceWeights[2], 0.0f, 1.0f);

				GFPhysVector3 faceNormal = (t1 - t0).Cross(t2 - t0);
				faceNormal.Normalize();

				STVSFCollidePair collidePair(m_sb, m_Rope ,softFace);
				collidePair.m_FaceWeihts[0] = FaceWeights[0];
				collidePair.m_FaceWeihts[1] = FaceWeights[1];
				collidePair.m_FaceWeihts[2] = FaceWeights[2];
				collidePair.m_ThreadWeight = EdgeWeight;
				//collidePair.m_ThreadWeigths[0] = 1 - EdgeWeight;
				//collidePair.m_ThreadWeigths[1] = EdgeWeight;

				int collideNormalSign = (collideNorm.Dot(faceNormal) > 0 ? 1 : -1);

				collidePair.SetEF_Collision(SegIndex, (collideNorm), (faceNormal));
				collidePair.m_IsPositiveCol = (collideNormalSign == 1 ? true : false);
				if (collideNormalSign == 1)
					m_CollidePairs.push_back(collidePair);//for edge - edge collision eliminate normal opposite to face
				//else
				//{
					//	m_InvCollidePairs.push_back(collidePair);
					//int k = 0;
					//k = -4;
				//}


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



			GFPhysVector3 l0 = n0->m_CurrPosition;
			GFPhysVector3 l1 = n1->m_CurrPosition;
			float usedMargin = m_Margin;

			float usedRadius = m_collideRadius;
#if(0)
			if (SegIndex >= m_Rope->GetRopeAnchorIndexMin() - 1 && SegIndex <= m_Rope->GetRopeAnchorIndexMax() + 1)
			{
				GFPhysVector3 dir = (l1 - l0);

				float length = dir.Length();

				if (length > usedRadius * 2.0f + 0.0001f)
				{
					dir /= length;
					l0 += dir * usedRadius;
					l1 -= dir * usedRadius;
					usedMargin = 0.01f;
				}
			}
#endif

			GFPhysLineSegmentShape segShape(l0, l1);
			segShape.SetMargin(usedRadius + usedMargin);

			GFPhysGJKCollideDetector gjkPairDetector(&segShape, &triShape, &VSimplexSolver, &PDSolver);

			gjkPairDetector.SetMinkowskiA(&segShape);

			gjkPairDetector.SetMinkowskiB(&triShape);

			input.m_maximumDistanceSquared = segShape.GetMargin() + triShape.GetMargin() + 0.04f;

			input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;

			input.m_transformA.SetIdentity();

			input.m_transformB.SetIdentity();

			TubeSegSoftFaceClosetResult cdResult;

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
				if (sum > FLT_EPSILON)
				{
					faceWeights[0] /= sum;
					faceWeights[1] /= sum;
					faceWeights[2] /= sum;
				}

				//extract thread collision point weight
				Real ThreadWeight;// s[2];
				GFPhysVector3 dn = n1->m_CurrPosition - n0->m_CurrPosition;
				Real dnLen2 = dn.Dot(dn);
				if (dnLen2 > FLT_EPSILON)
				{
					ThreadWeight = (pointOnSegNoMargin - n0->m_CurrPosition).Dot(dn) / dnLen2;
				}
				else
				{
					ThreadWeight = 0;
				}
				GPClamp(ThreadWeight, 0.0f, 1.0f);

				//ThreadWeights[0] = 1 - ThreadWeights[1];

				//
				GFPhysVector3 faceNormal = (SoftFaceVerts[1] - SoftFaceVerts[0]).Cross(SoftFaceVerts[2] - SoftFaceVerts[0]);
				faceNormal.Normalize();

				STVSFCollidePair collidePair(m_sb, m_Rope , softFace);
				collidePair.m_FaceWeihts[0] = faceWeights[0];
				collidePair.m_FaceWeihts[1] = faceWeights[1];
				collidePair.m_FaceWeihts[2] = faceWeights[2];
				collidePair.m_ThreadWeight = ThreadWeight;
				//collidePair.m_ThreadWeigths[0] = ThreadWeights[0];
				//collidePair.m_ThreadWeigths[1] = ThreadWeights[1];

				int collideNormalSign = (cdResult.m_NormalOnB.Dot(faceNormal) > 0 ? 1 : -1);

				collidePair.SetEF_Collision(SegIndex, cdResult.m_NormalOnB, faceNormal);
				collidePair.m_IsPositiveCol = (collideNormalSign == 1 ? true : false);
				if (collideNormalSign == 1)
				    m_CollidePairs.push_back(collidePair);//for edge - edge collision eliminate normal opposite to face
				//else
				//m_InvCollidePairs.push_back(collidePair);
			}
			return;
		}
}