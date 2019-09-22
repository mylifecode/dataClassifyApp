/**Author:zx**/
#include "../Include/stdafx.h"
#include "Tool.h"
#include <time.h>
#include "XMLWrapperMeshNode.h"
#include "XMLWrapperTool.h"
#include "IMXDefine.h"
#include "XMLWrapperTraining.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperPart.h"
#include "XMLWrapperPursue.h"
#include "XMLWrapperHardwareConfig.h"
#include "MXOgreWrapper.h"
#include <OgreManualObject.h>
#include <OgreMovableObject.h>
#include <vector>
#include "BasicTraining.h"
#include "EffectManager.h"
#include "MisRobotInput.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "InputSystem.h"
#include <QtDebug>
#include "ScreenEffect.h"
#include "Helper.h"
#include "KeyboardInput.h"
#include "PerformanceDebug.h"
#include "math/GophysTransformUtil.h"
#include "../NewTrain/PhysicsWrapper.h"
#include "../NewTrain/MisMedicOrganOrdinary.h"
#include "MisMedicCToolPluginInterface.h"
#include "../NewTrain/VeinConnectObject.h"
#include "../NewTrain/MisToolCollideDataConfig.h"
#include "MisCTool_PluginClamp.h"
#include "MisCTool_PluginRigidHold.h"
#include "MisMedicThreadRope.h"
#include "MisMedicOrganAttachment.h"
#include "SutureThreadV2.h"
#include "TrainUtils.h"
#include "BubbleManager.h"
#include <QDebug>
#define CYLINDERRENDNUM 10

using namespace::std;
extern bool PointInSphere(Ogre::Vector3 v3SphereCenter, float fSphereRadius, Ogre::Vector3 v3Pos);


#define CUTETUBESHAFT 7 //剪刀夹闭角度 
#define MoveFastestLimit 0.1f

TubeCutStateStruct::TubeCutStateStruct()
{
	m_maxShaftAngleContact = -1.0f;
	m_leftContactDist = 0;
	m_IsLeftBladeContact = false;

	m_rightContactDist = 0;
	m_IsRightBladeContact = false;
}
//========================================================================================================
NewTrainToolConvexData::NewTrainToolConvexData()
{
		m_isvalid = false;
		m_debugdrawnode = 0;
		m_rigidbody = 0;
		m_firstTimeSetTrans = true;
		//m_IsFrustConvex = false;
		//m_IsConvex = false;
		//m_IsCylinder = false;
		m_AttachedNode = 0;
		m_RSCollideAlgoType = 0;
		m_CompoundShape = 0;
		m_CollideShapeRelOffset = Ogre::Vector3::ZERO;

		m_CollideShapeRelRot = Ogre::Quaternion::IDENTITY;

		m_bSharp = false;//
}
//========================================================================================================
NewTrainToolConvexData::~NewTrainToolConvexData()
{
	if(m_rigidbody)
	{
		PhysicsWrapper::GetSingleTon().DestoryRigidBody(m_rigidbody);
		m_rigidbody = 0;
	}

	for(size_t c = 0 ; c < m_CollideShapesData.size() ; c++)
	{
		PhysicsWrapper::GetSingleTon().DestoryCollisionShape(m_CollideShapesData[c].m_CollideShape);
	}
	m_CollideShapesData.clear();

	if(m_CompoundShape)
	{
	   PhysicsWrapper::GetSingleTon().DestoryCollisionShape(m_CompoundShape);
	}

    if (m_debugdrawnode)
    {
        Ogre::SceneManager * pSceneManager = MXOgreWrapper::Get()->GetDefaultSceneManger();
        pSceneManager->getRootSceneNode()->removeAndDestroyChild(m_debugdrawnode->getName());
        m_debugdrawnode = 0;
    }
}
//========================================================================================================
void NewTrainToolConvexData::SetAttachedNode(Ogre::Node * attachnode)
{
	m_AttachedNode = attachnode;
}

void NewTrainToolConvexData::EnableDoubleFaceRSCollideMode()
{
	if(m_rigidbody)
	   m_rigidbody->EnableDoubleFaceCollision();
}

void NewTrainToolConvexData::DisableDoubleFaceRSCollideMode()
{
	if(m_rigidbody)
	   m_rigidbody->DisableDoubleFaceCollision();
}

