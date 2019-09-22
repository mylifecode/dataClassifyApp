#include "SutureNeedle.h"
#include "MisNewTraining.h"
#include "MXOgreGraphic.h"
#include "collision/CollisionShapes/GoPhysCapsuleShape.h"
#include "dynamic/Constraint/GoPhysSoftBodyConstraint.h"
#include "MisMedicThreadRope.h"
#include "TrainingMgr.h"
#include "MisMedicSutureKnot.h"

#define NEEDLE_FROM_FILE
#define SOLVENEEDLEANCHOROLDMETHOD 0

#define ROPEANCHORMOVINGSPEED 1.0f

#define ROPEANCHORSWITCHSPEED 20.0f

const int SNeedleNodeNum = 15;
//const int SThreadNodeNum = 25 * 1;
//const Real ThreadRSLEN = 6.0f * 1.0f;

const Real NeedleMassPerSegment = 0.25f;
Real _coherentYaw   = 0;
Real _coherentPicth = 0;
Real _coherentRoll  = 0;

Real _minDis = 0.1f;
//============================================================================================================================
GFPhysQuaternion CalcFaceRotate(const GFPhysVector3 srcPos[3] , const GFPhysVector3 dstPos[3])
{
	GFPhysVector3 srcCom(0,0,0);
	GFPhysVector3 dstCom(0,0,0);

	GFPhysVector3 normalSrc = (srcPos[1]-srcPos[0]).Cross(srcPos[2]-srcPos[0]).Normalized();
	GFPhysVector3 normalDst = (dstPos[1]-dstPos[0]).Cross(dstPos[2]-dstPos[0]).Normalized();

	GFPhysVector3 extendSrcPos[3];
	GFPhysVector3 extendDstPos[3];

	for(int c = 0 ; c < 3 ; c++)
	{
		extendSrcPos[c] = srcPos[c] + normalSrc * 0.1f;
		extendDstPos[c] = dstPos[c] + normalDst * 0.1f;

		srcCom += srcPos[c];
		dstCom += dstPos[c];

		srcCom += extendSrcPos[c];
		dstCom += extendDstPos[c];
	}
	srcCom /= 6.0f;
	dstCom /= 6.0f;


	GFPhysMatrix3 A_pq;
	A_pq.SetZero();

	for(int c = 0 ; c < 3 ; c++)
	{
		GFPhysVector3 q = srcPos[c] - srcCom;
		GFPhysVector3 p = dstPos[c] - dstCom;

		A_pq[0][0] += p[0] * q[0]; A_pq[0][1] += p[0] * q[1]; A_pq[0][2] += p[0] * q[2];
		A_pq[1][0] += p[1] * q[0]; A_pq[1][1] += p[1] * q[1]; A_pq[1][2] += p[1] * q[2];
		A_pq[2][0] += p[2] * q[0]; A_pq[2][1] += p[2] * q[1]; A_pq[2][2] += p[2] * q[2];
	}

	for(int c = 0 ; c < 3 ; c++)
	{
		GFPhysVector3 q = extendSrcPos[c] - srcCom;
		GFPhysVector3 p = extendDstPos[c] - dstCom;

		A_pq[0][0] += p[0] * q[0]; A_pq[0][1] += p[0] * q[1]; A_pq[0][2] += p[0] * q[2];
		A_pq[1][0] += p[1] * q[0]; A_pq[1][1] += p[1] * q[1]; A_pq[1][2] += p[1] * q[2];
		A_pq[2][0] += p[2] * q[0]; A_pq[2][1] += p[2] * q[1]; A_pq[2][2] += p[2] * q[2];
	}

	GFPhysMatrix3 R;
	R.SetIdentity();
	polarDecomposition2(A_pq , 1e-6f, R);

	GFPhysQuaternion rotQuat;
	R.GetRotation(rotQuat);

	return rotQuat;
}
//===================================================================================================================
GFPhysQuaternion CalcFaceFrameOrient(GFPhysSoftBodyFace * face)
{
	GFPhysVector3 srcPos[3];

	GFPhysVector3 dstPos[3];
	
	for(int n = 0 ; n < 3 ; n++)
	{
	    srcPos[n] = face->m_Nodes[n]->m_UnDeformedPos;
	    dstPos[n] = face->m_Nodes[n]->m_CurrPosition;
	}
	GFPhysQuaternion faceOrn = CalcFaceRotate(srcPos , dstPos);
	return faceOrn;
}
//===================================================================================================================
GFPhysFaceNeedleAnchor::GFPhysFaceNeedleAnchor(GFPhysVector3* localpoint,GFPhysSoftBodyFace * face,Real* weight,SutureNeedle * sutureNeedle,AnchorType type)
{
    m_SutNeedle = sutureNeedle;
    m_Rigid = sutureNeedle->GetPhysicBody();
    m_Face  = face;

    m_Type = type;

	m_weights[0] = weight[0];
	m_weights[1] = weight[1];
	m_weights[2] = weight[2];


	GFPhysVector3 needledir = m_SutNeedle->GetNeedleDirction();
	GFPhysVector3 faceNormal = (m_Face->m_Nodes[1]->m_CurrPosition - m_Face->m_Nodes[0]->m_CurrPosition).Cross(m_Face->m_Nodes[2]->m_CurrPosition - m_Face->m_Nodes[0]->m_CurrPosition);

	faceNormal.Normalize();

	m_initAngle = CalcAngleBetweenLineAndFace(needledir, faceNormal);

    m_LocalPoint = *(localpoint);

    m_Friction = 0.0f;

    m_LastWorldPoint = sutureNeedle->m_NeedleNodeWorldPos[SNeedleNodeNum-1];

    AnchorPos lastworld = { SNeedleNodeNum-2 , 1.0f };
    m_lastworld = lastworld;

	m_RelRot = m_Rigid->GetCenterOfMassTransform().GetRotation().Inverse() * CalcFaceFrameOrient(m_Face);

    m_BHeadOut = false;
    m_BTailOut = false;
}
//================================================================================================================
GFPhysFaceNeedleAnchor::~GFPhysFaceNeedleAnchor()
{
    //if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
    //{
    //    PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
    //}
}
//================================================================================================================
void GFPhysFaceNeedleAnchor::SetFriction(float friction)
{
    m_Friction = friction;
}
//================================================================================================================
void GFPhysFaceNeedleAnchor::PrepareSolveConstraint(Real Stiffness, Real TimeStep)
{
	m_ForceFeedBack = GFPhysVector3(0,0,0);
	m_ForceFeedBackAlongTan = GFPhysVector3(0, 0, 0);
	GFPhysVector3 nPlaneNorm = m_SutNeedle->GetNeedleNormalDirction();

	/*m_TangentDir*/GFPhysVector3 tanNeedle = m_SutNeedle->GetNeedleSegmentTangent(m_lastworld.segment, m_lastworld.lambda);
#if(0)
	m_TangentDir = (m_Face->m_Nodes[1]->m_CurrPosition - m_Face->m_Nodes[0]->m_CurrPosition).Cross(m_Face->m_Nodes[2]->m_CurrPosition - m_Face->m_Nodes[0]->m_CurrPosition).Normalized();
	m_TangentDir = (m_TangentDir - nPlaneNorm * m_TangentDir.Dot(nPlaneNorm));
	float tanLen = m_TangentDir.Length();
	if (tanLen > 0.00001f)
	{
		m_TangentDir /= tanLen;
	}
	else
	{
		m_TangentDir = tanNeedle;
	}
	if (m_TangentDir.Dot(tanNeedle) < 0)
	{
		m_TangentDir *= -1.0f;
	}
#else
	m_TangentDir = tanNeedle;
#endif
	m_NormalDir[0] = Perpendicular(m_TangentDir).Normalized();
    m_NormalDir[1] = m_NormalDir[0].Cross(m_TangentDir).Normalized();
	
	m_LastRation = m_SutNeedle->AnchorPos2Ratio(m_lastworld);

	m_LastWorldPoint = (1 - m_lastworld.lambda) * m_SutNeedle->m_NeedleNodeWorldPos[m_lastworld.segment] 
	                      + m_lastworld.lambda  * m_SutNeedle->m_NeedleNodeWorldPos[m_lastworld.segment+1];
	m_R = m_LastWorldPoint - m_Rigid->GetWorldTransform().GetOrigin();
   
	m_RLocal = QuatRotate(m_Rigid->GetWorldTransform().GetRotation().Inverse() , m_R);

	Real FaceInvM = m_weights[0]*m_weights[0]*m_Face->m_Nodes[0]->m_InvM
		          + m_weights[1]*m_weights[1]*m_Face->m_Nodes[1]->m_InvM
		          + m_weights[2]*m_weights[2]*m_Face->m_Nodes[2]->m_InvM;// 3.0f / (m_Face->m_Nodes[0]->m_Mass + m_Face->m_Nodes[1]->m_Mass + m_Face->m_Nodes[2]->m_Mass);

	//build impluse matrix etc for solve constraints
	m_ImpMat = GFPhysSoftBodyConstraint::K_DtMatrix(TimeStep,
		                                            FaceInvM,
		                                            m_Rigid->GetInvMass(),
		                                            m_Rigid->GetInvInertiaTensorWorld(),
		                                            m_R);
	//m_DtInvMa = TimeStep * FaceInvM;
   
	//friction
	m_AccumFriction = 0;
	m_AccmAngularFrict = 0;
	m_FaceCurrOrient = CalcFaceFrameOrient(m_Face);

	m_IterorCount = 0;

	//temp we Just Active Rigid Body
	m_Rigid -> Activate();
	m_RigidImpulse = GFPhysVector3(0.0f,0.0f,0.0f);
}
//------------------------------------------------------------------------------------------------
void GFPhysFaceNeedleAnchor::SolveConstraint(Real Stiffniss,Real TimeStep)
{
    if (m_Face->m_Nodes[0]->m_InvM < GP_EPSILON || m_Face->m_Nodes[1]->m_InvM < GP_EPSILON || m_Face->m_Nodes[2]->m_InvM < GP_EPSILON)
    {
        return;
    }
	if (m_LastRation <= 0.0f)
	{
		return;
	}
	m_IterorCount++;
	
	const GFPhysTransform & rigidTrans = m_Rigid->GetCenterOfMassTransform();
	
	m_R = QuatRotate(rigidTrans.GetRotation(), m_RLocal);

	GFPhysVector3 anchorRigid = m_R + rigidTrans.GetOrigin();

	/*float sumwsqr = m_weights[0] * m_weights[0] * m_Face->m_Nodes[0]->m_InvM
		          + m_weights[1] * m_weights[1] * m_Face->m_Nodes[1]->m_InvM
		          + m_weights[2] * m_weights[2] * m_Face->m_Nodes[2]->m_InvM;

	float w0 = m_weights[0] * m_Face->m_Nodes[0]->m_InvM / sumwsqr;
	float w1 = m_weights[1] * m_Face->m_Nodes[1]->m_InvM / sumwsqr;
	float w2 = m_weights[2] * m_Face->m_Nodes[2]->m_InvM / sumwsqr;*/

	//one slide freedom , so two constraint direction
	for(int c = 0 ; c < 2 ; c++)
	{
		GFPhysVector3 worldPtOnFace = m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0]+
			                          m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1]+
			                          m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2];

		GFPhysVector3 CS_Dir = m_NormalDir[c];
		
		GFPhysVector3 va = m_Rigid->GetVelocityInLocalPoint(m_R)*TimeStep;

		const GFPhysVector3 vr = (va + anchorRigid - worldPtOnFace);

		Real vn = vr.Dot(CS_Dir);

		//
		Real t = (m_ImpMat * CS_Dir).Dot(CS_Dir);

		if(t > GP_EPSILON)
		{
			Real c = vn / t;

			GFPhysVector3 impluse = CS_Dir * c;

			//apply positive impluse on soft
			//GFPhysVector3 NodeCorr = impluse * m_DtInvMa;
			m_Face->m_Nodes[0]->m_CurrPosition += impluse * m_weights[0] * m_Face->m_Nodes[0]->m_InvM * TimeStep;// NodeCorr*w0;
			m_Face->m_Nodes[1]->m_CurrPosition += impluse * m_weights[1] * m_Face->m_Nodes[1]->m_InvM * TimeStep;// NodeCorr*w1;
			m_Face->m_Nodes[2]->m_CurrPosition += impluse * m_weights[2] * m_Face->m_Nodes[2]->m_InvM * TimeStep;// NodeCorr*w2;

			//apply negative impluse on rigid
			m_Rigid->ApplyImpulse(-impluse , m_R);

			//record feedback
			m_ForceFeedBack += (-impluse * TimeStep);
		}
	}
