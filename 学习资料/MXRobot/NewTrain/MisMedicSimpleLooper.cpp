#include "MisMedicSimpleLooper.h"
#include "MisNewTraining.h"
#include "Dynamic\Constraint\GoPhysSoftBodyDistConstraint.h"
#include "collision\NarrowPhase\GoPhysSoftbodyFaceConvexCollision.h"
#include "math\GoPhysTransformUtil.h"
#include "Math\GoPhysSIMDMath.h"
#include "Math\GoPhysMathUtil.h"
#include "CustomCollision.h"
#include "MisMedicOrganOrdinary.h"
#include "PhysicsWrapper.h"
#include "TrainingMgr.h"
#include "IObjDefine.h"
#include "MXOgreGraphic.h"
#include "Instruments/Tool.h"
#include "MXEventsDump.h"



class LoopSegSoftFaceCallback : public GFPhysNodeOverlapCallback
{
public:
	LoopSegSoftFaceCallback( float Margin ,
							 GFPhysSoftBody * sb ,
							 float collideradius , 
							 MisMedicSimpleLooper * looperObject , 
							 GFPhysAlignedVectorObj<CollideWithSoftPair> & paircd
						    ) : m_sb(sb),
								m_collideRadius(collideradius+Margin),
								m_LooperObject(looperObject),
								m_CollidePairs(paircd)

	{
		m_InvLoopTrans = m_LooperObject->GetWorldTransform().Inverse();
	}
	virtual ~LoopSegSoftFaceCallback()
	{}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodes , GFPhysAABBNode * staticnode)
	{}

	virtual void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
	{
		GFPhysSoftBodyFace * softFace = (GFPhysSoftBodyFace*)dynNodeA->m_UserData;		
		int SegIndex = (int)dynNodeB->m_UserData;

		GFPhysVector3 SoftFaceVerts[3];
		SoftFaceVerts[0] = softFace->m_Nodes[0]->m_CurrPosition;
		SoftFaceVerts[1] = softFace->m_Nodes[1]->m_CurrPosition;
		SoftFaceVerts[2] = softFace->m_Nodes[2]->m_CurrPosition;

		GFPhysVector3 segmentPoints[2];
		segmentPoints[0] = m_LooperObject->m_LooperNodeWorldPos[SegIndex];
		segmentPoints[1] = m_LooperObject->m_LooperNodeWorldPos[SegIndex+1];


		float segmentRadius = m_LooperObject->GetCollideRadius() + m_LooperObject->m_Margin;

		GFPhysTransform identyTrans;
		identyTrans.SetIdentity();

		GFPhysVector3 closetPointFace;
		
		GFPhysVector3 closetPointSeg;
		

		GFPhysGJKCollideDetector::ClosestPointInput input;

		GFPhysVSimplexCloseCalculator VSimplexSolver;

		GFPhysGjkEpaPDCalculor PDSolver;

		GFPhysTriangleShape triShape(SoftFaceVerts[0] , SoftFaceVerts[1] , SoftFaceVerts[2]);
		triShape.SetMargin(0.0f);

		float segRealMargin = segmentRadius;
		
		float segThickness  = segmentRadius;

		GFPhysLineSegmentShape segShape(segmentPoints[0] , segmentPoints[1]);
		segShape.SetMargin(segThickness + segRealMargin);

		GFPhysGJKCollideDetector gjkPairDetector(&segShape , &triShape , &VSimplexSolver,&PDSolver);

		gjkPairDetector.SetMinkowskiA(&segShape);

		gjkPairDetector.SetMinkowskiB(&triShape);

		input.m_maximumDistanceSquared = segShape.GetMargin() + triShape.GetMargin() + 0.02f;//1.0f;

		input.m_maximumDistanceSquared *= input.m_maximumDistanceSquared;

		input.m_transformA.SetIdentity();

		input.m_transformB.SetIdentity();

		MyTriSegClosetResult cdResult;

		gjkPairDetector.GetClosestPoints(false , input , cdResult);

		if(cdResult.m_Valid && cdResult.m_Depth < 0)
		{
			GFPhysVector3 pointFace = cdResult.m_PointOnB;
			
			GFPhysVector3 pointSeg  = pointFace + cdResult.m_NormalOnB*cdResult.m_Depth;
			pointSeg += cdResult.m_NormalOnB * segRealMargin;//minus segment real margin to segment point with thickness

			CollideWithSoftPair collidePair(m_sb , softFace);
			collidePair.m_CollideNormalOnFace = cdResult.m_NormalOnB;
			
			collidePair.m_LocalPos = m_InvLoopTrans(pointSeg);
			
			collidePair.m_Segment = SegIndex;

			collidePair.m_Depth = cdResult.m_Depth;

			CalcBaryCentric(SoftFaceVerts[0] , 
							SoftFaceVerts[1] , 
							SoftFaceVerts[2] , 
							pointFace , 
							collidePair.m_FaceWeights[0] , 
							collidePair.m_FaceWeights[1] ,
							collidePair.m_FaceWeights[2]);

			m_CollidePairs.push_back(collidePair);
		}
	
	}

	GFPhysSoftBody * m_sb;
	MisMedicSimpleLooper * m_LooperObject;
	GFPhysTransform m_InvLoopTrans;

	GFPhysAlignedVectorObj<CollideWithSoftPair> & m_CollidePairs;
	float m_collideRadius;
};

