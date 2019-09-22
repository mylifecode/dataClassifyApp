#include "SutureNeedleV2.h"
#include "MisNewTraining.h"
#include "MXOgreGraphic.h"
#include "collision/CollisionShapes/GoPhysCapsuleShape.h"
#include "dynamic/Constraint/GoPhysSoftBodyConstraint.h"
#include "MisMedicThreadRope.h"
#include "TrainingMgr.h"
#include "MisMedicSutureKnot.h"
#include "MisMedicSutureKnotV2.h"
#include "SutureThreadV2.h"
#include <QDebug>
#define NEEDLE_FROM_FILE
#define SOLVENEEDLEANCHOROLDMETHOD 0

#define ROPEANCHORMOVINGSPEED 1.0f

#define ROPEANCHORSWITCHSPEED 20.0f

const int SNeedleNodeNum = 15;
//const int SThreadNodeNum = 25 * 1;
//const Real ThreadRSLEN = 6.0f * 1.0f;

const Real NeedleMassPerSegment = 0.2f;
Real _coherentYawV2   = 0;
Real _coherentPicthV2 = 0;
Real _coherentRollV2  = 0;

Real _minDisV2 = 0.1f;

GFPhysFaceNeedleAnchorV2::GFPhysFaceNeedleAnchorV2(GFPhysVector3* localpoint, GFPhysSoftBodyFace* face, Real* weight, SutureNeedleV2* sutureNeedle, AnchorTypeV2 type)
{
	m_SutNeedle = sutureNeedle;
	m_Rigid = sutureNeedle->GetPhysicBody();
	m_Face = face;

	m_Type = type;

	m_weights[0] = weight[0];
	m_weights[1] = weight[1];
	m_weights[2] = weight[2];

	m_AccumNormal[0] = m_AccumNormal[1] = 0;
	m_AccumFriction = 0;

	GFPhysVector3 needledir = m_SutNeedle->GetNeedleDirction();
	GFPhysVector3 faceNormal = (m_Face->m_Nodes[1]->m_CurrPosition - m_Face->m_Nodes[0]->m_CurrPosition).Cross(m_Face->m_Nodes[2]->m_CurrPosition - m_Face->m_Nodes[0]->m_CurrPosition);

	faceNormal.Normalize();

	m_initAngle = CalcAngleBetweenLineAndFace(needledir, faceNormal);

	m_Friction = 0.0f;

	AnchorPosV2 anchorInfo = { SNeedleNodeNum - 2, 1.0f };

	SetAnchorInfo(anchorInfo);

	m_BHeadOut = false;
	m_BTailOut = false;
}
//------------------------------------------------------------------------------------------------
GFPhysFaceNeedleAnchorV2::~GFPhysFaceNeedleAnchorV2()
{

}
//================================================================================================================
void GFPhysFaceNeedleAnchorV2::SetFriction(float friction)
{
	m_Friction = friction;
}
//================================================================================================================
GFPhysVector3 GFPhysFaceNeedleAnchorV2::GetAnchorLocalPos()
{
	return m_AnchorLocalPoint;
}
//================================================================================================================
GFPhysVector3 GFPhysFaceNeedleAnchorV2::GetAnchorWorldPos()
{
	return m_Rigid->GetWorldTransform() * m_AnchorLocalPoint;
}
//================================================================================================================
AnchorPosV2   GFPhysFaceNeedleAnchorV2::GetAnchorInfo()
{
	return m_AnchorInfo;
}
//================================================================================================================
void  GFPhysFaceNeedleAnchorV2::SetAnchorInfo(AnchorPosV2 info)
{
	m_AnchorInfo = info;
	m_AnchorRation = m_SutNeedle->AnchorInfo2Ratio(m_AnchorInfo);
	m_AnchorLocalPoint = (1 - m_AnchorInfo.lambda) * m_SutNeedle->m_NeedleNodeLocalPos[m_AnchorInfo.segment]
		                    + m_AnchorInfo.lambda * m_SutNeedle->m_NeedleNodeLocalPos[m_AnchorInfo.segment + 1];
}
//================================================================================================================
void GFPhysFaceNeedleAnchorV2::PrepareSolveConstraint(Real Stiffness, Real TimeStep)
{
	m_ForceFeedBack = GFPhysVector3(0, 0, 0);
	
	m_ForceFeedBackAlongTan = GFPhysVector3(0, 0, 0);
	
	GFPhysVector3 tanNeedle = m_SutNeedle->GetNeedleSegmentTangent(m_AnchorInfo.segment, m_AnchorInfo.lambda);

	m_TangentDir = tanNeedle;

	m_NormalDir[0] = Perpendicular(m_TangentDir).Normalized();
	m_NormalDir[1] = m_NormalDir[0].Cross(m_TangentDir).Normalized();

	m_R = QuatRotate(m_Rigid->GetWorldTransform().GetRotation(), m_AnchorLocalPoint);////m_LastWorldPoint - m_Rigid->GetWorldTransform().GetOrigin();

	m_AnchorWorld = m_Rigid->GetWorldTransform() * m_AnchorLocalPoint;

	Real FaceInvM = m_weights[0] * m_weights[0] * m_Face->m_Nodes[0]->m_InvM
		          + m_weights[1] * m_weights[1] * m_Face->m_Nodes[1]->m_InvM
		          + m_weights[2] * m_weights[2] * m_Face->m_Nodes[2]->m_InvM;// 3.0f / (m_Face->m_Nodes[0]->m_Mass + m_Face->m_Nodes[1]->m_Mass + m_Face->m_Nodes[2]->m_Mass);

	float invRigidMass = m_Rigid->GetInvMass();

	GFPhysMatrix3 invRigidInertia = m_Rigid->GetInvInertiaTensorWorld();

	if (m_SutNeedle->GetBeHold())
	{
		invRigidMass = 0;
		invRigidInertia.SetZero();
	}
	//build impluse matrix etc for solve constraints
	m_ImpMat = GFPhysSoftBodyConstraint::K_DtMatrix(TimeStep,
		FaceInvM,
		invRigidMass,
		invRigidInertia,
		m_R);

	//friction
	m_AccumFriction = 0;
	m_AccumNormal[0] = m_AccumNormal[1] = 0;

	m_IterorCount = 0;

	//temp we Just Active Rigid Body
	m_Rigid->Activate();
	//m_RigidImpulse = GFPhysVector3(0.0f, 0.0f, 0.0f);
}
//------------------------------------------------------------------------------------------------
void GFPhysFaceNeedleAnchorV2::SolveConstraint(Real Stiffniss, Real TimeStep)
{
	if (m_Face->m_Nodes[0]->m_InvM < GP_EPSILON || m_Face->m_Nodes[1]->m_InvM < GP_EPSILON || m_Face->m_Nodes[2]->m_InvM < GP_EPSILON)
	{
		return;
	}
	if (m_AnchorRation <= 0.0f)
	{
		return;
	}
	m_IterorCount++;

	//one slide freedom , so two constraint direction+ 1 slide direction
	for (int dir = 0; dir < 3; dir++)
	{
		GFPhysVector3 worldPtOnFace = m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0] +
			m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1] +
			m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2];

		GFPhysVector3 CS_Dir = (dir < 2 ? m_NormalDir[dir] : m_TangentDir);

		GFPhysVector3 va = m_Rigid->GetVelocityInLocalPoint(m_R)*TimeStep;

		const GFPhysVector3 vr = (va + m_AnchorWorld - worldPtOnFace);

		Real vn = vr.Dot(CS_Dir);

		Real t = (m_ImpMat * CS_Dir).Dot(CS_Dir);

		if (t > GP_EPSILON)
		{
			Real delta = vn / t;

			if (dir == 2)//friction direction
			{
				float normalForce = (m_NormalDir[0] * m_AccumNormal[0] + m_NormalDir[1] * m_AccumNormal[1]).Length();

				float maxFrict = normalForce * 0.3f;

				float old = m_AccumFriction;

				m_AccumFriction = GPClamped(m_AccumFriction + delta, -maxFrict, maxFrict);

				delta = m_AccumFriction - old;
			}
			else
			{
				m_AccumNormal[dir] += delta;
			}
			GFPhysVector3 impluse = CS_Dir * delta;

			//apply positive impluse on soft
			//GFPhysVector3 NodeCorr = impluse * m_DtInvMa;
			m_Face->m_Nodes[0]->m_CurrPosition += impluse * m_weights[0] * m_Face->m_Nodes[0]->m_InvM * TimeStep;// NodeCorr*w0;
			m_Face->m_Nodes[1]->m_CurrPosition += impluse * m_weights[1] * m_Face->m_Nodes[1]->m_InvM * TimeStep;// NodeCorr*w1;
			m_Face->m_Nodes[2]->m_CurrPosition += impluse * m_weights[2] * m_Face->m_Nodes[2]->m_InvM * TimeStep;// NodeCorr*w2;

			//apply negative impluse on rigid
			if (m_SutNeedle->GetBeHold() == false)
			{
				m_Rigid->ApplyImpulse(-impluse, m_R);
			}
			//record feedback
			m_ForceFeedBack += (-impluse * TimeStep);
		}
	}
}
//------------------------------------------------------------------------------------------------
void  GFPhysFaceNeedleAnchorV2::SlipAnchor()
{
	GFPhysVector3 worldPtOnFace = m_Face->m_Nodes[0]->m_CurrPosition * m_weights[0] +
		m_Face->m_Nodes[1]->m_CurrPosition * m_weights[1] +
		m_Face->m_Nodes[2]->m_CurrPosition * m_weights[2];

	GFPhysVector3 localPtOnRigis = (1 - m_AnchorInfo.lambda) * m_SutNeedle->m_NeedleNodeLocalPos[m_AnchorInfo.segment]
		                           + m_AnchorInfo.lambda * m_SutNeedle->m_NeedleNodeLocalPos[m_AnchorInfo.segment + 1];

	GFPhysVector3 worldPtOnRigid = m_Rigid->GetWorldTransform() * localPtOnRigis;

	GFPhysVector3 worldPtOnRigid2 = (1 - m_AnchorInfo.lambda) * m_SutNeedle->m_NeedleNodeWorldPos[m_AnchorInfo.segment]
		                            + m_AnchorInfo.lambda  * m_SutNeedle->m_NeedleNodeWorldPos[m_AnchorInfo.segment + 1];

	Real tanMovement = -(worldPtOnRigid - worldPtOnFace).Dot(m_TangentDir);

	Real slidethreshold = 0.0025f;

	if (fabsf(tanMovement) < slidethreshold)
	{
		tanMovement = 0;
	}
	else
	{
		if (tanMovement > 0)
		{
			tanMovement -= slidethreshold;
		}
		else
		{
			tanMovement += slidethreshold;
		}
	}

	Real currDistRatio = m_AnchorRation + m_SutNeedle->ConvertRealDistToRatioDist(tanMovement);

	if (currDistRatio >= 1.0f && tanMovement > 0.005f)
	{
		m_BHeadOut = true;
		return;
	}

	if (currDistRatio <= 0.0f && tanMovement < 0.0f)
	{
		m_BTailOut = true;
		m_SuperfluousOffset = fabsf(currDistRatio);
		return;
	}
	GPClamp(currDistRatio, 0.0f, 1.0f);

	AnchorPosV2 anchor = m_SutNeedle->Ratio2AnchorInfo(currDistRatio);

	SetAnchorInfo(anchor);
}