#if SOLVENEEDLEANCHOROLDMETHOD
	//slip friction
	float maxFriction = m_Friction;// .88f;

	GFPhysVector3 worldPtOnFace = m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0] +
		                          m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1] +
		                          m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2];

	GFPhysVector3 va = m_Rigid->GetVelocityInLocalPoint(m_R);

	const GFPhysVector3 dr = ((va * TimeStep + anchorRigid) - worldPtOnFace);
	
	float vt = dr.Dot(m_TangentDir);
    
	Real  t = (m_ImpMat * m_TangentDir).Dot(m_TangentDir);

	if(t > GP_EPSILON)
	{
	   Real  c = vt / t;

	   float lastfriciton = m_AccumFriction;

	   m_AccumFriction = GPClamped(m_AccumFriction + c , -maxFriction , maxFriction);

	   GFPhysVector3 FrictionImpluse = m_TangentDir * (m_AccumFriction - lastfriciton);

	   //apply positive impluse on soft
	   //GFPhysVector3 NodeCorr = FrictionImpluse * m_DtInvMa;
	   m_Face->m_Nodes[0]->m_CurrPosition += FrictionImpluse * m_weights[0] * m_Face->m_Nodes[0]->m_InvM * TimeStep;//NodeCorr*w0;
	   m_Face->m_Nodes[1]->m_CurrPosition += FrictionImpluse * m_weights[1] * m_Face->m_Nodes[1]->m_InvM * TimeStep;//NodeCorr*w1;
	   m_Face->m_Nodes[2]->m_CurrPosition += FrictionImpluse * m_weights[2] * m_Face->m_Nodes[2]->m_InvM * TimeStep;//NodeCorr*w2;

	   //apply negative impluse on rigid
	   m_Rigid->ApplyImpulse(-FrictionImpluse , m_R);
	}

	//rotate friction
	/*
	float maxAngularFrict = 10.0f;

	GFPhysQuaternion rigidWorldRot = m_Rigid->GetCenterOfMassTransform().GetRotation();

	GFPhysQuaternion qCurRel = rigidWorldRot.Inverse() * m_FaceCurrOrient;

	GFPhysVector3 errorAxis;

	float         errorAngle;

	GFPhysTransformUtil::CalculateDiffAxisAngleQuaternion(m_RelRot , qCurRel , errorAxis , errorAngle);

	GFPhysVector3 errorRotVec = m_Rigid->GetCenterOfMassTransform().GetBasis() * (errorAxis * errorAngle);// --> to world space

	GFPhysVector3 currAngularVel = m_Rigid->GetAngularVelocity();

	float angularFrictDelta = (0.0f * errorRotVec.Dot(m_TangentDir) / TimeStep - currAngularVel.Dot(m_TangentDir));

	float lastAccumAngularFrict = m_AccmAngularFrict;

	m_AccmAngularFrict = GPClamped(m_AccmAngularFrict + angularFrictDelta , -maxAngularFrict , maxAngularFrict);

	GFPhysVector3 AngularFrict = m_TangentDir * (m_AccmAngularFrict - lastAccumAngularFrict);

	m_Rigid->SetAngularVelocity(currAngularVel + AngularFrict);*/
#endif
	//project tangent movement in to needle body
	if(m_IterorCount == GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverItorNum)
	{
		GFPhysVector3 worldPtOnFace = m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0] +
			m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1] +
			m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2];
#if SOLVENEEDLEANCHOROLDMETHOD
		GFPhysVector3 va = m_Rigid->GetVelocityInLocalPoint(m_R)*TimeStep;

		const GFPhysVector3 vr = (worldPtOnFace - (va + anchorRigid));
		Real tanMovement = vr.Dot(m_TangentDir);

		Real slidethreshold = 0.05f;
		if (fabsf(tanMovement) < slidethreshold)
		{
			tanMovement = 0;
		}
#else

		GFPhysVector3 CS_Dir = m_TangentDir;

		GFPhysVector3 va = m_Rigid->GetVelocityInLocalPoint(m_R)*TimeStep;

		GFPhysVector3 pr = (va + anchorRigid - worldPtOnFace);

		Real vn = pr.Dot(CS_Dir);

		//
		Real t = (m_ImpMat * CS_Dir).Dot(CS_Dir);

		if (t > GP_EPSILON)
		{
			Real c = vn / t;

			GFPhysVector3 impluse = CS_Dir * c;

			//apply positive impluse on soft
			//GFPhysVector3 NodeCorr = impluse * m_DtInvMa;
			m_Face->m_Nodes[0]->m_CurrPosition += impluse * m_weights[0] * m_Face->m_Nodes[0]->m_InvM * TimeStep;// NodeCorr*w0;
			m_Face->m_Nodes[1]->m_CurrPosition += impluse * m_weights[1] * m_Face->m_Nodes[1]->m_InvM * TimeStep;// NodeCorr*w1;
			m_Face->m_Nodes[2]->m_CurrPosition += impluse * m_weights[2] * m_Face->m_Nodes[2]->m_InvM * TimeStep;// NodeCorr*w2;

			//apply negative impluse on rigid
			m_Rigid->ApplyImpulse(-impluse, m_R);

			//record feedback
			m_ForceFeedBackAlongTan += (-impluse * TimeStep);
		}
		Real tanMovement = m_ForceFeedBackAlongTan.Dot(m_TangentDir);

		Real slidethreshold = 0.0025f;

		if (fabsf(tanMovement) < slidethreshold)
		{
			tanMovement = 0;
		}
		else
		{
			if(tanMovement > 0)
			{
				tanMovement -= slidethreshold;
			}
			else
			{
				tanMovement += slidethreshold;
			}
		}
#endif

		Real currDistRatio = m_LastRation + m_SutNeedle->ConvertRealDistToRatioDist(tanMovement);
		if (currDistRatio >= 1.0f && tanMovement > 0.005f)
		{
#if(0)
			GFPhysVector3 headPos = m_SutNeedle->GetNeedleHeadNodeWorldPos();

			GFPhysVector3 worldPtOnFace = m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0] +
				                          m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1] +
				                          m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2];

			GFPhysVector3 movement = (headPos - worldPtOnFace);
			m_Face->m_Nodes[0]->m_CurrPosition += movement;
			m_Face->m_Nodes[1]->m_CurrPosition += movement;
			m_Face->m_Nodes[2]->m_CurrPosition += movement;
#endif
            //Ogre::LogManager::getSingleton().logMessage(Ogre::String("the needle head is outside soft."));
            m_BHeadOut = true;
			return;
		}

		if (currDistRatio <= 0.0f && tanMovement < 0.0f)
        {
#if(0)
			GFPhysVector3 tailPos = m_SutNeedle->GetNeedleTailNodeWorldPos();

			GFPhysVector3 worldPtOnFace = m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0] +
				                          m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1] +
				                          m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2];

			GFPhysVector3 movement = (tailPos - worldPtOnFace);
			m_Face->m_Nodes[0]->m_CurrPosition += movement;
			m_Face->m_Nodes[1]->m_CurrPosition += movement;
			m_Face->m_Nodes[2]->m_CurrPosition += movement;
#endif
            //Ogre::LogManager::getSingleton().logMessage(Ogre::String("the needle end is outside soft."));
            m_BTailOut = true;
			m_SuperfluousOffset = fabsf(currDistRatio);
			return;
        }
        GPClamp(currDistRatio , 0.0f , 1.0f);

		m_lastworld = m_SutNeedle->Ratio2AnchorPos(currDistRatio);

		//recalculate relative rotate of face and rigid
		GFPhysQuaternion rigidOrn = m_Rigid->GetCenterOfMassTransform().GetRotation();//rigidOrn.normalize();
		
        GFPhysQuaternion faceOrn = CalcFaceFrameOrient(m_Face);

		m_RelRot = rigidOrn.Inverse() * faceOrn;
	}
}
//===================================================================================================================
GFPhysFaceRopeAnchor::GFPhysFaceRopeAnchor( GFPhysSoftBodyFace* face,Real* face_weight,SutureThread * thread,int index,Real* ThreadWeights)
{
	m_Face = face;
	m_weights[0] = face_weight[0];
	m_weights[1] = face_weight[1];
	m_weights[2] = face_weight[2];

	m_thread = thread;
	m_NodeIndex = index;
	m_ThreadWeights[0] = ThreadWeights[0];
	m_ThreadWeights[1] = ThreadWeights[1];
	
	m_Friction = 0.0f;

    SutureRopeNode& n0 = m_thread->GetThreadNodeRef(m_NodeIndex);
    SutureRopeNode& n1 = m_thread->GetThreadNodeRef(m_NodeIndex+1);

    m_Last_Pos = n0.m_CurrPosition * m_ThreadWeights[0]
               + n1.m_CurrPosition * m_ThreadWeights[1];	
}
//////////////////////////////////////////////////////////////////////////
GFPhysFaceRopeAnchor::~GFPhysFaceRopeAnchor()
{
    //if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
    //{
    //    PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
    //}
}

