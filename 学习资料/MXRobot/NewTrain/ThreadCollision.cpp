#include "ThreadCollision.h"
#include "MXOgreGraphic.h"

TRCollidePair::TRCollidePair(GFPhysRigidBody * rigid,
	                         GFPhysTransform & rbTrans,
	                         const GFPhysVector3 & rigidWorldPt,
	                         const GFPhysVector3 & rigidNormal,
	                         float SegmentWeight,
	                         float depth,
	                         int segIndex)
{
	m_Rigid = rigid;

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

void TRCollidePair::BuildAdditional(const GFPhysVector3 segPosInCollideTime[2],
	const GFPhysVector3 & PosInConvexInCollideTime,
	float segNodeInvMass[2],
	const GFPhysVector3 & NormalInCOnvexInCollideTime,
	float segRadius,
	GFPhysTransform & rbTransInCollideTime)
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

void TRCollidePair::PrepareForSolveContact(float dt)
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

void TRCollidePair::SolveContact(float dt, GFPhysVector3 & segNodePos0, GFPhysVector3 & segNodePos1)
{
	for (int n = 0; n < 2; n++)
	{
		GFPhysVector3 & NodeCurrPos = (n == 0 ? segNodePos0 : segNodePos1);

		GFPhysVector3 va = m_Rigid->GetVelocityInLocalPoint(m_RWithRadius[n]) * dt;

		GFPhysVector3 vr = va + m_Rigid->GetCenterOfMassPosition() + m_RWithRadius[n] - NodeCurrPos;

		float  vn = vr.Dot(m_NormalOnRigid);

		if (vn > 0)
		{
			float t = (m_KMatrix[n] * m_NormalOnRigid).Dot(m_NormalOnRigid);

			if (t > FLT_EPSILON)
			{
				Real c = vn / t;

				GFPhysVector3 impluse = m_NormalOnRigid * c;

				//apply positive impluse on soft
				NodeCurrPos += impluse * dt * m_NodeInvMass[n];

				//apply negative impluse on rigid
				m_Rigid->ApplyImpulse(-impluse, m_RWithRadius[n]);

				m_ImpluseNormalOnRigid -= impluse.Length();
			}
		}

		//friction
		for (int n = 0; n < 2; n++)
		{
			GFPhysVector3 & NodeCurrPos = (n == 0 ? segNodePos0 : segNodePos1);

			GFPhysVector3 va = m_Rigid->GetVelocityInLocalPoint(m_RWithRadius[n]) * dt;

			GFPhysVector3 vr = va + m_Rigid->GetCenterOfMassPosition() + m_RWithRadius[n] - NodeCurrPos;

			//relative displacement between collide point in tangent direction
			GFPhysVector3 tanDeviate = vr - (m_NormalOnRigid * vr.Dot(m_NormalOnRigid));

			Real vt = tanDeviate.Length();

			float FrictStiff = 0.02f;
			//follow is simple equation for test must take accumulate in normal impluse in to account;
			if (vt > 0.001f)
			{
				GFPhysVector3 tanDir = tanDeviate / vt; //normalized tangent direction

				vt = vt * FrictStiff;

				NodeCurrPos += tanDir * vt;
			}
		}
	}
}


//================================================================================================================
TFCollidePair::TFCollidePair(GFPhysSoftBody * sb)
{
	m_CollideBody = sb;
	m_ImpluseOnThread = Ogre::Vector3(0, 0, 0);
}
//================================================================================================================
TFCollidePair::TFCollidePair(GFPhysSoftBody * sb, GFPhysSoftBodyFace * face)
{
	m_CollideBody = sb;
	m_SoftFace = face;
	m_CollideNormal = GPVec3ToOgre((m_SoftFace->m_Nodes[1]->m_CurrPosition - m_SoftFace->m_Nodes[0]->m_CurrPosition).Cross(m_SoftFace->m_Nodes[2]->m_CurrPosition - m_SoftFace->m_Nodes[0]->m_CurrPosition));
	m_CollideNormal.normalise();
	m_ImpluseOnThread = Ogre::Vector3(0, 0, 0);

}
//================================================================================================================
int TFCollidePair::GetCollideSegmentIndex() const
{
	if (m_CollideType == TFCollidePair::TFCD_EE)
		return m_e3;
	else if (m_CollideType == TFCollidePair::TFCD_EE)
		return m_e1;
	else if (m_CollideType == TFCollidePair::TFCD_EF)
		return m_e3;
	else
		return -1;
}
//================================================================================================================
GFPhysSoftBody * TFCollidePair::GetCollideBody()  const
{
	return m_CollideBody;
}
//================================================================================================================
GFPhysSoftBodyFace * TFCollidePair::GetCollideSoftFace()  const
{
	return m_SoftFace;
}
//================================================================================================================
void TFCollidePair::SetEE_Collision(int e1, int e2, int e3, int e4, const Ogre::Vector3 & collideNormal, const Ogre::Vector3 & FaceNormal)
{
	m_CollideType = TFCollidePair::TFCD_EE;
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
//================================================================================================================
void TFCollidePair::SetVF_Collision(int nt, const Ogre::Vector3 & collideNormal, const Ogre::Vector3 & FaceNormal)
{
	m_CollideType = TFCollidePair::TFCD_VF;
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
}
void TFCollidePair::SetEF_Collision(int segIndex, const Ogre::Vector3 & collideNormal, const Ogre::Vector3 & FaceNormal)
{
	m_CollideType = TFCollidePair::TFCD_EF;
	m_CollideNormal = collideNormal;
	m_FaceNormal = FaceNormal;
	m_e3 = segIndex;
	m_e4 = segIndex + 1;

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
}