//============================================================================================================================
SutureNeedleV2::SutureNeedleV2()
{
	m_AttachedRope = 0;
    m_NeedlePhysBody = 0;
	m_pNeedleSceneNode = 0;
    m_OwnerTraining = 0;
    m_NeedleCollideShape = 0;
	m_CollideRadius = 0.04f;
	m_RecentClampTool = 0;

	//m_FaceNeedleAnchors.clear();
	//m_FaceRopeAnchors.clear();
}
//===================================================================================================================
SutureNeedleV2::~SutureNeedleV2()
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
    //if(!m_FaceRopeAnchors.empty())
    //{
    //    for (size_t i = 0, ni = m_FaceRopeAnchors.size(); i <ni; ++i)
    //    {
    //        delete m_FaceRopeAnchors[i]; 
    //        m_FaceRopeAnchors[i] = 0;
    //    }        
    //}

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
void SutureNeedleV2::GFPhysQuaternionFromEuler(GFPhysQuaternion& q,Real yawDegree,Real pitchDegree,Real rollDegree)
{
	Real yaw   = Ogre::Math::DegreesToRadians(yawDegree);
	Real picth = Ogre::Math::DegreesToRadians(pitchDegree);
	Real roll  = Ogre::Math::DegreesToRadians(rollDegree);

	q.SetEuler(yaw,roll,picth);
}
//===================================================================================================================
void SutureNeedleV2::OgreQuaternionFromEuler(Ogre::Quaternion& q,const Real yawDegree,const Real pitchDegree,const Real rollDegree)
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
void SutureNeedleV2::CalcNeedleMassProperty(GFPhysVector3 NodePos[] , 
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
void SutureNeedleV2::CreateVisibleMesh(const char* meshFileName,int id)
{
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();
	
	Ogre::Entity* needleEnt = pSceneManager->createEntity(meshFileName);
	
    Ogre::SceneNode* MeshNode = pSceneManager->getRootSceneNode()->createChildSceneNode("zhen" + Ogre::StringConverter::toString(id));
	
	MeshNode->attachObject(needleEnt);
	
	//set position and orientation
	MeshNode->setPosition(0.0f,0.0f,0.0f);
	Ogre::Matrix3  test;

	Ogre::Degree dYaw   = Ogre::Degree(_coherentYawV2);
	Ogre::Degree dPicth = Ogre::Degree(_coherentPicthV2);
	Ogre::Degree dRoll  = Ogre::Degree(_coherentRollV2);
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
void SutureNeedleV2::CreateSutureNeedleV2(MisNewTraining * newTrain, int threadnodenum, Real restlen,const Ogre::String & needleskeleton)
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
	Real test_yaw   = _coherentYawV2;
	Real test_picth = _coherentPicthV2;
	Real test_roll  = _coherentRollV2;

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

    m_AttachedRope = new SutureThreadV2( MXOgre_SCENEMANAGER , newTrain);
	m_AttachedRope->Create(threadStartWorld, threadEndWorld, threadnodenum - 1, ThreadMass, rotMassPerSeg, false);
	m_AttachedRope->SetGravity(ThreadGravity);
    m_AttachedRope->m_RopeRSFriction = 0.04f;
	m_AttachedRope->m_NeedleAttchedThread = this;
	m_AttachedRope->SetNodeAsFix(m_AttachedRope->GetNumSegments() - 1);
    

    //weld first segment to needle
	m_AttachedRope->WeldToRigid(m_NeedlePhysBody , 0 , 10000.0f , 0.8f , 0.9f);
    
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
int SutureNeedleV2::GetNeedleNodeNum()
{
	return m_NeedleCollideShape->GetNumComponent() + 1;
}
//======================================================================================
int SutureNeedleV2::GetNeedleSegNum()
{
	return m_NeedleCollideShape->GetNumComponent();
}
//==============================================================================
GFPhysVector3 SutureNeedleV2::GetNeedleNodePosByIndex(int index)
{
	return m_NeedleNodeWorldPos[index];//pos;
}
//==============================================================================
GFPhysVector3 SutureNeedleV2::GetNeedleDirction()//获得针尖方向
{
    GFPhysVector3 dir = m_NeedleNodeWorldPos[m_NeedleNodeWorldPos.size()-1] - m_NeedleNodeWorldPos[m_NeedleNodeWorldPos.size()-2];
    return dir.Normalized();
}
//==============================================================================
GFPhysVector3 SutureNeedleV2::GetNeedleNormalDirction()//获得针所在平面的法向量
{
    int k = (int)m_NeedleNodeWorldPos.size() * 0.5;
    GFPhysVector3 dir1 = m_NeedleNodeWorldPos[0] - m_NeedleNodeWorldPos[k];
    
    GFPhysVector3 dir2 = m_NeedleNodeWorldPos[m_NeedleNodeWorldPos.size()-1] - m_NeedleNodeWorldPos[k];

    GFPhysVector3 dir = dir1.Cross(dir2);
    return dir.Normalized();
}

//==============================================================================
GFPhysVector3 SutureNeedleV2::GetNeedleHeadNodeWorldPos()   //获得针尖质心位置(世界坐标系)
{
	return m_NeedleNodeWorldPos[m_NeedleNodeWorldPos.size()-1];
}
//==============================================================================
GFPhysVector3 SutureNeedleV2::GetNeedleTailNodeWorldPos()   //获得针尾质心位置(世界坐标系)
{
	return m_NeedleNodeWorldPos[0]; 
}
//==============================================================================
void SutureNeedleV2::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	for (int i = 0, ni = m_FaceNeedleAnchors.size(); i < ni;i++)
	{
		m_FaceNeedleAnchors[i].pAnchor->PrepareSolveConstraint(Stiffness, TimeStep);
	}

	if (m_AttachedRope)
	{
		m_AttachedRope->PrepareSolveConstraint(Stiffness, TimeStep);
	}
}
//===================================================================================================================
void SutureNeedleV2::SolveConstraint(Real Stiffness,Real TimeStep)
{	
	for (int i = 0, ni = m_FaceNeedleAnchors.size(); i < ni; i++)
	{
		m_FaceNeedleAnchors[i].pAnchor->SolveConstraint(Stiffness, TimeStep);
	}

	if (m_AttachedRope)
	{
		m_AttachedRope->SolveConstraint(Stiffness, TimeStep);
		m_AttachedRope->SolveWeldConstraint(TimeStep);
	}
}
//===================================================================================================================
void SutureNeedleV2::SetVisibleMeshMaterial(bool bClamped)
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
void SutureNeedleV2::UpdateNeedleNodeWorldPos()
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
void SutureNeedleV2::SynchronizeState()
{
    UpdateNeedleNodeWorldPos();
}
//===================================================================================================================
void SutureNeedleV2::UpdateMesh()
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
void SutureNeedleV2::InternalSimulationStart(int currStep , int TotalStep , float dt)
{     
    if (m_AttachedRope)
    {
        m_AttachedRope->BeginSimulatePhysics(dt);
    }    

    if (m_NeedlePhysBody)
    {
        m_NeedlePhysBody->m_inverseMass = m_NeedleOriginInvMass;
        m_NeedlePhysBody->SetLocalInvInertiaTensor(m_NeedleOriginInvTensor);
        m_NeedlePhysBody->UpdateInertiaTensor();
    }

	//////////////////////////////////////////////////////////////////////////
	Real Max = INT_MIN;
	Real Min = INT_MAX;
	for (size_t t = 0, nt = m_AttachedRope->m_FaceRopeAnchors.size(); t < nt; ++t)
	{
		if (Max < m_AttachedRope->m_FaceRopeAnchors[t]->GetSegPos())
		{
			Max = m_AttachedRope->m_FaceRopeAnchors[t]->GetSegPos();
		}
		if (Min > m_AttachedRope->m_FaceRopeAnchors[t]->GetSegPos())
		{
			Min = m_AttachedRope->m_FaceRopeAnchors[t]->GetSegPos();
		}
	}
	if (Max != INT_MIN && Min != INT_MAX)
	{
		m_AttachedRope->SetRopeAnchorIndexMax(Max);
		m_AttachedRope->SetRopeAnchorIndexMin(Min);
	}
}
//==================================================================================================================
void SutureNeedleV2::OnRigidBodyCollided(GFPhysCollideObject * ca,
	                                   GFPhysCollideObject * cb,
									   const GFPhysManifoldPoint * contactPoints,
									   int NumContactPoints)
{
	if (ca == m_NeedlePhysBody && cb)
	{
		m_CollidedRigid.insert(cb);
	}
	else if (cb == m_NeedlePhysBody)
	{
		m_CollidedRigid.insert(ca);
	}
}
//================================================================================================================
void SutureNeedleV2::OnRigidBodySeperate(GFPhysCollideObject * ra, GFPhysCollideObject * rb)
{
	if (ra == m_NeedlePhysBody)
	{
		m_CollidedRigid.erase(rb);
	}
	else if (rb == m_NeedlePhysBody)
	{
		m_CollidedRigid.erase(ra);
	}
}
//===================================================================================================================
void SutureNeedleV2::InternalSimulationEnd(int currStep , int TotalStep , float dt)
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

    	m_AttachedRope->EndSimulatePhysics(dt);


	
    }
}