void GFPhysFaceRopeAnchor::SetFriction( float friction )
{
	m_Friction = friction;
}
//////////////////////////////////////////////////////////////////////////
void GFPhysFaceRopeAnchor::PrepareSolveConstraint( Real Stiffness,Real TimeStep )
{
	//m_thread->GetThreadSegmentTangentLerped(m_Tangent, m_NodeIndex, m_ThreadWeights[1]);
	m_IterorCount = 0;
	m_AccumFriction = 0.0f;
    m_AccumPressure = GFPhysVector3(0.0f, 0.0f, 0.0f);
    return;
}
//////////////////////////////////////////////////////////////////////////
void GFPhysFaceRopeAnchor::SolveConstraint(Real Stiffniss, Real TimeStep)
{	
	if (m_Face->m_Nodes[0]->m_InvM < GP_EPSILON || m_Face->m_Nodes[1]->m_InvM < GP_EPSILON || m_Face->m_Nodes[2]->m_InvM < GP_EPSILON)
	{
		return;
	}
	if (m_NodeIndex < 0 || m_NodeIndex >= m_thread->GetNumSegments())
	{
		return;
	}

	m_IterorCount++;
	Real RestLen = 0.0f;
	SutureRopeNode & t0 = m_thread->GetThreadNodeRef(m_NodeIndex);
	SutureRopeNode & t1 = m_thread->GetThreadNodeRef(m_NodeIndex + 1);


#if 0

	Real w1 = 0.5f*(t0.GetSolverInvMass() + t1.GetSolverInvMass());

	Real w2 = 0.3333f*(m_Face->m_Nodes[0]->m_InvM
					 + m_Face->m_Nodes[1]->m_InvM
		             + m_Face->m_Nodes[2]->m_InvM);

	Real w = t0.GetSolverInvMass() * 0.1f * m_ThreadWeights[0] * m_ThreadWeights[0]
		+ t1.GetSolverInvMass() * 0.1f * m_ThreadWeights[1] * m_ThreadWeights[1]
		+ m_Face->m_Nodes[0]->m_InvM * m_weights[0] * m_weights[0]
		+ m_Face->m_Nodes[1]->m_InvM * m_weights[1] * m_weights[1]
		+ m_Face->m_Nodes[2]->m_InvM * m_weights[2] * m_weights[2];
#else
	Real w1 = 0.1f;

	Real w2 = 1.0f;

	Real w = w1 * m_ThreadWeights[0] * m_ThreadWeights[0]
		+ w1 * m_ThreadWeights[1] * m_ThreadWeights[1]
		+ w2 * m_weights[0] * m_weights[0]
		+ w2 * m_weights[1] * m_weights[1]
		+ w2 * m_weights[2] * m_weights[2];
#endif

	Real Stiffness = 1.0f;

	if (w > GP_EPSILON)
	{
		GFPhysVector3 ptThread = t0.m_CurrPosition * m_ThreadWeights[0] + t1.m_CurrPosition * m_ThreadWeights[1];
		
		GFPhysVector3 ptFace = m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0]
			+ m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1]
			+ m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2];

		GFPhysVector3 Diff = ptThread - ptFace;

		Real Length = Diff.Length();

		if (Length > FLT_EPSILON)
		{
			Real InvLength = 1.0f / Length;

			Real Invw1w2 = 1.0f / w;

			Real Temp = (Length - RestLen) * InvLength;///Length;

			GFPhysVector3 Deta10 = -(w1 * m_ThreadWeights[0] * Invw1w2) * Temp * Diff;
			GFPhysVector3 Deta11 = -(w1 * m_ThreadWeights[1] * Invw1w2) * Temp * Diff;

			GFPhysVector3 Deta20 = (w2 * m_weights[0] * Invw1w2) * Temp * Diff;
			GFPhysVector3 Deta21 = (w2 * m_weights[1] * Invw1w2) * Temp * Diff;
			GFPhysVector3 Deta22 = (w2 * m_weights[2] * Invw1w2) * Temp * Diff;

			//绳子是不可拉伸的
			t0.m_CurrPosition += Deta10 * Stiffness;
			t1.m_CurrPosition += Deta11 * Stiffness;

			m_AccumPressure += (Deta10 * m_ThreadWeights[0] + Deta11 * m_ThreadWeights[1]) * Stiffness;

			m_Face->m_Nodes[0]->m_CurrPosition += Deta20 * Stiffness;
			m_Face->m_Nodes[1]->m_CurrPosition += Deta21 * Stiffness;
			m_Face->m_Nodes[2]->m_CurrPosition += Deta22 * Stiffness;
		}
	}

	if (false && m_IterorCount == GFPhysGlobalConfig::GetGlobalConfig().m_SoftSolverItorNum)
	{

		GFPhysVector3 Tangent;
		m_thread->GetThreadSegmentTangentLerped(Tangent, m_NodeIndex, m_ThreadWeights[1]);

		Real tanOffset = GetAccumPressure().Dot(Tangent);

		Real maxFriction = 0.00f;

		if (tanOffset > maxFriction)
			tanOffset -= maxFriction;
		else if (tanOffset < -maxFriction)
			tanOffset += maxFriction;
		else
			tanOffset = 0;

		tanOffset *= ROPEANCHORMOVINGSPEED;

		if (tanOffset > 0.0)//可以模拟带倒刺的缝合线
		{
			tanOffset *= 1.2f;
		}

		Real currTanDist = 0.0f;

		m_thread->RelativePos2CurrLen(m_NodeIndex, m_ThreadWeights[0], currTanDist);

		currTanDist += tanOffset;

		//Real TotalThreadLen = m_thread->GetTotalLen(false);

		//GPClamp(currTanDist, 0.0f, TotalThreadLen);

		int nodeindex = -1;
		Real weight = 0.0f;

		m_thread->CurrLen2RelativePos(currTanDist, nodeindex, weight);

		m_NodeIndex = nodeindex;
		m_ThreadWeights[0] = weight;
		m_ThreadWeights[1] = 1.0f - weight;
	}
}