bool NewTrainToolConvexData::IsDoubleFaceRSCollideMode()
{
	if(m_rigidbody)
	   return m_rigidbody->IsDoubleFaceCollisionMode();
	else
	   return false;
}
void NewTrainToolConvexData::SetCollideCategry(uint32 cat)
{
	if(m_rigidbody)
		m_rigidbody->m_CategoryBits = cat;
}
//========================================================================================================
void NewTrainToolConvexData::CreateRigidBodyByConfig(const MisToolCollidePart & DataConfig)
{
	 m_RSCollideAlgoType = (DataConfig.m_IsConvexHull == true ? 0 : 1);

	 bool hasPolyhydron = false;

     for(size_t c = 0 ; c < DataConfig.m_CollideShapes.size() ; c++)
	 {
		 const MisToolCollidePart::ToolCollideShapeData & cdShapeCfg = (*DataConfig.m_CollideShapes[c]);

		 if(cdShapeCfg.m_Type == 0)// box collide shape
		 {
            GFPhysCollideShape * collideshape = PhysicsWrapper::GetSingleTon().CreateBoxCollideShape(cdShapeCfg.m_BoxExtend, DataConfig.m_IsConvexHull);
            collideshape->m_UseCCD = false;
			NewTrainToolConvexData::CollideShapeData cdshapeData(cdShapeCfg.m_BoxCenter ,
				                                                 cdShapeCfg.m_BoxExtend , 
																 cdShapeCfg.m_Rotate,
																 collideshape,
																 cdShapeCfg.m_Type);
			m_CollideShapesData.push_back(cdshapeData);
		 }
		 else if(cdShapeCfg.m_Type == 1)
		 { 
			 hasPolyhydron = true;
			 if(cdShapeCfg.m_ConvexFace.size() == 0)
			 {
				 int SegNum = cdShapeCfg.m_VertexPos.size() / 8;
				 if(SegNum >= 1)
				 {
					 for(int c = 0 ; c < SegNum  ; c++)
					 {
						 std::vector<Ogre::Vector3> PartConvexVertex;
						 std::vector<int> PartFaceVertIndex;
						 std::vector<int> PartFaceVertNum;

						 int adv = c*8;

						 for(int t = 0 ; t < 8 ; t++)
							 PartConvexVertex.push_back(cdShapeCfg.m_VertexPos[t+adv]);

						 int tempf[] = {0,3,2,1 , 7,6,2,3 , 4,0,1,5 , 7,3,0,4 , 6,5,1,2 , 4,5,6,7};

						 for(int i = 0 ; i < 24 ; i++)
							 PartFaceVertIndex.push_back(tempf[i]);

						 for(int f = 0 ; f < 6 ; f++)
							 PartFaceVertNum.push_back(4);

						 GFPhysAlignedVectorObj<GFPhysVector3> GPhullVerts;
						 for(size_t c = 0 ; c < PartConvexVertex.size() ; c++)
						 {
                             GPhullVerts.push_back(OgreToGPVec3(PartConvexVertex[c]));
						 }

						 Ogre::Vector3 childShapeCenter = Ogre::Vector3::ZERO;
                         Ogre::Quaternion childShapeRot = Ogre::Quaternion::IDENTITY;

						 GFPhysVector3 center;
						 GFPhysMatrix3 rotMat;
						 GFPhysVector3 extend(0,0,0);
						 bool succed = CalConvexHullBestFitFrame(GPhullVerts , center , rotMat , extend);//(GPhullVerts , ObbExt , ObbAixs);
                         if(succed == true)
						 {
							 for(size_t c = 0 ; c < PartConvexVertex.size() ; c++)//transform back to local frame
							 {
								 PartConvexVertex[c] = GPVec3ToOgre(rotMat.Inverse() * (GPhullVerts[c]-center));
							 }
							 GFPhysQuaternion gprotQuat;
							 rotMat.GetRotation(gprotQuat);

							 childShapeCenter = GPVec3ToOgre(center);
							 childShapeRot = GPQuaternionToOgre(gprotQuat);
						 }
						 GFPhysCollideShape * collideshape = PhysicsWrapper::GetSingleTon().CreateConvexCollidesShape(PartConvexVertex, 
																													  PartFaceVertIndex, 
																													  PartFaceVertNum);

						 NewTrainToolConvexData::CollideShapeData cdshapeData(childShapeCenter ,
																			  GPVec3ToOgre(extend) , 
																			  childShapeRot,
																			  collideshape,
																			  cdShapeCfg.m_Type
																			  );
						 m_CollideShapesData.push_back(cdshapeData);
					 }
				 }
			 }
			 else//use index
			 {
				 std::vector<Ogre::Vector3> PartConvexVertex;
				 for(int t = 0 ; t < cdShapeCfg.m_VertexPos.size() ; t++)
					 PartConvexVertex.push_back(cdShapeCfg.m_VertexPos[t]);

				 GFPhysAlignedVectorObj<GFPhysVector3> GPhullVerts;
				 for(size_t c = 0 ; c < PartConvexVertex.size() ; c++)
				 {
					 GPhullVerts.push_back(OgreToGPVec3(PartConvexVertex[c]));
				 }

				 Ogre::Vector3 childShapeCenter = Ogre::Vector3::ZERO;
				 Ogre::Quaternion childShapeRot = Ogre::Quaternion::IDENTITY;

				 GFPhysVector3 center;
				 GFPhysMatrix3 rotMat;
				 GFPhysVector3 extend(0,0,0);
				 bool succed = CalConvexHullBestFitFrame(GPhullVerts , center , rotMat , extend);//(GPhullVerts , ObbExt , ObbAixs);
				 if(succed == true)
				 {
					 for(size_t c = 0 ; c < PartConvexVertex.size() ; c++)//transform back to local frame
					 {
						 PartConvexVertex[c] = GPVec3ToOgre(rotMat.Inverse() * (GPhullVerts[c]-center));
					 }
					 GFPhysQuaternion gprotQuat;
					 rotMat.GetRotation(gprotQuat);

					 childShapeCenter = GPVec3ToOgre(center);
					 childShapeRot = GPQuaternionToOgre(gprotQuat);
				 }

				 GFPhysCollideShape * collideshape = PhysicsWrapper::GetSingleTon().CreateConvexCollidesShape(PartConvexVertex, 
					                                                                                          cdShapeCfg.m_ConvexFaceVertIndex, 
					                                                                                          cdShapeCfg.m_ConvexFace);

				 NewTrainToolConvexData::CollideShapeData cdshapeData(childShapeCenter ,
					                                                  GPVec3ToOgre(extend) , 
					                                                  childShapeRot,
					                                                  collideshape,
					                                                  cdShapeCfg.m_Type
					                                                 );
				 m_CollideShapesData.push_back(cdshapeData);
			 }
		 }
		 else if(cdShapeCfg.m_Type == 2)
		 {
			 //to add support cylinder

             GFPhysCollideShape * collideshape = PhysicsWrapper::GetSingleTon().CreateCynlinderCollideShape(cdShapeCfg.m_CapPointA, cdShapeCfg.m_CapPointB, cdShapeCfg.m_Radius);
             collideshape->m_UseCCD = false;
             //////////////////////////////////////////////////////////////////////////
             Ogre::Vector3 Axis = cdShapeCfg.m_CapPointA - cdShapeCfg.m_CapPointB;

             Ogre::Quaternion quat0 = Ogre::Vector3::UNIT_Z.getRotationTo(Axis.normalisedCopy());

             Ogre::Quaternion quat1 = Ogre::Vector3::UNIT_Z.getRotationTo(-Axis.normalisedCopy());

             Ogre::Quaternion nearRotate = (fabsf(quat0.w) < fabsf(quat1.w) ? quat0 : quat1);

             //////////////////////////////////////////////////////////////////////////
             NewTrainToolConvexData::CollideShapeData cdshapeData(cdShapeCfg.m_BoxCenter,
                                                                  cdShapeCfg.m_BoxExtend,
                                                                  nearRotate,
                                                                  collideshape,
                                                                  cdShapeCfg.m_Type);
             m_CollideShapesData.push_back(cdshapeData);
		 }
	 }

	 m_CompoundShape = 0;

	 GFPhysCollideShape  * singleCollideShape = 0;
	 
	 //always put convex collision shape to compound shape since we may introduce local rotate 
	 //so make local rotate to sub-shape is good , since the rigid has not change it's locate
	 if (m_CollideShapesData.size() > 1 || hasPolyhydron)
	 {
		m_CompoundShape = PhysicsWrapper::GetSingleTon().CreateCompoundCollisionShape();
	 }
	 for(size_t c = 0 ;  c < m_CollideShapesData.size() ; c++)
	 {
		GFPhysTransform rigidTrans;
		rigidTrans.SetOrigin(OgreToGPVec3(m_CollideShapesData[c].m_boxcenter));
        rigidTrans.SetRotation(OgreToGPQuaternion(m_CollideShapesData[c].m_boxrotate));
		 
		if(m_CompoundShape)
		{
           m_CompoundShape->AddComponent(rigidTrans , m_CollideShapesData[c].m_CollideShape);
		}
		else
		{
			singleCollideShape = m_CollideShapesData[c].m_CollideShape;
		}
	 }

	 if(m_CompoundShape)
	 {
	    GFPhysVector3 localInerta(0,0,0);
		//if(mass != 0)
		//m_CompoundShape->CalculateLocalInertia(mass , localInerta);

		GFPhysTransform startTrans;
		startTrans.SetIdentity();

	    m_rigidbody = PhysicsWrapper::GetSingleTon().CreateRigidBody(0 , m_CompoundShape , localInerta , startTrans);
		m_rigidbody->SetFriction(1.0f);
		m_CollideShapeRelOffset = Ogre::Vector3::ZERO;
		m_CollideShapeRelRot = Ogre::Quaternion::IDENTITY;
	 }
	 else if(singleCollideShape)
	 {
		GFPhysVector3 localInerta(0,0,0);
		
		//if(mass != 0)
		//  singleCollideShape->CalculateLocalInertia(0 , localInerta);

		GFPhysTransform startTrans;
		startTrans.SetOrigin(OgreToGPVec3(m_CollideShapesData[0].m_boxcenter));
		startTrans.SetRotation(OgreToGPQuaternion(m_CollideShapesData[0].m_boxrotate));

		m_rigidbody = PhysicsWrapper::GetSingleTon().CreateRigidBody(0 , singleCollideShape , localInerta , startTrans);
		m_rigidbody->SetFriction(1.0f);
		m_CollideShapeRelOffset = GPVec3ToOgre(startTrans.GetOrigin());
		m_CollideShapeRelRot   = GPQuaternionToOgre(startTrans.GetRotation());
	 }

	 if(m_rigidbody)
	 {
		m_rigidbody->GetCollisionShape()->SetMargin(0);
		m_rigidbody->DisableDeactivation();//器械不触碰软体或者其他运动刚体的时候会被GPSDK睡眠，所以这里禁止系统对他sleep
        m_isvalid = true;
	 }
	 else
	 {
        m_isvalid = false;
	 }
}
//==========================================================================================================
void NewTrainToolConvexData::SetNextWorldTransform(Ogre::Vector3 derivedpos , Ogre::Quaternion derivedorient)
{
	m_rotateInWorld = m_rotateInWorldNext;
	m_centerInWorld = m_centerInWorldNext;

	m_rotateInWorldNext = derivedorient * m_CollideShapeRelRot;//m_boxrotate;
	m_centerInWorldNext = derivedorient * m_CollideShapeRelOffset/*m_boxcenter*/+ derivedpos;

	if(m_debugdrawnode)
	{
		m_debugdrawnode->setPosition(m_centerInWorldNext);
		m_debugdrawnode->setOrientation(m_rotateInWorldNext);
	}

	if(m_firstTimeSetTrans)
	{
		m_firstTimeSetTrans = false;
		m_rotateInWorld = m_rotateInWorldNext;
		m_centerInWorld = m_centerInWorldNext;
	}
	
}
//==========================================================================================================
void NewTrainToolConvexData::UpdatePhysVelocity(float percent , float dt, bool clampmove)
{
	if(m_rigidbody)
	{
		m_DstPhysCom = m_centerInWorldNext * percent + m_centerInWorld*(1-percent);
		

		GFPhysVector3 RotAxis;
		Real RotAngle;
		GFPhysTransformUtil::CalculateDiffAxisAngleQuaternion(OgreToGPQuaternion(m_rotateInWorld) , 
			                                                  OgreToGPQuaternion(m_rotateInWorldNext) ,
			                                                  RotAxis , 
			                                                  RotAngle);

		m_DstPhysOrient = Ogre::Quaternion(Ogre::Radian(RotAngle * percent) , GPVec3ToOgre(RotAxis)) * m_rotateInWorld;//Ogre::Quaternion::nlerp(percent , m_rotateInWorld , m_rotateInWorldNext);
		
		Ogre::Vector3 currBodyCom = GPVec3ToOgre(m_rigidbody->GetCenterOfMassTransform().GetOrigin());
		
		Ogre::Quaternion currBodyRot = GPQuaternionToOgre(m_rigidbody->GetCenterOfMassTransform().GetRotation());

		//clamp value
		if(clampmove)
		{
			Ogre::Vector3 translate = m_DstPhysCom-currBodyCom;
			float moveDist = translate.length();
			if(moveDist > 0.1f)
			{
			   Real clampedMovDist = moveDist * 0.25f;
			   
			   if(clampedMovDist < 0.1f)
			      clampedMovDist = 0.1f;
			   
			   translate = translate * (clampedMovDist / moveDist);//(0.1f / translate.length());
			   
			   m_DstPhysCom = currBodyCom + translate;
			}

			//
			Ogre::Quaternion rotate = m_DstPhysOrient * currBodyRot.Inverse();
			Ogre::Radian   rotRadian;
			Ogre:: Vector3 rotAxis;
			rotate.ToAngleAxis(rotRadian , rotAxis);

			Real rotateRad = rotRadian.valueRadians();

			//use the shortest rotate
			if(rotateRad > Ogre::Math::PI)
			{
				rotAxis *= -1.0f;
				rotateRad = Ogre::Math::TWO_PI-rotateRad;
			}
			//
			rotAxis.normalise();//no need just for ensure
			if(rotateRad > Ogre::Math::PI / 20.0f)
			{          
	           rotateRad = Ogre::Math::PI / 20.0f;
			   rotate.FromAngleAxis(Ogre::Radian(rotateRad) , rotAxis);
			   m_DstPhysOrient = rotate * currBodyRot;
			}
		}
		
		Ogre::Vector3 linearVel  = (m_DstPhysCom - currBodyCom) / dt;
		
		Ogre::Quaternion physRot = m_DstPhysOrient * currBodyRot.Inverse();
		
		if(physRot.w < 0)//inverse rotate axis if rotate angle large than PI this ensure we get a minimal rotate
		{
			physRot.x *= -1.0f;
			physRot.y *= -1.0f;
			physRot.z *= -1.0f;
			physRot.w *= -1.0f;
		}

		Ogre::Vector3 rotAxis;
		Ogre::Radian  rotAngle;
		physRot.ToAngleAxis(rotAngle, rotAxis);

		float valueAngle = rotAngle.valueRadians();

		if(valueAngle > 0.22f)
		   valueAngle = 0.22f;

		Ogre::Vector3 AngularVel = rotAxis * valueAngle / dt;

		m_rigidbody->SetLinearVelocity(OgreToGPVec3(linearVel));
		
		m_rigidbody->SetAngularVelocity(OgreToGPVec3(AngularVel));

		//GFPhysTransform bodycomtrans;
		//bodycomtrans.SetOrigin(OgreToGPVec3(m_DstPhysCom));
		////bodycomtrans.SetRotation(OgreToGPQuaternion(m_DstPhysOrient));
		//m_rigidbody->SetCenterOfMassTransform(bodycomtrans);
	}
}
//==========================================================================================================
void NewTrainToolConvexData::UpdatePhysTransform()
{
	GFPhysTransform bodycomtrans;
	bodycomtrans.SetOrigin(OgreToGPVec3(m_DstPhysCom));
	bodycomtrans.SetRotation(OgreToGPQuaternion(m_DstPhysOrient));
	m_rigidbody->SetCenterOfMassTransform(bodycomtrans);
}
//==========================================================================================================
void NewTrainToolConvexData::CreateDebugDrawable(Ogre::SceneManager * pScenemgr)
{
	if(m_debugdrawnode != 0)
		return;
	
	static int boxid = 0;
	boxid++;
	
	m_debugdrawnode = pScenemgr->getRootSceneNode()->createChildSceneNode();

	if(m_CollideShapesData.size() == 1 && m_CollideShapesData[0].m_ShapeType == 0)
	{
		Ogre::Entity* cubeentity = pScenemgr->createEntity("Cube"+Ogre::StringConverter::toString(boxid), Ogre::SceneManager::PT_CUBE);

		cubeentity->setMaterialName("BaseWhiteNoLighting");

		// Attach the 2 new entities to the root of the scene
			
		m_debugdrawnode->attachObject(cubeentity);
		m_debugdrawnode->setPosition(m_centerInWorld);
		m_debugdrawnode->setScale(m_CollideShapesData[0].m_boxextends / 50);
		m_debugdrawnode->setOrientation(m_rotateInWorld);
	}
    else if (m_CollideShapesData.size() == 1 && m_CollideShapesData[0].m_ShapeType == 2)
    {
#if 0
        Ogre::Entity* cubeentity = pScenemgr->createEntity("Cube" + Ogre::StringConverter::toString(boxid), Ogre::SceneManager::PT_CUBE);

        cubeentity->setMaterialName("BaseWhiteNoLighting");

        // Attach the 2 new entities to the root of the scene

        m_debugdrawnode->attachObject(cubeentity);
        m_debugdrawnode->setPosition(m_centerInWorld);
        m_debugdrawnode->setScale(Ogre::Vector3(1, 1, 1));
        //m_debugdrawnode->setScale(m_CollideShapesData[0].m_boxextends/50);
        m_debugdrawnode->setOrientation(m_rotateInWorld);

#else
        
        Ogre::Vector3 pointA = Ogre::Vector3(0.0f, 0.0f, m_CollideShapesData[0].m_boxextends.z);
        Ogre::Vector3 pointB = Ogre::Vector3(0.0f, 0.0f, -1.0f*m_CollideShapesData[0].m_boxextends.z);

        Real RendRadius = m_CollideShapesData[0].m_boxextends.x;

        Ogre::ManualObject * manualobj = pScenemgr->createManualObject();
        m_debugdrawnode->attachObject(manualobj);        

        m_debugdrawnode->setPosition(Ogre::Vector3(0, 0, 0));
        
        m_debugdrawnode->setScale(Ogre::Vector3(1, 1, 1));
        
        m_debugdrawnode->setOrientation(m_rotateInWorld);
        
        manualobj->clear();
        manualobj->begin("CollideBodyDebug", Ogre::RenderOperation::OT_TRIANGLE_LIST);        

        Ogre::Vector3 RotateAxis = pointA - pointB;
        RotateAxis.normalise();
        Ogre::Vector3 TangentAxis = RotateAxis.perpendicular();

        Ogre::Vector3 capVertexA[CYLINDERRENDNUM];
        Ogre::Vector3 capVertexB[CYLINDERRENDNUM];

        float invetubecapvnum = 1.0f / (float)CYLINDERRENDNUM;

        for (int a = 0; a < CYLINDERRENDNUM; a++)
        {
            float rotateRadian = float(Ogre::Math::TWO_PI * a) * invetubecapvnum;
            Ogre::Vector3 capvertex = Ogre::Quaternion(Ogre::Radian(rotateRadian), RotateAxis)*TangentAxis;
            capVertexA[a] = pointA + capvertex * RendRadius;   
            capVertexB[a] = pointB + capvertex * RendRadius;
        }

        for (int a = 0; a < CYLINDERRENDNUM; a++)
        {
            manualobj->position(capVertexA[a].x, capVertexA[a].y, capVertexA[a].z);
            manualobj->colour(1, 0, 0);
        }
        for (int a = 0; a < CYLINDERRENDNUM; a++)
        {
            manualobj->position(capVertexB[a].x, capVertexB[a].y, capVertexB[a].z);
            manualobj->colour(1, 0, 0);
        }

        for (int i = 0; i < CYLINDERRENDNUM-1;i++)
        {
            manualobj->triangle(i, CYLINDERRENDNUM + i + 1, CYLINDERRENDNUM + i);
            manualobj->triangle(CYLINDERRENDNUM + i + 1, i, i + 1);
        }

        manualobj->triangle(CYLINDERRENDNUM - 1, CYLINDERRENDNUM, CYLINDERRENDNUM * 2 - 1);
        manualobj->triangle(CYLINDERRENDNUM, CYLINDERRENDNUM - 1, 0);

        manualobj->end();

#endif
    }
	else
	{
		Ogre::ManualObject * manualobj = pScenemgr->createManualObject();
		m_debugdrawnode->attachObject(manualobj);
		m_debugdrawnode->setPosition(m_centerInWorld);
		m_debugdrawnode->setScale(Ogre::Vector3(1,1,1));
		m_debugdrawnode->setOrientation(m_rotateInWorld);
		manualobj->clear();
		manualobj->begin("CollideBodyDebug", Ogre::RenderOperation::OT_TRIANGLE_LIST);
		std::vector<GFPhysConvexHullShape*> allHulls;
		GFPhysAlignedVectorObj<GFPhysTransform> allChildTrans;
		std::vector<Ogre::Vector3> allChildBounds;

		GFPhysConvexHullShape * singleconvex = dynamic_cast<GFPhysConvexHullShape *>(m_rigidbody->GetCollisionShape());
		if(singleconvex)
		{
			allHulls.push_back(singleconvex);
			GFPhysTransform childTrans;
			childTrans.SetIdentity();
			allChildTrans.push_back(childTrans);
		}
		else
		{
			GFPhysCompoundShape * combineConvex = dynamic_cast<GFPhysCompoundShape *>(m_rigidbody->GetCollisionShape());
			for(int t = 0 ; t < combineConvex->GetNumComponent();t++)
			{
				GFPhysConvexHullShape * singleconvex = dynamic_cast<GFPhysConvexHullShape *>(combineConvex->GetComponentShape(t));
				GFPhysTransform childTrans = combineConvex->GetComponentTransform(t);
				if(singleconvex)
				{
				   allHulls.push_back(singleconvex);
				   allChildTrans.push_back(childTrans);
				}
			}
		}

		int Indexoffset = 0;
		for(size_t c = 0 ;c < allHulls.size(); c++)
		{
			GFPhysConvexHullShape * convexHull = allHulls[c];
			
			int NumVerts = convexHull->GetNumVertices();

			int NumFaces = convexHull->GetNumFaces();

			for(int v = 0 ; v < NumVerts ; v++)
			{
				GFPhysVector3 polypos = allChildTrans[c] * convexHull->GetPolyhedraVertex(v);
				manualobj->position(polypos.m_x , polypos.m_y , polypos.m_z);
				manualobj->colour(1, 1, 1);
			}

			for(int f = 0 ; f < NumFaces ; f++)
			{
				const GFPhysPolyhedraFace & polyface = convexHull->GetPolyhedraFace(f);
				for(int n = 1 ; n < polyface.m_NumVerts-1 ; n++)
				{
					manualobj->triangle(polyface.m_VertsIndex[0]+Indexoffset , polyface.m_VertsIndex[n]+Indexoffset , polyface.m_VertsIndex[n+1]+Indexoffset);
				}
			}
			Indexoffset += NumVerts;

			//
			if(allHulls.size() > 0)
			{
				/*
				GFPhysTransform childTrans = allChildTrans[c];
				Ogre::Entity* cubeentity = pScenemgr->createEntity("Frame"+Ogre::StringConverter::toString(boxid)+Ogre::StringConverter::toString(c), Ogre::SceneManager::PT_CUBE);
				cubeentity->setMaterialName("BaseWhiteNoLighting");
				Ogre::SceneNode * childscenenode = pScenemgr->createSceneNode();
				childscenenode->attachObject(cubeentity);
				childscenenode->setPosition(GPVec3ToOgre(childTrans.GetOrigin()));
				childscenenode->setScale(m_CollideShapesData[c].m_boxextends / 50);
				childscenenode->setOrientation(GPQuaternionToOgre(childTrans.GetRotation()));*/

				//m_debugdrawnode->addChild(childscenenode);
			}


		}
		

		manualobj->end();

	}
}


CTool::CTool():  m_fBloodTimeBefore(0)
				,m_pOwnerTraining(NULL)
				,m_tosmoke(false)
				,m_UseNewToolMode(false)
				,m_NewCanClampTube(false)
				,m_CanPunctureOgran(false)
				,m_PuncThresholdMultiply(1.0f)
				,m_InClampState(false)
				,m_BubbleWhenBurn(false)
		
{
	Reset();
}

CTool::CTool(CXMLWrapperTool * pToolConfig): m_fBloodTimeBefore(0)
											,m_pOwnerTraining(NULL)
											,m_bWarnState(false)
											,m_WarnTick(0)
											,m_bFirstEnterWarn(true)
											,m_tosmoke(false)
											,m_UseNewToolMode(false)
											,m_NewCanClampTube(false)
											,m_CanPunctureOgran(false)
											,m_PuncThresholdMultiply(1.0f)
											,m_InClampState(false)
											,m_BubbleWhenBurn(false)
{
	m_pToolConfig = pToolConfig;

	//default value
	m_centertoolpartconvex.m_bSharp = false;
	m_lefttoolpartconvex.m_bSharp = true;
	m_righttoolpartconvex.m_bSharp = true;
	m_pluginclamp = 0;
	Reset();
}

CTool::~CTool(void)
{
	Terminate();
	//call plugins
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
		delete m_ToolPlugins[c];
	}
	m_ToolPlugins.clear();
}

void CTool::SetVisible(bool vis)
{
	if (m_pNodeKernel)
	    m_pNodeKernel->setVisible(vis);
}