//////////////////////////////////////////////////////////////////////////

void SutureNeedleV2::UpdateAnchor()
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
   
	//check impluse exceed limit
	/*
	for (int c = 0; c < m_FaceNeedleAnchors.size(); c++)
	{
		if (m_FaceNeedleAnchors[c].pAnchor)
		{
			float normalImp = m_FaceNeedleAnchors[c].pAnchor->GetNormalImpulse().Length();

			if (normalImp > 40.0f)
			{
				if (m_FaceNeedleAnchors[c].pAnchor->m_Type == STATE_IN)
				{
					if (c < m_FaceNeedleAnchors.size() - 1)
					{
						notifyRemoveInAnchor(m_FaceNeedleAnchors[c + 1].pAnchor->GetFace(), m_FaceNeedleAnchors[c + 1].pAnchor->m_weights);
						delete m_FaceNeedleAnchors[c + 1].pAnchor;
						m_FaceNeedleAnchors[c + 1].pAnchor = 0;
					}
				}
				else if (m_FaceNeedleAnchors[c].pAnchor->m_Type == STATE_OUT)
				{
					if (c > 0)
					{
						notifyRemoveInAnchor(m_FaceNeedleAnchors[c - 1].pAnchor->GetFace(), m_FaceNeedleAnchors[c - 1].pAnchor->m_weights);
						delete m_FaceNeedleAnchors[c - 1].pAnchor;
						m_FaceNeedleAnchors[c - 1].pAnchor = 0;
					}
				}
				notifyRemoveInAnchor(m_FaceNeedleAnchors[c].pAnchor->GetFace(), m_FaceNeedleAnchors[c].pAnchor->m_weights);
				delete m_FaceNeedleAnchors[c].pAnchor;
				m_FaceNeedleAnchors[c].pAnchor = 0;
			}
		}
	}
	GFPhysVectorObj<FaceAnchorInfoV2>::iterator itor = m_FaceNeedleAnchors.begin();
	while (itor != m_FaceNeedleAnchors.end())
	{
		if (itor->pAnchor == 0)
		{
			itor = m_FaceNeedleAnchors.erase(itor);
		}
		else
		{
			itor++;
		}
	}
	*/

	for (int i = 0; i < (int)m_FaceNeedleAnchors.size(); i++)
	{
		//slip
		m_FaceNeedleAnchors[i].pAnchor->SlipAnchor();
	}

	float lastRation;

	for (int i = 0; i < (int)m_FaceNeedleAnchors.size(); i++)
	{
		FaceAnchorInfoV2 & info = m_FaceNeedleAnchors[i];

		//confirm anchor not pass over previous one
		float currRation = AnchorInfo2Ratio(info.pAnchor->GetAnchorInfo());
		if (i > 0 && currRation < lastRation + 0.1f)
		{
			currRation = lastRation + 0.1f;
			info.pAnchor->SetAnchorInfo(Ratio2AnchorInfo(currRation));
		}
		lastRation = currRation;

		
		//if (GetBeHold())
		//	info.pAnchor->SetFriction(0.6f);
		//else
		//	info.pAnchor->SetFriction(100000.0f);
	}

    std::vector<Real> nodeIndex;
    if (m_AttachedRope->m_FaceRopeAnchors.size() > 0)
    {
		for (int i = 0, ni = (int)m_AttachedRope->m_FaceRopeAnchors.size(); i <ni; i++)
        {
			GFPhysFaceRopeAnchorV2 * info = m_AttachedRope->m_FaceRopeAnchors[i];
		
			////////////////////////////////////////////////////////////////////////////
			float ThreadWeights[2];
			ThreadWeights[0] = 1 - info->GetSegWeight();
			ThreadWeights[1] = info->GetSegWeight();

			notifyMovingRopeAnchor(info->m_thread, info->GetSegIndex(), ThreadWeights);
			Real p = m_AttachedRope->m_FaceRopeAnchors[i]->GetSegPos();
            if (p > 1.0f)
            {
                nodeIndex.push_back(p);
            }
        }
        m_AttachedRope->SetRopeAnchorIndex(nodeIndex);
    }

 	SlipFaceRopeAnchors();

	RemoveHeadAnchor();

	RemoveTailAnchor();

	m_AttachedRope->RemoveSlipOuttedAnchor();
}