GoPhys::GFPhysVector3 GFPhysFaceRopeAnchor::GetAccumPressure() const
{
	if (m_AccumPressure.Length2() > 0.002f*0.002f)
	{
		return m_AccumPressure;
	}
	else
	{
		return GFPhysVector3(0, 0, 0);
	}
}
//===================================================================================================================
SutureNeedle::SutureNeedle()
{
	m_AttachedRope = 0;
    m_NeedlePhysBody = 0;
	m_pNeedleSceneNode = 0;
    m_OwnerTraining = 0;
    m_NeedleCollideShape = 0;
	m_CollideRadius = 0.04f;
	m_RecentClampTool = 0;

	m_FaceNeedleAnchors.clear();
	m_FaceRopeAnchors.clear();
}
//===================================================================================================================
SutureNeedle::~SutureNeedle()
{
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	{
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
	}

	//attached  rope
	if( m_AttachedRope )
	{
		m_OwnerTraining->OnRemoveSutureThreadFromWorld(m_AttachedRope);
		delete m_AttachedRope;
		m_AttachedRope = NULL;
	}	
	//free attached mesh
	if( m_pNeedleSceneNode )
	{
		Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();		
		pSceneManager->getRootSceneNode()->removeAndDestroyChild(m_pNeedleSceneNode->getName());
		m_pNeedleSceneNode = 0;
	}

//////////////////////////////////////////////////////////////////////////
    if(!m_FaceNeedleAnchors.empty())
    {
        for (size_t i = 0, ni = m_FaceNeedleAnchors.size(); i <ni; ++i)
        {
            delete m_FaceNeedleAnchors[i].pAnchor; 
            m_FaceNeedleAnchors[i].pAnchor = 0;
        }        
    }

    //m_CurrFaceNeedleAnchor = 0;

//////////////////////////////////////////////////////////////////////////	
    if(!m_FaceRopeAnchors.empty())
    {
        for (size_t i = 0, ni = m_FaceRopeAnchors.size(); i <ni; ++i)
        {
            delete m_FaceRopeAnchors[i]; 
            m_FaceRopeAnchors[i] = 0;
        }        
    }

    //need destroy collision shape and rigid body
	if(m_NeedleCollideShape)
	{
        for (int c = 0,nc = m_NeedleCollideShape->GetNumComponent(); c < nc; c++)
	   {
          GFPhysCollideShape * subShape = m_NeedleCollideShape->GetComponentShape(c);
          delete subShape;
	   }
	   delete m_NeedleCollideShape;
       m_NeedleCollideShape = 0;
	}
	if(m_NeedlePhysBody)
	{
		PhysicsWrapper::GetSingleTon().DestoryRigidBody(m_NeedlePhysBody);
		m_NeedlePhysBody = 0;
	}

	m_NeedleAnchorOrgans.clear();
	m_NeedleAnchorDisableFaces.clear();

    removeAllNeedleActionListener();
    
}
//===================================================================================================================
void SutureNeedle::GFPhysQuaternionFromEuler(GFPhysQuaternion& q,Real yawDegree,Real pitchDegree,Real rollDegree)
{
	Real yaw   = Ogre::Math::DegreesToRadians(yawDegree);
	Real picth = Ogre::Math::DegreesToRadians(pitchDegree);
	Real roll  = Ogre::Math::DegreesToRadians(rollDegree);

	q.SetEuler(yaw,roll,picth);
}
//===================================================================================================================
void SutureNeedle::OgreQuaternionFromEuler(Ogre::Quaternion& q,const Real yawDegree,const Real pitchDegree,const Real rollDegree)
{

	Ogre::Degree yaw(yawDegree);
	Ogre::Degree picth(pitchDegree);
	Ogre::Degree roll(rollDegree);

	Ogre::Radian yAngel(yaw);
	Ogre::Radian xAngel(picth);
	Ogre::Radian zAngel(roll);

	Ogre::Matrix3  test;
	test.FromEulerAnglesYZX(yAngel,xAngel,zAngel);
	q.FromRotationMatrix(test);
}
//========================================================================
void SutureNeedle::CalcNeedleMassProperty(GFPhysVector3 NodePos[] , 
										  int   NumNode , 
										  float massPerNode ,
										  float collideradius,
										  float & totalMass ,
										  GFPhysVector3 & com , 
										  GFPhysMatrix3 & inertiaTesnor)
{
	//a fake algorithm to calculate the node cluster shape's mass & inertia tensor property
	totalMass = 0;

	com = GFPhysVector3(0,0,0);

	for(int m = 0 ; m < NumNode ; m++)
	{
		totalMass += massPerNode;
		com += NodePos[m]*massPerNode;
	}
	com *= (1.0f / totalMass);

	//now inertia tensor
	inertiaTesnor.SetZero();
	for(int m = 0 ; m < NumNode ; m++)
	{
		GFPhysVector3 refPos = NodePos[m]-com;

		float Mass = massPerNode / 6.0f;//0.16666667f*
		//the node has none thickness so will make inertia tensor illness
		//this trick give thick ness to th inertia tensor thus make it strong
		for(int t = 0 ; t < 3 ; t++)
		{
			GFPhysVector3 axis(0,0,0);
			axis[t] = 1.0f;

			for(int c = 0 ; c < 2 ; c++)
			{
				GFPhysVector3 r;//samplePos

				if(c == 0)
				   r = refPos + axis * collideradius;
				else
				   r = refPos - axis * collideradius;

				inertiaTesnor[0][0] += Mass*(r.m_y*r.m_y + r.m_z*r.m_z);

				inertiaTesnor[1][1] += Mass*(r.m_x*r.m_x + r.m_z*r.m_z);

				inertiaTesnor[2][2] += Mass*(r.m_x*r.m_x + r.m_y*r.m_y);

				inertiaTesnor[0][1] -= Mass*(r.m_x*r.m_y);

				inertiaTesnor[0][2] -= Mass*(r.m_x*r.m_z);

				inertiaTesnor[1][2] -= Mass*(r.m_y*r.m_z);
			}
		}
		
	}
	inertiaTesnor[1][0] = inertiaTesnor[0][1];

	inertiaTesnor[2][0] = inertiaTesnor[0][2];

	inertiaTesnor[2][1] = inertiaTesnor[1][2];
}
//========================================================================
void SutureNeedle::CreateVisibleMesh(const char* meshFileName,int id)
{
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();
	
	Ogre::Entity* needleEnt = pSceneManager->createEntity(meshFileName);
	
    Ogre::SceneNode* MeshNode = pSceneManager->getRootSceneNode()->createChildSceneNode("zhen" + Ogre::StringConverter::toString(id));
	
	MeshNode->attachObject(needleEnt);
	
	//set position and orientation
	MeshNode->setPosition(0.0f,0.0f,0.0f);
	Ogre::Matrix3  test;

	Ogre::Degree dYaw   = Ogre::Degree(_coherentYaw);
	Ogre::Degree dPicth = Ogre::Degree(_coherentPicth);
	Ogre::Degree dRoll  = Ogre::Degree(_coherentRoll);
	Ogre::Radian yAngel(dYaw);
	Ogre::Radian xAngel(dPicth);
	Ogre::Radian zAngel(dRoll);

	test.FromEulerAnglesYZX(yAngel,xAngel,zAngel);
	Ogre::Quaternion qtest;
	qtest.FromRotationMatrix(test);
	MeshNode->setOrientation(qtest);

	//bool b = false;
	//Real yaw   = qtest.getYaw(b).valueDegrees();
	//Real pitch = qtest.getPitch(b).valueDegrees();
	//Real roll  = qtest.getRoll(b).valueDegrees();

	needleEnt->setMaterialName("qixiejinshu");
	m_pNeedleSceneNode = MeshNode;

}
//========================================================================
void SutureNeedle::CreateSutureNeedle(MisNewTraining * newTrain, int threadnodenum, Real restlen,const Ogre::String & needleskeleton)
{
	//parameter of needle data	
	GFPhysVector3 NeedleInitOffset = GFPhysVector3(0, 0 , 0);
	GFPhysVector3 NeedleGravity(0 , -8 , 0);
	GFPhysVector3 ThreadGravity(0 , -8 , 0);
	float ThreadDampRate = 4.0f;
	//
    m_OwnerTraining = newTrain;

    GFPhysVector3 NeedleNodesPos[SNeedleNodeNum];

#ifdef NEEDLE_FROM_FILE

	//ifstream infile("..\\Config\\MultiPortConfig\\Supplementary\\NBT_NeedleTest\\needle_position.txt");
    ifstream infile(needleskeleton);
	Real posx,posy,posz;

	for( int i = 0 ; i < SNeedleNodeNum ; i++ )
	{
		infile >> posx >> posy >> posz;
		
		NeedleNodesPos[i] = GFPhysVector3(posx , posz , -posy);
	}
	infile.close();
#else
	NeedleNodesPos[0]  = GFPhysVector3(-0.004f, 0, 1.32f  );
	NeedleNodesPos[1]  = GFPhysVector3( 0.148f, 0, 1.32f);
	NeedleNodesPos[2]  = GFPhysVector3( 0.31f , 0, 1.32f  );
	NeedleNodesPos[3]  = GFPhysVector3( 0.45f , 0, 1.32f); 
	NeedleNodesPos[4]  = GFPhysVector3( 0.59f , 0, 1.32f );
	NeedleNodesPos[5]  = GFPhysVector3( 0.726f, 0, 1.32f);
	NeedleNodesPos[6]  = GFPhysVector3( 0.84f , 0, 1.32f );
	NeedleNodesPos[7]  = GFPhysVector3( 0.955f, 0, 1.32f);
	NeedleNodesPos[8]  = GFPhysVector3( 1.062f, 0, 1.32f );
	NeedleNodesPos[9]  = GFPhysVector3( 1.14f , 0, 1.32f);
	NeedleNodesPos[10] = GFPhysVector3( 1.207f, 0, 1.32f );
	NeedleNodesPos[11] = GFPhysVector3( 1.26f, 0, 1.32f );
	NeedleNodesPos[12] = GFPhysVector3( 1.296f , 0, 1.32f);
	NeedleNodesPos[13] = GFPhysVector3( 1.309f, 0, 1.32f);
	NeedleNodesPos[14] = GFPhysVector3( 1.67f ,0, 1.32f);
#endif

	//create collision shape first - compound of capsules
	m_NeedleCollideShape = new GFPhysCompoundShape();

    
	for(size_t c = 0 ; c < SNeedleNodeNum-1 ; c++)
	{
		Ogre::Vector3 t0 = GPVec3ToOgre(NeedleNodesPos[c]);

		Ogre::Vector3 t1 = GPVec3ToOgre(NeedleNodesPos[c+1]);

#if(0)
        Ogre::Vector3 Axis = t1 - t0;

        Ogre::Quaternion quat = Ogre::Vector3::UNIT_X.getRotationTo(Axis.normalisedCopy());

        GFPhysQuaternion capsuleRotate(quat.x, quat.y, quat.z, quat.w);

        GFPhysVector3    capsuleCenter(OgreToGPVec3((t0 + t1)*0.5f));

        GFPhysTransform capsuleTrans(capsuleRotate, capsuleCenter);

        GFPhysCapsuleShapeX * capsule =  new GFPhysCapsuleShapeX(m_CollideRadius , (t1-t0).length());
		capsule->SetMargin(0.1f);

        m_NeedleCollideShape->AddComponent(capsuleTrans, capsule);
#else
		if (c == 0)//针缩进
		{
			t0 += (t1 - t0).normalisedCopy()*m_CollideRadius;
		}

		if (c == SNeedleNodeNum - 2)//针尖缩进
		{
			t1 += (t0 - t1).normalisedCopy()*m_CollideRadius;
		}
        GFPhysLineSegmentShape * line = new GFPhysLineSegmentShape(OgreToGPVec3(t0), OgreToGPVec3(t1), m_CollideRadius);

        line->SetMargin(0.04f);//default is 0.04f

        m_NeedleCollideShape->AddComponent(GFPhysVector3(0, 0, 0), line);

		m_NeedleCollideShape->m_UseCCD = true;

#endif
 		
	}

	//create compound collision shape and shift to center of mass
	float massTotal;

	GFPhysMatrix3 localInertiaTensor;

	CalcNeedleMassProperty( NeedleNodesPos , 
							SNeedleNodeNum , 
							NeedleMassPerSegment ,
							m_CollideRadius*0.5f,
							massTotal ,
							m_CenterOfMass , 
							localInertiaTensor);

	m_NeedleCollideShape->ShiftToMassOfCenter(m_CenterOfMass);
	m_NeedleCollideShape->SetMargin(0.03f);

	//now create needle rigid body use the collision shape and mass property
	GFPhysTransform localComTrans;
	localComTrans.SetIdentity();
	localComTrans.SetOrigin(m_CenterOfMass);
	m_vMeshPosOffset = m_CenterOfMass;
	
	//bacon add for test
	Real test_yaw   = _coherentYaw;
	Real test_picth = _coherentPicth;
	Real test_roll  = _coherentRoll;

	GFPhysQuaternion q;
	GFPhysQuaternionFromEuler(q,test_yaw,test_picth,test_roll);

	GFPhysTransform initWorldCom(q , localComTrans.GetOrigin() + NeedleInitOffset);
  
	GFPhysVector3 localInertialTensor(localInertiaTensor[0][0] , localInertiaTensor[1][1] , localInertiaTensor[2][2]);
	
	GFPhysMatrix3 invLocalTensor = localInertiaTensor.Inverse();

	m_NeedlePhysBody = PhysicsWrapper::GetSingleTon().CreateRigidBody(massTotal , m_NeedleCollideShape , localInertialTensor , initWorldCom);
	m_NeedlePhysBody->SetLocalInvInertiaTensorMatrix(invLocalTensor);
	m_NeedlePhysBody->SetGravity(NeedleGravity);
	m_NeedlePhysBody->SetFriction(1.6f);
	m_NeedlePhysBody->SetDamping(3.5f , 3.5f);
	m_NeedlePhysBody->SetSleepingThresholds(0.0f , 0.0f);
	m_NeedlePhysBody->EnableDoubleFaceCollision();


	//transform local node position relative to new center of mass
	m_NeedleTotalLen = 0.0f;
	
	GFPhysTransform invcomTrans = localComTrans.Inverse();
	
	for(size_t c = 0 ; c < SNeedleNodeNum ; c++)
	{
		m_NeedleNodeLocalPos.push_back(invcomTrans(NeedleNodesPos[c]));
		m_NeedleNodeWorldPos.push_back(invcomTrans(NeedleNodesPos[c]));
	}
	
    for (int i = 0, ni = (int)m_NeedleNodeLocalPos.size() - 1; i <ni; ++i)
	{
		m_NeedleTotalLen +=  (m_NeedleNodeLocalPos[i+1]-m_NeedleNodeLocalPos[i]).Length();
	}
	
	UpdateNeedleNodeWorldPos();

	//debug object for needle collision shape
    static int s_NeedleId = 0;
    s_NeedleId++;
    Ogre::String strNeedleName = "testNeedle" + Ogre::StringConverter::toString(s_NeedleId);
    m_RendObject.CreateRendPart(strNeedleName, MXOgre_SCENEMANAGER);
	//create visible mesh
    CreateVisibleMesh("needle.mesh", s_NeedleId);


//////////////////////////////////////////////////////////////////////////
#if 1

    GFPhysVector3 threadStartWorld  = GetNeedleNodePosByIndex(0); //m_NeedlePhysBody->GetWorldTransform()();
    GFPhysVector3 threadDirWorld    = threadStartWorld - GetNeedleNodePosByIndex(SNeedleNodeNum - 1);
    
    threadDirWorld.Normalize();

    GFPhysVector3 threadEndWorld = threadStartWorld + threadDirWorld * restlen;

    m_AttachedRope = new SutureThread( MXOgre_SCENEMANAGER , newTrain );

    m_AttachedRope->m_NeedleAttchedThread = this;

    m_AttachedRope->CreateFreeThread(threadStartWorld, threadEndWorld, threadnodenum - 1, ThreadMass);
    m_AttachedRope->SetGravity(ThreadGravity);
    m_AttachedRope->m_DampingRate = ThreadDampRate;
    m_AttachedRope->m_RopeRSFriction = 0.04f;

    m_AttachedRope->LockRopeNode(m_AttachedRope->GetNumThreadNodes() - 1);
    m_AttachedRope->m_islock = true;    



    //also shift local attach point
    GFPhysTransform invNeedleTran = m_NeedlePhysBody->GetWorldTransform().Inverse();

    AddAttachPoint(invNeedleTran * m_AttachedRope->GetThreadNode(0).m_CurrPosition , 0 , 1000.0f);

    AddAttachPoint(invNeedleTran * m_AttachedRope->GetThreadNode(1).m_CurrPosition , 1 , 400.0f);
    AddAttachPoint(invNeedleTran * m_AttachedRope->GetThreadNode(2).m_CurrPosition , 2 , 100.0f);

    m_AttachedRope->GetThreadNodeRef(0).MarkAsStickInNeedle(true);

	//////////////////////////////////////////////////////////////////////////
#endif
	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	{
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this, 1);
	}
	m_NeedleOriginInvMass = m_NeedlePhysBody->GetInvMass();
	GFPhysMatrix3 tensor = m_NeedlePhysBody->GetLocalInvInertiaTensor();
	m_NeedleOriginInvTensor = GFPhysVector3(tensor[0][0], tensor[1][1], tensor[2][2]);

	m_BForceSeparate = false;


}
//==============================================================================
void SutureNeedle::AddAttachPoint(const GFPhysVector3 & attachlocal , int NodeIndex , float stiffness)
{
    SutureRopeAttachPoint attachPoint;
    attachPoint.m_LocalAttachPt = attachlocal;
    attachPoint.m_PointIndex = NodeIndex;
	attachPoint.m_InvStiff = (stiffness > 0 ? 1.0f / stiffness : 0.0f);// GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(stiffness);
    m_AttchPoints.push_back(attachPoint);
}
//==============================================================================
int SutureNeedle::GetNeedleNodeNum()
{
	return m_NeedleCollideShape->GetNumComponent() + 1;
}
//==============================================================================
GFPhysVector3 SutureNeedle::GetNeedleNodePosByIndex(int index)
{
	return m_NeedleNodeWorldPos[index];//pos;
}
//==============================================================================
GFPhysVector3 SutureNeedle::GetNeedleDirction()//获得针尖方向
{
    GFPhysVector3 dir = m_NeedleNodeWorldPos[m_NeedleNodeWorldPos.size()-1] - m_NeedleNodeWorldPos[m_NeedleNodeWorldPos.size()-2];
    return dir.Normalized();
}
//==============================================================================
GFPhysVector3 SutureNeedle::GetNeedleNormalDirction()//获得针所在平面的法向量
{
    int k = (int)m_NeedleNodeWorldPos.size() * 0.5;
    GFPhysVector3 dir1 = m_NeedleNodeWorldPos[0] - m_NeedleNodeWorldPos[k];
    
    GFPhysVector3 dir2 = m_NeedleNodeWorldPos[m_NeedleNodeWorldPos.size()-1] - m_NeedleNodeWorldPos[k];

    GFPhysVector3 dir = dir1.Cross(dir2);
    return dir.Normalized();
}