void CTool::Reset()
{
	m_pNodeKernel = NULL;
	m_pNodeLeft = NULL;
	m_pNodeRight = NULL;
	m_strName = "";
	m_bIsInUse = false;
	
	m_pTrainingConfig = NULL;
	m_bCollision2m = false;

	m_pManualObj= NULL;
	m_ps4mNode = NULL;

//	m_RawShaftAside = 0;
	m_nShaftAside = 0;
	m_nMinShaftAside = 0;
	m_nMaxShaftAside = FLT_MAX;
	m_bForceRelease = false;
	
	m_leftShaftAsideScale = 1.0;
	m_rightShaftAsdieScale = 1.0;

	m_nTrianglePointsID = 0;
	m_pTriangleObj = NULL;
	m_pTriangleNode = NULL;

	m_toolOwner = ITool::TO_None;
	//m_enmState = TS_NONE;
	m_bElectricButton = false;
	m_bElectricAttribute = false;
	m_hasRealElectricAttribute = false;
	m_totalElectricTime = 0.f;				//器械通电总时间
	m_validElectricTime = 0.f;				//有效通电时间
	m_leftPadElectricTime = 0.f;
	m_rightPadElectricTime = 0.f;
	m_tempElectricBeginTime = 0.f;
	m_maxKeeppingElectricBeginTime = 0.f;
	m_maxKeeppingElectricTime = 0.f;
	m_hasElectricAtPreFrame = false;
	m_nReleasedTitanicClip = 0;				//释放的钛夹数
	m_toolClosedTimes = 0;
	m_canClosed = false;
	m_isClosed = false;
	m_isClosedInsertion = true;
	m_canCheckClosedInsertion = true;
	m_TimesAfterLastStatistic = 0.f;
	m_movedTime = 0.f;
	m_movedDistance = 0.f;
	m_moveFastestState = false;
	m_moveFastestTimes = 0;
	//m_beignMoveFastTime = 0;
	//m_dwLastCuttingTickCount = 0;
	m_bDisable = false;
	m_isFirstFrame = true;

	m_bIsPointHaveBreak = false;
	//m_nDeviceLabel = -1;

	m_bCollideFlag=false;

	m_bOrganForce=false;
	m_fForce_X_K=0;
	m_fForce_Y_K=0;
	m_fForce_Z_K=0;

	m_vecToolFirstPos=Ogre::Vector3::ZERO;

	//m_vecCurrentPoints.clear();
	m_bCheckToolIn=false;
	m_bClampThenCut=false;

	m_ElecPriority = 0;
	m_IsIgnoreElecPriority = false;

	if (m_pToolConfig)
	{
		m_nMaxShaftAside = m_pToolConfig->m_MaxShaftAside;
		m_ElecPriority = m_pToolConfig->m_ElecPriority;
		m_IsIgnoreElecPriority = m_pToolConfig->m_IsIgnoreElecPriority;
	}

	//New Train Data
	m_ToolFightingForce = GFPhysVector3(0,0,0);
	m_PentrateForce = 0;
	//m_LastFeedBackForceDir = GFPhysVector3(0,0,0);
	m_ToolForceBackRate = 1.0f;

	m_pTipNode = NULL;

	m_IsTouchingOrgan = false;
	m_SeparateTimeWithOrgan = 0.0f;
	m_IsClosedInSeparateTime = true;

    m_TextureCoord = Ogre::Vector2::ZERO;
    m_CumTime = 0.0f;

	m_currentExternalForce[0] = 0.0;
	m_currentExternalForce[1] = 0.0;
	m_currentExternalForce[2] = 0.0;

	RawSurfaceForceFeedLeft=GFPhysVector3(0, 0, 0);

	RawSurfaceForceFeedRight = GFPhysVector3(0, 0, 0);

	RawSurfaceForceFeedCenter = GFPhysVector3(0, 0, 0);

	m_pivotPosition = Ogre::Vector3::ZERO;
	m_lastKernelNodePosition = Ogre::Vector3::ZERO;
	m_lastKernelNodeQuaternion = Ogre::Quaternion::IDENTITY;
}

void CTool::SetToSmoke(Ogre::Vector3 smokepos)
{
	m_tosmoke = true;
	m_tosmokepos = smokepos;
}

std::string CTool::GetCollisionConfigEntryName()
{
	return "";
}
bool CTool::Initialize(CXMLWrapperTraining * pTraining)
{
	m_pTrainingConfig = pTraining;

	m_centertoolpartconvex.SetAttachedNode(m_pNodeKernel);
	m_lefttoolpartconvex.SetAttachedNode(m_pNodeLeft);
	m_righttoolpartconvex.SetAttachedNode(m_pNodeRight);

	SetOriginalInfo(CTool::TC_KERNEL);
	SetOriginalInfo(CTool::TC_LEFT);
	SetOriginalInfo(CTool::TC_RIGHT);

	if (m_pToolConfig)
	{
		m_nMaxShaftAside = m_pToolConfig->m_MaxShaftAside;
		m_CanPunctureOgran = m_pToolConfig->m_CanPuncture;
	}

	ReadCollisionData(GetCollisionConfigEntryName());

	m_CutBladeLeft.m_AttachedRB  = m_lefttoolpartconvex.m_rigidbody;
	m_CutBladeRight.m_AttachedRB = m_righttoolpartconvex.m_rigidbody;

	return true;
}

void CTool::InitializeConvexTransform()
{
	if(m_lefttoolpartconvex.m_isvalid)
	{
		GFPhysTransform trans;

		Ogre::Vector3 derivedPos = m_lefttoolpartconvex.m_AttachedNode->_getDerivedPosition();

		Ogre::Quaternion  derivedRotate = m_lefttoolpartconvex.m_AttachedNode->_getDerivedOrientation();

		Ogre::Vector3 centerInWorld = derivedRotate * m_lefttoolpartconvex.m_CollideShapeRelOffset + derivedPos;

		Ogre::Quaternion rotateInWorld = derivedRotate * m_lefttoolpartconvex.m_CollideShapeRelRot;

		trans.SetOrigin(OgreToGPVec3(centerInWorld));

		trans.SetRotation(OgreToGPQuaternion(rotateInWorld));

		m_lefttoolpartconvex.m_rigidbody->SetWorldTransform(trans);
	}
	if(m_righttoolpartconvex.m_isvalid)
	{
		GFPhysTransform trans;

		Ogre::Vector3 derivedPos = m_righttoolpartconvex.m_AttachedNode->_getDerivedPosition();

		Ogre::Quaternion  derivedRotate = m_righttoolpartconvex.m_AttachedNode->_getDerivedOrientation();

		Ogre::Vector3 centerInWorld = derivedRotate * m_righttoolpartconvex.m_CollideShapeRelOffset + derivedPos;

		Ogre::Quaternion rotateInWorld = derivedRotate * m_righttoolpartconvex.m_CollideShapeRelRot;

		trans.SetOrigin(OgreToGPVec3(centerInWorld));

		trans.SetRotation(OgreToGPQuaternion(rotateInWorld));

		m_righttoolpartconvex.m_rigidbody->SetWorldTransform(trans);
	}
	if(m_centertoolpartconvex.m_isvalid)
	{
		GFPhysTransform trans;

		Ogre::Vector3 derivedPos = m_centertoolpartconvex.m_AttachedNode->_getDerivedPosition();

		Ogre::Quaternion  derivedRotate = m_centertoolpartconvex.m_AttachedNode->_getDerivedOrientation();

		Ogre::Vector3 centerInWorld = derivedRotate * m_centertoolpartconvex.m_CollideShapeRelOffset + derivedPos;

		Ogre::Quaternion rotateInWorld = derivedRotate * m_centertoolpartconvex.m_CollideShapeRelRot;

		trans.SetOrigin(OgreToGPVec3(centerInWorld));

		trans.SetRotation(OgreToGPQuaternion(rotateInWorld));

		m_centertoolpartconvex.m_rigidbody->SetWorldTransform(trans);
	}
}

ITool::ToolSide CTool::GetToolSide()
{
	return m_enmSide;
}

float CTool::SyncShaftAsideByHardWare(float nShaftAside, float dt)
{
	float nMaxShaftAside = GetMaxShaftAside();
	float nMinShaftAside = GetMinShaftAside();

	float CurrShaft  = GPClamped(nShaftAside, nMinShaftAside, nMaxShaftAside);
	
	float PrevShaft  = GetShaftAside();
	float deltaShaft = (CurrShaft - PrevShaft);
	
	float maxSpeed   = GetMaxShaftSpeed();
	if (fabsf(deltaShaft) > dt * maxSpeed)
	{
		deltaShaft = (deltaShaft > 0 ? dt * maxSpeed : -dt * maxSpeed);
		CurrShaft  = PrevShaft + deltaShaft;
	}

	m_nShaftAside = CurrShaft;

	//
	if (TT_CLIP_APPLICATOR == GetType())
	{
		//使用键盘操作双动器械的时候，每次角度变化1个单位。对于钛夹钳的模型
		//本身来说,如果器械头每次张开闭合移动的距离都是1个单位，就有些大了，
		//这个距离用m_fClAppliperDistance来设定。
		Ogre::SceneNode * pLeftNode    = GetLeftNode();
		Ogre::SceneNode * pRightNode   = GetRightNode();
		Ogre::SceneNode * pLeft_a1Node = GetLeft_a1Node();
		Ogre::SceneNode * pLeft_a2Node = GetLeft_a2Node();
		Ogre::SceneNode * pRight_b1Node = GetRight_b1Node();
		Ogre::SceneNode * pRight_b2Node = GetRight_b2Node();
		Ogre::SceneNode * pCenterNode   = GetCenterNode();
		
		CXMLWrapperHardwareConfig * pHardwareConfig = GetOwnerTraining()->m_pToolsMgr->GetHardwareConfigPointer();
	
		float LeftShaftScale   = pHardwareConfig->m_ClAppliper_Left_Shaft_K;
		float LeftShaftOffset  = pHardwareConfig->m_ClAppliper_Left_Shaft_B;
		
		float RightShaftScale  = pHardwareConfig->m_ClAppliper_Right_Shaft_K;
		float RightShaftOffset = pHardwareConfig->m_ClAppliper_Right_Shaft_B;
		
		if (pLeftNode != NULL)
		{
			pLeftNode->setPosition(Ogre::Vector3(0.0f, -m_nShaftAside * LeftShaftScale + LeftShaftOffset, 0.0f));

		}
		if (pRightNode != NULL)
		{
			pRightNode->setPosition(Ogre::Vector3(0.0f, m_nShaftAside * RightShaftScale + RightShaftOffset, 0.0f));
		}
		if (pLeft_a1Node != NULL)
		{
			pLeft_a1Node->setPosition(Ogre::Vector3(0.0f, -m_nShaftAside * LeftShaftScale + LeftShaftOffset, 0.0f));
		}
		if (pRight_b1Node != NULL)
		{
			pRight_b1Node->setPosition(Ogre::Vector3(0.0f, m_nShaftAside * RightShaftScale + RightShaftOffset, 0.0f));
		}

		if (pCenterNode != NULL)
		{

			pCenterNode->setPosition(Ogre::Vector3(0.0f, 0.0f, -m_nShaftAside * LeftShaftScale));
		}

		if (pLeft_a2Node != NULL)
		{
			Ogre::Vector3 v3Normal = Ogre::Vector3::UNIT_X;
			Ogre::Quaternion quatLeft(Ogre::Radian(Ogre::Degree(m_nShaftAside)), v3Normal);
			pLeft_a2Node->setOrientation(quatLeft);
			pLeft_a2Node->setPosition(Ogre::Vector3(0.0f, 0.0f, m_nShaftAside * LeftShaftScale));
		}
		if (pRight_b2Node != NULL)
		{
			Ogre::Vector3 v3Normal = Ogre::Vector3::UNIT_X;
			Ogre::Quaternion quatRight(Ogre::Radian(Ogre::Degree(-m_nShaftAside)), v3Normal);
			pRight_b2Node->setOrientation(quatRight);
			pRight_b2Node->setPosition(Ogre::Vector3(0.0f, 0.0f, m_nShaftAside * RightShaftScale));
		}
	}
	else
	{
		//rotate child node with shaft aside
		Ogre::SceneNode * pLeftNode  = GetLeftNode();
		Ogre::SceneNode * pRightNode = GetRightNode();

		float leftScale  = GetLeftShaftAsideScale();
		float rightScale = GetRightShaftAsideScale();

		if (pLeftNode)
		{
			Ogre::Vector3 v3Normal = Ogre::Vector3::UNIT_X;
			Ogre::Quaternion quatLeft(Ogre::Radian(Ogre::Degree(m_nShaftAside * leftScale)), v3Normal);
			pLeftNode->setOrientation(quatLeft);
		}
		if (pRightNode)
		{
			Ogre::Vector3 v3Normal = Ogre::Vector3::UNIT_X;
			Ogre::Quaternion quatRight(Ogre::Radian(Ogre::Degree(m_nShaftAside * (-1) * rightScale)), v3Normal);
			pRightNode->setOrientation(quatRight);
		}
	}
	//
	return m_nShaftAside;
}

void CTool::SyncToolPostureByHardware(const Ogre::Vector3 & PivotPos,//tool's pivot position in w.c.sthis should remain const all the time
	                                  const Ogre::Vector3 & TopPos, // tool's head position in w.c.s
	                                  const Ogre::Quaternion & Orient//tool orientation from z-axis
	                                  )
{
	m_pivotPosition = PivotPos;
	m_lastKernelNodePosition = m_pNodeKernel->getPosition();
	m_lastKernelNodeQuaternion = m_pNodeKernel->getOrientation();

	m_pNodeKernel->setPosition(TopPos);
	m_pNodeKernel->setOrientation(Orient);
}

void CTool::CorrectKernelNode(const Ogre::Vector3& newPos, const Ogre::Quaternion& newQuaternion)
{
	m_pNodeKernel->setPosition(newPos);
	m_pNodeKernel->setOrientation(newQuaternion);
}

void CTool::SetToolSide(ITool::ToolSide tside)
{
	m_enmSide = tside;
	if(m_enmSide == TSD_LEFT)
	{
	   m_lefttoolpartconvex.SetCollideCategry(MMRC_LeftTool);
	   m_righttoolpartconvex.SetCollideCategry(MMRC_LeftTool);
	   m_centertoolpartconvex.SetCollideCategry(MMRC_LeftTool);
	}
	else if(m_enmSide == TSD_RIGHT)
	{
	   m_lefttoolpartconvex.SetCollideCategry(MMRC_RightTool);
	   m_righttoolpartconvex.SetCollideCategry(MMRC_RightTool);
	   m_centertoolpartconvex.SetCollideCategry(MMRC_RightTool);
	}
	else
	{
	   m_lefttoolpartconvex.SetCollideCategry(MMRC_None);
	   m_righttoolpartconvex.SetCollideCategry(MMRC_None);
	   m_centertoolpartconvex.SetCollideCategry(MMRC_None);
	}
}

void CTool::CollectOperationData(float dt)
{
	//统计夹闭次数
	if(m_canClosed)
	{
		if(!m_isClosed)
		{
			if(m_nShaftAside <= m_nMinShaftAside)
			{
				m_isClosed = true;
				m_toolClosedTimes++;
			}

			//判断器械在非操作情况下是否处于闭合状态
			if(m_IsClosedInSeparateTime)
			{
				if(m_IsTouchingOrgan == false)
				{
					m_SeparateTimeWithOrgan += dt;

					if(m_SeparateTimeWithOrgan > 5.f)
						m_IsClosedInSeparateTime = false;
				}
				else
					m_SeparateTimeWithOrgan = 0.f;
			}
		}
		else
		{
			if(m_nShaftAside > m_nMinShaftAside)
				m_isClosed = false;

			m_SeparateTimeWithOrgan = 0.f;
		}
	}

	//update moved distanc
	m_TimesAfterLastStatistic += dt;
	
	if(m_pNodeKernel)
	{
		//计算移动距离
		const Ogre::Vector3& curPos = m_pNodeKernel->_getDerivedPosition();
		float dis = curPos.distance(m_curKernelPos);
		if(dis > 0.1)
		{
			m_movedDistance += dis;
			m_movedTime += m_TimesAfterLastStatistic;
			m_curKernelPos = curPos;
			float fastspeed = dis / m_TimesAfterLastStatistic;
			m_moveFastestState = (fastspeed > MoveFastestLimit) ? true : false;
			m_TimesAfterLastStatistic = 0;
		}

		//检测是否闭合插入
		if(m_canClosed && m_canCheckClosedInsertion && m_isClosedInsertion)
		{
			float dis = m_kernelInitPos.distance(m_curKernelPos);
			if(dis < 1.f)
			{
				if(!m_isClosed)
				{
					m_canCheckClosedInsertion = false;
					m_isClosedInsertion = false;
				}
			}
			else
				m_canCheckClosedInsertion = false;
		}
	}

	//计算通电持续时间
	if(m_bElectricButton)
	{
		if(!m_hasElectricAtPreFrame)
		{
			m_hasElectricAtPreFrame = true;
			m_tempElectricBeginTime = m_pOwnerTraining->GetElapsedTime();
		}
	}
	else
	{
		if(m_hasElectricAtPreFrame)
		{
			float time = m_pOwnerTraining->GetElapsedTime() - m_tempElectricBeginTime;
			if(time > m_maxKeeppingElectricTime)
			{
				m_maxKeeppingElectricBeginTime = m_tempElectricBeginTime;
				m_maxKeeppingElectricTime = time;
			}
		}
		m_hasElectricAtPreFrame = false;
	}

	clock_t nowtime = clock();
	if (m_moveFastestState)
	{
		//clock_t endtime = m_beignMoveFastTime + CLOCKS_PER_SEC / 1.5;
		//if (nowtime > endtime)
		//{
			//m_moveFastestTimes++;
		//}
		//m_beignMoveFastTime = nowtime;
	}
}

