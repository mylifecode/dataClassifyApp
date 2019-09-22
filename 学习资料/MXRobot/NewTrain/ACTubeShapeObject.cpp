#include "ACTubeShapeObject.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "PhysicsWrapper.h"
const Real DefaultThreadMass = 0.05f;
const Real DefaultrotMassPerSeg = 0.1f;
#define CIRCLESEGNUM 10
ACTubeShapeObject::ACTubeShapeObject(int index, CBasicTraining * ownertrain)
	              : MisMedicOrganInterface(DOT_TUBE, EDOT_NO_TYPE, index, ownertrain)
{
	 m_PhysicsBody = 0;
}
//-----------------------------------------------------------------------------------------
ACTubeShapeObject::~ACTubeShapeObject()
{

}
void ACTubeShapeObject::Create(MisMedicDynObjConstructInfo &)
{

}
//-----------------------------------------------------------------------------------------
void ACTubeShapeObject::CreateToturs(Ogre::SceneManager * sceneMgr,
	                                 const GFPhysVector3 & circlecenter,
									 const GFPhysVector3 & circleNormal, 
									 float radius)
{
	 m_PhysicsBody = new GFPhysSoftTube();
	 m_RendRadius = 0.15f;

	 GFPhysAlignedVectorObj<GFPhysVector3> TubeNodePos;
	 GFPhysVector3 x = Perpendicular(circleNormal);
	 GFPhysVector3 y = circleNormal.Cross(x).Normalized();
	 int numSeg = 16;
	 float theta = GP_2PI / float(numSeg);
	 for (int c = 0; c < numSeg; c++)
	 {
		  GFPhysVector3 nodePos = circlecenter + x * (cosf(theta*c) * radius) + y * (sinf(theta*c) * radius);
		  TubeNodePos.push_back(nodePos);
	 }
	 m_PhysicsBody->Create(TubeNodePos, DefaultThreadMass, DefaultrotMassPerSeg , true);
	 m_PhysicsBody->SetGravity(GFPhysVector3(0, -20, 0));
	 m_PhysicsBody->SetCollisionRadius(m_RendRadius);
	 m_PhysicsBody->GetCollisionShape()->SetMargin(m_RendRadius*0.5f);
	 // m_PhysicsBody->GetNode(0)->SetMass(0);
	 
	 //material parameter
	 m_PhysicsBody->m_StretchShearInvStiff = 1.0f / 5000.0f;
	 //m_PhysicsBody->m_StretchShearDamping  = 0.0025f;
	 m_PhysicsBody->m_RopeBendInvStiff  = 1.0f / 5000.0f;
	 m_PhysicsBody->m_RopeTwistInvStiff = 1.0f / 5000.0f;
	 m_PhysicsBody->m_BendTwistSor = GFPhysVector3(1.f, 1.f, 1.f);
	 PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddSoftTube(m_PhysicsBody);
	 PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);//todo put simulation of tube in to GPSDK
	 //
	
	 static int s_TubeId = 0;
	 s_TubeId++;
	 Ogre::String tubeName = "TueShapeObj" + Ogre::StringConverter::toString(s_TubeId);
#if(0)
	 m_RendObject.CreateRendPart(strSutureThreadName, sceneMgr);
#else
	 m_SkinRendObject = sceneMgr->createManualObject(tubeName);
	 m_SkinRendObject->setDynamic(true);
	 m_SkinRendObject->setVisible(true);
	 m_SkinRendObject->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);

	 if (m_SkinRendObject)
	 {
		 m_Node = sceneMgr->getRootSceneNode()->createChildSceneNode(tubeName + "_Node");
		 if (m_Node)
			 m_Node->attachObject(m_SkinRendObject);
	 }
	 CreateTotursMesh(m_RendRadius, CIRCLESEGNUM);