//==============================================================================
GFPhysVector3 SutureNeedle::GetNeedleHeadNodeWorldPos()   //获得针尖质心位置(世界坐标系)
{
	return m_NeedleNodeWorldPos[m_NeedleNodeWorldPos.size()-1];
}
//==============================================================================
GFPhysVector3 SutureNeedle::GetNeedleTailNodeWorldPos()   //获得针尾质心位置(世界坐标系)
{
	return m_NeedleNodeWorldPos[0]; 
}
//==============================================================================
void SutureNeedle::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
    for (size_t c = 0, nc = m_AttchPoints.size(); c < nc; c++)
    {
        SutureRopeNode & Node = m_AttachedRope->GetThreadNodeRef(m_AttchPoints[c].m_PointIndex);

        m_AttchPoints[c].m_R = m_NeedlePhysBody->GetWorldTransform().GetBasis()*m_AttchPoints[c].m_LocalAttachPt;

        float rgInvMass = m_NeedlePhysBody->GetInvMass();

        GFPhysMatrix3 inveInertiaTensor = m_NeedlePhysBody->GetInvInertiaTensorWorld();

        int collisiontag = m_NeedlePhysBody->GetCollisionFlags();
        if((collisiontag & GFPhysCollideObject::CF_STATIC_OBJECT)||(collisiontag & GFPhysCollideObject::CF_KINEMATIC_OBJECT))
        {
            rgInvMass = 0;
            inveInertiaTensor.SetZero();
        }

		//build J M-1 Jt etc
		//float InvStiff = 1.0f / 1000.0f;a
		
		GFPhysMatrix3 JInvMJt = GFPhysSoftBodyConstraint::K_DtMatrix(TimeStep, Node.GetSolverInvMass(), rgInvMass, inveInertiaTensor, m_AttchPoints[c].m_R);
		JInvMJt[0][0] += (m_AttchPoints[c].m_InvStiff / TimeStep);
		JInvMJt[1][1] += (m_AttchPoints[c].m_InvStiff / TimeStep);
		JInvMJt[2][2] += (m_AttchPoints[c].m_InvStiff / TimeStep);


		m_AttchPoints[c].m_ImpMat = JInvMJt.Inverse();
		
		/* GFPhysSoftBodyConstraint::ImpulseMatrix(TimeStep,
            Node.GetInvMass(),
            rgInvMass,
            inveInertiaTensor,
            m_AttchPoints[c].m_R);*/

		m_AttchPoints[c].m_InvStiffDivDt = m_AttchPoints[c].m_InvStiff / TimeStep;

		m_AttchPoints[c].m_DtInvMa = TimeStep * Node.GetSolverInvMass();

		m_AttchPoints[c].m_Lambda = GFPhysVector3(0, 0, 0);

        m_NeedlePhysBody -> Activate();
    }

	//ruoyu add for holded needle when solve couple between needle and softbody make needl as static (mass = 0)
	if (GetBeHold())
	{
		m_NeedlePhysBody->m_inverseMass = 0;
		m_NeedlePhysBody->m_invInertiaTensorWorld.SetZero();
	}

	for (int i = 0, ni = m_FaceNeedleAnchors.size(); i < ni;i++)
	{
		m_FaceNeedleAnchors[i].pAnchor->PrepareSolveConstraint(Stiffness, TimeStep);
	}

	for (int i = 0, ni = m_FaceRopeAnchors.size(); i < ni; i++)
	{
		m_FaceRopeAnchors[i]->PrepareSolveConstraint(Stiffness, TimeStep);
	}
}
//===================================================================================================================
void SutureNeedle::SolveConstraint(Real Stiffness,Real TimeStep)
{
    for (size_t c = 0, nc = m_AttchPoints.size(); c < nc; c++)
    {
        SutureRopeNode & Node = m_AttachedRope->GetThreadNodeRef(m_AttchPoints[c].m_PointIndex);

        const GFPhysTransform & Trans = m_NeedlePhysBody->GetWorldTransform();

        const GFPhysVector3		wa  = Trans * m_AttchPoints[c].m_LocalAttachPt;

        const GFPhysVector3		va  = m_NeedlePhysBody->GetVelocityInLocalPoint(m_AttchPoints[c].m_R)*TimeStep;

		GFPhysVector3		vr = (va + wa - Node.m_CurrPosition)*0.8f;//0.8 - sor ;

		vr += m_AttchPoints[c].m_Lambda * m_AttchPoints[c].m_InvStiffDivDt;

        const GFPhysVector3	 impulse = m_AttchPoints[c].m_ImpMat * (-vr);

        Node.m_CurrPosition += (-impulse) * m_AttchPoints[c].m_DtInvMa;//bacon omit for test

        int collisiontag = m_NeedlePhysBody->GetCollisionFlags();

        if(!(collisiontag & GFPhysCollideObject::CF_STATIC_OBJECT)||(collisiontag & GFPhysCollideObject::CF_KINEMATIC_OBJECT))
            m_NeedlePhysBody->ApplyImpulse(impulse,m_AttchPoints[c].m_R);
    }
    //m_NeedlePhysBody->ApplyImpulse(GFPhysVector3(0.0f,0.0f,0.0f),GFPhysVector3(0.0f,0.0f,0.0f));

	for (int i = 0, ni = m_FaceNeedleAnchors.size(); i < ni; i++)
	{
		m_FaceNeedleAnchors[i].pAnchor->SolveConstraint(Stiffness, TimeStep);
	}

	for (int i = 0, ni = m_FaceRopeAnchors.size(); i < ni; i++)
	{
		m_FaceRopeAnchors[i]->SolveConstraint(Stiffness, TimeStep);
	}
}
//===================================================================================================================
void SutureNeedle::SetVisibleMeshMaterial(bool bClamped)
{
	if( !m_pNeedleSceneNode )
		return;

	Ogre::Entity * pAttachEntity = (Ogre::Entity*)m_pNeedleSceneNode->getAttachedObject(0);
	if( pAttachEntity )
	{
		if( bClamped )
		{
		    pAttachEntity->setMaterialName("jinshu");
		}
		else
		{ 
		    pAttachEntity->setMaterialName("Material#1");
		}
	}
}
//===================================================================================================================
void SutureNeedle::UpdateNeedleNodeWorldPos()
{
    if (m_NeedlePhysBody == NULL)
    {
        return;
    }
	GFPhysTransform trNeedleTran = m_NeedlePhysBody->GetWorldTransform();

    for (size_t c = 0, nc = m_NeedleNodeLocalPos.size(); c <nc; c++)
	{
		m_NeedleNodeWorldPos[c] = (trNeedleTran(m_NeedleNodeLocalPos[c]));//更新针各段首尾的世界坐标
	}

	//std::vector<Ogre::Vector3> centerPosition;

	//if (m_FaceNeedleAnchors.size() == 0)
	//{
	//	for (int v = 0; v < 14; v++)
	//	{
	//		centerPosition.push_back(GPVec3ToOgre(m_NeedleNodeWorldPos[v]));
	//	}
	//}
	//m_RendObject.UpdateRendSegment(centerPosition, m_CollideRadius * 1.25f, "green");
	//GFPhysTransform rgtrans = m_NeedlePhysBody->GetCenterOfMassTransform();
	//m_CenterOfMass = rgtrans(m_CenterOfMass);

	Real totalMass = 0.0f;

	m_CenterOfMass = GFPhysVector3(0.0f, 0.0f, 0.0f);

    for (int m = 0; m < SNeedleNodeNum; m++)
	{
        totalMass += NeedleMassPerSegment;
        m_CenterOfMass += m_NeedleNodeWorldPos[m] * NeedleMassPerSegment;
	}
	m_CenterOfMass *= (1.0f / totalMass);
}
//===================================================================================================================
void SutureNeedle::SynchronizeState()
{
    UpdateNeedleNodeWorldPos();
}
//===================================================================================================================
void SutureNeedle::UpdateMesh()
{
    if (m_NeedlePhysBody && m_pNeedleSceneNode)
	{
//convert physics transform to graphic transform
		
        GFPhysTransform trNeedleTran = m_NeedlePhysBody->GetWorldTransform();

        GFPhysVector3 vPosition = trNeedleTran.GetOrigin() - QuatRotate(trNeedleTran.GetRotation(), m_vMeshPosOffset);
		
		GFPhysQuaternion vRotation = trNeedleTran.GetRotation();
		
		//Ogre::Quaternion qNeedleRot(vRotation.GetW(),vRotation.GetX(),vRotation.GetY(),vRotation.GetZ());
		
		//m_pNeedleSceneNode->setPosition(vPosition.m_x,vPosition.m_y,vPosition.m_z); 
        //m_pNeedleSceneNode->setOrientation(qNeedleRot);

        m_pNeedleSceneNode->setPosition(GPVec3ToOgre(vPosition));

        m_pNeedleSceneNode->setOrientation(GPQuaternionToOgre(vRotation));
	
		//draw debug collide body
        //std::vector<Ogre::Vector3> centerPosition;
        //for (int v = 0; v <= m_NeedleCollideShape->GetNumComponent(); v++)
        //{
        //    /*GFPhysTransform childTrans = m_NeedleCollideShape->GetComponentTransform(v);
        //    GFPhysTransform fullTrans = m_NeedlePhysBody->GetWorldTransform() * childTrans;
        //    GFPhysVector3   pos = fullTrans(GFPhysVector3(0, 0, 0));
        //    centerPosition.push_back(GPVec3ToOgre(pos));*/
        //    
        //    centerPosition.push_back(GPVec3ToOgre(m_NeedleNodeWorldPos[v]));//ultra solution
        //}

        //m_RendObject.UpdateRendSegment(centerPosition, m_CollideRadius);		
	}
    if (m_AttachedRope)
        m_AttachedRope->UpdateMesh();
}