bool CTool::Update(float dt)
{
	/*
	GFPhysAlignedVectorObj<GFPhysVector3> hullVerts;
	hullVerts.push_back(GFPhysVector3(1 , 5 , 0));
	hullVerts.push_back(GFPhysVector3(1 ,-5 , 0));
	hullVerts.push_back(GFPhysVector3(-1, 5 , 0));
	hullVerts.push_back(GFPhysVector3(-1,-5, 0));

	hullVerts.push_back(GFPhysVector3(1 , 5 , 1));
	hullVerts.push_back(GFPhysVector3(1 ,-5 , 1));
	hullVerts.push_back(GFPhysVector3(-1, 5 , 1));
	hullVerts.push_back(GFPhysVector3(-1,-5, 1));

    GFPhysVector3 ObbExt;
    GFPhysVector3 ObbAixs[3];
   CalConvexHullObb(hullVerts , ObbExt , ObbAixs);*/





	if(m_isFirstFrame)
	   OnFirstFrame();

	if (m_bDisable) 
	    return true;
	
	UpdateConvex();//bacon omit

	if(m_bElectricLeftPad)
	   m_leftPadElectricTime += dt;

	if(m_bElectricRightPad)
	   m_rightPadElectricTime += dt;

	checkWarn();

	//收集器械数据
	CollectOperationData(dt);

	ReleaseRopeWhenClampNeedleAndRope();

	return true;
}

float CTool::GetMaxKeeppingElectricBeginTime()
{
	float beginTime = m_maxKeeppingElectricBeginTime;
	if(m_hasElectricAtPreFrame && m_pOwnerTraining)
	{
		if(m_pOwnerTraining->GetElapsedTime() - m_tempElectricBeginTime > m_maxKeeppingElectricTime)
			beginTime = m_tempElectricBeginTime;
	}
	
	return beginTime;
}

float CTool::GetMaxKeeppingElectricTime()
{
	float time = m_maxKeeppingElectricTime;
	if(m_hasElectricAtPreFrame && m_pOwnerTraining)
	{
		float t = m_pOwnerTraining->GetElapsedTime() - m_tempElectricBeginTime;
		if(t > m_maxKeeppingElectricTime)
			time = t;
	}

	return time;
}

void CTool::UpdateValidElectricTime(float dt)
{
	m_validElectricTime += dt;
}

void CTool::UpdateTotalElectricTime(float dt)
{
	m_totalElectricTime += dt;
}

void CTool::UpdateAffectTimeOfCheckObject(float dt)
{	
	//更新通电对周围物体造成的影响时间
	if(!m_hasRealElectricAttribute || (m_bElectricLeftPad || m_bElectricRightPad) == false)
    {
        m_CumTime = 0.0f;
        return;
    }
	
	GFPhysVector3 closePoint[2];
	GFPhysRigidBody* rigidBodys[3];
	rigidBodys[0] = m_lefttoolpartconvex.m_rigidbody;
	rigidBodys[1] = m_righttoolpartconvex.m_rigidbody;
	rigidBodys[2] = m_centertoolpartconvex.m_rigidbody;
	
	for(size_t i = 0;i < m_curFrameToolElectricCheckObject.size();++i)
	{
		bool checkSuccess = false;
		float threshold = 0.f;
		float minDis = 10000.f;
        GFPhysSoftBodyFace face;
        Ogre::Vector2 textureCoord;

		ToolElectricCheckObject & checkObject = m_curFrameToolElectricCheckObject[i];
		switch(checkObject.GetType())
		{
		case ToolElectricCheckObject::OT_OrdinaryOrgan:
			{
				MisMedicOrgan_Ordinary * organ = static_cast<MisMedicOrgan_Ordinary*>(checkObject.GetObject());
				threshold = checkObject.GetEffectDistance();
				for(int r = 0;r < 3;++r)
				{
					if(rigidBodys[r])
					{
						checkSuccess = GetRigidSoftClosestDist(rigidBodys[r],organ->m_physbody,threshold,closePoint,face,minDis);
						if(checkSuccess)
						{
							//m_pOwnerTraining->showDebugInfo(minDis,3);
                            //获取soft上最近点的纹理坐标                            
                            float weis[3];
                                                        
                            CalcBaryCentric(face.m_Nodes[0]->m_CurrPosition,
                                face.m_Nodes[1]->m_CurrPosition,
                                face.m_Nodes[2]->m_CurrPosition,
                                closePoint[1],
                                weis[0],
                                weis[1],
                                weis[2]);

                            textureCoord = organ->GetTextureCoord(&face , weis);							
                            break;
						}
					}
				}
			}
			break;
		case ToolElectricCheckObject::OT_HemoClip:
			{
				static GFPhysVector3 aabbMin,aabbMax;
				static GFPhysVector3 translation;
				static GFPhysQuaternion direction;

				MisMedicTitaniumClampV2 * pHemoClip = static_cast<MisMedicTitaniumClampV2*>(checkObject.GetObject());
				threshold = checkObject.GetEffectDistance();
				pHemoClip->GetLocalAABB(aabbMin,aabbMax);

				for(int r = 0;r < 3;++r)
				{
					if(rigidBodys[r])
					{
						checkSuccess = GetRigidAabbClosestDist(rigidBodys[r],
																aabbMin,
																aabbMax,
																pHemoClip->GetTranslation(),
																pHemoClip->GetQuaternion(),
																threshold,
																closePoint,
																minDis);
						if(checkSuccess)
						{
							//m_pOwnerTraining->showDebugInfo(minDis,3);//这段很耗性能请查原因
							break;
						}
					}
					
				}
			}
			break;
		default:
			continue;
		}

		if(checkSuccess)
		{
			checkObject.AddEffectTime(dt);
           
            if(m_TextureCoord.distance(textureCoord) < 1.0f)
            {                                
                m_CumTime += dt;
            }
            else
            {                
                m_CumTime = dt;
            }
            m_TextureCoord = textureCoord;
            //m_pOwnerTraining->showDebugInfo(m_CumTime,3);            
		}
	}

	//merg object info and clear object
	for(size_t i = 0;i < m_curFrameToolElectricCheckObject.size();++i)
	{
		ToolElectricCheckObject& checkObject = m_curFrameToolElectricCheckObject[i];
		void * pObject = checkObject.GetObject();
		std::map<void*,ToolElectricCheckObject>::iterator itr = m_toolElectricCheckObjectMap.find(pObject);
		if(itr != m_toolElectricCheckObjectMap.end())
        {
            itr->second.AddEffectTime(checkObject.GetEffecTime());                        
        }
		else
			m_toolElectricCheckObjectMap.insert(std::make_pair(pObject,checkObject));
	}

	m_curFrameToolElectricCheckObject.clear();
}

bool CTool::Terminate()
{
	//Release();

	//ClearHeldPoints();

	// destroy manual object
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();

	if (m_pManualObj && m_ps4mNode)
	{
		m_ps4mNode->detachObject(m_pManualObj);
		pSceneManager->destroyManualObject(m_pManualObj);
		pSceneManager->getRootSceneNode()->removeChild(m_ps4mNode);
		pSceneManager->destroySceneNode(m_ps4mNode);
	}

	if (m_pTriangleObj && m_pTriangleNode)
	{
		m_pTriangleNode->detachObject(m_pTriangleObj);
		pSceneManager->destroyManualObject(m_pTriangleObj);
		pSceneManager->getRootSceneNode()->removeChild(m_pTriangleNode);
		pSceneManager->destroySceneNode(m_pTriangleNode);
	}

	Ogre::SceneNode::ChildNodeIterator iterObject = m_pNodeKernel->getChildIterator();
	while ( iterObject.hasMoreElements() )
	{
		Ogre::SceneNode * pNode = (Ogre::SceneNode *)iterObject.getNext();
		Ogre::SceneNode::ObjectIterator iterAttachedObject = pNode->getAttachedObjectIterator();

		while ( iterAttachedObject.hasMoreElements() )
		{
			Ogre::Entity * pEntity = (Ogre::Entity *)iterAttachedObject.getNext();
			pNode->detachObject(pEntity);
			pSceneManager->destroyEntity(pEntity);

		}
		pSceneManager->destroySceneNode(pNode);
	}

	pSceneManager->getRootSceneNode()->removeChild(m_pNodeKernel);
	m_pNodeKernel->detachAllObjects();
	pSceneManager->destroySceneNode(m_pNodeKernel);
	m_pNodeKernel = NULL;
	//m_pNodeKernel->setVisible(false);
	m_bDisable = true;
	m_bIsInUse = false;

	vector< Ogre::String > & VecMaterial = (m_enmSide==TSD_LEFT) ?  m_vecToolMaterial_backup:m_vecToolMaterial;
	for ( size_t i = 0; i< VecMaterial.size(); i++ )
	{
		EffectManager::Instance()->SetMaterialAmbient(VecMaterial[i],m_vecAmbient[i]);
		EffectManager::Instance()->SetMaterialDiffuse(VecMaterial[i],m_vecDiffuse[i]);
	}

	return true;
}
//=================================================================================
void CTool::SetOriginalInfo(ToolComponent enmToolComponent)
{
	//set left Original info

	Ogre::Node * pNode = NULL;
	if (enmToolComponent == TC_LEFT)
	{
		pNode = GetLeftNode();
	}
	else if (enmToolComponent == TC_RIGHT)
	{
		pNode = GetRightNode();
	}
	else if (enmToolComponent == TC_KERNEL)
	{
		pNode = GetKernelNode();
	}

	if (!pNode)
	{
		return;
	}

	Ogre::Vector3 v3OriginalPos = pNode->_getDerivedPosition();
	Ogre::Quaternion quatOriginal = pNode->_getDerivedOrientation();
	Ogre::Vector3 v3OriginalScale = pNode->_getDerivedScale();
	Ogre::Matrix4 mxTrans(1,0,0,0, 0,1,0,0, 0,0,1,0, v3OriginalPos.x, v3OriginalPos.y, v3OriginalPos.z, 1);
	Ogre::Matrix4 mxScale(v3OriginalScale.x,0,0,0, 0,v3OriginalScale.y,0,0, 0,0,v3OriginalScale.z,0, 0,0,0,1);
	Ogre::Matrix3 mx3Rotate;
	quatOriginal.ToRotationMatrix(mx3Rotate);
	Ogre::Matrix4 mx4Rotate(mx3Rotate[0][0], mx3Rotate[1][0], mx3Rotate[2][0], 0,
		mx3Rotate[0][1], mx3Rotate[1][1], mx3Rotate[2][1], 0,
		mx3Rotate[0][2], mx3Rotate[1][2], mx3Rotate[2][2], 0,
		0, 0, 0, 1);
	//Ogre::Matrix4 mx4Rotate(mx3Rotate[0][0], mx3Rotate[0][1], mx3Rotate[0][2], 0,
	//						mx3Rotate[1][0], mx3Rotate[1][1], mx3Rotate[1][2], 0,
	//						mx3Rotate[2][0], mx3Rotate[2][1], mx3Rotate[2][2], 0,
	//						0, 0, 0, 1);
	Ogre::Matrix4 mxOriginal = mxScale * mx4Rotate * mxTrans;

	if (enmToolComponent == TC_LEFT)
	{
		SetOriginalLeftPos(v3OriginalPos);
		SetOriginalLeftOrientation(quatOriginal);
		SetOriginalLeftMatrix(mxOriginal);
	}
	else if (enmToolComponent == TC_RIGHT)
	{
		SetOriginalRightPos(v3OriginalPos);
		SetOriginalRightOrientation(quatOriginal);
		SetOriginalRightMatrix(mxOriginal);
	}
	else if (enmToolComponent == TC_KERNEL)
	{
		SetOriginalKernelPos(v3OriginalPos);
		SetOriginalKernelOrientation(quatOriginal);
		SetOriginalKernelMatrix(mxOriginal);
	}
}

void CTool::checkWarn()
{
	if (!m_bWarnState)  return;
	if (m_bFirstEnterWarn)
	{
		m_bFirstEnterWarn = false;
		m_WarnTick = GetTickCount(); 
		vector< Ogre::String > & VecMaterial = (m_enmSide==TSD_LEFT) ?  m_vecToolMaterial_backup:m_vecToolMaterial;
		for ( size_t i = 0; i< VecMaterial.size(); i++ )
		{
			EffectManager::Instance()->SetMaterialAmbient(VecMaterial[i],Ogre::ColourValue(5,0,0,1));
			EffectManager::Instance()->SetMaterialDiffuse(VecMaterial[i],Ogre::ColourValue(5,0,0,1));
			//EffectManager::Instance()->SetMaterialColour(VecMaterial[i],0.8f,Ogre::LBS_MANUAL,Ogre::LBS_MANUAL,Ogre::ColourValue(1,0,0),Ogre::ColourValue(1,0,0));
		}
	}
	else
	{
		Ogre::Real tick = GetTickCount()-m_WarnTick;
		if ( tick > 500 )
		{
			m_bFirstEnterWarn = true;
			m_bWarnState = false;
			vector< Ogre::String > & VecMaterial = (m_enmSide==TSD_LEFT) ?  m_vecToolMaterial_backup:m_vecToolMaterial;
			for ( size_t i = 0; i< VecMaterial.size(); i++ )
			{
				EffectManager::Instance()->SetMaterialAmbient(VecMaterial[i],m_vecAmbient[i]);
				EffectManager::Instance()->SetMaterialDiffuse(VecMaterial[i],m_vecDiffuse[i]);
				//EffectManager::Instance()->SetMaterialColour(VecMaterial[i],0.0f,Ogre::LBS_TEXTURE,Ogre::LBS_TEXTURE);
			}
		}
	}
}

void CTool::warn()
{
	m_WarnTick = GetTickCount();
	m_bWarnState = true;
}

void CTool::SetBackupMaterial()
{
	Ogre::SceneNode *  pKernelNode =  GetKernelNode();
	if (!pKernelNode)  return; 
	m_vecToolMaterial.clear();
	m_vecToolMaterial_backup.clear();
	m_vecAmbient.clear();
	m_vecDiffuse.clear();

	SetNodeMaterial(pKernelNode);
}

void CTool::SetNodeMaterial( Ogre::Node * node )
{
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();
	if (!node)  return;
	int childNum = node->numChildren();
	for (int k=0;k<childNum;k++)
	{
		Ogre::Node *  childNode = node->getChild(k);
		if (childNode->numChildren())
		{
			SetNodeMaterial(childNode);//递归遍历所有结点
		}
		if (pSceneManager->hasEntity(childNode->getName()))
		{
			Ogre::Entity *  entity = pSceneManager->getEntity(childNode->getName());
			int numSub = entity->getNumSubEntities();
			for (int m=0;m<numSub;m++)
			{
				Ogre::SubEntity*  subEntity = entity->getSubEntity(m);
				Ogre::String name = subEntity->getMaterialName();
				Ogre::String materialName = name+"_backup";
				if (Ogre::MaterialManager::getSingleton().resourceExists(materialName))
				{
					Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(materialName);
					if (m_enmSide == TSD_LEFT )//左手器械使用备份材质
					{
						subEntity->setMaterialName(materialName);
					}
					m_vecToolMaterial.push_back(name);
					m_vecToolMaterial_backup.push_back(materialName);

					Ogre::Pass *  pass = material->getTechnique(0)->getPass(0);
					m_vecAmbient.push_back(pass->getAmbient());
					m_vecDiffuse.push_back(pass->getDiffuse());
				}
			}
		}
	}
}

