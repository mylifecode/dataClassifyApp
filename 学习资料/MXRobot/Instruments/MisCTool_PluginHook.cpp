#include "MisCTool_PluginHook.h"
#include "Tool.h"
#include "MisMedicOrganInterface.h"
#include "MisMedicOrganOrdinary.h"
#include "VeinConnectObject.h"
#include "CustomCollision.h"
#include "MisMedicThreadRope.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "ITraining.h"
#include "Collision/CollisionDispatch/GoPhysSoftRigidCollision.h"


MisCTool_PluginHook::MisCTool_PluginHook(CTool * tool , GFPhysRigidBody * hookPart) : MisMedicCToolPluginInterface(tool)
{
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

	m_TimeSinceLastRelease = 0;
	m_ElectricKeepTime = 0;
	m_HookPart = hookPart;
}
//===========================================================================================================
MisCTool_PluginHook::~MisCTool_PluginHook()
{
	ReleaseHook();

	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}
//==========================================================================================================
GFPhysVector3 MisCTool_PluginHook::GetPluginForceFeedBack()
{
	if (m_FaceBeHooked.size() > 0)
		return -m_FaceBeHooked[0].m_HookImpluse*20.0f;
	else
		return GFPhysVector3(0, 0, 0);
}
//===========================================================================================================
void MisCTool_PluginHook::UpdateHinHookLine(const GFPhysVector3 & pointHead , const GFPhysVector3 & pointEnd , const GFPhysVector3 & hintOffsetVec)
{
	m_HookDir = (pointHead - pointEnd).Normalized();
	m_HookLineHead = pointHead;
    m_HookLineEnd  = pointEnd;
	m_HookHintOffset = hintOffsetVec;
}
//===========================================================================================================
void MisCTool_PluginHook::ToolElectriced(int touchtype , float dt)
{
	if(m_FaceBeHooked.size() <= 0)
	   return;

	FaceBeHookeed & hookData = m_FaceBeHooked[0];

	GFPhysSoftBody * physBody = hookData.m_SoftBody;

	GFPhysSoftBodyFace * physFace = hookData.m_PhysFace;

	MisMedicOrganInterface * organif = (MisMedicOrganInterface *)physBody->GetUserPointer();
	
	MisMedicOrgan_Ordinary * organMesh = dynamic_cast<MisMedicOrgan_Ordinary*>(organif);
	
	if (organMesh)
	{
		float BurnValue = organMesh->GetCreateInfo().m_BurnRation * dt;

		GPClamp(BurnValue, 0.0f, 1.0f);

		organMesh->Tool_InElec_TouchFacePoint(m_ToolObject, physFace, hookData.m_PointInFaceWeight, 0, dt);

		m_ElectricKeepTime += dt;

		if (m_ElectricKeepTime > 1.0f)
		{
			ElectricCutAtHookPoint();
			m_ElectricKeepTime = 0;
		}
	}
}
//===========================================================================================================
void MisCTool_PluginHook::ElectricCutAtHookPoint()
{
	if(m_FaceBeHooked.size() > 0)//simple method need optimize
	{
		FaceBeHookeed & hookData = m_FaceBeHooked[0];

		GFPhysSoftBody * physBody = hookData.m_SoftBody;

		GFPhysSoftBodyFace * physFace = hookData.m_PhysFace;

		MisMedicOrganInterface * organif = (MisMedicOrganInterface *)physBody->GetUserPointer();
		MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(organif);

		float * ptWeights = m_FaceBeHooked[0].m_PointInFaceWeight;
		GFPhysSoftBodyNode * nearestNode;

		if(ptWeights[0] > ptWeights[1])
		{
		   if(ptWeights[0] > ptWeights[2])
			  nearestNode = physFace->m_Nodes[0];
		   else
			  nearestNode = physFace->m_Nodes[2];
		}
		else
		{
		   if(ptWeights[1] > ptWeights[2])
			  nearestNode = physFace->m_Nodes[1];
		   else
			  nearestNode = physFace->m_Nodes[2];
		}

		ReleaseHook();

		organ->DestroyTissueAroundNode(physFace  , ptWeights , false);// (nearestNode->m_UnDeformedPos, false);

		m_TimeSinceLastRelease = 0;
	}
}
//===========================================================================================================
void MisCTool_PluginHook::OneFrameUpdateStarted(float timeelapsed)
{
	m_TimeSinceLastRelease += timeelapsed;
}
//===========================================================================================================
void MisCTool_PluginHook::OneFrameUpdateEnded()
{
	if(m_FaceBeHooked.size() > 0)//simple method need optimize
	{
		FaceBeHookeed & hookData = m_FaceBeHooked[0];

		GFPhysSoftBody * physBody = hookData.m_SoftBody;
		
		GFPhysSoftBodyFace * physFace = hookData.m_PhysFace;
#if(0)
	   GFPhysVector3 currPos0 = physFace->m_Nodes[0]->m_CurrPosition;
	   GFPhysVector3 currPos1 = physFace->m_Nodes[1]->m_CurrPosition;
	   GFPhysVector3 currPos2 = physFace->m_Nodes[2]->m_CurrPosition;

	   GFPhysVector3 undeformPos0 = physFace->m_Nodes[0]->m_UnDeformedPos;
	   GFPhysVector3 undeformPos1 = physFace->m_Nodes[1]->m_UnDeformedPos;
	   GFPhysVector3 undeformPos2 = physFace->m_Nodes[2]->m_UnDeformedPos;


	   GFPhysVector3 edge0 = (currPos1 - currPos0);
	   GFPhysVector3 edge1 = (currPos1 - currPos2);
	   GFPhysVector3 edge2 = (currPos2 - currPos0);

	   float restLen0 = (undeformPos1 - undeformPos0).Length();
	   float restLen1 = (undeformPos1 - undeformPos2).Length();
	   float restLen2 = (undeformPos2 - undeformPos0).Length();

	   float currLen0 = edge0.Length();
	   float currLen1 = edge1.Length();
	   float currLen2 = edge2.Length();

	   float scale0 = (restLen0 > 0.0001f ? currLen0 / restLen0 : 1.0f);
	   float scale1 = (restLen1 > 0.0001f ? currLen1 / restLen1 : 1.0f);
	   float scale2 = (restLen2 > 0.0001f ? currLen2 / restLen2 : 1.0f);

       if(scale0 + scale1 + scale2 > 4.0f)
	   {
		   int i = 0;
		   int j = i+1;//
	   }
#endif
	   GFPhysVector3 hookDirWorld = QuatRotate(hookData.m_RigidBody->GetCenterOfMassTransform().GetRotation() , m_HookDir);
	   GFPhysVector3 hookSupDir = QuatRotate(hookData.m_RigidBody->GetCenterOfMassTransform().GetRotation(), m_HookHintOffset.Normalized());
	   float normalHook = hookData.m_HookImpluse.Dot(hookDirWorld);
	   float hookImpluse = hookData.m_HookImpluse.Length();
       float faceImpluse = hookData.m_HookImpluse.Dot(physFace->m_FaceNormal);
	   /*
	   if(normalHook < -0.8f)
	   {
		  //ReleaseHook();
	   }
	   else */if (hookImpluse > 2.5f && hookData.m_HookImpluse.Dot(hookSupDir) > 0.6f)//&& faceImpluse > 0)
	   {
		  MisMedicOrganInterface * organif = (MisMedicOrganInterface *)physBody->GetUserPointer();
		  MisMedicOrgan_Ordinary * organ = dynamic_cast<MisMedicOrgan_Ordinary*>(organif);
		
		  float * ptWeights = m_FaceBeHooked[0].m_PointInFaceWeight;
		  GFPhysSoftBodyNode * nearestNode;

		  if(ptWeights[0] > ptWeights[1])
		  {
			 if(ptWeights[0] > ptWeights[2])
			      nearestNode = physFace->m_Nodes[0];
			 else
			      nearestNode = physFace->m_Nodes[2];
		  }
		  else
		  {
			 if(ptWeights[1] > ptWeights[2])
				nearestNode = physFace->m_Nodes[1];
			 else
				nearestNode = physFace->m_Nodes[2];
		  }
		  
		  organ->AddInjuryPoint(physFace, ptWeights, 0);

		  ReleaseHook();
		  
		  organ->DestroyTissueAroundNode(physFace, ptWeights, false);// (nearestNode->m_UnDeformedPos, false);
 
		  m_TimeSinceLastRelease = 0;
	   }
	}
}	
//===========================================================================================================
void MisCTool_PluginHook::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	for(size_t c = 0 ; c < m_FaceBeHooked.size() ; c++)
	{
	    m_FaceBeHooked[c].m_HookImpluse = GFPhysVector3(0,0,0);
	}
}
//===========================================================================================================
void MisCTool_PluginHook::SolveConstraint(Real Stiffness,Real TimeStep)
{
	for(size_t c = 0 ; c < m_FaceBeHooked.size() ; c++)
	{
		const MisCTool_PluginHook::FaceBeHookeed & hookedFace = m_FaceBeHooked[c];

		const GFPhysTransform & Trans = hookedFace.m_RigidBody->GetCenterOfMassTransform();
		GFPhysVector3 PointOnRigid = Trans * hookedFace.m_PointInRigidLocal;

		

#if(1)
		//float ww = hookedFace.m_PointInFaceWeight[0]*hookedFace.m_PointInFaceWeight[0]
		//+hookedFace.m_PointInFaceWeight[1]*hookedFace.m_PointInFaceWeight[1]
		//+hookedFace.m_PointInFaceWeight[2]*hookedFace.m_PointInFaceWeight[2];

		//float w[3];
		//w[0] = hookedFace.m_PointInFaceWeight[0] / ww;
		//w[1] = hookedFace.m_PointInFaceWeight[1] / ww;
		//w[2] = hookedFace.m_PointInFaceWeight[2] / ww;

		GFPhysVector3 PointOnFace = hookedFace.m_PhysFace->m_Nodes[0]->m_CurrPosition * hookedFace.m_PointInFaceWeight[0]
		                           +hookedFace.m_PhysFace->m_Nodes[1]->m_CurrPosition * hookedFace.m_PointInFaceWeight[1]
		                           +hookedFace.m_PhysFace->m_Nodes[2]->m_CurrPosition * hookedFace.m_PointInFaceWeight[2];


		GFPhysVector3 delta = PointOnRigid-PointOnFace;

		for (int n = 0; n < 3; n++)
		{
			if (hookedFace.m_PointInFaceWeight[n])
			    hookedFace.m_PhysFace->m_Nodes[n]->m_CurrPosition += delta;// *w[0];
		}
	
#else
        GFPhysVector3 PointOnSoft = hookedFace.m_Node->m_CurrPosition;
        GFPhysVector3 delta = PointOnRigid-PointOnSoft;
		hookedFace.m_Node->m_CurrPosition += delta;
#endif
		m_FaceBeHooked[c].m_HookImpluse += delta;
	}
}
//===========================================================================================================
void MisCTool_PluginHook::onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair)
{
	
}
//==================================================================================================
void MisCTool_PluginHook::PhysicsSimulationEnd(int currStep , int TotalStep , float dt)
{
	
}
//==================================================================================================
void MisCTool_PluginHook::onRSFaceContactsBuildFinish( GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints)
{
	
}
//==================================================================================================
void MisCTool_PluginHook::onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints)
{
	if(m_TimeSinceLastRelease < 1.0f)
	   return;

	if(m_FaceBeHooked.size() > 0)
	   return;

	uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);

	if(RSContactConstraints.size() > 0)
	{
	   for(size_t c = 0 ; c < RSContactConstraints.size() ; c++)
	   {
           const GFPhysSoftFaceRigidContact & srContact = RSContactConstraints[c];
           
		   MisMedicOrgan_Ordinary * organif = (MisMedicOrgan_Ordinary *)srContact.m_SoftBody->GetUserPointer();

		   if(organif && organif->CanBeHook() && srContact.m_Rigid == m_HookPart)
		   {
			   float contactImpluse = srContact.GetNormalImpluse(0) + srContact.GetNormalImpluse(1) + srContact.GetNormalImpluse(2);

			   GFPhysVector3 hookDirWorld = QuatRotate(srContact.m_Rigid->GetCenterOfMassTransform().GetRotation() , m_HookDir);
			  
			   GFPhysVector3 hookInnerDirWorld = QuatRotate(srContact.m_Rigid->GetCenterOfMassTransform().GetRotation(), m_HookHintOffset.Normalized());
			  
			   bool canHook = false;

			   if (contactImpluse > 1.0f && hookDirWorld.Dot(srContact.m_NormOnRigidWorld) > 0.7f)
				   canHook = true;
			   
			   if (contactImpluse > 0.8f && hookInnerDirWorld.Dot(srContact.m_NormOnRigidWorld) > 0.7f)
				   canHook = true;

			   if (canHook)
			   {
				  MisCTool_PluginHook::FaceBeHookeed hookedFace;

				  hookedFace.m_RigidBody = srContact.m_Rigid;
				  hookedFace.m_SoftBody = srContact.m_SoftBody;
				  hookedFace.m_PhysFace = srContact.m_SoftFace;

				  hookedFace.m_PointInFaceWeight[0] = srContact.m_FaceWeights[0];
				  hookedFace.m_PointInFaceWeight[1] = srContact.m_FaceWeights[1];
				  hookedFace.m_PointInFaceWeight[2] = srContact.m_FaceWeights[2];

				  if(hookedFace.m_PhysFace->m_Nodes[0]->m_StateFlag & GPSESF_CONNECTED)//do not hook connected node
				     hookedFace.m_PointInFaceWeight[0] = 0;
				  
				  if(hookedFace.m_PhysFace->m_Nodes[1]->m_StateFlag & GPSESF_CONNECTED)
					 hookedFace.m_PointInFaceWeight[1] = 0;
				  
				  if(hookedFace.m_PhysFace->m_Nodes[2]->m_StateFlag & GPSESF_CONNECTED)
					 hookedFace.m_PointInFaceWeight[2] = 0;

				  float sumW = hookedFace.m_PointInFaceWeight[0]
					  + hookedFace.m_PointInFaceWeight[1]
					  + hookedFace.m_PointInFaceWeight[2];

				  if (sumW == 0)
				     continue;

				  hookedFace.m_PointInFaceWeight[0] /= sumW;
				  hookedFace.m_PointInFaceWeight[1] /= sumW;
				  hookedFace.m_PointInFaceWeight[2] /= sumW;

				  hookedFace.m_PointInRigidLocal = srContact.m_PointOnRigidLocal;

				  float t = (hookedFace.m_PointInRigidLocal - m_HookLineHead).Dot(m_HookDir);
				  
				  hookedFace.m_PointInRigidLocal = m_HookLineHead + m_HookDir * t ;
				   hookedFace.m_PointInRigidLocal = hookedFace.m_PointInRigidLocal + (m_HookHintOffset*0.05f - m_HookDir*0.05f);//offset backward and upward a little to get a more obvious "hook effect"
				  			  
				  if(m_FaceBeHooked.size() == 0)
				  {
				     GFPhysVector3 undeformedHookPos = hookedFace.m_PhysFace->m_Nodes[0]->m_UnDeformedPos * hookedFace.m_PointInFaceWeight[0]
						                             + hookedFace.m_PhysFace->m_Nodes[1]->m_UnDeformedPos * hookedFace.m_PointInFaceWeight[1]
					                                 + hookedFace.m_PhysFace->m_Nodes[2]->m_UnDeformedPos * hookedFace.m_PointInFaceWeight[2];

					 std::vector<GFPhysSoftBodyFace*> AroundedFaces;
                     organif->SelectPhysFaceAroundPoint(AroundedFaces ,undeformedHookPos, 0.5f , false);

					 for(size_t c = 0 ; c < AroundedFaces.size(); c++)
					 {    
						 AroundedFaces[c]->m_RSCollisionMask &= (~toolcat);
                         hookedFace.m_HookedFaceUID.push_back(AroundedFaces[c]->m_uid);
					 }
					 //temp disable node collision in face -- to do only disable with this rigid
					 //hookedFace.m_PhysFace->m_Nodes[0]->m_RSCollisionMask = 0;
					 //hookedFace.m_PhysFace->m_Nodes[1]->m_RSCollisionMask = 0;
					// hookedFace.m_PhysFace->m_Nodes[2]->m_RSCollisionMask = 0;

					 //
					 m_FaceBeHooked.push_back(hookedFace);
					 
				  }
			   }
		   }
	   }
	}
}
//==================================================================================================
void MisCTool_PluginHook::ReleaseHook()
{
	uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);

	for(size_t c = 0 ; c < m_FaceBeHooked.size() ; c++)
	{
		//GFPhysSoftBodyFace * physFace = m_FaceBeHooked[c].m_PhysFace;

		//temp disable node collision in face
		//physFace->m_Nodes[0]->m_RSCollisionMask = (~0);
		//physFace->m_Nodes[1]->m_RSCollisionMask = (~0);
		//physFace->m_Nodes[2]->m_RSCollisionMask = (~0);
		//
        GFPhysSoftBodyShape & softbodyShape = m_FaceBeHooked[c].m_SoftBody->GetSoftBodyShape();

		for(size_t f = 0 ; f < m_FaceBeHooked[c].m_HookedFaceUID.size(); f++)
		{    
			int faceid = m_FaceBeHooked[c].m_HookedFaceUID[f];
            GFPhysSoftBodyFace * physFace = softbodyShape.GetFaceByUID(faceid);
			if(physFace)
			{
				physFace->m_RSCollisionMask |= toolcat;// (~0);
			}
		}
	}
	m_FaceBeHooked.clear();
}