bool SortFaceRopeAnchor(const GFPhysFaceRopeAnchorV2* v1, const GFPhysFaceRopeAnchorV2* v2)
{
	return v1->GetSegPos() < v2->GetSegPos();//降序排列  
}
//

void SutureNeedleV2::SlipFaceRopeAnchors()
{
	if (NULL == m_AttachedRope)
	{
		return;
	}
	
	int minSegKnot = 100000;
	int maxSegKnot = 100000;
	if (m_AttachedRope->m_KnotsInThread)
	    m_AttachedRope->m_KnotsInThread->GetKnotsSegMinMax(minSegKnot, maxSegKnot);

	bool reachHeadLimit = false;
	if (m_AttachedRope->m_FaceRopeAnchors.size() > 0)
	{
		if (m_AttachedRope->m_FaceRopeAnchors[0]->GetSegIndex() == 0
		 && m_AttachedRope->m_FaceRopeAnchors[0]->GetSegWeight() <= 0.5f + GP_EPSILON)
			
		    reachHeadLimit = true;
	}

	std::vector<bool> SegmentHasAnchors;
	SegmentHasAnchors.resize(m_AttachedRope->GetNumSegments());
	for (int c = 0; c < SegmentHasAnchors.size(); c++)
	{
		SegmentHasAnchors[c] = false;
	}
	for (int c = 0; c < (int)m_AttachedRope->m_FaceRopeAnchors.size(); c++)
	{
		GFPhysFaceRopeAnchorV2 * faceropeanchor = m_AttachedRope->m_FaceRopeAnchors[c];
		SegmentHasAnchors[faceropeanchor->GetSegIndex()] = true;
	}
	float maxFriction = 0.01f;
 
	for (int c = 0; c < (int)m_AttachedRope->m_FaceRopeAnchors.size(); c++)
	{
		 GFPhysFaceRopeAnchorV2 * faceropeanchor = m_AttachedRope->m_FaceRopeAnchors[c];
		 
		 int segmentIndex = faceropeanchor->GetSegIndex();

		 if (faceropeanchor->m_IsSlipDisabled)//anchor between knot is fixed
		 {
			 faceropeanchor->m_ToSlipDist = 0;
			 continue;
		 }

		 float prevSegStretch = 0;
		
		 float nextSegStretch = 0;
#if(0)	 
		 float prevSegDelta = 0;

		 float nextSegDelta = 0;
		
		 if (c == 0)
		 {
			 prevSegDelta = faceropeanchor->GetSegPos();
			 prevSegStretch = m_AttachedRope->GetStretchedDistBetweenSegments(0, 0.0f, faceropeanchor->GetSegIndex(), faceropeanchor->GetSegWeight());
		 }
		 else
		 {
			 GFPhysFaceRopeAnchorV2 * prevFaceAnchor = m_AttachedRope->m_FaceRopeAnchors[c - 1];
			 prevSegStretch = m_AttachedRope->GetStretchedDistBetweenSegments(prevFaceAnchor->GetSegIndex(), prevFaceAnchor->GetSegWeight(), faceropeanchor->GetSegIndex(), faceropeanchor->GetSegWeight());

			 prevSegDelta = fabsf(prevFaceAnchor->GetSegPos() - faceropeanchor->GetSegPos());
		 }

		 if (c == m_AttachedRope->m_FaceRopeAnchors.size() - 1)
		 {
			 nextSegStretch = m_AttachedRope->GetStretchedDistBetweenSegments(faceropeanchor->GetSegIndex(), faceropeanchor->GetSegWeight(), m_AttachedRope->GetNumSegments() - 1, 1.0f);
			 nextSegDelta = m_AttachedRope->GetNumSegments() - faceropeanchor->GetSegPos();
		 }
		 else
		 {
			 GFPhysFaceRopeAnchorV2 * nextFaceAnchor = m_AttachedRope->m_FaceRopeAnchors[c + 1];
			 nextSegStretch = m_AttachedRope->GetStretchedDistBetweenSegments(faceropeanchor->GetSegIndex(), faceropeanchor->GetSegWeight(), nextFaceAnchor->GetSegIndex(), nextFaceAnchor->GetSegWeight());

			 nextSegDelta = fabsf(nextFaceAnchor->GetSegPos() - faceropeanchor->GetSegPos());
		 }

#else
		 GFPhysSoftTubeSegment * prevSeg = 0;

		 GFPhysSoftTubeSegment * nextSeg = 0;

		 GFPhysSoftTubeSegment * currSeg = &(m_AttachedRope->GetSegment(segmentIndex));

		 int maxSegIndex = m_AttachedRope->GetNumSegments() - 1;

		 if (segmentIndex > 0)
		 {
			 if (SegmentHasAnchors[segmentIndex - 1] && segmentIndex < maxSegIndex)
			 {
				 prevSeg = currSeg;
			 }
			 else
			     prevSeg = &(m_AttachedRope->GetSegment(segmentIndex - 1));
		 }
		 else
		 {
			 prevSeg = currSeg;
		 }

		 if (segmentIndex < maxSegIndex)
		 {
			 if (SegmentHasAnchors[segmentIndex + 1] && prevSeg != currSeg)
			 {
				 nextSeg = currSeg;
			 }
			 else
			 {
				 nextSeg = &(m_AttachedRope->GetSegment(segmentIndex + 1));
			 }
		 }
		 else
		 {
			 nextSeg = currSeg;
		 }

		 if (nextSeg == prevSeg)
		 {
			 MessageBoxA(0, "errpr", "nextseg = prevset!!!", 0);
		 }

		 float restlen = prevSeg->GetRestLen();

		 float currlen = (prevSeg->m_Node0->m_CurrPosition - prevSeg->m_Node1->m_CurrPosition).Length();

		 prevSegStretch = currlen - restlen;


		 restlen = nextSeg->GetRestLen();

		 currlen = (nextSeg->m_Node0->m_CurrPosition - nextSeg->m_Node1->m_CurrPosition).Length();

		 nextSegStretch = currlen - restlen;
#endif

		 if (faceropeanchor->m_type == STATE_IN)
		 {
			 faceropeanchor->m_NbSegStretch = nextSegStretch;
		 }
		 else
		 {
			 faceropeanchor->m_NbSegStretch = prevSegStretch;
		 }
		 
		 prevSegStretch -= maxFriction;
		 if (prevSegStretch < 0)
			 prevSegStretch = 0;

		 nextSegStretch -= maxFriction;
		 if (nextSegStretch < 0)
			 nextSegStretch = 0;

		 float slipDist = 0;

		 if (prevSegStretch > GP_EPSILON && prevSegStretch > nextSegStretch)
		 {
			 slipDist = prevSegStretch;
		 }
		 else if (nextSegStretch > GP_EPSILON && nextSegStretch > prevSegStretch)
		 {
			 slipDist = -nextSegStretch;
		 }
	
		 if (reachHeadLimit && slipDist < 0)
			 slipDist = 0;

		 faceropeanchor->m_ToSlipDist = slipDist;
		 faceropeanchor->m_IsDragedTwoEnd = false;
	}

	//检查最小最大锚点两边是否被拉伸
	//这里偷懒将整绳上的锚点一起考虑。没有考虑锚点被打结，死锚点隔开的情况（一般情况下医生总是穿好线再打结，不会打完结再用线穿）
	int minHoldSeg = INT_MAX;
	int maxHoldSeg = INT_MIN;
	if (GetBeHold())
	{
		minHoldSeg = maxHoldSeg = 0;
	}

	for (int c = 0; c < m_AttachedRope->m_SegmentsBeClamped.size(); c++)
	{
		 int segIndex = m_AttachedRope->m_SegmentsBeClamped[c];
		
		 if (segIndex > maxHoldSeg)
			 maxHoldSeg = segIndex;
		
		 if (segIndex < minHoldSeg)
			 minHoldSeg = segIndex;
	}
	
	bool BeDraggedTwoEnd = false;

	if ( m_AttachedRope->m_FaceRopeAnchors.size() > 1 && minHoldSeg <= maxHoldSeg)
	{
		 int MinAnchorSeg = m_AttachedRope->m_FaceRopeAnchors[0]->GetSegIndex();
		
		 int MaxAnchorSeg = m_AttachedRope->m_FaceRopeAnchors[m_AttachedRope->m_FaceRopeAnchors.size() - 1]->GetSegIndex();

		 float RestLen0 = 0;
		
		 float CurrLen0 = 0;
		
		 int   NumSeg = 0;

		 for (int c = minHoldSeg + 1; c <= MinAnchorSeg - 1; c++)
		 {
			 //float segLen = m_AttachedRope->GetSegmentCurrLen(c);
			 //CurrLen0 += segLen;
             RestLen0 += m_AttachedRope->GetSegment(c).GetRestLen();

			 NumSeg++;
		 }

		 if (NumSeg > 0)
		 {
			CurrLen0 = (m_AttachedRope->GetSegment(minHoldSeg).m_Node1->m_CurrPosition 
				      - m_AttachedRope->GetSegment(MinAnchorSeg).m_Node0->m_CurrPosition).Length();
			
			if ((CurrLen0 - RestLen0) / float(NumSeg) > 0.05f)
			{
				RestLen0 = 0;
				
				CurrLen0 = 0;
				
				NumSeg = 0;
				
				for (int c = MaxAnchorSeg + 1; c <= maxHoldSeg - 1; c++)
				{
					 //float segLen = m_AttachedRope->GetSegmentCurrLen(c);
					 //CurrLen0 += segLen;
					 RestLen0 += m_AttachedRope->GetSegment(c).GetRestLen();;

					 NumSeg++;
				}

				if (NumSeg > 0)
				{
					CurrLen0 = (m_AttachedRope->GetSegment(MaxAnchorSeg).m_Node1->m_CurrPosition
						      - m_AttachedRope->GetSegment(maxHoldSeg).m_Node0->m_CurrPosition).Length();

					if ((CurrLen0 - RestLen0) / float(NumSeg) > 0.05f)
					{
						BeDraggedTwoEnd = true;
					}
				}
			}
		 }

		 if (BeDraggedTwoEnd)
		 {
			for (int t = 0; t < (int)m_AttachedRope->m_FaceRopeAnchors.size(); t++)
			{
				m_AttachedRope->m_FaceRopeAnchors[t]->m_ToSlipDist = 0.0f;
				m_AttachedRope->m_FaceRopeAnchors[t]->m_IsDragedTwoEnd = true;
			}
		 }
		
	 }
   
 	 for (int c = 0; c < (int)m_AttachedRope->m_FaceRopeAnchors.size(); c++)
	 {
		 GFPhysFaceRopeAnchorV2 * faceropeanchor = m_AttachedRope->m_FaceRopeAnchors[c];

		 if (fabsf(faceropeanchor->m_ToSlipDist) > FLT_EPSILON && (faceropeanchor->m_IsSlipDisabled == false))
		 {
			 float currDist = 0.0f;

			 m_AttachedRope->RelativePos2RestLen(faceropeanchor->GetSegIndex(), faceropeanchor->GetSegWeight(), currDist);

			 currDist += GPClamped(faceropeanchor->m_ToSlipDist, -FLT_MAX, FLT_MAX);

			 int segindex = -1;

			 float weight = 0.0f;

			 m_AttachedRope->RestLen2RelativePos(currDist, segindex, weight);

			 faceropeanchor->SetAnchorSegAndWeight(segindex, weight);
		 }		
	 }
	 m_AttachedRope->StandarlizeRopeAnchors();
}
//=================================================================================================================================
void SutureNeedleV2::RemoveHeadAnchor()
{    
	 int MinInvalid = -1;

	 for (int j = 0 ; j < (int)m_FaceNeedleAnchors.size(); j++)
	 {
		  if (m_FaceNeedleAnchors[j].pAnchor->m_BHeadOut)//delete anchor behind it at all
		  {
			  MinInvalid = j;
			  break;
		  }
	 }
	
	 if (MinInvalid >= 0)
	 {
		 for (int c = MinInvalid; c < (int)m_FaceNeedleAnchors.size(); c++)
		 {
			  FaceAnchorInfoV2& anchorinfo = m_FaceNeedleAnchors[c];
				
			  notifyRemoveInAnchor(anchorinfo.pAnchor->GetFace(), anchorinfo.pAnchor->m_weights);

			  for (int f = 0 ; f < (int)anchorinfo.pFacesVector.size(); f++)
			  {
				  m_NeedleAnchorDisableFaces.erase(anchorinfo.pFacesVector[f]);
			  }

			  delete anchorinfo.pAnchor;
			  anchorinfo.pAnchor = 0;
		  }
		  m_FaceNeedleAnchors.resize(MinInvalid);
	  }

}
//=================================================================================================================================
void SutureNeedleV2::RemoveTailAnchor()
{    
	 int MaxInvalid = -1;
	 
	 for (int j = m_FaceNeedleAnchors.size()-1; j >= 0; j--)
	 {
		 if (m_FaceNeedleAnchors[j].pAnchor->m_BTailOut )
		 {
			 MaxInvalid = j;
			 break;
		 }
	}

	if (MaxInvalid >= 0)
	{
		for (int c = 0; c <= MaxInvalid; c++)
		{
			 FaceAnchorInfoV2& anchorinfo = m_FaceNeedleAnchors[c];
				
			 notifySwitchAnchor(anchorinfo.pAnchor->GetFace(), anchorinfo.pAnchor->m_weights);
				
			 if (m_AttachedRope)//从针尾滑出滑入线内
			 {
				 float unitLen = m_AttachedRope->GetSegment(0).GetRestLen();
				 float initOffset = unitLen * 0.5f;	 
				 m_AttachedRope->SlipInFaceRopeAnchor(anchorinfo.pAnchor->m_Type, anchorinfo.pAnchor->GetFace(), anchorinfo.pAnchor->m_weights, initOffset);
			 }

			 for (int f = 0; f < (int)anchorinfo.pFacesVector.size(); f++)
			 {
				 m_NeedleAnchorDisableFaces.erase(anchorinfo.pFacesVector[f]);
			 }

			 delete anchorinfo.pAnchor;
			 anchorinfo.pAnchor = 0;
		 }

		 for (int c = MaxInvalid + 1; c < (int)m_FaceNeedleAnchors.size(); c++)
		 {
			 m_FaceNeedleAnchors[c - MaxInvalid - 1] = m_FaceNeedleAnchors[c];
		 }
		 m_FaceNeedleAnchors.resize(m_FaceNeedleAnchors.size() - MaxInvalid - 1);
	}
}
//=================================================================================================================================
void SutureNeedleV2::CreateNeedleAnchor( const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints )
{    
    if(RSContactConstraints.size() > 0 && m_NeedlePhysBody)
    {
		GFPhysVector3 NeedleDirWorld = GetNeedleDirction();

		float ImpluseNeedle = 0;

		int   selectFace = -1;

		float maxNdn = -FLT_MAX;

		for (int c = 0; c < (int)RSContactConstraints.size(); c++)
		{
			GFPhysSoftFaceRigidContact srContact = RSContactConstraints[c];
			
			if (srContact.m_Rigid != m_NeedlePhysBody)
			{
				continue;
			}

			float ndn = NeedleDirWorld.Dot(srContact.m_NormOnRigidWorld);

			GFPhysVector3 vecFromNHead = srContact.m_PointOnRigidLocal - m_NeedleNodeLocalPos[SNeedleNodeNum - 1];

			float distToHead = vecFromNHead.Length();

			if (distToHead > GP_EPSILON)
				vecFromNHead /= distToHead;

			GFPhysVector3 dirNeedleLocal = (m_NeedleNodeLocalPos[SNeedleNodeNum - 1] - m_NeedleNodeLocalPos[SNeedleNodeNum - 2]).Normalized();

			//float td = vecFromNHead.Dot(dirNeedleLocal);

			//bool isClosetToNeedleHead = (td > -0.2f);

			if (srContact.m_Rigid == m_NeedlePhysBody && ndn > 0 && distToHead < m_CollideRadius*2.5f)//collision raidus + margin approximty 2.5 collision radius
			{
				Real contactImpluse = srContact.GetNormalImpluse(0) + srContact.GetNormalImpluse(1) + srContact.GetNormalImpluse(2);

				ImpluseNeedle += contactImpluse * ndn;

				if (ndn > maxNdn && contactImpluse * ndn > 0)
				{
					if (srContact.m_SoftFace->m_Nodes[0]->m_InvM > 0.0f || srContact.m_SoftFace->m_Nodes[1]->m_InvM > 0.0f || srContact.m_SoftFace->m_Nodes[2]->m_InvM > 0.0f)
					{
						maxNdn = ndn;
						selectFace = c;// srContact.m_SoftFace;
					}
				}
			}
		}

		if (ImpluseNeedle > 0.75f && selectFace >= 0)
		{
			 GFPhysSoftFaceRigidContact  srContact = RSContactConstraints[selectFace];
		//}
        //for (size_t c = 0, nc = RSContactConstraints.size(); c < nc; c++)
       // {            
           // GFPhysSoftFaceRigidContact srContact = RSContactConstraints[c];
           // if (srContact.m_Rigid != m_NeedlePhysBody)
           // {
            //    continue;
           // }
           // if (srContact.m_SoftFace->m_Nodes[0]->m_InvM > 0.0f
           //  && srContact.m_SoftFace->m_Nodes[1]->m_InvM > 0.0f
           //  && srContact.m_SoftFace->m_Nodes[2]->m_InvM > 0.0f)
           // {
                
				//Real contactImpluse = srContact.GetNormalImpluse(0) + srContact.GetNormalImpluse(1) + srContact.GetNormalImpluse(2);
				//GFPhysVector3 dist = srContact.m_PointOnRigidLocal - m_NeedleNodeLocalPos[SNeedleNodeNum - 1];
				//GFPhysVector3 NeedleDirWorld = GetNeedleDirction();
				//Real punctureAngle = NeedleDirWorld.Dot(srContact.m_NormOnRigidWorld);// m_SoftFace->m_FaceNormal);
				//bool oldcriterion = dist.Length() < 0.1f && contactImpluse * punctureAngle > 0.5f;
				
				GFPhysFaceNeedleAnchorV2* CurrFaceNeedleAnchor = NULL;
				//if (oldcriterion)
				//{
					MisMedicOrgan_Ordinary* NeedleTestOrgan = (MisMedicOrgan_Ordinary*)srContact.m_SoftBody->GetUserPointer();

					GFPhysVector3 pos = srContact.m_SoftFace->m_Nodes[0]->m_CurrPosition * srContact.m_FaceWeights[0] +
						srContact.m_SoftFace->m_Nodes[1]->m_CurrPosition * srContact.m_FaceWeights[1] +
						srContact.m_SoftFace->m_Nodes[2]->m_CurrPosition * srContact.m_FaceWeights[2];

					if (m_FaceNeedleAnchors.empty())
					{
						CurrFaceNeedleAnchor = new GFPhysFaceNeedleAnchorV2(&srContact.m_PointOnRigidLocal, srContact.m_SoftFace, srContact.m_FaceWeights, this, STATE_IN);
					}
					else if (!m_FaceNeedleAnchors.empty())
					{
						FaceAnchorInfoV2& lastAnchor = m_FaceNeedleAnchors[m_FaceNeedleAnchors.size() - 1];
						if (lastAnchor.pAnchor->m_Type == STATE_OUT)
						{
							CurrFaceNeedleAnchor = new GFPhysFaceNeedleAnchorV2(&srContact.m_PointOnRigidLocal, srContact.m_SoftFace, srContact.m_FaceWeights, this, STATE_IN);
						}
						else if (lastAnchor.pAnchor->m_Type == STATE_IN)
						{
							AnchorPosV2 lastInAnchorpos = lastAnchor.pAnchor->GetAnchorInfo();
							Real LastInPos = lastInAnchorpos.segment + lastInAnchorpos.lambda;							

							if (LastInPos < SNeedleNodeNum - 2.0f && lastAnchor.POrgan == NeedleTestOrgan)
							{
								CurrFaceNeedleAnchor = new GFPhysFaceNeedleAnchorV2(&srContact.m_PointOnRigidLocal, srContact.m_SoftFace, srContact.m_FaceWeights, this, STATE_OUT);
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

						FaceAnchorInfoV2 currFaceAnchor = { NeedleTestOrgan, CurrFaceNeedleAnchor, aroundfaces };
						m_FaceNeedleAnchors.push_back(currFaceAnchor);

						if (CurrFaceNeedleAnchor->m_Type == STATE_IN)
						{
							notifyCreateInAnchor(srContact.m_SoftFace, srContact.m_FaceWeights);
						}
						else if (CurrFaceNeedleAnchor->m_Type == STATE_OUT)
						{
							notifyCreateOutAnchor(srContact.m_SoftFace, srContact.m_FaceWeights);
						}

						CurrFaceNeedleAnchor = NULL;
					}
				//}
		//	}
		}
    }
}
//===================================================================================================================================
GFPhysVector3 SutureNeedleV2::GetNeedleSegmentTangent (int segment , float lambda)
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
//=================================================================================================================
GFPhysVector3 SutureNeedleV2::GetForceFeedBack()
{
	GFPhysVector3 totalForce = GFPhysVector3(0, 0, 0);
	for (int c = 0; c < (int)m_FaceNeedleAnchors.size(); c++)
	{
		totalForce += m_FaceNeedleAnchors[c].pAnchor->m_ForceFeedBack;
	}
	return totalForce;
}
//=================================================================================================================
bool SutureNeedleV2::Getinterval(const GFPhysVector3& pos, int& index0, int& index1)
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
//------------------------------------------------------------------------------------------------
void SutureNeedleV2::RendGreen(int begin, int end)
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
//================================================================================================================
void SutureNeedleV2::addNeedleActionListener(NeedleActionListenerV2* listener)
{
	for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
	{
		if (m_Listeners[i] == listener)
			return;
	}
	m_Listeners.push_back(listener);
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedleV2::removeNeedleActionListener(NeedleActionListenerV2 * listener)
{
	for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
	{
		if (m_Listeners[i] == listener)
		{
			m_Listeners.erase(m_Listeners.begin() + i);
			return;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedleV2::removeAllNeedleActionListener()
{
	while (m_Listeners.begin() != m_Listeners.end())
	{
		m_Listeners.erase(m_Listeners.begin());
	}
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedleV2::notifyCreateInAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
	{
		m_Listeners[i]->OnCreateInAnchor(face, weights);
	}
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedleV2::notifyCreateOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
	{
		m_Listeners[i]->OnCreateOutAnchor(face, weights);
	}
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedleV2::notifyRemoveInAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
	{
		m_Listeners[i]->OnRemoveInAnchor(face, weights);
	}
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedleV2::notifyRemoveOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	for (size_t i = 0, ni = m_Listeners.size(); i < ni; i++)
	{
		m_Listeners[i]->OnRemoveOutAnchor(face, weights);
	}
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedleV2::notifySwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	for (size_t i = 0; i < m_Listeners.size(); i++)
	{
		m_Listeners[i]->OnSwitchAnchor(face, weights);
	}
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedleV2::notifyMovingRopeAnchor(const SutureThreadV2 * thread, const int index, const Real weights[])
{
	for (size_t i = 0; i < m_Listeners.size(); i++)
	{
		m_Listeners[i]->OnMovingRopeAnchor(thread, index, weights);
	}
}
//////////////////////////////////////////////////////////////////////////
void SutureNeedleV2::notifyRemoveRopeAnchor(const GFPhysSoftBodyFace* face, const Real weights[])
{
	for (size_t i = 0; i < m_Listeners.size(); i++)
	{
		m_Listeners[i]->OnRemoveRopeAnchor(face, weights);
	}
}

Real SutureNeedleV2::AnchorInfo2Ratio(const AnchorPosV2& pos)
{
	//input filter missing
	assert(pos.lambda < 1.0f && pos.lambda >= 0.0f);
	if (pos.segment >= SNeedleNodeNum - 1 && pos.lambda > 0.0f)
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
		partsum += (m_NeedleNodeLocalPos[i + 1] - m_NeedleNodeLocalPos[i]).Length();
	}
	if (pos.lambda > GP_EPSILON)
	{
		partsum += (pos.lambda)*(m_NeedleNodeLocalPos[pos.segment + 1] - m_NeedleNodeLocalPos[pos.segment]).Length();
	}

	return partsum / m_NeedleTotalLen;
}

Real SutureNeedleV2::ConvertRealDistToRatioDist(Real realdist)
{
	return realdist / m_NeedleTotalLen;
}

AnchorPosV2 SutureNeedleV2::Ratio2AnchorInfo(const Real& ratio)
{
	//input filter missing
	GPClamped(ratio, 0.0f, 1.0f);
	

	if (fabsf(ratio) < GP_EPSILON)
	{
		AnchorPosV2 pos = { 0 , 0.0f };
		return pos;
	}
	else if (ratio > 1.0f - GP_EPSILON)
	{
		AnchorPosV2 pos = { GetNeedleSegNum() - 1, 1.0f };
		return pos;
	}

	int   segment = 0;
	float lambda  = 0.0f;
	float partsum = 0.0f;

	for (int i = 0; i < (int)m_NeedleNodeLocalPos.size() - 1; ++i)
	{
		Real seglen = (m_NeedleNodeLocalPos[i + 1] - m_NeedleNodeLocalPos[i]).Length();
		partsum += seglen;
		Real k = partsum / m_NeedleTotalLen;
		if (k > ratio)
		{
			segment = i;
			lambda = 1.0f - (partsum - ratio * m_NeedleTotalLen) / seglen;
			lambda = GPClamped(lambda, 0.0f, 1.0f);
			break;
		}
	}

	AnchorPosV2 pos = { segment, lambda };
	return pos;
}

void SutureNeedleV2::Disappear()
{
	//m_AttchPoints.clear();

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

	GFPhysVectorObj<KnotInSutureRopeV2> Knots;
	GetSutureThread()->m_KnotsInThread->GetDeadKnots(Knots);
	if (Knots.size() == 2)
	{
		int min = Knots[1].m_knotcon0.m_A;
		int max = Knots[1].m_knotcon1.m_B;
		for (int i = 0; i < min - 2; i++)
		{
			SutureThreadNodeV2& node = m_AttachedRope->GetThreadNodeRefReal(i);
			node.Disappear();

			m_AttachedRope->GetTubeWireSegment(i).m_CanCollide = false;
			m_AttachedRope->GetTubeWireSegment(i).SetCanCollideRigid(false);
			m_AttachedRope->GetTubeWireSegment(i).SetCanCollideSelf(false);
			m_AttachedRope->GetTubeWireSegment(i).SetCanCollideSoft(false);
		}
		for (int i = max + 2; i < m_AttachedRope->GetNumThreadNodes(); i++)
		{
			SutureThreadNodeV2& node = m_AttachedRope->GetThreadNodeRefReal(i);
			node.Disappear();

			m_AttachedRope->GetTubeWireSegment(i).m_CanCollide = false;
			m_AttachedRope->GetTubeWireSegment(i).SetCanCollideRigid(false);
			m_AttachedRope->GetTubeWireSegment(i).SetCanCollideSelf(false);
			m_AttachedRope->GetTubeWireSegment(i).SetCanCollideSoft(false);
		}
	}
}