//for new train start
void CTool::ReadCollisionData(const Ogre::String & toolname)
{
	//collision part
	if (toolname == "")
		return;

	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();
	MisToolCollideDataConfig & toolcollidecfg = MisToolCollideDataConfigMgr::Instance()->GetToolCollideConfig(toolname);
	
	if( toolcollidecfg.m_left.m_Valid)
	{
		m_lefttoolpartconvex.CreateRigidBodyByConfig(toolcollidecfg.m_left);

		if(m_lefttoolpartconvex.m_rigidbody)
		{
		   m_lefttoolpartconvex.m_rigidbody->EnableDoubleFaceCollision();

		   m_lefttoolpartconvex.m_rigidbody->SetDamping(0.0f , 0.0f);

		   m_lefttoolpartconvex.m_rigidbody->SetCollisionFlags(GFPhysCollideObject::CF_KINEMATIC_OBJECT);

		   if(toolcollidecfg.m_left.m_ShowDebug)
			  m_lefttoolpartconvex.CreateDebugDrawable(pSceneManager);
		}
	}

	if( toolcollidecfg.m_right.m_Valid)
	{
		m_righttoolpartconvex.CreateRigidBodyByConfig(toolcollidecfg.m_right);

		if(m_righttoolpartconvex.m_rigidbody)
		{
			m_righttoolpartconvex.m_rigidbody->EnableDoubleFaceCollision();

			m_righttoolpartconvex.m_rigidbody->SetDamping(0.0f , 0.0f);

			m_righttoolpartconvex.m_rigidbody->SetCollisionFlags(GFPhysCollideObject::CF_KINEMATIC_OBJECT);

			if(toolcollidecfg.m_right.m_ShowDebug)
			   m_righttoolpartconvex.CreateDebugDrawable(pSceneManager);
		}
	}

	if( toolcollidecfg.m_center.m_Valid)
	{
		m_centertoolpartconvex.CreateRigidBodyByConfig(toolcollidecfg.m_center);

		if(m_centertoolpartconvex.m_rigidbody)
		{
			m_centertoolpartconvex.m_rigidbody->EnableDoubleFaceCollision();

			m_centertoolpartconvex.m_rigidbody->SetDamping(0.0f , 0.0f);

			m_centertoolpartconvex.m_rigidbody->SetCollisionFlags(GFPhysCollideObject::CF_KINEMATIC_OBJECT);

			if(toolcollidecfg.m_center.m_ShowDebug)
				m_centertoolpartconvex.CreateDebugDrawable(pSceneManager);
		}
	}
}
//=====================================================================================================
int CTool::GetRigidBodyPart(GFPhysCollideObject * rigidbody)//0-left 1-right 2-center -1no
{
	if(m_lefttoolpartconvex.m_rigidbody == rigidbody)
	   return 0;
	else if(m_righttoolpartconvex.m_rigidbody == rigidbody)
	   return 1;
	else if(m_centertoolpartconvex.m_rigidbody == rigidbody)
	   return 2;
	else
	   return -1;
}
//================================================================================================
const NewTrainToolConvexData * CTool::GetRigidBodyBelongPart(GFPhysCollideObject * rigidbody)
{
	if (m_lefttoolpartconvex.m_rigidbody == rigidbody)
		return &m_lefttoolpartconvex;
	
	else if (m_righttoolpartconvex.m_rigidbody == rigidbody)
		return &m_righttoolpartconvex;
	
	else if (m_centertoolpartconvex.m_rigidbody == rigidbody)
		return &m_centertoolpartconvex;
	else
		return 0;
}
//================================================================================================
void CTool::GetSoftNodeBeingGrasped(std::set<GFPhysSoftBodyNode*> & NodesBeGrasped)
{
	return;
}
//=========================================================================
void CTool::onFrameUpdateStarted(float timeelapsed)
{
	//call plugins
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->OneFrameUpdateStarted(timeelapsed);
	}
}
//=========================================================================
void CTool::onFrameUpdateEnded()
{
	if(m_CanPunctureOgran)//check whether
	{
		int   maxPunctueFaceIndex = -1;
		float maxPunctueDist = 0;
		for(size_t f = 0 ; f < m_ToolColliedFaces.size() ; f++)
		{
			const ToolCollidedFace & cdface = m_ToolColliedFaces[f];

			const NewTrainToolConvexData * toolPart = GetRigidBodyBelongPart(cdface.m_collideRigid);

			if (toolPart == 0)
				continue;

			if (toolPart -> m_bSharp == false)
				continue;

			GFPhysSoftBodyFace * face = cdface.m_collideFace;

			if(face->m_GenFace && face->m_GenFace->m_ShareTetrahedrons.size() > 0)
			{
				GFPhysSoftBodyTetrahedron * tetra = face->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;
				if(tetra)
				{
					/*
					GFPhysVector3 faceNorm = (face->m_Nodes[1]->m_UnDeformedPos-face->m_Nodes[0]->m_UnDeformedPos).Cross(face->m_Nodes[2]->m_UnDeformedPos-face->m_Nodes[0]->m_UnDeformedPos);
					faceNorm.Normalize();

					GFPhysVector3 deformderv = GFPhysSoftBody::CalTetraDeformationDerivative(*tetra , faceNorm);
					float deformValue = deformderv.Dot(faceNorm)-1.0f;

					if(deformValue < maxPunctueDist)
					{
						maxPunctueDist = deformValue;
						maxPunctueFaceIndex = f;
					}*/
					
					GFPhysSoftBodyNode * NodeOppsite = 0;
					for(size_t c = 0 ; c < 4 ; c++)
					{
						GFPhysSoftBodyNode * temp = tetra->m_TetraNodes[c];
						if(temp != face->m_Nodes[0] && temp != face->m_Nodes[1] && temp != face->m_Nodes[2])
						{
							NodeOppsite = temp;
						}
					}
					GFPhysVector3 FaceNormalCurr = (face->m_Nodes[1]->m_CurrPosition-face->m_Nodes[0]->m_CurrPosition).Cross(face->m_Nodes[2]->m_CurrPosition-face->m_Nodes[0]->m_CurrPosition);
					float currFaceArea2 = FaceNormalCurr.Length();
					FaceNormalCurr = FaceNormalCurr / currFaceArea2;
					float distCurr = (NodeOppsite->m_CurrPosition-face->m_Nodes[0]->m_CurrPosition).Dot(FaceNormalCurr);

					GFPhysVector3 FaceNormalOrigin = (face->m_Nodes[1]->m_UnDeformedPos-face->m_Nodes[0]->m_UnDeformedPos).Cross(face->m_Nodes[2]->m_UnDeformedPos-face->m_Nodes[0]->m_UnDeformedPos);
					float originFaceArea2 = FaceNormalOrigin.Length();
					FaceNormalOrigin = FaceNormalOrigin / originFaceArea2;
					float distOrigin = (NodeOppsite->m_UnDeformedPos-face->m_Nodes[0]->m_UnDeformedPos).Dot(FaceNormalOrigin);
					
					if(currFaceArea2 > originFaceArea2)
					{
					   distOrigin *= (originFaceArea2 / currFaceArea2);
					}
					float delta = distCurr-distOrigin;
					if(delta > maxPunctueDist)
					{
						maxPunctueDist = delta;
					    maxPunctueFaceIndex = f;
					}
					
				}
			}
		}

		maxPunctueDist *= -1.0f;

		if(maxPunctueFaceIndex >= 0)
		{
			const ToolCollidedFace & cdface = m_ToolColliedFaces[maxPunctueFaceIndex];
			MisMedicOrganInterface * organif = (MisMedicOrganInterface * )cdface.m_collideSoft->GetUserPointer();
			if(organif && maxPunctueDist < (-organif->GetMinPunctureDist()*m_PuncThresholdMultiply))//> organif->GetMinPunctureDist()*m_PuncThresholdMultiply)
			   organif->ToolPunctureSurface(this , cdface.m_collideFace , cdface.m_collideWeights);

		}
		
	}
	
	//call plugins
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->OneFrameUpdateEnded();
	}
}