//==========================================================================
MisMedicSimpleLooper::MisMedicSimpleLooper(Ogre::SceneManager * sceneMgr  , MisNewTraining * ownertrain) : MisMedicThreadRope(sceneMgr , ownertrain)
{
	m_IsLoopBinded = false;
}
//===================================================================================================
MisMedicSimpleLooper::~MisMedicSimpleLooper()
{
	DestoryRope();
}
//===================================================================================================
GFPhysMatrix3 MisMedicSimpleLooper::CalcLocalInertiaTensorAndCom(float thickness)
{
	GFPhysMatrix3  InertialTensor;
	InertialTensor.SetZero();

	GFPhysVector3 com(0,0,0);
	float SumMass = 0;
	for(size_t n = 0 ; n < m_LooperNodeRestPos.size() ; n++)
	{	
		com += m_LooperNodeRestPos[n] * m_LooperNodeMass[n];
		SumMass += m_LooperNodeMass[n];
	}

	if(SumMass > FLT_EPSILON)
	   com /= SumMass;

	for(size_t n = 0 ; n < m_LooperNodeRestPos.size() ; n++)
	{
		Real Mass = m_LooperNodeMass[n];

		GFPhysVector3 refPos =  m_LooperNodeRestPos[n] - com;
		
		//use 6 sample point to represent a node sphere in +- x ,y , z axis, this will prevent
		//flat configuration which make inertia tensor no invertible

		Mass /= 6.0f;

		for(int t = 0 ; t < 3 ; t++)
		{
			GFPhysVector3 axis(0,0,0);
			axis[t] = 1.0f;
			
			for(int c = 0 ; c < 2 ; c++)
			{
				GFPhysVector3 r;//samplePos
			
				if(c == 0)
				   r = refPos + axis * thickness;
				else
				   r = refPos - axis * thickness;

				InertialTensor[0][0] += Mass*(r.m_z * r.m_z + r.m_y * r.m_y);
				InertialTensor[0][1] += Mass*(-r.m_y * r.m_x);
				InertialTensor[0][2] += Mass*(-r.m_z * r.m_x) ;

				InertialTensor[1][0] += Mass*(-r.m_y * r.m_x);
				InertialTensor[1][1] += Mass*(r.m_z * r.m_z + r.m_x * r.m_x);
				InertialTensor[1][2] += Mass*(-r.m_z * r.m_y) ;

				InertialTensor[2][0] += Mass*(-r.m_z * r.m_x);
				InertialTensor[2][1] += Mass*(-r.m_z * r.m_y);
				InertialTensor[2][2] += Mass*(r.m_y * r.m_y + r.m_x * r.m_x);
			}
		}
	}
	
	m_LoopMass = SumMass;
	
	m_LocalLoopCom  = com;
	
	m_LocalInvInertial = InertialTensor.Inverse();

	return m_LocalInvInertial;
}
//=========================================================================================================
void MisMedicSimpleLooper::UpdateCollisionData()
{
	//update tree
	m_SegmentBVTree.Clear();//clear first
	for(int n = 0 ; n < m_LooperNodeRestPos.size()-1 ; n++)
	{
		GFPhysVector3 posn0 = m_LooperNodeWorldPos[n];
		GFPhysVector3 posn1 = m_LooperNodeWorldPos[n+1];

		GFPhysVector3 minPos = posn0;
		GFPhysVector3 maxPos = posn0;

		minPos.SetMin(posn1);
		maxPos.SetMax(posn1);

		GFPhysVector3 span(m_RopeCollideRadius+m_Margin,m_RopeCollideRadius+m_Margin,m_RopeCollideRadius+m_Margin);

		GFPhysDBVNode * bvNode = m_SegmentBVTree.InsertAABBNode(minPos-span , maxPos+span);
		bvNode->m_UserData = (void*)n;
	}
}
//===============================================================================================================
void MisMedicSimpleLooper::CheckCollisionWithOrgan()
{
	m_CollidePairs.clear();
	
	if(m_IsLoopBinded)
	   return;
	//check collision
	ITraining * itrain = CTrainingMgr::Instance()->GetCurTraining();
	MisNewTraining * newTrain = dynamic_cast<MisNewTraining * >(itrain);

	std::vector<MisMedicOrganInterface*> organsInTrain;
	newTrain->GetAllOrgan(organsInTrain);

	//check collision with soft organ
	for(size_t c = 0 ; c < organsInTrain.size() ; c++)
	{
		MisMedicOrganInterface * oif = organsInTrain[c];

		GFPhysSoftBody * physbody = 0;

		MisMedicOrgan_Ordinary * organOrdinary = dynamic_cast<MisMedicOrgan_Ordinary *>(oif);

		if(organOrdinary)
		{
		   physbody = organOrdinary->m_physbody;
		}

		if(physbody)
		{
			GFPhysVectorObj<GFPhysDBVTree*> bvTrees = physbody->GetSoftBodyShape().GetFaceBVTrees();

			LoopSegSoftFaceCallback collideCallBack(m_Margin , physbody , m_RopeCollideRadius , this , m_CollidePairs);

			for(size_t t = 0 ; t < bvTrees.size() ; t++)
			{
				GFPhysDBVTree * bvTree = bvTrees[t];
				bvTree->CollideWithDBVTree(m_SegmentBVTree , &collideCallBack);
			}
		}
	}
}
//=========================================================================================================
void MisMedicSimpleLooper::BuildLoopRestPosition(float radius)
{
	GFPhysVector3 circleCenter = m_LocalFixPoint + m_LoopDirectionLocal * radius;
	
	int NumSegment = m_LooperNodeRestPos.size()-1;

	for(int s = 0 ; s <= NumSegment ; s++)
	{
		float angle = 2 * 3.141592f * ((float)s / (float)NumSegment);

		GFPhysQuaternion rotate(m_LoopNormalLocal , angle);

		GFPhysVector3 localLoopPos = QuatRotate(rotate , -m_LoopDirectionLocal * radius);

		localLoopPos = localLoopPos + circleCenter;

		m_LooperNodeRestPos[s] = localLoopPos;
	}
}
//=========================================================================================================
void MisMedicSimpleLooper::CreateLoopedThread(const GFPhysVector3 & circleCenter,
											  const GFPhysVector3 & circelAxis,
											  const GFPhysVector3 & StartOffsetVec,
											  Ogre::SceneNode * AttachNode,
											  ObjPhysCatogry threadCat)
{
	DestoryRope();

	float thickness = 0.1f;

	int segmentCount = 20;

	SetRigidForceRange(segmentCount+1);

	m_RigidForceMagnitude = 0.5f;

	m_TorqueStiffness = 0.05f;

	m_DampingRate = 10.0f;

	m_Catergory = OPC_THREADLOOPRIGHTOOL;

	m_LoopNormalLocal    = circelAxis;
	
	m_LoopDirectionLocal = -StartOffsetVec;
	
	m_LoopDirectionLocal.Normalize();

	m_LocalFixPoint = circleCenter+StartOffsetVec;

	m_CircleRadius = StartOffsetVec.Length();

	m_ToolKernalNode = AttachNode;

	GFPhysVector3 looperCenter = circleCenter;
	
	m_LooperNodeRestPos.resize(segmentCount+1);

	m_LooperNodeMass.resize(segmentCount+1);
	
	m_LooperNodeWorldPos.resize(segmentCount+1);
	
	for(int s = 0 ; s <= segmentCount ; s++)
	{
		m_LooperNodeMass[s]	= 1.0f;
	}

	//Build Loop Rest Position
	BuildLoopRestPosition(m_CircleRadius);
	//

	m_LocalInvInertial = CalcLocalInertiaTensorAndCom(0.02f);

	m_LoopRigidTrans.SetIdentity();
	
	m_LoopRigidTrans.SetOrigin(m_LocalLoopCom);

	m_WorldInvInertial = m_LoopRigidTrans.GetBasis()* m_LocalInvInertial * m_LoopRigidTrans.GetBasis().Transpose();

	//shift transform to center of mass
	for(size_t t = 0 ; t <= segmentCount ; t++)
	{
		m_LooperNodeRestPos[t] -= m_LocalLoopCom;
	}

	m_LinearVelocity = m_AnularVelocity = GFPhysVector3(0,0,0);

	m_FixPoints.push_back(FixPointAnchor(GFPhysVector3(0,0,0) - m_LocalLoopCom));

	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

	m_TopoType = MisMedicThreadRope::TTT_LOOP;
}
//=========================================================================================================
void MisMedicSimpleLooper::Shrink(float percent)
{
	percent = 0.999f;//temp test

	if(m_CircleRadius < 0.7f)
	{
	   if(m_IsLoopBinded == false)
	   {
	      TryBind();
		  m_IsLoopBinded = true;
	   }
	   return;
	}

	GPClamp(percent , 0.99f , 0.99f);
	
	m_CircleRadius *= percent;
	
	//rebuild rest position in local frame
	BuildLoopRestPosition(m_CircleRadius);

	//recalculate new com & inertia tensor
	m_LocalInvInertial = CalcLocalInertiaTensorAndCom(0.02f);

	//shift loop local pos to center of mass
	for(int t = 0 ; t <= m_LooperNodeRestPos.size()-1 ; t++)
	{
		m_LooperNodeRestPos[t] -= m_LocalLoopCom;
	}

	//refresh fix point local pos
	if(m_FixPoints.size() > 0)
	{
	   m_FixPoints[0].m_LocalPoint = GFPhysVector3(0,0,0) - m_LocalLoopCom;
	}
}
//=========================================================================================================
bool MisMedicSimpleLooper::TryBind()
{
	CheckCollisionWithOrgan();

	for(size_t c = 0 ; c < m_CollidePairs.size() ; c++)
	{
		m_ConstraintedPairs.push_back(m_CollidePairs[c]);
	}
	m_CollidePairs.clear();

	//test
	CutBindedOrgan();
	m_ConstraintedPairs.clear();
	//

	return false;
}
//===========================================================================================================
void MisMedicSimpleLooper::CutBindedOrgan()
{	
	if(m_ConstraintedPairs.size() <= 0)
	   return;

	GFPhysSoftBody * sb =(GFPhysSoftBody*)m_ConstraintedPairs[0].m_CollideBody;
	
	ITraining * itrain = CTrainingMgr::Instance()->GetCurTraining();
	MisNewTraining * newTrain = dynamic_cast<MisNewTraining * >(itrain);

	std::vector<MisMedicOrganInterface*> organsInTrain;
	MisMedicOrgan_Ordinary * organSelected = dynamic_cast<MisMedicOrgan_Ordinary*>(newTrain->GetOrgan(sb));
	
	if(organSelected)
	{
		//build cut plane
		GFPhysVector3 cutCenter = m_LoopRigidTrans.GetOrigin();

		GFPhysVector3 dir1_L = m_LoopDirectionLocal.Cross(m_LoopNormalLocal).Normalized();

		GFPhysVector3 dir0_W = m_LoopRigidTrans.GetBasis() * m_LoopDirectionLocal;
		
		GFPhysVector3 dir1_W = m_LoopRigidTrans.GetBasis() * dir1_L;

		GFPhysVector3 quadVerts[4];

		float cutRadius = m_CircleRadius * 1.2f;

		quadVerts[0] = cutCenter - dir1_W*cutRadius - dir0_W*cutRadius;
		quadVerts[1] = cutCenter - dir1_W*cutRadius + dir0_W*cutRadius;
		
		quadVerts[2] = cutCenter + dir1_W*cutRadius - dir0_W*cutRadius;
		quadVerts[3] = cutCenter + dir1_W*cutRadius + dir0_W*cutRadius;


		organSelected->TearOrganBySemiInfinteQuad(quadVerts , true);
	}
}
//=========================================================================================================
void MisMedicSimpleLooper::BeginSimulateThreadPhysics(float dt)
{
	//update world  inertia tensor
	m_WorldInvInertial = m_LoopRigidTrans.GetBasis()* m_LocalInvInertial * m_LoopRigidTrans.GetBasis().Transpose();

	//update world node pos
	for(size_t n = 0 ; n < m_LooperNodeRestPos.size() ; n++)
	{
		m_LooperNodeWorldPos[n] = m_LoopRigidTrans(m_LooperNodeRestPos[n]);
	}

	UpdateCollisionData();

	CheckCollisionWithOrgan();

	float linearDamping  = 8.0f;

	float angularDamping = 8.0f;

	//m_LinearVelocity += GFPhysVector3(0,20.0 , 0)*dt;

	m_LinearVelocity  *= GPClamped((Real(1.) - dt * linearDamping), (Real)Real(0.0), (Real)Real(1.0));
	
	m_AnularVelocity  *= GPClamped((Real(1.) - dt * angularDamping), (Real)Real(0.0), (Real)Real(1.0));

	return;
}
//================================================================================================
void MisMedicSimpleLooper::EndSimuolateThreadPhysics(float dt)
{
	GFPhysTransform predictTransform;
	
	//integrate transform 
	GFPhysTransformUtil::IntegrateTransform(m_LoopRigidTrans , m_LinearVelocity , m_AnularVelocity , dt , predictTransform);
	m_LoopRigidTrans = predictTransform;
	
	

	for(size_t t = 0  ; t < m_CollidePairs.size() ; t++)
	{
		CollideWithSoftPair & tfPair = m_CollidePairs[t];
		GFPhysSoftBodyFace * softFace = tfPair.m_SoftFace;
		if(softFace)
		{
			softFace->m_Nodes[0]->m_InvM = softFace->m_Nodes[0]->m_OriginInvMass;
			softFace->m_Nodes[1]->m_InvM = softFace->m_Nodes[1]->m_OriginInvMass;
			softFace->m_Nodes[2]->m_InvM = softFace->m_Nodes[2]->m_OriginInvMass;
		}
	}
}
//================================================================================================
void MisMedicSimpleLooper::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	//
	for(size_t t = 0  ; t < m_CollidePairs.size() ; t++)
	{
		CollideWithSoftPair & tfPair = m_CollidePairs[t];

		GFPhysSoftBodyFace * softFace = tfPair.m_SoftFace;
		
		if(softFace)
		{
		   softFace->m_Nodes[0]->m_InvM = softFace->m_Nodes[0]->m_OriginInvMass*0.5f;
		    softFace->m_Nodes[1]->m_InvM = softFace->m_Nodes[1]->m_OriginInvMass*0.5f;
			softFace->m_Nodes[2]->m_InvM = softFace->m_Nodes[2]->m_OriginInvMass*0.5f;
		}
	}
}
//================================================================================================
void MisMedicSimpleLooper::SolveBend(float dt  , float stiffness)
{
	Ogre::Quaternion toolRot   = m_ToolKernalNode->_getDerivedOrientation();

	Ogre::Quaternion LoopCurrRot = GPQuaternionToOgre(m_LoopRigidTrans.GetRotation());

	Ogre::Quaternion NeedCorrQuat = toolRot * LoopCurrRot.Inverse();

	if(NeedCorrQuat.w < 0)//inverse rotate axis if rotate angle large than PI this ensure we get a minimal rotate
	{
		NeedCorrQuat.x *= -1.0f;
		NeedCorrQuat.y *= -1.0f;
		NeedCorrQuat.z *= -1.0f;
		NeedCorrQuat.w *= -1.0f;
	}

	Ogre::Vector3 rotAxis;
	Ogre::Radian  rotAngle;
	NeedCorrQuat.ToAngleAxis(rotAngle , rotAxis);

	Ogre::Vector3 DestAngularVel = rotAxis * rotAngle.valueRadians() / dt;

	GFPhysVector3 AngularCorrect = OgreToGPVec3(DestAngularVel) - m_AnularVelocity;

	m_AnularVelocity += AngularCorrect*stiffness;//0.1f;
}
//================================================================================================
void MisMedicSimpleLooper::SolveConstraint(Real Stiffness,Real TimeStep)
{
	Ogre::Vector3 toolPos    = m_ToolKernalNode->_getDerivedPosition();
	Ogre::Quaternion toolRot = m_ToolKernalNode->_getDerivedOrientation();
	
	GFPhysTransform transform;
	
	transform.SetOrigin(OgreToGPVec3(toolPos));
	transform.SetRotation(OgreToGPQuaternion(toolRot));

	SolveConstraintedPairs(TimeStep);

	SolveBend(TimeStep , 0.04);

	SolveFixPoints(transform , TimeStep);

	SolveSoftContact(TimeStep);
	
	
}
//============================================================================
void MisMedicSimpleLooper::SolveFixPoints(const GFPhysTransform & fixPointTrans , float dt)
{
	GFPhysVector3 fixPos = fixPointTrans.GetOrigin();

	float InvLoopMass = 1.0f / m_LoopMass;

	for(size_t c = 0 ; c < m_FixPoints.size() ; c++)
	{
		GFPhysVector3 wR = m_LoopRigidTrans.GetBasis() * m_FixPoints[c].m_LocalPoint;
		
		const GFPhysVector3 wa = m_LoopRigidTrans(m_FixPoints[c].m_LocalPoint);
		
		const GFPhysVector3	va = (m_LinearVelocity + m_AnularVelocity.Cross(wR)) * dt;

		GFPhysMatrix3 Impmat = GFPhysSoftBodyConstraint::ImpulseMatrix( dt,
																		0,
																		InvLoopMass,
																		m_WorldInvInertial,
																		wR);

		GFPhysVector3 DstCorect = (va + wa - fixPos) * 0.9f;

		GFPhysVector3 impluse   = (Impmat * DstCorect) * (-1.0f);

		m_LinearVelocity  += impluse * InvLoopMass;

		m_AnularVelocity  += m_WorldInvInertial * (wR.Cross(impluse));
	}
}
//============================================================================
void MisMedicSimpleLooper::SolveSoftContact(float dt)
{
	float InvLoopMass = 1.0f / m_LoopMass;

	for(size_t p = 0 ; p < m_CollidePairs.size() ; p++)
	{
#if(0)
		GFPhysSoftBodyFace * collideFace = m_CollidePairs[p].m_SoftFace;
		
		float * FaceWeight = m_CollidePairs[p].m_FaceWeights;
		
		GFPhysVector3 currFacePos = collideFace->m_Nodes[0]->m_CurrPosition * FaceWeight[0]
								   +collideFace->m_Nodes[1]->m_CurrPosition * FaceWeight[1]
								   +collideFace->m_Nodes[2]->m_CurrPosition * FaceWeight[2];

		 GFPhysVector3 wa = m_LoopRigidTrans(m_CollidePairs[p].m_LocalPos);

		 GFPhysVector3 wR = m_LoopRigidTrans.GetBasis()*(m_CollidePairs[p].m_LocalPos);

		 GFPhysVector3 va = (m_LinearVelocity + m_AnularVelocity.Cross(wR)) * dt;
			
		 GFPhysVector3	vr = (va + wa - currFacePos) * 0.98f;

		GFPhysVector3 NormalOnFace = m_CollidePairs[p].m_CollideNormalOnFace;

		Real vn = vr.Dot(NormalOnFace);

		if(vn < 0)
		{
			float faceInvMass = InvLoopMass * 10.0f;//temp

			GFPhysMatrix3 Impmat = GFPhysSoftBodyConstraint::ImpulseMatrix( dt,
																			faceInvMass,
																			InvLoopMass,
																			m_WorldInvInertial,
																			wR);
			GFPhysMatrix3 testMat = Impmat.Inverse();

			Impmat *= dt;

			Impmat = Impmat.Inverse(); 

			Impmat *= dt;

			float t = (Impmat * NormalOnFace).Dot(NormalOnFace);
			
			if(t > FLT_EPSILON)
			{
				float c = vn / t;

				GFPhysVector3 impluse = NormalOnFace * c;

				float corrected = (Impmat * impluse).Dot(NormalOnFace);
				//positive impluse on soft
				GFPhysVector3 faceCorrect = impluse * faceInvMass * dt;
				collideFace->m_Nodes[0]->m_CurrPosition += faceCorrect;
				collideFace->m_Nodes[1]->m_CurrPosition += faceCorrect;
				collideFace->m_Nodes[2]->m_CurrPosition += faceCorrect;
				
				//negative impluse on rigid
				m_LinearVelocity += (-impluse) * InvLoopMass;
				m_AnularVelocity += m_WorldInvInertial * (wR.Cross(-impluse));


				//test
				GFPhysVector3 dl = (-impluse) * InvLoopMass;;
				GFPhysVector3 da = m_WorldInvInertial * (wR.Cross(-impluse));

				GFPhysVector3 dr = (dl + da.Cross(wR));
				dr = dr*dt;
				float rigidcn = dr.Dot(NormalOnFace);
				float softcn = faceCorrect.Dot(NormalOnFace);
				int k = rigidcn +1;
			}

			GFPhysVector3 newFacePos = collideFace->m_Nodes[0]->m_CurrPosition * FaceWeight[0]
			+collideFace->m_Nodes[1]->m_CurrPosition * FaceWeight[1]
			+collideFace->m_Nodes[2]->m_CurrPosition * FaceWeight[2];
			
			wa = m_LoopRigidTrans(m_CollidePairs[p].m_LocalPos);

			wR = m_LoopRigidTrans.GetBasis()*(m_CollidePairs[p].m_LocalPos);

			va = (m_LinearVelocity + m_AnularVelocity.Cross(wR)) * dt;

			vr = (va + wa - newFacePos);

			NormalOnFace = m_CollidePairs[p].m_CollideNormalOnFace;

			vn = vr.Dot(NormalOnFace);

			int k = vn +1;
		}
#else
		GFPhysSoftBodyFace * collideFace = m_CollidePairs[p].m_SoftFace;

		float * FaceWeight = m_CollidePairs[p].m_FaceWeights;

		GFPhysVector3 NormalOnFace = m_CollidePairs[p].m_CollideNormalOnFace;

		//GFPhysVector3 currFacePos = collideFace->m_Nodes[0]->m_CurrPosition * FaceWeight[0]
		// +collideFace->m_Nodes[1]->m_CurrPosition * FaceWeight[1]
		//+collideFace->m_Nodes[2]->m_CurrPosition * FaceWeight[2];

		const GFPhysVector3 wa = m_LoopRigidTrans(m_CollidePairs[p].m_LocalPos);

		const GFPhysVector3 wR = m_LoopRigidTrans.GetBasis()*(m_CollidePairs[p].m_LocalPos);


		for(int nid = 0 ; nid < 3 ; nid++)
		{
			const GFPhysVector3 va = (m_LinearVelocity + m_AnularVelocity.Cross(wR)) * dt;

			const GFPhysVector3	vr = (va + wa - collideFace->m_Nodes[nid]->m_CurrPosition) * 0.98f;

			Real vn = vr.Dot(NormalOnFace);

			if(vn < 0)
			{
				float faceInvMass = InvLoopMass * 30.0f;//temp

				GFPhysMatrix3 loopinertiaMat;
				loopinertiaMat.SetZero();//disable rotate effect seems more good 
				
				GFPhysMatrix3 Impmat = GFPhysSoftBodyConstraint::ImpulseMatrix( dt,
																				faceInvMass,
																				InvLoopMass,
																				loopinertiaMat,
																				wR);

				Impmat = Impmat.Inverse(); //re - inverse to get origin matrix

				float t = (Impmat * NormalOnFace).Dot(NormalOnFace);

				if(t > FLT_EPSILON)
				{
					float c = vn / t;

					GFPhysVector3 impluse = NormalOnFace * c;

					//positive impluse on soft
					GFPhysVector3 faceCorrect = impluse * faceInvMass * dt;
					collideFace->m_Nodes[nid]->m_CurrPosition += faceCorrect;
					//collideFace->m_Nodes[1]->m_CurrPosition += faceCorrect;
					//collideFace->m_Nodes[2]->m_CurrPosition += faceCorrect;

					//negative impluse on rigid
					m_LinearVelocity += (-impluse) * InvLoopMass;
					m_AnularVelocity += loopinertiaMat * (wR.Cross(-impluse));
				}

			}		
		}
#endif
	}
}
//============================================================================
void MisMedicSimpleLooper::SolveConstraintedPairs(float dt)
{
	float InvLoopMass = 1.0f / m_LoopMass;
	
	for(size_t p = 0 ; p < m_ConstraintedPairs.size() ; p++)
	{
		CollideWithSoftPair & cpair = m_ConstraintedPairs[p];

		float NodeInvMass[3];

		NodeInvMass[0] = InvLoopMass * 30.0f;//temply use uniformed mass cpair.m_SoftFace->m_Nodes[0]->m_InvM;
		NodeInvMass[1] = InvLoopMass * 30.0f;//temply use uniformed mass cpair.m_SoftFace->m_Nodes[1]->m_InvM;
		NodeInvMass[2] = InvLoopMass * 30.0f;//temply use uniformed mass cpair.m_SoftFace->m_Nodes[2]->m_InvM;

		float faceInvMass = (cpair.m_FaceWeights[0] * cpair.m_FaceWeights[0] * NodeInvMass[0]
							+cpair.m_FaceWeights[1] * cpair.m_FaceWeights[1] * NodeInvMass[1]
							+cpair.m_FaceWeights[2] * cpair.m_FaceWeights[2] * NodeInvMass[2]);
		
		GFPhysVector3 facePoint = (cpair.m_SoftFace->m_Nodes[0]->m_CurrPosition * cpair.m_FaceWeights[0]
								  +cpair.m_SoftFace->m_Nodes[1]->m_CurrPosition * cpair.m_FaceWeights[1] 
								  +cpair.m_SoftFace->m_Nodes[2]->m_CurrPosition * cpair.m_FaceWeights[2]);

	    if(faceInvMass > FLT_EPSILON)
		{
			GFPhysVector3 wR = m_LoopRigidTrans.GetBasis()*(cpair.m_LocalPos);

			GFPhysVector3 wa = m_LoopRigidTrans(cpair.m_LocalPos);

			GFPhysVector3 va = (m_LinearVelocity + m_AnularVelocity.Cross(wR)) * dt;

			GFPhysVector3 vr = (va + wa - facePoint) * 0.9f;

			GFPhysMatrix3 loopinertiaMat = m_WorldInvInertial;

			GFPhysMatrix3 Impmat = GFPhysSoftBodyConstraint::ImpulseMatrix( dt,
																			faceInvMass,
																			InvLoopMass,
																			loopinertiaMat,
																			wR);

			GFPhysVector3 impluse = Impmat * vr;

			//apply impluse to soft face node
			for(int n = 0 ; n < 3 ; n++)
			{
				GFPhysVector3 PosCorrect = (impluse * cpair.m_FaceWeights[n] * NodeInvMass[n]) * dt;
				cpair.m_SoftFace->m_Nodes[n]->m_CurrPosition += PosCorrect;
			}
	
			//apply impluse to rigid body
			m_LinearVelocity += (-impluse) * InvLoopMass;
			m_AnularVelocity += loopinertiaMat * (wR.Cross(-impluse));

		}
	}
}
//============================================================================
GFPhysVector3 MisMedicSimpleLooper::CalcLoopForceFeedBack()
{
	return GFPhysVector3(0,0,0);
}
//===================================================================================================
void MisMedicSimpleLooper::UpdateMesh()
{	
	std::vector<Ogre::Vector3> RendNodes;
	
	for(size_t n = 0 ; n < m_LooperNodeRestPos.size() ; n++)
	{
		GFPhysVector3 localPos = m_LooperNodeRestPos[n];
		
		GFPhysVector3 worldPos = m_LoopRigidTrans(localPos);

		RendNodes.push_back(Ogre::Vector3(worldPos.x() , worldPos.y() , worldPos.z()));
	}	
	//
	//
	int insertPointNum = 3;

	float delta = 1.0f / float(insertPointNum+1);

	std::vector<Ogre::Vector3> RefinedNodes;
	RefinedNodes.reserve(RendNodes.size() + (RendNodes.size()-1)*insertPointNum);

	int NumSegment = RendNodes.size()-1;
	for(int s = 0 ; s < NumSegment ; s++)
	{
		RefinedNodes.push_back(RendNodes[s]);

		if(s > 0 && s < NumSegment-1)
		{
			Ogre::Vector3 P1 = RendNodes[s-1];
			Ogre::Vector3 P2 = RendNodes[s];
			Ogre::Vector3 P3 = RendNodes[s+1];
			Ogre::Vector3 P4 = RendNodes[s+2];

			for(int t = 1 ; t <= insertPointNum; t++)
			{
				Ogre::Vector3 interplotPt = GetInterplotPointPos(P1 , P2 , P3 , P4 , t*delta , 0.5f);
				RefinedNodes.push_back(interplotPt);
			}
		}
	}
	RefinedNodes.push_back(RendNodes[RendNodes.size()-1]);//last points
	//
	m_RendObject.UpdateRendSegment(RefinedNodes , m_RopeRendRadius);
}
//===================================================================================================
void MisMedicSimpleLooper::UpdateMeshByCustomedRendNodes(std::vector<Ogre::Vector3> & RendNodes)
{
	
}