#endif
	Ogre::MaterialManager::getSingleton().load("ACTech/RingYellow", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	 Ogre::MaterialPtr matYellow = Ogre::MaterialManager::getSingleton().getByName("ACTech/RingYellow");

	 m_OwnerMaterialPtr = matYellow->clone("TubeOwnMat_" + tubeName);

}
//----------------------------------------------------------------------------------------------
void ACTubeShapeObject::InternalSimulateStart(int currStep, int TotalStep, Real dt)
{
	//just do nothing (predict unconstrained motion already done in GPADK)
}
//---------------------------------------------------------------------------------------------------
void ACTubeShapeObject::PrepareSolveConstraint(Real Stiffness, Real TimeStep)
{
	 m_PhysicsBody->PrepareSolveData(TimeStep, false);
}
//----------------------------------------------------------------------------------------------
void ACTubeShapeObject::SolveConstraint(Real Stiffness, Real TimeStep)
{
	 //solve constraint
	 m_PhysicsBody->SolveStretchBendTwist(1.0f, TimeStep);
	 m_PhysicsBody->SolveCollisions(TimeStep);
}
//----------------------------------------------------------------------------------------------
void ACTubeShapeObject::InternalSimulateEnd(int currStep, int TotalStep, Real dt)
{
	m_PhysicsBody->UpdateVelocity(dt);
	m_PhysicsBody->DampingVelocity(dt, 4.0f, GFPhysVector3(4.0f, 4.0f, 4.0f));
}
//-----------------------------------------------------------------------------------------
void ACTubeShapeObject::UpdateScene(float dt, Ogre::Camera * camera)
{
	MisMedicOrganInterface::UpdateScene(dt, camera);
#if(0)
	std::vector<Ogre::Vector3> RendNodes;
	for (int n = 0; n < (int)m_PhysicsBody->GetNumNodes(); n++)
	{
		GFPhysVector3 temp = m_PhysicsBody->GetNode(n)->m_CurrPosition;
		RendNodes.push_back(Ogre::Vector3(temp.x(), temp.y(), temp.z()));
	}

	m_RendObject.UpdateRendSegment(RendNodes, m_RendRadius ,"",true);
#else
	AnimatedSkinMesh();
	UpdateRendBuffer(CIRCLESEGNUM);
#endif
	
}
//------------------------------------------------------------------------------------------------------------------
void ACTubeShapeObject::CreateTotursMesh(float radius , int circlesegnum)
{
	int NumNode    = m_PhysicsBody->GetNumNodes();

	int NumSegment = m_PhysicsBody->GetNumSegments();

	//GFPhysAlignedVectorObj<GFPhysMatrix3> SegFrameAxis;
	//for (int s = 0; s < NumSegment; s++)
	//{

	//}
	for (int n = 0; n < m_PhysicsBody->GetNumNodes(); n++)
	{
		 GFPhysSoftTubeSegment & seg1 = m_PhysicsBody->GetSegment(n);

		 GFPhysSoftTubeSegment & seg0 = (n > 0 ? m_PhysicsBody->GetSegment(n - 1) : m_PhysicsBody->GetSegment(NumSegment - 1));

		 GFPhysVector3 axis1  = (seg1.m_Node1->m_UnDeformedPos - seg1.m_Node0->m_UnDeformedPos).Normalized();
		 
		 GFPhysVector3 axis0  = (seg0.m_Node0->m_UnDeformedPos - seg0.m_Node1->m_UnDeformedPos).Normalized();

		 GFPhysVector3 rotVec  = (axis0 + axis1).Normalized();

		 GFPhysVector3 rotAxis = (axis1 - axis0).Normalized();
		 
		 //make sure perpendicular
		 rotVec = (rotVec - rotAxis*rotVec.Dot(rotAxis)).Normalized() * radius;

		 for (int c = 0; c < circlesegnum; c++)
		 {
			 GFPhysQuaternion rotQuat(rotAxis, c * GP_2PI / float(circlesegnum));
			 
			 GFPhysVector3 localvertex = QuatRotate(rotQuat, rotVec);

			 SkinedVertex  skinVert;
			 skinVert.m_RestPos    = GPVec3ToOgre(localvertex + seg1.m_Node0->m_UnDeformedPos);
			 skinVert.m_RestNormal = GPVec3ToOgre(localvertex.Normalized());
			 //build bone coordinate
			 skinVert.m_BoneSegIndex[0] = seg0.m_SegIndex;
			 skinVert.m_BoneSegIndex[1] = seg1.m_SegIndex;
			 skinVert.m_NumBoneSeg = 2;
			
			 for (int s = 0; s < 2; s++)
			 {
				  GFPhysVector3 d1, d2, d3;

				  GFPhysSoftTubeSegment & boneSeg = (s == 0 ? seg0 : seg1);
				 
				  boneSeg.GetMaterialFrameAxis(d1, d2, d3, false);

				  skinVert.m_BoneCoord[s].x = localvertex.Dot(d1);
				  skinVert.m_BoneCoord[s].y = localvertex.Dot(d2);
				  skinVert.m_BoneCoord[s].z = localvertex.Dot(d3);

				  skinVert.m_NormCoord[s].x = skinVert.m_RestNormal.dotProduct(GPVec3ToOgre(d1));
				  skinVert.m_NormCoord[s].y = skinVert.m_RestNormal.dotProduct(GPVec3ToOgre(d2));
				  skinVert.m_NormCoord[s].z = skinVert.m_RestNormal.dotProduct(GPVec3ToOgre(d3));
			 }
			 m_SkinedVertex.push_back(skinVert);
		 }
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------
void ACTubeShapeObject::AnimatedSkinMesh()
{
	//calculate segment frame and store
	GFPhysAlignedVectorObj<GFPhysMatrix3> boneFrames;
	for (int s = 0; s < m_PhysicsBody->GetNumSegments(); s++)
	{
		 GFPhysSoftTubeSegment & boneSeg = m_PhysicsBody->GetSegment(s);
		 GFPhysVector3 d1, d2, d3;
		 boneSeg.GetMaterialFrameAxis(d1, d2, d3, true);
		 GFPhysMatrix3 matFrame;
		 matFrame.GetRow(0) = d1;
		 matFrame.GetRow(1) = d2;
		 matFrame.GetRow(2) = d3;
		 boneFrames.push_back(matFrame);
	}
	//
	for (int c = 0; c < m_SkinedVertex.size(); c++)
	{
		 SkinedVertex & skinVertex = m_SkinedVertex[c];

		 GFPhysVector3 origin = m_PhysicsBody->GetSegment(skinVertex.m_BoneSegIndex[1]).m_Node0->m_CurrPosition;

		 GFPhysVector3 skinedPos(0, 0, 0);

		 GFPhysVector3 skinedNorm(0, 0, 0);

		 for (int s = 0; s < 2; s++)
		 {
			 int SegIndex = skinVertex.m_BoneSegIndex[s];

			 const GFPhysVector3 & d1 = boneFrames[SegIndex].GetRow(0);
			
			 const GFPhysVector3 & d2 = boneFrames[SegIndex].GetRow(1);
			 
			 const GFPhysVector3 & d3 = boneFrames[SegIndex].GetRow(2);

			 skinedPos  += (skinVertex.m_BoneCoord[s].x * d1 + skinVertex.m_BoneCoord[s].y * d2 + skinVertex.m_BoneCoord[s].z * d3)*0.5f;

			 skinedNorm += (skinVertex.m_NormCoord[s].x * d1 + skinVertex.m_NormCoord[s].y * d2 + skinVertex.m_NormCoord[s].z * d3)*0.5f;
		 }
		 skinVertex.m_SKinedNorm = GPVec3ToOgre(skinedNorm.Normalized());
		 skinVertex.m_SkinedPos  = GPVec3ToOgre(skinedPos + origin);
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------
void ACTubeShapeObject::UpdateRendBuffer(int circleSegnum)
{
	m_SkinRendObject->clear();
	m_SkinRendObject->setDynamic(true);
	m_SkinRendObject->estimateVertexCount(1000);
	m_SkinRendObject->estimateIndexCount(3000);

	m_SkinRendObject->begin(m_OwnerMaterialPtr->getName(), Ogre::RenderOperation::OT_TRIANGLE_LIST);

	//UPDATE VERTEX
	for (int c = 0; c < m_SkinedVertex.size(); c++)
	{
		m_SkinRendObject->position(m_SkinedVertex[c].m_SkinedPos);
		m_SkinRendObject->normal(m_SkinedVertex[c].m_SKinedNorm);
	}

	//build index (optimize this is const)
	int numTubeSegment = m_PhysicsBody->GetNumSegments();
	
	int Offset = 0;
	
	for (int s = 0; s < numTubeSegment; s++)
	{
		for (int n = 0; n < circleSegnum; n++)
		{
			int i0 = Offset + n;
			int i1 = Offset + (n + 1) % circleSegnum;
			int j0 = i0 + circleSegnum;
			int j1 = i1 + circleSegnum;

			if (s == numTubeSegment - 1)
			{
				j0 = n; 
				j1 = (n + 1) % circleSegnum;
			}
			m_SkinRendObject->triangle(i0, i1, j0);
			m_SkinRendObject->triangle(j0, i1, j1);
		}
		Offset += circleSegnum;
	}

	m_SkinRendObject->end();
}