//=========================================================================
bool CTool::GetForceFeedBack(Ogre::Vector3 & contactForce, Ogre::Vector3 & dragForce)
{
	int ForceCount = (int)m_ForceFeedRecords.size();

	contactForce = Ogre::Vector3(0, 0, 0);

	dragForce = Ogre::Vector3(0, 0, 0);

	//float averageStickPercent = 0;

	//if(ForceCount > 0)
	//{
	for(int f = 0 ; f < ForceCount ; f++)
	{
		contactForce += m_ForceFeedRecords[f].m_ContactForce;
		dragForce += m_ForceFeedRecords[f].m_DragForce;
	}
	return true;
		//averageForce /= ForceCount;
		//averageStickPercent /= ForceCount;
	//}

	//stickforceepercent = averageStickPercent;

	//return Ogre::Vector3(averageForce.x , averageForce.y , averageForce.z);
}
//========================================================================
Ogre::Vector3 CTool::GetForceFeedBackPoint()
{
	return m_FeedBackForcePoint;
}
//=================================================================================
void CTool::ClearForceFeedBack()
{
	m_RawForceFeed.clear();
}
//=================================================================================
float CTool::ModifySurfaceFeedback(float CurrSurfaceFeed , float LastSurfaceFeed)
{
	if(CurrSurfaceFeed < LastSurfaceFeed)
		return CurrSurfaceFeed;

	float BaseValue = 0.7f;

	if(CurrSurfaceFeed <= BaseValue)
		return CurrSurfaceFeed;

	else
	{
		float AdjustValue = CurrSurfaceFeed - LastSurfaceFeed;

		AdjustValue = log(1+AdjustValue*0.1) / log(3.0f);

		return AdjustValue+LastSurfaceFeed;
	}
}
GFPhysVector3 CTool::CalculateToolCustomForceFeedBack()
{
	//Total Drag force
	GFPhysVector3 TotalDragForce(0,0,0);

	for(size_t p = 0 ; p < m_ToolPlugins.size() ; p++)
	{
		MisMedicCToolPluginInterface * clampPlugin = m_ToolPlugins[p];// dynamic_cast<MisCTool_PluginClamp*>(m_ToolPlugins[p]);

		if(clampPlugin)
		{
			TotalDragForce += clampPlugin->GetPluginForceFeedBack();
		}
	}

	TotalDragForce += GFPhysVector3(m_currentExternalForce[0], m_currentExternalForce[1], m_currentExternalForce[2]);
	//if (m_currentExternalForce[0] != 0.0) printf("EXTFORCE: %f %f %f\n", m_currentExternalForce[0], m_currentExternalForce[1], m_currentExternalForce[2]);
	m_currentExternalForce[0] = 0.0;
	m_currentExternalForce[1] = 0.0;
	m_currentExternalForce[2] = 0.0;


	return TotalDragForce;
}
//=============================================================================================================
void  CTool::CalculateToolForceFeedBack(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	
}
//==========================================================================================
void  CTool::CalculateToolForceFeedBack(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSFaceContacts)
{
	RawSurfaceForceFeedLeft=GFPhysVector3(0, 0, 0);
	
	RawSurfaceForceFeedRight = GFPhysVector3(0, 0, 0);

	RawSurfaceForceFeedCenter = GFPhysVector3(0, 0, 0);

	//Get Raw right and left part surface feed back
	for (size_t c = 0; c < RSFaceContacts.size(); c++)
	{
		const GFPhysSoftFaceRigidContact & rsContact = RSFaceContacts[c];

		//if (rsContact.m_NormOnRigidWorld.Dot(rsContact.m_SoftFace->m_FaceNormal) > 0)
		//	continue;

		float sumImpluse = rsContact.GetNormalImpluse(0) + rsContact.GetNormalImpluse(1) + rsContact.GetNormalImpluse(2);

		//sumImpluse = sumImpluse*(rsContact.m_SoftFace->m_RestAreaMult2 > 1.0f ? 1.0f : rsContact.m_SoftFace->m_RestAreaMult2);

		GFPhysVector3 contactImpluse = ((-rsContact.m_NormOnRigidWorld) * sumImpluse);

		MisMedicOrganInterface * dynif = (MisMedicOrganInterface *)rsContact.m_SoftBody->GetUserPointer();

		MisMedicOrgan_Ordinary * organDyn = dynamic_cast<MisMedicOrgan_Ordinary *>(dynif);

		float forcerate = dynif->GetForceFeedBackRation()*m_ToolForceBackRate*0.03f;

		contactImpluse *= forcerate;

		bool toolunattch = true;
		if (rsContact.m_Rigid == m_lefttoolpartconvex.m_rigidbody)
		{
			toolunattch = false;
			RawSurfaceForceFeedLeft += contactImpluse;						// 两个区
		}
		else if (rsContact.m_Rigid == m_righttoolpartconvex.m_rigidbody)
		{
			toolunattch = false;
			RawSurfaceForceFeedRight += contactImpluse;
		}
		else if (rsContact.m_Rigid == m_centertoolpartconvex.m_rigidbody)
		{
			toolunattch = false;
			RawSurfaceForceFeedCenter += contactImpluse;
		}
		if (m_enmSide == TSD_LEFT)
		{
			dynif->setFlag(EMMOI_Touching_By_Tool_Left, toolunattch);
		}
		else
		{
			dynif->setFlag(EMMOI_Touching_By_Tool_Right, toolunattch);
		}
	}

	
	{

	}
	//Total contact force
	GFPhysVector3 TotalContactForce = RawSurfaceForceFeedLeft + RawSurfaceForceFeedRight + RawSurfaceForceFeedCenter;
	//float organContactMag = TotalContactForce.Length();
	//if (organContactMag > FLT_EPSILON)
	//{
	//	GFPhysVector3 organContactForceDir = TotalContactForce / organContactMag;
	//	TotalContactForce = organContactForceDir * (powf(organContactMag, 0.75)) * 1.5f;
	//}
	//if (TotalContactForce.Length() > 1.5f)
	//{
	//	TotalContactForce *= (1.5f / TotalContactForce.Length());
	//}
	//force form Training defined Rigid body
	GFPhysVector3 TrainingDefinedForece = m_pOwnerTraining->GetTrainingForceFeedBack(this);

	//Drag force
	GFPhysVector3 TotalDragForce = CalculateToolCustomForceFeedBack() * 0.012f;
	if (TotalDragForce.Length() > 0.00001f)
	{
		TotalContactForce = GFPhysVector3(0, 0, 0);//disable force when drag force has
	}

	// force form thread
	std::set<MisMedicThreadRope*> RopesInContact;
	for (size_t c = 0; c < m_ToolCollidedThreads.size(); c++)
	{
		ToolCollideThreadSegment & segment = m_ToolCollidedThreads[c];
		RopesInContact.insert(segment.m_collideThread);
	}

	std::set<MisMedicThreadRope*>::iterator ropeitor = RopesInContact.begin();
	while (ropeitor != RopesInContact.end())
	{
		MisMedicThreadRope * collideRop = (*ropeitor);
		if (collideRop)
		{
			GFPhysVector3 threadforce(0, 0, 0);
			const GFPhysAlignedVectorObj<TRCollidePair> & trPairs = collideRop->GetCollidePairsWithRigid();
			for (size_t c = 0; c < trPairs.size(); c++)
			{
				const TRCollidePair & trpair = trPairs[c];
				if (trpair.m_Rigid == m_lefttoolpartconvex.m_rigidbody
					|| trpair.m_Rigid == m_righttoolpartconvex.m_rigidbody
					|| trpair.m_Rigid == m_centertoolpartconvex.m_rigidbody)
				{
					GFPhysVector3 trContactImpluse = trpair.m_NormalOnRigid *trpair.m_ImpluseNormalOnRigid;
					threadforce += trContactImpluse;
				}
			}
			threadforce.Normalize();

			float originLen = collideRop->GetTotalLen(false);

			float currLen = collideRop->GetTotalLen(true);

			int segmentNum = collideRop->GetNumThreadNodes() - 1;

			int unitTolerant = collideRop->GetUnitLen() * 0.15f;

			float delatLen = (currLen - (originLen + unitTolerant*segmentNum));//give some tolerant

			if (delatLen <= 0)
				delatLen = 0;

			TotalContactForce += threadforce*delatLen*0.15f;
		}
		ropeitor++;
	}
#pragma region MyRegion
	//////////////////////////////////////////////////////////////////////////
	// force form thread
	std::set<SutureThread*> SutureInContact;
	for (size_t c = 0; c < m_ToolCollidedSutureThreads.size(); c++)
	{
		ToolCollideSutureThreadSegment & segment = m_ToolCollidedSutureThreads[c];
		SutureInContact.insert(segment.m_collideSutureThread);
	}

	std::set<SutureThread*>::iterator sutitor = SutureInContact.begin();
	while (sutitor != SutureInContact.end())
	{
		SutureThread * collideRop = (*sutitor);
		if (collideRop)
		{
			GFPhysVector3 threadforce(0, 0, 0);
			const GFPhysAlignedVectorObj<TRCollidePair> & trPairs = collideRop->GetCollidePairsWithRigid();
			for (size_t c = 0; c < trPairs.size(); c++)
			{
				const TRCollidePair & trpair = trPairs[c];
				if (trpair.m_Rigid == m_lefttoolpartconvex.m_rigidbody
					|| trpair.m_Rigid == m_righttoolpartconvex.m_rigidbody
					|| trpair.m_Rigid == m_centertoolpartconvex.m_rigidbody)
				{
					GFPhysVector3 trContactImpluse = trpair.m_NormalOnRigid *trpair.m_ImpluseNormalOnRigid;
					threadforce += trContactImpluse;
				}
			}
			threadforce.Normalize();

			float originLen = collideRop->GetTotalLen(false);

			float currLen = collideRop->GetTotalLen(true);

			int segmentNum = collideRop->GetNumThreadNodes() - 1;

			int unitTolerant = collideRop->GetRestLen() * 0.15f;

			float delatLen = (currLen - (originLen + unitTolerant*segmentNum));//give some tolerant

			if (delatLen <= 0)
				delatLen = 0;

			TotalContactForce += threadforce*delatLen*0.15f;

			//qDebug() << "TotalContactForce" << ":" << TotalContactForce.Length();

			//printf("%f \n", TotalContactForce.Length());

		}
		sutitor++;
	}


	// force form threadv2
	std::set<SutureThreadV2*> SutureInContact2;
	for (size_t c = 0; c < m_ToolCollidedSutureThreadsV2.size(); c++)
	{
		ToolCollideSutureThreadSegmentV2 & segment = m_ToolCollidedSutureThreadsV2[c];
		SutureInContact2.insert(segment.m_collideSutureThread);
	}

	std::set<SutureThreadV2*>::iterator sutitor2 = SutureInContact2.begin();
	while (sutitor2 != SutureInContact2.end())
	{
		SutureThreadV2 * collideRop = (*sutitor2);
		if (collideRop)
		{
			GFPhysVector3 threadforce(0, 0, 0);
			const GFPhysAlignedVectorObj<STVRGCollidePair> & trPairs = collideRop->GetCollidePairsWithRigid();
			for (size_t c = 0; c < trPairs.size(); c++)
			{
				const STVRGCollidePair & trpair = trPairs[c];
				if (trpair.m_Rigid == m_lefttoolpartconvex.m_rigidbody
					|| trpair.m_Rigid == m_righttoolpartconvex.m_rigidbody
					|| trpair.m_Rigid == m_centertoolpartconvex.m_rigidbody)
				{
					GFPhysVector3 trContactImpluse = trpair.m_NormalOnRigid *trpair.m_ImpluseNormalOnRigid;
					threadforce += trContactImpluse;
				}
			}
			threadforce.Normalize();

			float originLen = collideRop->GetThreadRestLen();

			float currLen = collideRop->GetThreadCurrLen();

			int segmentNum = collideRop->GetNumThreadNodes() - 1;

			int unitTolerant = collideRop->GetRestLen() * 0.15f;

			float delatLen = (currLen - (originLen + unitTolerant*segmentNum));//give some tolerant

			if (delatLen <= 0)
				delatLen = 0;

			TotalContactForce += threadforce*delatLen*0.15f;

			//qDebug() << "TotalContactForce" << ":" << TotalContactForce.Length();

			//printf("%f \n", TotalContactForce.Length());

		}
		sutitor2++;
	}
	//////////////////////////////////////////////////////////////////////////
#pragma endregion

	


	//Total Impulse
	if (m_ForceFeedRecords.size() > 0)
	{
		ForceFeedBackRecord & lastrecord = m_ForceFeedRecords[m_ForceFeedRecords.size() - 1];

		if (TotalContactForce.Length2() > 0 && lastrecord.m_ContactForce.squaredLength() > 0)
		{
			float ChangeThreshold = 0.05f;
			float SmoothFactor = 0.1f;

			TotalContactForce = TotalContactForce * SmoothFactor
				+ OgreToGPVec3(lastrecord.m_ContactForce) * (1 - SmoothFactor);

			GFPhysVector3 forceDeviate = TotalContactForce - OgreToGPVec3(lastrecord.m_ContactForce);
			if (forceDeviate.Length2() > ChangeThreshold*ChangeThreshold)
				forceDeviate *= (ChangeThreshold / forceDeviate.Length());
			TotalContactForce = OgreToGPVec3(lastrecord.m_ContactForce) + forceDeviate;
		}

		if (TotalDragForce.Length2() > 0 && lastrecord.m_DragForce.squaredLength() > 0)
		{
			float SmoothFactor = 0.1f;
			TotalDragForce = TotalDragForce * SmoothFactor
				+ OgreToGPVec3(lastrecord.m_DragForce) * (1 - SmoothFactor);
		}
	}

	//clamp max magnitude of force
	//if(CurrForceFeed > 2.4f)
	//  CurrForceFeed = 2.4f;

	m_FeedBackForcePoint = Ogre::Vector3::ZERO;

	int MaxSignaleNum = 2;

	while ((int)m_ForceFeedRecords.size() >= MaxSignaleNum)
	{
		m_ForceFeedRecords.erase(m_ForceFeedRecords.begin());
	}

	//add current record
	ForceFeedBackRecord currFeedRecord;
	currFeedRecord.m_ContactForce = Ogre::Vector3(TotalContactForce);
	currFeedRecord.m_DragForce = Ogre::Vector3(TotalDragForce);
	m_ForceFeedRecords.push_back(currFeedRecord);
}
//==========================================================================================
int CTool::GetCollideVeinObjectBody(GFPhysRigidBody * bodies[3])
{
	GFPhysRigidBody * bodyl = m_lefttoolpartconvex.m_rigidbody;
	GFPhysRigidBody * bodyr = m_righttoolpartconvex.m_rigidbody;
	bodies[0] = bodyl;
	bodies[1] = bodyr;
	return 2;
}
//=====================================================================================================
/*
void CTool::AddDisableRSCollideFaces(const std::vector<GFPhysSoftBodyFace *> & faces)
{
	for(size_t f = 0 ; f < faces.size(); f++)
	{
		//m_DisableCollisionFaces.insert(faces[f]);
		m_DisableCollisionNodes.insert(faces[f]->m_Nodes[0]);
		m_DisableCollisionNodes.insert(faces[f]->m_Nodes[1]);
		m_DisableCollisionNodes.insert(faces[f]->m_Nodes[2]);
	}
}
void CTool::AddDisableRSCollideFace(GFPhysSoftBodyFace * face)
{
	m_DisableCollisionNodes.insert(face->m_Nodes[0]);
	m_DisableCollisionNodes.insert(face->m_Nodes[1]);
	m_DisableCollisionNodes.insert(face->m_Nodes[2]);
}
//=====================================================================================================
void CTool::ClearAllDisableRsCollideFace()
{
	//m_DisableCollisionFaces.clear();
	m_DisableCollisionNodes.clear();
}
*/
//==========================================================================================
void CTool::OnOrganBeRemoved(MisMedicOrganInterface * organif)
{
	for(size_t c = 0 ; c <m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->OnOrganBeRemovedFromWorld(organif);
	}
}
//==========================================================================================
void CTool::OnCustomSimObjBeRemovedFromWorld(MisCustomSimObj * rope)
{
	for(size_t c = 0 ; c <m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->OnCustomSimObjBeRemovedFromWorld(rope);
	}
}
//==========================================================================================
void CTool::OnRigidBodyBeRemoved(GFPhysRigidBody * rb)
{
	for(size_t c = 0 ; c <m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->OnRigidBodyBeRemovedFromWorld(rb);
	}
}
//==========================================================================================
int CTool::onBeginCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj ) 
{
	int algotype_s = 0;

	MisMedicOrganInterface * orgif = m_pOwnerTraining->GetOrgan(GFPhysSoftBody::Upcast(softobj));
    
	if(orgif)
	   algotype_s = orgif->m_RSCollideAlgoType;

	//call all plugins
	if(rigidobj == m_lefttoolpartconvex.m_rigidbody 
	|| rigidobj == m_righttoolpartconvex.m_rigidbody
	|| rigidobj == m_centertoolpartconvex.m_rigidbody)//collide with this tool
	{
		for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
		{
			m_ToolPlugins[c]->BeginCheckSoftBodyCollision(softobj , rigidobj);
		}
		if(algotype_s == 1)
		{
			if(rigidobj == m_lefttoolpartconvex.m_rigidbody && m_lefttoolpartconvex.m_RSCollideAlgoType == 1)
			{
			   return 1;
			}
			
			else if(rigidobj == m_righttoolpartconvex.m_rigidbody && m_righttoolpartconvex.m_RSCollideAlgoType == 1)
			{
				return 1;
			}
			
			else if(rigidobj == m_centertoolpartconvex.m_rigidbody && m_centertoolpartconvex.m_RSCollideAlgoType == 1)
			{
				return 1;
			}
		}
		return 0;
	}
	else
	{
		return -1;//
	}
}
//==========================================================================================
/*void CTool::onFaceConvexCollided(GFPhysCollideObject * rigidobj , 
								 GFPhysCollideObject * softobj,
								 GFPhysSoftBodyFace * facecollide,
								 const GFPhysVector3 &   CdpointOnFace,
								 const GFPhysVector3 &   CdnormalOnFace,
								 float depth,
								 float weights[3],
								 int   contactmode
								 ) 
{
	

}
*/
//===========================================================================================================
void  CTool::OnThreadSegmentCollided( GFPhysCollideObject * rigidobj ,
									  MisMedicThreadRope * rope,
									  int segIndex,
									  const GFPhysVector3 & pointOnRigid,
									  const GFPhysVector3 & normalOnRigid,
									  float weights,
									  float depth
									  )
{
	GFPhysRigidBody * rigidbody = GFPhysRigidBody::Upcast(rigidobj);

	if(rigidobj == m_lefttoolpartconvex.m_rigidbody 
	|| rigidobj == m_righttoolpartconvex.m_rigidbody
	|| rigidobj == m_centertoolpartconvex.m_rigidbody)
	{
		GFPhysTransform   invrigidtrans = rigidbody->GetWorldTransform().Inverse();

		ToolCollideThreadSegment collideSeg;
		collideSeg.m_collideRigid = rigidbody;
		collideSeg.m_collideThread = rope;
		collideSeg.m_NormalOnRigid = normalOnRigid;
		collideSeg.m_localpointInRigid = invrigidtrans*pointOnRigid;
		collideSeg.m_SegmentIndex = segIndex;
		collideSeg.m_collideWeights = weights;

		collideSeg.m_depth = depth;

		m_ToolCollidedThreads.push_back(collideSeg);
	}
}	