//============================================================================================================
void SutureNeedle::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	m_CollidedRigid.clear();
        
    if (m_AttachedRope)
    {
        m_AttachedRope->BeginSimulateSutureThreadPhysics(dt);
    }    

    if (m_NeedlePhysBody)
    {
        m_NeedlePhysBody->m_inverseMass = m_NeedleOriginInvMass;
        m_NeedlePhysBody->SetLocalInvInertiaTensor(m_NeedleOriginInvTensor);
        m_NeedlePhysBody->UpdateInertiaTensor();
    }
}
//==================================================================================================================
void SutureNeedle::OnRigidBodyCollided(GFPhysCollideObject * ca,
	                                   GFPhysCollideObject * cb,
									   const GFPhysManifoldPoint * contactPoints,
									   int NumContactPoints)
{
	if (ca == m_NeedlePhysBody)
	{
		m_CollidedRigid.insert(cb);
	}
	else if (cb == m_NeedlePhysBody)
	{
		m_CollidedRigid.insert(ca);
	}
}
//================================================================================================================
void SutureNeedle::OnRigidBodySeperate(GFPhysCollideObject * ra, GFPhysCollideObject * rb)
{
	if (ra == m_NeedlePhysBody)
	{
		m_CollidedRigid.erase(rb);
	}
	else if (rb == m_NeedlePhysBody)
	{
		m_CollidedRigid.insert(ra);
	}
}
//===================================================================================================================
void SutureNeedle::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{
	//disable collide between needle and around faces;
	for (std::set<MisMedicOrgan_Ordinary*>::iterator it = m_NeedleAnchorOrgans.begin(); it != m_NeedleAnchorOrgans.end(); it++)
	{
		for (size_t f = 0; f < (*it)->m_physbody->GetNumFace(); f++)
		{
			GFPhysSoftBodyFace * face = (*it)->m_physbody->GetFaceAtIndex(f);
			face->EnableCollideWithRigid();
		}
	}
	for (std::multiset<GFPhysSoftBodyFace*>::iterator it = m_NeedleAnchorDisableFaces.begin(); it != m_NeedleAnchorDisableFaces.end(); it++)
	{
		GFPhysSoftBodyFace* face = *it;
		face->DisableCollideWithRigid();
	}


    SynchronizeState();
    UpdateAnchor();    
	//NormalizeFaceNeedleAnchor();

    if(m_AttachedRope)
    {
//////////////////////////////////////////////////////////////////////////
        Real Max = INT_MIN;
        Real Min = INT_MAX;
        for (size_t t = 0, nt = m_FaceRopeAnchors.size(); t < nt; ++t)
        {
             if (Max < m_FaceRopeAnchors[t]->GetSegPos())
             {
                 Max = m_FaceRopeAnchors[t]->GetSegPos();
             }
             if (Min > m_FaceRopeAnchors[t]->GetSegPos())
             {
                 Min = m_FaceRopeAnchors[t]->GetSegPos();
             }
        }
        if (Max != INT_MIN && Min != INT_MAX)
        {
            m_AttachedRope->SetRopeAnchorIndexMax(Max);
            m_AttachedRope->SetRopeAnchorIndexMin(Min);
        }
        
//////////////////////////////////////////////////////////////////////////

    	m_AttachedRope->EndSimulateThreadPhysics(dt);


        for (int r = 0, nr = m_AttachedRope->GetNumThreadNodes(); r < nr; r++)
        {
            SutureRopeNode& refNode =  m_AttachedRope->GetThreadNodeRef(r);
            refNode.SetCanCollideSoft(true);
        }
		//////////////////////////////////////////////////////////////////////////
		if (Min < 1)
		{
			for (int r = 0; r < 2; r++)//尝试克服线拔出时的抖动
			{
				SutureRopeNode& refNode = m_AttachedRope->GetThreadNodeRef(r);
				refNode.SetCanCollideSoft(false);
			}
		}		
		//////////////////////////////////////////////////////////////////////////
        if (m_FaceRopeAnchors.size() > 0)
        {
            for (int n = (int)(m_AttachedRope->GetRopeAnchorIndexMin()), num_n = (int)(m_AttachedRope->GetRopeAnchorIndexMax()) + 1; n < num_n; n++)
            {
                SutureRopeNode& refNode = m_AttachedRope->GetThreadNodeRef(n);
                refNode.SetCanCollideSoft(false);
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////

void SutureNeedle::UpdateAnchor()
{
    if (GetPhysicBody())
    {
        if (m_FaceNeedleAnchors.size() > 0 && GetBeHold() == false)//并且针未被持针钳夹
        {
            GetPhysicBody()->SetinvInertiaTensorWorldFactor(0.0f);
            GetPhysicBody()->SetAngularVelocity(GFPhysVector3(0.0f, 0.0f, 0.0f));
        }
        else
        {
            GetPhysicBody()->SetinvInertiaTensorWorldFactor(1.0f);
        }
    }
    //////////////////////////////////////////////////////////////////////////
    for (int i = 0, ni = (int)m_FaceNeedleAnchors.size(); i <ni; i++)
	{
		FaceAnchorInfo & info = m_FaceNeedleAnchors[i];
		////////////////////////////////////////////////////////////////////////////

		/*if (info.pAnchor->m_ForceFeedBack.Length2() > 0.5f)//pressure is big enough 只在拉的时候起作用，压不影响
		{
			if (!info.pAnchor->m_BHeadOut)
			{
				info.pAnchor->m_BHeadOut = true;
			}
			if (!m_BForceSeparate)
			{
				m_BForceSeparate = true;
				m_SeparateOrgan = info.POrgan;
			}
		}*/
		//////////////////////////////////////////////////////////////////////////
		if (GetBeHold())
			info.pAnchor->SetFriction(0.6f);
		else
			info.pAnchor->SetFriction(100000.0f);
	}
    std::vector<Real> nodeIndex;
    if (m_FaceRopeAnchors.size() > 0)
    {
        for (int i = 0, ni = (int)m_FaceRopeAnchors.size(); i <ni; i++)
        {
			GFPhysFaceRopeAnchor * info = m_FaceRopeAnchors[i];
			//////////////////////////////////////////////////////////////////////////
			/*if (info->GetAccumPressure().Length2() > 1.0f)//pressure is big enough 只在拉的时候起作用
			{				
				if (m_AttachedRope &&m_AttachedRope->m_KnotsInThread)
				{
					GFPhysVectorObj<KnotInSutureRope> Knots;
					m_AttachedRope->m_KnotsInThread->GetAllKnots(Knots);
					
					if (Knots.size() == 0)
					{
						if (!info->m_ForcedSeparation)
						{
							info->m_ForcedSeparation = true;
						}
					}
					else
					{
						//一旦形成结就暂不删除线肉锚点
					}
				}
				else
				{
					if (!info->m_ForcedSeparation)
					{
						info->m_ForcedSeparation = true;
					}
				}				
			}*/

			//////////////////////////////////////////////////////////////////////////
			notifyMovingRopeAnchor(info->m_thread, info->m_NodeIndex, info->m_ThreadWeights);
            Real p = m_FaceRopeAnchors[i]->GetSegPos();
            if (p > 1.0f)
            {
                nodeIndex.push_back(p);
            }
        }
        m_AttachedRope->SetRopeAnchorIndex(nodeIndex);
    }

    NormalizeFaceNeedleAnchor();

	NormalizeFaceRopeAnchor();//锚点移动并防止锚点翻转

	RemoveHeadAnchor();

	RemoveTailAnchor();
	
	RemoveThreadAnchor();
}

//////////////////////////////////////////////////////////////////////////
//自定义排序函数  
bool SortBySegPosition(const GFPhysFaceRopeAnchor* v1, const GFPhysFaceRopeAnchor* v2)
{
    return v1->GetSegPos() > v2->GetSegPos();//降序排列  
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::NormalizeFaceRopeAnchor()
{	
	if (NULL == m_AttachedRope)
	{
		return;
	}
    //std::sort(m_FaceRopeAnchors.begin(), m_FaceRopeAnchors.end(), SortBySegPosition);

	size_t ni = m_FaceRopeAnchors.size();
	if (ni > 1)
	{
		bool avoidInvert0 = false;
		bool avoidInvert1 = false;

		Real offset0 = 0.0f;
		Real offset1 = 0.0f;
		for (size_t i = 0; i < ni; ++i)
		{
			GFPhysFaceRopeAnchor* faceropeanchor = m_FaceRopeAnchors[i];
			GFPhysVector3 pressure = faceropeanchor->GetAccumPressure();

			GFPhysVector3 tangent;
			m_AttachedRope->GetThreadSegmentTangentLerped(tangent, faceropeanchor->m_NodeIndex, faceropeanchor->m_ThreadWeights[1]);

			Real offset = pressure.Dot(tangent);

			if (i == 0 && fabsf(offset) > 0.05f)
			{
				avoidInvert0 = true;
				offset0 = offset;
			}
			if (i == ni - 1 && fabsf(offset) > 0.05f)
			{
				avoidInvert1 = true;
				offset1 = offset;
			}
		}
		if (avoidInvert0 && avoidInvert1 && offset0 * offset1 < 0)
		{
			return;
		}
	}


//////////////////////////////////////////////////////////////////////////
	//GFPhysVectorObj<KnotInSutureRope> deadKnots;
	GFPhysVectorObj<KnotInSutureRope> Knots;

	if (m_AttachedRope->m_KnotsInThread)
	{
		m_AttachedRope->m_KnotsInThread->GetAllKnots(Knots);
		//m_AttachedRope->m_KnotsInThread->GetDeadKnots(deadKnots);
	}

	/*std::vector<Real> lengthvec;
	lengthvec.reserve(100);
	for (int g = 0; g < m_AttachedRope->GetNumThreadNodes() - 1; g++)
	{
		Real length0 = m_AttachedRope->GetTotalLen(true, g, g + 1);
		lengthvec.push_back(length0);
	}*/

	Real TotalFriction = m_AttachedRope->m_TotalRopeAnchorFriction;
    for (size_t i = 0; i <ni; ++i)
    {
        bool bNeedSlide = true;
        GFPhysFaceRopeAnchor* faceropeanchor = m_FaceRopeAnchors[i];

        for (size_t c = 0, nc = Knots.size(); c < nc; c++)
        {
            if (faceropeanchor->m_NodeIndex + faceropeanchor->m_ThreadWeights[0] >= Knots[c].m_knotcon1.m_A + Knots[c].m_knotcon1.m_weightA
                && faceropeanchor->m_NodeIndex + faceropeanchor->m_ThreadWeights[0] <= Knots[c].m_knotcon0.m_B + Knots[c].m_knotcon0.m_weightB)
            {
                bNeedSlide = false;
                break;
            }
        }

		if (bNeedSlide)
		{
			GFPhysVector3 tangent;
			m_AttachedRope->GetThreadSegmentTangentLerped(tangent, faceropeanchor->m_NodeIndex, faceropeanchor->m_ThreadWeights[1]);

			GFPhysVector3 pressure = faceropeanchor->GetAccumPressure();
			
			Real tanOffset = pressure.Dot(tangent);

			Real maxFriction = std::min(TotalFriction * 4 / (ni*ni*ni),0.001f);

			if (tanOffset > maxFriction)
				tanOffset -= maxFriction;
			else if (tanOffset < -maxFriction)
				tanOffset += maxFriction;
			else
				continue;
			
			tanOffset *= ROPEANCHORMOVINGSPEED;

			if (tanOffset > 0.0)//可以模拟带倒刺的缝合线
			{
				tanOffset *= 1.2f;
			}

            Real currTanDist = 0.0f;

			m_AttachedRope->RelativePos2CurrLen(faceropeanchor->m_NodeIndex, faceropeanchor->m_ThreadWeights[0], currTanDist);
			
            currTanDist += tanOffset;

            //Real TotalThreadLen = faceropeanchor->m_thread->GetTotalLen(false);

            //GPClamp(currTanDist, 0.0f, TotalThreadLen);

            int nodeindex = -1;
            Real weight = 0.0f;

            m_AttachedRope->CurrLen2RelativePos(currTanDist, nodeindex, weight);

			faceropeanchor->m_NodeIndex = nodeindex;
			faceropeanchor->m_ThreadWeights[0] = weight;
			faceropeanchor->m_ThreadWeights[1] = 1.0f - weight;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::NormalizeFaceNeedleAnchor()
{	
	return;
    if (m_FaceNeedleAnchors.size() > 1)
    {
        for (GFPhysVectorObj<FaceAnchorInfo>::iterator iter = m_FaceNeedleAnchors.begin();
            iter != m_FaceNeedleAnchors.end();)    
        {
            FaceAnchorInfo& anchorinfo = (*iter);
            GFPhysVectorObj<FaceAnchorInfo>::iterator nextIter = ++iter;
            if (nextIter == m_FaceNeedleAnchors.end())
            {
                break;
            }
            FaceAnchorInfo nextanchorinfo = (*nextIter);

            if (anchorinfo.pAnchor->m_Type == nextanchorinfo.pAnchor->m_Type)
            {
                for (std::vector<GFPhysSoftBodyFace*>::iterator pos = anchorinfo.pFacesVector.begin();
                    pos != anchorinfo.pFacesVector.end();
                    ++pos)
                {
                    GFPhysSoftBodyFace* face = (*pos);
                    face->EnableCollideWithRigid();
                }                      
                
                delete anchorinfo.pAnchor;
                anchorinfo.pAnchor = 0;

                iter = m_FaceNeedleAnchors.erase(--iter);         
            }
            else
            {
                ++iter;
            }
        }
    }    
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::RemoveHeadAnchor()
{    
	if (m_FaceNeedleAnchors.size() >= 2)
	{
		for (int j = 0, nj = m_FaceNeedleAnchors.size(); j < nj-1; j++)
		{
			if (m_FaceNeedleAnchors[j].pAnchor->m_BHeadOut && m_FaceNeedleAnchors[j].pAnchor->m_Type == State_IN)//delete in -->delete out together.
			{
				if (m_FaceNeedleAnchors[j + 1].pAnchor->m_BHeadOut == false && m_FaceNeedleAnchors[j + 1].pAnchor->m_Type == State_OUT)
				{
					m_FaceNeedleAnchors[j + 1].pAnchor->m_BHeadOut = true;
				}				
			}
		}
	}
	

    for (GFPhysVectorObj<FaceAnchorInfo>::iterator iter = m_FaceNeedleAnchors.begin();
        iter != m_FaceNeedleAnchors.end();)    
    {
        //如何删除约束
        FaceAnchorInfo& anchorinfo = (*iter);

		if (anchorinfo.pAnchor->m_BHeadOut)
		{
			if (anchorinfo.pAnchor->m_Type == State_IN)
			{
				notifyRemoveInAnchor(anchorinfo.pAnchor->GetFace(), anchorinfo.pAnchor->m_weights);
			}
			else if (anchorinfo.pAnchor->m_Type == State_OUT)
			{
				notifyRemoveOutAnchor(anchorinfo.pAnchor->GetFace(), anchorinfo.pAnchor->m_weights);
			}

			for (std::vector<GFPhysSoftBodyFace*>::iterator pos = anchorinfo.pFacesVector.begin();
				pos != anchorinfo.pFacesVector.end();
				++pos)
			{
				m_NeedleAnchorDisableFaces.erase(*pos);
			}

            delete anchorinfo.pAnchor;
            anchorinfo.pAnchor = 0;

            iter = m_FaceNeedleAnchors.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::RemoveTailAnchor()
{    
	//////////////////////////////////////////////////////////////////////////
	//在这里处理out锚点先转换到线上造成的扭曲的情况
	//for (int i = 0, ni = m_FaceNeedleAnchors.size(); i < ni; i++)
	//{
	//	if (m_FaceNeedleAnchors[i].pAnchor->m_BTailOut)
	//	{
	//		for (int j = 0; j < i; j++)
	//		{
	//			m_FaceNeedleAnchors[j].pAnchor->m_BTailOut = true;
	//		}
	//	}
	//}

	//减少一次循环
	if (m_FaceNeedleAnchors.size() >= 2)
	{
		for (int j = m_FaceNeedleAnchors.size()-1; j >= 1; j--)
		{
			if (m_FaceNeedleAnchors[j].pAnchor->m_BTailOut )
			{
				m_FaceNeedleAnchors[j - 1].pAnchor->m_BTailOut = true;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

    for (GFPhysVectorObj<FaceAnchorInfo>::iterator iter = m_FaceNeedleAnchors.begin();
        iter != m_FaceNeedleAnchors.end();)
    {
        //如何删除约束
        FaceAnchorInfo& anchorinfo = (*iter);

		if (anchorinfo.pAnchor->m_BTailOut)
        {
			if (anchorinfo.pAnchor->m_Type == State_IN || anchorinfo.pAnchor->m_Type == State_OUT)
            {
                notifySwitchAnchor(anchorinfo.pAnchor->GetFace(), anchorinfo.pAnchor->m_weights);
            }
            
            if (m_AttachedRope)
            {
				CreateRopeAnchor(anchorinfo.pAnchor->m_Type, anchorinfo.pAnchor->GetFace(), anchorinfo.pAnchor->m_weights, anchorinfo.pAnchor->m_SuperfluousOffset);
            }        

			for (std::vector<GFPhysSoftBodyFace*>::iterator pos = anchorinfo.pFacesVector.begin();
				pos != anchorinfo.pFacesVector.end();
				++pos)
			{
				m_NeedleAnchorDisableFaces.erase(*pos);
			}

            delete anchorinfo.pAnchor;
            anchorinfo.pAnchor = 0;

            iter = m_FaceNeedleAnchors.erase(iter);            
        }
        else
        {
              ++iter;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::CreateNeedleAnchor( const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints )
{    
    if(RSContactConstraints.size() > 0 && m_NeedlePhysBody)
    {
        for (size_t c = 0, nc = RSContactConstraints.size(); c < nc; c++)
        {            
            GFPhysSoftFaceRigidContact srContact = RSContactConstraints[c];
            if (srContact.m_Rigid != m_NeedlePhysBody)
            {
                continue;
            }
            if (srContact.m_SoftFace->m_Nodes[0]->m_InvM > 0.0f
             && srContact.m_SoftFace->m_Nodes[1]->m_InvM > 0.0f
             && srContact.m_SoftFace->m_Nodes[2]->m_InvM > 0.0f)
            {
                //////////////////////////////////////////////////////////////////////////
				/*bool puncture = false;
				Real weight;
				GFPhysVector3 instersectpt;
				Real TriangleWeight[3];
				Real ration;
				bool succed = false;
				int end = SNeedleNodeNum - 3;//(int)(NeedleNodeNum * 0.5f);
				for (int i = SNeedleNodeNum - 1; i > end; --i)
				{
					succed = LineIntersectTriangle(
						srContact.m_SoftFace->m_Nodes[0]->m_CurrPosition,
						srContact.m_SoftFace->m_Nodes[1]->m_CurrPosition,
						srContact.m_SoftFace->m_Nodes[2]->m_CurrPosition,
						m_NeedleNodeWorldPos[i - 1],
						m_NeedleNodeWorldPos[i],
						weight,
						instersectpt,
						TriangleWeight);
					if (succed && weight >= 0 && weight <= 1)
					{
						AnchorPos intersectPos = { i - 1, weight };

						ration = AnchorPos2Ratio(intersectPos);
						if (ration < (1.0f - 1.0f / 20.0f))
						{
							puncture = true;
							break;
						}
					}
				}
				bool newcriterion = puncture;*/
				//////////////////////////////////////////////////////////////////////////

				Real contactImpluse = srContact.GetNormalImpluse(0) + srContact.GetNormalImpluse(1) + srContact.GetNormalImpluse(2);
				GFPhysVector3 dist = srContact.m_PointOnRigidLocal - m_NeedleNodeLocalPos[SNeedleNodeNum - 1];
				GFPhysVector3 NeedleDirWorld = GetNeedleDirction();
				Real punctureAngle = NeedleDirWorld.Dot(srContact.m_NormOnRigidWorld);// m_SoftFace->m_FaceNormal);
				bool oldcriterion = dist.Length() < _minDis && contactImpluse * punctureAngle > 0.5f;
				
				GFPhysFaceNeedleAnchor* CurrFaceNeedleAnchor = NULL;
				if (oldcriterion)
				{
					MisMedicOrgan_Ordinary* NeedleTestOrgan = (MisMedicOrgan_Ordinary*)srContact.m_SoftBody->GetUserPointer();

					GFPhysVector3 pos = srContact.m_SoftFace->m_Nodes[0]->m_CurrPosition * srContact.m_FaceWeights[0] +
						srContact.m_SoftFace->m_Nodes[1]->m_CurrPosition * srContact.m_FaceWeights[1] +
						srContact.m_SoftFace->m_Nodes[2]->m_CurrPosition * srContact.m_FaceWeights[2];

					if (m_FaceNeedleAnchors.empty() /*&& punctureAngle > 0.15f*/)
					{
						CurrFaceNeedleAnchor = new GFPhysFaceNeedleAnchor(&srContact.m_PointOnRigidLocal, srContact.m_SoftFace, srContact.m_FaceWeights, this, State_IN);
					}
					else if (!m_FaceNeedleAnchors.empty())
					{
						FaceAnchorInfo& lastAnchor = m_FaceNeedleAnchors[m_FaceNeedleAnchors.size() - 1];
						if (lastAnchor.pAnchor->m_Type == State_OUT/* && punctureAngle > 0.15f*/)
						{
							CurrFaceNeedleAnchor = new GFPhysFaceNeedleAnchor(&srContact.m_PointOnRigidLocal, srContact.m_SoftFace, srContact.m_FaceWeights, this, State_IN);
						}
						else if (lastAnchor.pAnchor->m_Type == State_IN /*&& punctureAngle > 0.0f*/)
						{
							AnchorPos lastInAnchorpos = lastAnchor.pAnchor->GetLastPos();
							Real LastInPos = lastInAnchorpos.segment + lastInAnchorpos.lambda;							

							if (LastInPos < SNeedleNodeNum - 2.0f && lastAnchor.POrgan == NeedleTestOrgan)
							{
								CurrFaceNeedleAnchor = new GFPhysFaceNeedleAnchor(&srContact.m_PointOnRigidLocal, srContact.m_SoftFace, srContact.m_FaceWeights, this, State_OUT);
							}
						}
					}

					if (CurrFaceNeedleAnchor)
					{
						std::vector<GFPhysSoftBodyFace*> aroundfaces;
						NeedleTestOrgan->CollectFacesAroundPoint(aroundfaces, pos, m_CollideRadius * 5.0f, true);

						m_NeedleAnchorOrgans.insert(NeedleTestOrgan);
						for (int i = 0, ni = aroundfaces.size(); i < ni; i++)
						{
							m_NeedleAnchorDisableFaces.insert(aroundfaces[i]);
						}

						FaceAnchorInfo currFaceAnchor = { NeedleTestOrgan, CurrFaceNeedleAnchor, aroundfaces };
						m_FaceNeedleAnchors.push_back(currFaceAnchor);

						if (CurrFaceNeedleAnchor->m_Type == State_IN)
						{
							notifyCreateInAnchor(srContact.m_SoftFace, srContact.m_FaceWeights);
						}
						else if (CurrFaceNeedleAnchor->m_Type == State_OUT)
						{
							notifyCreateOutAnchor(srContact.m_SoftFace, srContact.m_FaceWeights);
						}

						CurrFaceNeedleAnchor = NULL;
					}
				}
			}
		}
    }
}
//////////////////////////////////////////////////////////////////////////
Real SutureNeedle::AnchorPos2Ratio(const AnchorPos& pos)
{       
    //input filter missing
    assert(pos.lambda < 1.0f && pos.lambda >= 0.0f);
    if (pos.segment >= SNeedleNodeNum -1 && pos.lambda > 0.0f)
    {
        return 1.0f;
    }
    if (pos.segment < 0 || pos.lambda < 0.0f)
    {
        return 0.0f;
    }

    Real partsum = 0.0f;
    for (int i = 0; i < pos.segment; i++)
    {
        partsum +=  (m_NeedleNodeLocalPos[i+1] - m_NeedleNodeLocalPos[i]).Length();
    }
    if (pos.lambda > GP_EPSILON)
    {
        partsum += (pos.lambda)*(m_NeedleNodeLocalPos[pos.segment+1]-m_NeedleNodeLocalPos[pos.segment]).Length();
    }

    return partsum/m_NeedleTotalLen;
}
//////////////////////////////////////////////////////////////////////////
AnchorPos SutureNeedle::Ratio2AnchorPos(const Real& ratio)
{
    //input filter missing
    GPClamped(ratio,0.0f,1.0f);
    int segment = 0;
    Real lambda = 0.0f;

    if (fabsf(ratio) < GP_EPSILON)
    {
        AnchorPos pos = {segment,lambda}; 
        return pos;
    }

    Real partsum = 0.0f;
    for (int i = 0, ni = (int)m_NeedleNodeLocalPos.size() - 1; i < ni; ++i)
    {
        Real seglen = (m_NeedleNodeLocalPos[i+1]-m_NeedleNodeLocalPos[i]).Length();
        partsum += seglen ;
        Real k = partsum/m_NeedleTotalLen;
        if (k > ratio)
        {
            segment = i;
            lambda = 1-(partsum - ratio*m_NeedleTotalLen)/seglen;
            lambda = GPClamped(lambda,0.0f,1.0f);
            break;
        }
        if (fabsf(k - ratio) < GP_EPSILON)
        {
            segment = i+1;
            lambda = 0.0f;
            break;
        }
    }

    AnchorPos pos = {segment,lambda}; 
    return pos;
}
//////////////////////////////////////////////////////////////////////////
Real SutureNeedle::ConvertRealDistToRatioDist(Real realdist)
{
	return realdist / m_NeedleTotalLen;
}
//////////////////////////////////////////////////////////////////////////
GFPhysVector3 SutureNeedle::GetNeedleSegmentTangent (int segment , float lambda)
{
	GFPhysVector3 tanget;

	if(segment >= 1 && segment < (int)m_NeedleNodeWorldPos.size()-1)
	{
	   tanget = m_NeedleNodeWorldPos[segment+1] - m_NeedleNodeWorldPos[segment-1];
	}
	else if(segment == 0)
	{
	   tanget = m_NeedleNodeWorldPos[1] - m_NeedleNodeWorldPos[0];
	}
	else if(segment == (int)m_NeedleNodeWorldPos.size()-1)
	{
	   tanget = m_NeedleNodeWorldPos[(int)m_NeedleNodeWorldPos.size()-1] - m_NeedleNodeWorldPos[(int)m_NeedleNodeWorldPos.size()-2];	
	}
	else
	{
	   tanget = GFPhysVector3(1,0,0);//error output log
	}
	return tanget.Normalized();
}
//////////////////////////////////////////////////////////////////////////
AnchorPos SutureNeedle::CalcNewNeedlePos( const AnchorPos& pos1,const AnchorPos& pos2,const Real& friction )
{
    Real t1 = AnchorPos2Ratio(pos1);
    Real t2 = AnchorPos2Ratio(pos2);
    Real t = t1 * friction + t2 * ( 1.0f - friction );
    return Ratio2AnchorPos(t);
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::CreateRopeAnchor(AnchorType type, GFPhysSoftBodyFace* face, Real * weights, Real offset)
{
	Real tanOffset = offset;

	tanOffset *= ROPEANCHORSWITCHSPEED;

	Real TotalThreadLen = GetSutureThread()->GetTotalLen(false);

	GPClamp(tanOffset, 0.0f, TotalThreadLen * 0.1f);

	int nodeindex = 0;
	Real weight = 0.0f;

	GetSutureThread()->CurrLen2RelativePos(tanOffset, nodeindex, weight);

	Real threadweights[2];
	threadweights[0] = weight;
	threadweights[1] = 1.0f - weight;

	if (m_FaceRopeAnchors.size() > 0)
	{
		int nodeindexLast = m_FaceRopeAnchors[m_FaceRopeAnchors.size() - 1]->m_NodeIndex;
		Real weightLast = m_FaceRopeAnchors[m_FaceRopeAnchors.size() - 1]->m_ThreadWeights[1];

		if (nodeindex + threadweights[1] > nodeindexLast + weightLast)
		{
			nodeindex = nodeindexLast;
			threadweights[1] = weightLast-GP_EPSILON;
			threadweights[0] = 1 - threadweights[1];
		}
	}

	GFPhysFaceRopeAnchor* CurrFaceRopeAnchor = new GFPhysFaceRopeAnchor(face, weights, m_AttachedRope, nodeindex, threadweights);

	CurrFaceRopeAnchor->m_type = type;
	m_FaceRopeAnchors.push_back(CurrFaceRopeAnchor);
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::RemoveThreadAnchor()
{
	if( GetSutureThread() )
	{		
        if (m_FaceRopeAnchors.size() > 0)
        {
            int num = GetSutureThread()->GetNumSegments();
            SutureRopeNode& n0 = GetSutureThread()->GetThreadNodeRef(num);
            GFPhysVector3 threadEndPos = n0.m_CurrPosition;

            for (GFPhysVectorObj<GFPhysFaceRopeAnchor*>::iterator iter = m_FaceRopeAnchors.begin();
                iter != m_FaceRopeAnchors.end();)
            {
                //如何删除约束
                GFPhysFaceRopeAnchor* ropeAnchor = (*iter);
				
				Real currpos = ropeAnchor->GetSegPos();
				if ((fabsf(currpos - num) < 0.25f))
                {
					notifyRemoveRopeAnchor(ropeAnchor->GetFace(), ropeAnchor->m_weights);
					delete ropeAnchor;
					ropeAnchor = 0;
					iter = m_FaceRopeAnchors.erase(iter);
                }
                else
                {
                    ++iter;
                }
            }
        }
	}		
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::addNeedleActionListener( NeedleActionListener* listener )
{
    for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
    {
        if(m_Listeners[i] == listener)
            return;
    }
    m_Listeners.push_back(listener);
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::removeNeedleActionListener(NeedleActionListener * listener)
{
    for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
    {
        if(m_Listeners[i] == listener)
        {
            m_Listeners.erase(m_Listeners.begin()+i);
            return;
        }
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::removeAllNeedleActionListener()
{
    while(m_Listeners.begin() != m_Listeners.end())
    {
        m_Listeners.erase(m_Listeners.begin());
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::notifyCreateInAnchor(const GFPhysSoftBodyFace* face,const Real weights[])
{
    for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
    {
        m_Listeners[i]->OnCreateInAnchor(face,weights);
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::notifyCreateOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[])
{
    for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
    {
        m_Listeners[i]->OnCreateOutAnchor(face,weights);
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::notifyRemoveInAnchor(const GFPhysSoftBodyFace* face,const Real weights[])
{
    for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
    {
        m_Listeners[i]->OnRemoveInAnchor(face,weights);
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::notifyRemoveOutAnchor(const GFPhysSoftBodyFace* face,const Real weights[])
{
    for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
    {
        m_Listeners[i]->OnRemoveOutAnchor(face,weights);
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::notifySwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
    for (size_t i = 0; i < m_Listeners.size(); i++)
    {
        m_Listeners[i]->OnSwitchAnchor(face, weights);
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::notifyMovingRopeAnchor(const SutureThread * thread, const int index, const Real weights[])
{
    for (size_t i = 0; i < m_Listeners.size(); i++)
    {
        m_Listeners[i]->OnMovingRopeAnchor(thread,index, weights);
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::notifyRemoveRopeAnchor( const GFPhysSoftBodyFace* face,const Real weights[] )
{
    for(size_t i = 0; i < m_Listeners.size(); i++)
    {
        m_Listeners[i]->OnRemoveRopeAnchor(face,weights);            
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::RendGreen(int begin, int end)
{
    std::vector<Ogre::Vector3> centerPosition;
#if 0
    for (int v = begin; v < end; v++)
    {
        GFPhysTransform childTrans = m_NeedleCollideShape->GetComponentTransform(v);
        GFPhysTransform fullTrans = m_NeedlePhysBody->GetWorldTransform() * childTrans;

        GFPhysVector3   pos = fullTrans(GFPhysVector3(0, 0, 0));
        centerPosition.push_back(GPVec3ToOgre(pos));
    }
#else
    if (m_FaceNeedleAnchors.size() == 0)
    {
        for (int v = begin; v < end; v++)
        {
            centerPosition.push_back(GPVec3ToOgre(m_NeedleNodeWorldPos[v]));
        }
    }    
#endif
    m_RendObject.UpdateRendSegment(centerPosition, m_CollideRadius * 1.25f, "green");

}
//////////////////////////////////////////////////////////////////////////
GFPhysVector3 SutureNeedle::GetForceFeedBack()
{
	GFPhysVector3 totalForce = GFPhysVector3(0, 0, 0);
	for (int c = 0; c < (int)m_FaceNeedleAnchors.size(); c++)
	{
		totalForce += m_FaceNeedleAnchors[c].pAnchor->m_ForceFeedBack;
	}
	return totalForce;
}
//////////////////////////////////////////////////////////////////////////
bool SutureNeedle::Getinterval(const GFPhysVector3& pos, int& index0, int& index1)
{
    Real minlen[2] = { FLT_MAX, FLT_MAX };
    Real len = 0.0f;
    
    for (int j = 0; j < 2; ++j)
    {
        for (int i = 0; i < SNeedleNodeNum; ++i)
        {
            if (j == 0)
            {
                len = (pos - m_NeedleNodeWorldPos[i]).Length();
                if (len < minlen[0])
                {
                    minlen[0] = len;
                    index0 = i;
                }
            }
            else
            {
                if (i != index0)
                {
                    len = (pos - m_NeedleNodeWorldPos[i]).Length();
                    if (len < minlen[1])
                    {
                        minlen[1] = len;
                        index1 = i;
                    }
                }
            }
        }
    }
    if (abs(index0 - index1) == 1 && index0 < SNeedleNodeNum && index1 < SNeedleNodeNum)
    {
        return true;
    }
    else
    {
        return false;
    }
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedle::Disappear()
{
    //////////////////////////////////////////////////////////////////////////
    //if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
    //{
    //    PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
    //}

	m_AttchPoints.clear();

    //free attached mesh
    if (m_pNeedleSceneNode)
    {
        Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();		
        pSceneManager->getRootSceneNode()->removeAndDestroyChild(m_pNeedleSceneNode->getName());
		m_pNeedleSceneNode = 0;
    }

    //////////////////////////////////////////////////////////////////////////
    if (!m_FaceNeedleAnchors.empty())
    {
        for (size_t i = 0; i < m_FaceNeedleAnchors.size(); ++i)
        {
            delete m_FaceNeedleAnchors[i].pAnchor;
            m_FaceNeedleAnchors[i].pAnchor = 0;
        }
    }

    //m_CurrFaceNeedleAnchor = 0;

    //need destroy collision shape and rigid body
    if (m_NeedleCollideShape)
    {
        for (int c = 0; c < m_NeedleCollideShape->GetNumComponent(); c++)
        {
            GFPhysCollideShape * subShape = m_NeedleCollideShape->GetComponentShape(c);
            delete subShape;
        }
        delete m_NeedleCollideShape;
        m_NeedleCollideShape = 0;
    }
    if (m_NeedlePhysBody)
    {
        PhysicsWrapper::GetSingleTon().DestoryRigidBody(m_NeedlePhysBody);
        m_NeedlePhysBody = 0;
    }

    removeAllNeedleActionListener();

    GFPhysVectorObj<KnotInSutureRope> Knots;
    GetSutureThread()->m_KnotsInThread->GetDeadKnots(Knots);
    if (Knots.size() == 2)
    {
        int min = Knots[1].m_knotcon0.m_A;
        int max = Knots[1].m_knotcon1.m_B;
        for (int i = 0; i < min - 2; i++)
        {
            SutureRopeNode& node = m_AttachedRope->GetThreadNodeRef(i);
            node.Disappear();
        }
        for (int i = max + 2; i < m_AttachedRope->GetNumThreadNodes(); i++)
        {
            SutureRopeNode& node = m_AttachedRope->GetThreadNodeRef(i);
            node.Disappear();
        }
    }
}