void  CTool::OnSutureThreadSegmentCollided( GFPhysCollideObject * rigidobj ,
                                           SutureThread * suturerope,
                                           int segIndex,
                                           const GFPhysVector3 & pointOnRigid,
                                           const GFPhysVector3 & normalOnRigid,
                                           float weights,
                                           float depth
                                           )
{
    GFPhysRigidBody * rigidbody = GFPhysRigidBody::Upcast(rigidobj);

    if(rigidobj == m_lefttoolpartconvex.m_rigidbody 
        || rigidobj == m_righttoolpartconvex.m_rigidbody
        || rigidobj == m_centertoolpartconvex.m_rigidbody)
    {
        GFPhysTransform   invrigidtrans = rigidbody->GetWorldTransform().Inverse();

        ToolCollideSutureThreadSegment collideSeg;
        collideSeg.m_collideRigid = rigidbody;
        collideSeg.m_collideSutureThread = suturerope;
        collideSeg.m_NormalOnRigid = normalOnRigid;
        collideSeg.m_localpointInRigid = invrigidtrans*pointOnRigid;
        collideSeg.m_SegmentIndex = segIndex;
        collideSeg.m_collideWeights = weights;

        collideSeg.m_depth = depth;

        m_ToolCollidedSutureThreads.push_back(collideSeg);
    }
}	
void  CTool::OnSutureThreadSegmentCollided(GFPhysCollideObject * rigidobj,
	SutureThreadV2 * suturerope,
	int segIndex,
	const GFPhysVector3 & pointOnRigid,
	const GFPhysVector3 & normalOnRigid,
	float weights,
	float depth
	)
{
	GFPhysRigidBody * rigidbody = GFPhysRigidBody::Upcast(rigidobj);

	if (rigidobj == m_lefttoolpartconvex.m_rigidbody
		|| rigidobj == m_righttoolpartconvex.m_rigidbody
		|| rigidobj == m_centertoolpartconvex.m_rigidbody)
	{
		GFPhysTransform   invrigidtrans = rigidbody->GetWorldTransform().Inverse();

		ToolCollideSutureThreadSegmentV2 collideSeg;
		collideSeg.m_collideRigid = rigidbody;
		collideSeg.m_collideSutureThread = suturerope;
		collideSeg.m_NormalOnRigid = normalOnRigid;
		collideSeg.m_localpointInRigid = invrigidtrans*pointOnRigid;
		collideSeg.m_SegmentIndex = segIndex;
		collideSeg.m_collideWeights = weights;

		collideSeg.m_depth = depth;

		m_ToolCollidedSutureThreadsV2.push_back(collideSeg);
	}
}
void CTool::OnRigidCollided(GFPhysRigidBody * ra , GFPhysRigidBody * rb , const GFPhysManifoldPoint * contactPoints , int NumContactPoints)
{
	NewTrainToolConvexData * convexData[3];
	convexData[0] = &m_lefttoolpartconvex;
	convexData[1] = &m_righttoolpartconvex;
	convexData[2] = &m_centertoolpartconvex;


	int  convexindex = -1;
	
	bool isToolObjB = true;
	
	for(int i = 0 ; i < 3 ; i++)
	{
		if(convexData[i]->m_rigidbody == rb)
		{
			convexindex = i;
			isToolObjB = true;
			break;
		}
		else if(convexData[i]->m_rigidbody == ra)
		{
			convexindex = i;
			isToolObjB = false;
			break;
		}
	}

	if(convexindex >= 0)
	{
		for(int n = 0 ; n < NumContactPoints ; n++)
		{
			ToolCollideRigidBodyPoint toolPoint;
			
			toolPoint.m_Dist = contactPoints[n].m_distance1;

			if(isToolObjB)
			{
				toolPoint.m_collideRigidTool  = rb;
				toolPoint.m_collideRigidOther = ra;

				toolPoint.m_localPointOther = contactPoints[n].m_localPointA;
				toolPoint.m_localPointTool  = contactPoints[n].m_localPointB;

				toolPoint.m_positionWorldOnOther = contactPoints[n].m_positionWorldOnA;
				toolPoint.m_positionWorldOnTool  = contactPoints[n].m_positionWorldOnB;

				toolPoint.m_normalWorldOnTool = contactPoints[n].m_normalWorldOnB;
			}
			else
			{
				toolPoint.m_collideRigidTool  = ra;
				toolPoint.m_collideRigidOther = rb;

				toolPoint.m_localPointOther = contactPoints[n].m_localPointB;
				toolPoint.m_localPointTool  = contactPoints[n].m_localPointA;

				toolPoint.m_positionWorldOnOther = contactPoints[n].m_positionWorldOnB;
				toolPoint.m_positionWorldOnTool  = contactPoints[n].m_positionWorldOnA;

				toolPoint.m_normalWorldOnTool = -contactPoints[n].m_normalWorldOnB;
			}
				
			m_ToolCollidedRigids.push_back(toolPoint);
		}
	}
	
}
//=============================================================================================================
void CTool::onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj , const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints)
{
	//call all plugins
	if(    rigidobj == m_lefttoolpartconvex.m_rigidbody 
		|| rigidobj == m_righttoolpartconvex.m_rigidbody
		|| rigidobj == m_centertoolpartconvex.m_rigidbody)//collide with this tool
	{
		
		/*
		GFPhysRigidBody * rigidbody = GFPhysRigidBody::Upcast(rigidobj);
		
		for(size_t c = 0 ; c < contactPoints.size(); c++)
		{
			const GFPhysRSManifoldPoint & rsManiPoint = contactPoints[c];

			float weights[3];

			GFPhysTransform invrigidtrans = rigidbody->GetWorldTransform().Inverse();

			GFPhysSoftBodyFace * facecollide = rsManiPoint.m_collideface;
			
			GFPhysVector3 CdnormalOnFace = rsManiPoint.m_WorldNormalOnElement;

			float depth = rsManiPoint.m_Depth;

			weights[0] = rsManiPoint.m_weights[0];
			weights[1] = rsManiPoint.m_weights[1];
			weights[2] = rsManiPoint.m_weights[2];

			GFPhysVector3 WorldPoint = facecollide->m_Nodes[0]->m_CurrPosition*weights[0]
									  +facecollide->m_Nodes[1]->m_CurrPosition*weights[1]
									  +facecollide->m_Nodes[2]->m_CurrPosition*weights[2];

			WorldPoint = WorldPoint + CdnormalOnFace * depth;

			GFPhysVector3 LocalPoint = invrigidtrans*WorldPoint;

			ToolCollidedFace collideFace;
			collideFace.m_collideRigid = rigidobj;
			collideFace.m_collideSoft = softobj;
			collideFace.m_collideFace = facecollide;
			collideFace.m_CollideNormal = CdnormalOnFace;
			collideFace.m_localpointInRigid = Ogre::Vector3(LocalPoint.x() , LocalPoint.y() , LocalPoint.z());
			collideFace.m_collideWeights[0] = weights[0];
			collideFace.m_collideWeights[1] = weights[1];
			collideFace.m_collideWeights[2] = weights[2];
			collideFace.m_depth = depth;
			
			GFPhysVector3 faceNorm = (facecollide->m_Nodes[1]->m_CurrPosition-facecollide->m_Nodes[0]->m_CurrPosition).Cross(facecollide->m_Nodes[2]->m_CurrPosition-facecollide->m_Nodes[0]->m_CurrPosition);
			faceNorm.Normalize();

			if(faceNorm.Dot(CdnormalOnFace) > 0)
			   collideFace.m_IsBackFace = false;
			else
			   collideFace.m_IsBackFace = true;
			m_ToolColliedFaces.push_back(collideFace);
		}*/
		for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
		{
			m_ToolPlugins[c]->EndCheckSoftBodyCollision(softobj , rigidobj);
		}

		if(contactPoints.size())
			m_IsTouchingOrgan = true;
		else
			m_IsTouchingOrgan = false;
	}
}
//=============================================================================================================
void CTool::onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair)
{
	//call all plugins
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->onRSContactsBuildBegin(collidePairs , NumCollidePair);
	}
}
//=============================================================================================================
void  CTool::onRSContactsBuildFinish( GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	//call all plugins
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->onRSContactsBuildFinish(RSContactConstraints);
	}
}
//=============================================================================================================
void CTool::onRSContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	/*CalculateToolForceFeedBack(RSContactConstraints);

	if(m_NewCanClampTube)
	{//collect contacted organ tubes
		m_ContactedOrganTubes.clear();

		for(size_t i = 0 ; i < RSContactConstraints.size(); i++)
		{
			const GFPhysSoftRigidContact & srContact = RSContactConstraints[i];

			GFPhysSoftBody * sb = srContact.m_SoftBody;

			MisMedicOrganInterface * meshif = m_pOwnerTraining->GetOrgan(sb);

			if(meshif && meshif->GetCreateInfo().m_objTopologyType == DOT_TUBE)
			{
				int OrganType = meshif->m_OrganID;
				m_ContactedOrganTubes.insert(OrganType);
			}
		}
	}*/

	//
	/*for(size_t i = 0 ; i < RSContactConstraints.size(); i++)
	{
		const GFPhysSoftRigidContact & rsContact = RSContactConstraints[i];
		
		GFPhysVector3 implusenew = rsContact.m_NormalOnRigid*rsContact.m_AccumulateImpluse;

		std::map<GFPhysSoftBodyNode* , GFPhysVector3>::iterator itor = m_NodeRSCollideImpluse.find(rsContact.m_SoftBodyNode);
		
		if(itor != m_NodeRSCollideImpluse.end())
		{
		   if(implusenew.Length() > itor->second.Length())
		      itor->second = implusenew;
		}
		else
		   m_NodeRSCollideImpluse.insert(std::make_pair(rsContact.m_SoftBodyNode , implusenew));
	}*/

	//call all plugins
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
	    m_ToolPlugins[c]->RigidSoftCollisionsSolved(RSContactConstraints);
	}
}
//=============================================================================================================
void  CTool::onRSFaceContactsBuildFinish( GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints)
{
	  //call all plugins
	  for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	  {
		 m_ToolPlugins[c]->onRSFaceContactsBuildFinish(RSContactConstraints);
	  }
}
//=============================================================================================================
void  CTool::onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSFaceContactConstraints)
{
	CalculateToolForceFeedBack(RSFaceContactConstraints);


	//
	//GFPhysRigidBody * rigidbody = GFPhysRigidBody::Upcast(rigidobj);

	for (size_t c = 0; c < RSFaceContactConstraints.size(); c++)
	{
		const GFPhysSoftFaceRigidContact & rsContact = RSFaceContactConstraints[c];

		GFPhysRigidBody * rigidbody = rsContact.m_Rigid;

		int contactPart = -1;
		
		if (rigidbody == m_lefttoolpartconvex.m_rigidbody)
			contactPart = 0;
		
		else if (rigidbody == m_righttoolpartconvex.m_rigidbody)
			contactPart = 1;
		
		else if (rigidbody == m_centertoolpartconvex.m_rigidbody)
			contactPart = 2;

		if (contactPart >= 0)
		{
			float weights[3];

			GFPhysTransform invrigidtrans = rigidbody->GetWorldTransform().Inverse();

			GFPhysSoftBodyFace * facecollide = rsContact.m_SoftFace;// rsManiPoint.m_collideface;

			GFPhysVector3 CdnormalOnFace = -rsContact.m_NormOnRigidWorld;// rsManiPoint.m_WorldNormalOnElement;

			float depth = rsContact.m_Dist;// rsManiPoint.m_Depth;

			weights[0] = rsContact.m_FaceWeights[0];
			weights[1] = rsContact.m_FaceWeights[1];
			weights[2] = rsContact.m_FaceWeights[2];

			GFPhysVector3 LocalPoint = rsContact.m_PointOnRigidLocal;// invrigidtrans*WorldPoint;

			ToolCollidedFace collideFace;
			collideFace.m_ContactToolPart = contactPart;
			collideFace.m_collideRigid = rigidbody;
			collideFace.m_collideSoft = rsContact.m_SoftBody;
			collideFace.m_collideFace = facecollide;
			collideFace.m_CollideNormal = CdnormalOnFace;
			collideFace.m_localpointInRigid = LocalPoint;
			collideFace.m_collideWeights[0] = weights[0];
			collideFace.m_collideWeights[1] = weights[1];
			collideFace.m_collideWeights[2] = weights[2];
			collideFace.m_depth = depth;
			collideFace.m_FaceContactImpluse[0] = rsContact.GetNormalImpluse(0);
			collideFace.m_FaceContactImpluse[1] = rsContact.GetNormalImpluse(1);
			collideFace.m_FaceContactImpluse[2] = rsContact.GetNormalImpluse(2);

			GFPhysVector3 faceNorm = (facecollide->m_Nodes[1]->m_CurrPosition - facecollide->m_Nodes[0]->m_CurrPosition).Cross(facecollide->m_Nodes[2]->m_CurrPosition - facecollide->m_Nodes[0]->m_CurrPosition);
			faceNorm.Normalize();

			if (faceNorm.Dot(CdnormalOnFace) > 0)
				collideFace.m_IsBackFace = false;
			else
				collideFace.m_IsBackFace = true;
			m_ToolColliedFaces.push_back(collideFace);
		}
	}

	//call all plugins
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->onRSFaceContactsSolveEnded(RSFaceContactConstraints);
	}
}
//=============================================================================================================
Ogre::SceneNode* CTool::GetTipNode()
{
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();
	Ogre::SceneNode *  pKernelNode =  GetKernelNode();
	int childNum = pKernelNode->numChildren();
	float maxdis = 0.0f;
	float dis = 0.0f;
	Ogre::SceneNode* node = NULL;
	for (int k=0;k<childNum;k++)
	{
		Ogre::SceneNode*  childNode = dynamic_cast<Ogre::SceneNode*>(pKernelNode->getChild(k));
		Ogre::Vector3 sv1 = childNode->_getWorldAABB().getMinimum();
		Ogre::Vector3 sv2 = childNode->_getWorldAABB().getMaximum();
		dis = sv1.distance(sv2);
		if (dis >= maxdis)
		{
			maxdis = dis;
			node = childNode;
		}
	}

	return node;
}

void CTool::GetKernelLine(Ogre::Vector3& vecmin, Ogre::Vector3& vecmax)
{
	if (!m_pTipNode)
	{
		m_pTipNode = GetTipNode();
	}
	Ogre::SceneManager * pSceneManager = MXOgreWrapper::Instance()->GetDefaultSceneManger();
	Ogre::SceneNode *  pKernelNode =  GetKernelNode();
	if (!pKernelNode)  return;
	int childNum = pKernelNode->numChildren();
	Ogre::Vector3 v1(0, 0, 0);
	Ogre::Vector3 v2(0, 0, 0);
	float maxdis = 0.0f;
	float dis = 0.0f;
	if (pKernelNode)
	{
		v1 = pKernelNode->getPosition();
	}
	if (m_pTipNode)
	{
        /*m_pTipNode->showBoundingBox(true);*/
		Ogre::Vector3 lerpv = m_pTipNode->_getWorldAABB().getCenter() - v1;
		v2 = v1 + (lerpv.normalisedCopy())*30;
	}
	

	vecmin = v1;
	vecmax = v2;
}
//===============================================================================================
void CTool::UpdateConvexVelocity(float percent , float dt)
{
	bool needclampMovement = true;
	
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
		MisCTool_PluginClamp * clampPlugin = dynamic_cast<MisCTool_PluginClamp*>(m_ToolPlugins[c]);
		if(clampPlugin && clampPlugin->isInClampState())
		{
			needclampMovement = false;
			break;
		}
		MisCTool_PluginRigidHold * rigidholdPlugin = dynamic_cast<MisCTool_PluginRigidHold*>(m_ToolPlugins[c]);
		if (rigidholdPlugin && rigidholdPlugin->GetRigidBodyHolded())
		{
			needclampMovement = false;
			break;
		}
	}

	if(m_lefttoolpartconvex.m_isvalid == true)
	{
		m_lefttoolpartconvex.UpdatePhysVelocity(percent , dt , needclampMovement);
	}

	if(m_righttoolpartconvex.m_isvalid == true)
	{
		m_righttoolpartconvex.UpdatePhysVelocity(percent , dt , needclampMovement);
	}

	if(m_centertoolpartconvex.m_isvalid == true)
	{
		m_centertoolpartconvex.UpdatePhysVelocity(percent , dt , needclampMovement);
	}
}
//===============================================================================================
void CTool::UpdateConvexTransform()
{
	if(m_lefttoolpartconvex.m_isvalid == true)
	{
	   m_lefttoolpartconvex.UpdatePhysTransform();
	}

	if(m_righttoolpartconvex.m_isvalid == true)
	{
	   m_righttoolpartconvex.UpdatePhysTransform();
	}

	if(m_centertoolpartconvex.m_isvalid == true)
	{
	   m_centertoolpartconvex.UpdatePhysTransform();
	}
}
//===============================================================================================
void CTool::UpdateConvex()
{
	if(m_lefttoolpartconvex.m_isvalid == true)
	{
		Ogre::Node * pNodeLeft = m_lefttoolpartconvex.m_AttachedNode;

		Ogre::Vector3 derivedPos = pNodeLeft->_getDerivedPosition();

		Ogre::Quaternion  derivedRotate = pNodeLeft->_getDerivedOrientation();

		m_lefttoolpartconvex.SetNextWorldTransform(derivedPos , derivedRotate);
	}


	if(m_righttoolpartconvex.m_isvalid == true)
	{
		Ogre::Node * pNodeRight = m_righttoolpartconvex.m_AttachedNode;

		Ogre::Vector3 derivedPos = pNodeRight->_getDerivedPosition();

		Ogre::Quaternion  derivedRotate = pNodeRight->_getDerivedOrientation();

		m_righttoolpartconvex.SetNextWorldTransform(derivedPos , derivedRotate);
	}

	if(m_centertoolpartconvex.m_isvalid == true)
	{
		Ogre::Node * pNodeCenter = m_centertoolpartconvex.m_AttachedNode;

		Ogre::Vector3 derivedPos = pNodeCenter->_getDerivedPosition();

		Ogre::Quaternion  derivedRotate = pNodeCenter->_getDerivedOrientation();

		m_centertoolpartconvex.SetNextWorldTransform(derivedPos , derivedRotate);
	}
}
//============================================================================================================
void CTool::InternalSimulationStart(int currStep , int TotalStep , float dt)
{
	//UpdateConvexVelocity((float)(currStep+1) / (float)(TotalStep));
	UpdateConvexVelocity((float)(currStep+1) / (float)(TotalStep) , dt);

	m_ToolColliedFaces.clear();
	
	m_ToolCollidedThreads.clear();

	m_ToolCollidedRigids.clear();

	m_ToolCollidedSutureThreads.clear();

	m_ToolCollidedSutureThreadsV2.clear();

	m_InClampState = false;
	//call plugins
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->PhysicsSimulationStart(currStep ,  TotalStep , dt);

		MisCTool_PluginClamp * clamplugin = dynamic_cast<MisCTool_PluginClamp*>(m_ToolPlugins[c]);
		if(clamplugin && clamplugin->isInClampState())
		{
		   m_InClampState = true;
		}
	}

	/*
	if(m_InClampState == true)
	{
		m_lefttoolpartconvex.DisableDoubleFaceRSCollideMode();
		m_righttoolpartconvex.DisableDoubleFaceRSCollideMode();
		m_centertoolpartconvex.DisableDoubleFaceRSCollideMode();
	}
	else
	{
		m_lefttoolpartconvex.EnableDoubleFaceRSCollideMode();
		m_righttoolpartconvex.EnableDoubleFaceRSCollideMode();
		m_centertoolpartconvex.EnableDoubleFaceRSCollideMode();
	}*/
}
//==================================================================================
void CTool::InternalSimulationEnd(int currStep , int TotalStep , float dt)
{
	UpdateConvexTransform();

	if (m_CutBladeLeft.m_AttachedRB)
	{
		m_CutBladeLeft.m_LinePointsWorld[0] = m_CutBladeLeft.m_AttachedRB->GetWorldTransform() * m_CutBladeLeft.m_LinPoints[0];
		m_CutBladeLeft.m_LinePointsWorld[1] = m_CutBladeLeft.m_AttachedRB->GetWorldTransform() * m_CutBladeLeft.m_LinPoints[1];
		m_CutBladeLeft.m_CuttDirectionWord  = m_CutBladeLeft.m_AttachedRB->GetWorldTransform().GetBasis() * m_CutBladeLeft.m_CuttDirection;
	}

	if (m_CutBladeRight.m_AttachedRB)
	{
		m_CutBladeRight.m_LinePointsWorld[0] = m_CutBladeRight.m_AttachedRB->GetWorldTransform() * m_CutBladeRight.m_LinPoints[0];
		m_CutBladeRight.m_LinePointsWorld[1] = m_CutBladeRight.m_AttachedRB->GetWorldTransform() * m_CutBladeRight.m_LinPoints[1];
		m_CutBladeRight.m_CuttDirectionWord  = m_CutBladeRight.m_AttachedRB->GetWorldTransform().GetBasis() * m_CutBladeRight.m_CuttDirection;
	}

	//call plugins
	for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
	{
		m_ToolPlugins[c]->PhysicsSimulationEnd(currStep ,  TotalStep , dt);
	}
}
//========================================================================================================
void CTool::OnCutBladeClampedTube(MisMedicOrgan_Tube & tubeclamp , int segment , int localsection , Real sectionWeight)
{
}
void CTool::OnVeinConnectPairCollide(MisMedicOrganInterface * veinobject ,
									   GFPhysCollideObject * convexobj,
									   int cluster , 
									   int pair,
									   const GFPhysVector3 & collidepoint)
{
	//call plugins
	VeinConnectObject * veinconnobj = dynamic_cast<VeinConnectObject*>(veinobject);

	if(veinconnobj)
	{
		for(size_t c = 0 ; c < m_ToolPlugins.size() ; c++)
		{
			m_ToolPlugins[c]->CollideVeinConnectPair(veinconnobj ,
													 convexobj,
													 cluster , 
													 pair,
													 collidepoint);
		}
	}
	
}
void CTool::OnSoftBodyNodesBeDeleted(GFPhysSoftBody *sb , const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes)
{
	for(size_t p = 0 ; p < m_ToolPlugins.size() ; p++)
	{
		m_ToolPlugins[p]->OnSoftBodyNodesBeDeleted(sb , nodes);
	}
}
//========================================================================================================
void CTool::OnSoftBodyFaceBeDeleted(GFPhysSoftBody * sb , GFPhysSoftBodyFace * face)
{
	for(size_t p = 0 ; p < m_ToolPlugins.size() ; p++)
	{
		m_ToolPlugins[p]->OnSoftBodyFaceBeDeleted(sb , face);
	}

	for (int c = 0; c < m_ToolColliedFaces.size(); c++)
	{
		if (m_ToolColliedFaces[c].m_collideFace == face)
		{
			m_ToolColliedFaces[c] = m_ToolColliedFaces[m_ToolColliedFaces.size() - 1];
			m_ToolColliedFaces.resize(m_ToolColliedFaces.size() - 1);
			break;
		}
	}

}
//========================================================================================================
void CTool::OnSoftBodyFaceBeAdded(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face)
{
	for(size_t p = 0 ; p < m_ToolPlugins.size() ; p++)
	{
		m_ToolPlugins[p]->OnSoftBodyFaceBeAdded(sb , face);
	}
}

void CTool::OnFirstFrame()
{
	m_isFirstFrame = false;
	if(m_pNodeKernel)
	{
		m_kernelInitPos = m_pNodeKernel->_getDerivedPosition();
		m_curKernelPos = m_kernelInitPos;
	}
	InitializeConvexTransform();
}

//========================================================================================================
void CTool::GetToolCutPlaneVerts(GFPhysVector3 cutQuads[4])
{
	GFPhysVector3 CutPlaneNormal = (m_CutBladeLeft.m_LinePointsWorld[1]-m_CutBladeLeft.m_LinePointsWorld[0]).Cross(m_CutBladeRight.m_LinePointsWorld[1]-m_CutBladeRight.m_LinePointsWorld[0]);

	GFPhysVector3 pnormal = CutPlaneNormal.Cross(m_CutBladeRight.m_LinePointsWorld[1]-m_CutBladeRight.m_LinePointsWorld[0]);
	pnormal.Normalize();

	float distleft0 = (m_CutBladeLeft.m_LinePointsWorld[0]-m_CutBladeRight.m_LinePointsWorld[0]).Dot(pnormal);
	float distleft1 = (m_CutBladeLeft.m_LinePointsWorld[1]-m_CutBladeRight.m_LinePointsWorld[0]).Dot(pnormal);

	cutQuads[0] = m_CutBladeLeft.m_LinePointsWorld[0];
	cutQuads[1] = m_CutBladeLeft.m_LinePointsWorld[1];
	cutQuads[2] = m_CutBladeRight.m_LinePointsWorld[0];
	cutQuads[3] = m_CutBladeRight.m_LinePointsWorld[1];

	//if(distleft0*distleft1 >= 0)
	//{
		//float tlength  = (cutQuads[3]-cutQuads[1]).Length();
		//cutQuads[3] = cutQuads[1]+(cutQuads[2]-cutQuads[0]).Normalized()*tlength;
	//}
	//else
	//{
		//GFPhysVector3 commonpoing = cutQuads[0] + (cutQuads[1]-cutQuads[0])*(distleft0 / (distleft0-distleft1));

		//GFPhysVector3 normdir = (cutQuads[2]-cutQuads[0]).Normalized();

		//float tlength = (cutQuads[2]-cutQuads[0]).Length();

		//cutQuads[3] = commonpoing+normdir*tlength*0.1f;

		//cutQuads[1] = commonpoing-normdir*tlength*0.1f;
	//}
}
//========================================================================================================
bool CTool::ElectricCutOrgan(MisMedicOrgan_Ordinary * organ , GFPhysSoftBodyFace * face  , float weights[3])
{
	//if(organ->CanBeCut() == false)
	{
		int IdInOrigidnFace = organ->GetOriginFaceIndexFromUsrData(face);
		if(IdInOrigidnFace >= 0)
		{
			organ->createBloodTrack(face , weights);
		    return true;
		}
	}
	return false;
}
//===============================================================================================================
int CTool::TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnpair)
{
	return 0;
}
//===============================================================================================================
void CTool::TryElecBurnTouchedOrgans(std::vector<OrganSelectedBurnFace> & touchedOrgan , float dt)
{
	touchedOrgan.clear();

	for (size_t c = 0; c < m_ToolColliedFaces.size(); c++)
	{
		 const ToolCollidedFace & collideFace = m_ToolColliedFaces[c];

		 if (collideFace.m_collideFace->m_IsMemFreed == true)//临时处理需要查找原因
			 continue;

		 if (collideFace.m_FaceContactImpluse[0] + collideFace.m_FaceContactImpluse[1] + collideFace.m_FaceContactImpluse[2] <= 0.00001f)
			 continue;

		 GFPhysRigidBody * rigid = GFPhysRigidBody::Upcast(collideFace.m_collideRigid);
		
		 GFPhysSoftBody  * sbTouched = GFPhysSoftBody::Upcast(collideFace.m_collideSoft);
		  
		 bool canBurn = (rigid == m_righttoolpartconvex.m_rigidbody && IsRightPartConductElectric())
			 || (rigid == m_lefttoolpartconvex.m_rigidbody && IsLeftPartConductElectric());
		
		 if (canBurn)
		 {
			 GFPhysSoftBodyFace * facecollide = collideFace.m_collideFace;

			 GFPhysVector3 closetPt;

			 GFPhysVector3 triVerts[3];
			 triVerts[0] = facecollide->m_Nodes[0]->m_CurrPosition;
			 triVerts[1] = facecollide->m_Nodes[1]->m_CurrPosition;
			 triVerts[2] = facecollide->m_Nodes[2]->m_CurrPosition;

			 float dist = GetFaceToElecBladeDist(triVerts);

			 if (dist >= 0)//radius * 1.5f)
			 {
				int  t = 0;
				for (t = 0; t < (int)touchedOrgan.size(); t++)
				{
					if (touchedOrgan[t].m_sb == sbTouched)
					{
						if (dist < touchedOrgan[t].m_ClosetDist)//this one is small
						{
							touchedOrgan[t].Reset(dist, collideFace.m_collideWeights, facecollide, sbTouched);
						}
						break;
					}
				}
				if (t == touchedOrgan.size())//not exist
				{
					touchedOrgan.push_back(OrganSelectedBurnFace(dist, collideFace.m_collideWeights, facecollide, sbTouched));
				}
			}
		 }
	}

	//call every organ be heated and choose global min organ to save
	for (int c = 0; c < (int)touchedOrgan.size(); c++)
	{
		OrganSelectedBurnFace & organTouched = touchedOrgan[c];
		
		MisMedicOrganInterface * organif = (MisMedicOrganInterface *)organTouched.m_sb->GetUserPointer();

		if (m_bElectricRightPad || m_bElectricLeftPad)
		{
			if (m_bElectricRightPad)
			{
				organif->Tool_InElec_TouchFacePoint(this, organTouched.m_MinDistFace, organTouched.m_MinPointWeights, 0, dt);
			}
			else if (m_bElectricLeftPad)
			{
				organif->Tool_InElec_TouchFacePoint(this, organTouched.m_MinDistFace, organTouched.m_MinPointWeights, 1, dt);
			}

			//
			if (m_BubbleWhenBurn)
			{
				BubbleManager * bubbleMgr = EffectManager::Instance()->GetBubbleManager();
				if (bubbleMgr->BeginRandomAddBubble())
				{
					BubbleControlInfoForClamp controlInfo;
					controlInfo.MaxDeviate = 0.02f;
					controlInfo.MinExplosionSize = 0.03f;
					controlInfo.MaxExplosionSize = 0.06f;
					controlInfo.ExpandRate = 0.075f;

					float faceArea = organTouched.m_MinDistFace->m_RestAreaMult2;
					bubbleMgr->addBubblesOnFace(organTouched.m_MinDistFace, organTouched.m_MinPointWeights, controlInfo, 5);
				}
			}
			//

			//float heatValue = organif->GetCreateInfo().m_BurnRation*dt;

			//organif->HeatAroundUndeformedPoint(organTouched.m_materialPos, 0.5f, heatValue);
		}
	}
}
//===============================================================================================================
const GFPhysCollideObject * CTool::GetDistCollideObject()
{
	if (m_OrganFaceSelToBurn.size() > 0)
		return m_OrganFaceSelToBurn[0].m_sb;
	else
		return 0;
}
//===============================================================================================================
const GFPhysSoftBodyFace * CTool::GetDistCollideFace()
{
	if (m_OrganFaceSelToBurn.size() > 0)
		return m_OrganFaceSelToBurn[0].m_MinDistFace;
	else
		return 0;
}
//===============================================================================================================
void CTool::GetDistPointWeight(float weights[3])
{
	assert(weights);
	if (m_OrganFaceSelToBurn.size() > 0)
	{
		for (int w = 0; w < 3; ++w)
			weights[w] = m_OrganFaceSelToBurn[0].m_MinPointWeights[w];
	}
}
//===============================================================================================================
GFPhysVector3 CTool::GetDistPoint()
{
	if (m_OrganFaceSelToBurn.size() > 0)
	{
		return m_OrganFaceSelToBurn[0].m_MinDistPoint;
	}
	else
	{
		return GFPhysVector3(0, 0, 0);
	}
}
//========================================================================================================
void CTool::ReleaseClampedOrgans()
{
    MisCTool_PluginClamp * clampPlugin = GetClampPlugin();
    if (clampPlugin)
    {
        clampPlugin->ReleaseClampedOrgans();
    }
}
//========================================================================================================
bool CTool::HasGraspSomeThing()
{
	MisCTool_PluginClamp * clampPlugin = GetClampPlugin();
	if (clampPlugin)
	{
		return clampPlugin->isInClampState();
	}
	return false;
}
//========================================================================================================
void CTool::ReleaseHoldRigid()
{
	for (size_t p = 0; p < m_ToolPlugins.size(); p++)
	{
		MisCTool_PluginRigidHold * rigidHoldPlugin = dynamic_cast<MisCTool_PluginRigidHold*>(m_ToolPlugins[p]);
		if (rigidHoldPlugin)
		{
			rigidHoldPlugin->ReleaseHoldingRigid();
		}
	}
}
//========================================================================================================
void CTool::ReleaseClampedRope()
{
	for (size_t p = 0; p < m_ToolPlugins.size(); p++)
	{
		MisCTool_PluginClamp * clampPlugin = dynamic_cast<MisCTool_PluginClamp*>(m_ToolPlugins[p]);
		if (clampPlugin)
		{
			clampPlugin->ReleaseClampedRope();
		}
	}
}
//========================================================================================================
void CTool::ReleaseRopeWhenClampNeedleAndRope()
{
	//////////////////////////////////////////////////////////////////////////
	//持针的判断角度比较大，在持针过程中，如果线落在器械中间，容易被误抓
	//当针和线同时被抓时，优先抓针，释放线
	bool releaseThreadWhenHoldNeedle = false;
	for (size_t p = 0; p < m_ToolPlugins.size(); p++)
	{
		MisCTool_PluginRigidHold * rigidPlugin = dynamic_cast<MisCTool_PluginRigidHold *>(m_ToolPlugins[p]);

		if (rigidPlugin && rigidPlugin->GetRigidBodyHolded())
		{
			releaseThreadWhenHoldNeedle = true;
		}
	}

	for (size_t p = 0; p < m_ToolPlugins.size(); p++)
	{
		MisCTool_PluginClamp * clampPlugin = dynamic_cast<MisCTool_PluginClamp *>(m_ToolPlugins[p]);
		if (clampPlugin)
		{
			if (releaseThreadWhenHoldNeedle && clampPlugin->GetThreadClampState())
			{
				if (clampPlugin->GetRopeBeClamped())
				{
					clampPlugin->ReleaseClampedRope();
				}
				if (clampPlugin->GetRopeBeClampedV2())
				{
					clampPlugin->ReleaseClampedRopeV2();
				}
				
				AssignShaftValueDirectly(GetShaftAside() - 1.0f);
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
}
void CTool::GetGraspedOrgans(std::vector<MisMedicOrgan_Ordinary*> & organs)
{
	MisCTool_PluginClamp * clampPlugin = GetClampPlugin();
	if (clampPlugin)
	{
		clampPlugin->GetOrgansBeClamped(organs);
	}
	return;
}

int CTool::DisconnectClampedVeinConnectPairs()
{
	MisCTool_PluginClamp * clampPlugin = GetClampPlugin();
	if (clampPlugin)
	{
		return clampPlugin->DisconnectClampedVeinConnectPairs();
	}
	return 0;
}
MisCTool_PluginClamp * CTool::GetClampPlugin()
{
	return m_pluginclamp;
}
//for new train end

//for new train ends