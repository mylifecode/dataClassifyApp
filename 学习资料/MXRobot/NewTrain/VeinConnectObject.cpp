#include "stdafx.h"
#include "VeinConnectObject.h"
#include "PhysicsWrapper.h"
#include "CustomCollision.h"
#include "MisMedicOrganOrdinary.h"
//#include "MisMedicOrganTube.h"
#include "ITraining.h"
#include "TextureBloodEffect.h"
#include "MXOgreGraphic.h"
#include "MXEventsDump.h"
#include "Dynamic/Solver/GoPhysParallelSolver.h"
//#define EDOT_GALLBLADDER 3
//#define EDOT_LIVER 5

//========================================================================================================================
/*VeinConnectPair::VeinConnectPair()
{
	m_faceA = m_faceB = 0;
	m_SoftFixGallCS = 0;
	m_GallLiverDistCS = 0;
}*/


VeinConnectPair::VeinConnectPair()
{	
	m_faceA = m_faceB = 0;
	//m_tubeA = m_tubeB = 0;
	
	//m_tubAConnectSeg = -1;
	//m_tubBConnectSeg = -1;

	m_BVNode = 0;

	m_type = VeinConnectPair::PCT_NONE;
	//m_TubeTubeConnect = 0;
	//m_TubeFaceConnect = 0;
	//m_FaceFaceConnect[0] = m_FaceFaceConnect[1] = 0;
	//m_FaceFaceWeakConnect = 0;
	
	texcoord[0] = texcoord[1] = texcoord[2] = texcoord[3] = Ogre::Vector2::ZERO;
	m_Valid = true;
	m_SpanScale = 1.0f;
	m_PairColor = Ogre::ColourValue::White;
	
	m_OrganAID = -1;//EDOT_NO_TYPE
	m_OrganBID = -1;//EDOT_NO_TYPE;
	m_TimeSinceLastBlood = 0;

    m_rebuildcount = 0;
    m_OwnObject = 0;

	m_StripNodeA = 0;
	m_StripNodeB = 0;
	m_FaceNodeA[0] = m_FaceNodeA[1] = m_FaceNodeA[2] = 0;
	m_FaceNodeB[0] = m_FaceNodeB[1] = m_FaceNodeB[2] = 0;

	m_InvFaceAMass = 0;
	m_InvNodeAMass = 0;

	m_InvFaceBMass = 0;
	m_InvNodeBMass = 0;
}
//========================================================================================================================
VeinConnectPair::~VeinConnectPair()
{
	
}
//========================================================================================================
bool VeinConnectPair::IsContact() const
{
	if(m_Valid)
	{
		if(m_BVNode)
		{
			VeinCollideData * cdata = (VeinCollideData *)m_BVNode->m_UserData;
			if(cdata)
			{
				return cdata->m_InContact;
			}
		}
	}
	return false;
}
//========================================================================================================
float VeinConnectPair::GetRestLength() const
{
	if(m_Valid)
	{
		if(m_BVNode)
		{
			VeinCollideData * cdata = (VeinCollideData *)m_BVNode->m_UserData;
			if(cdata)
			{
#if SEGMENT_COLLIDE
                return (cdata->m_PointA - cdata->m_PointB).Length();
#else
                return (cdata->m_PointA0 - cdata->m_PointB0).Length();
#endif				
			}
		}
	}
	return 0;
}
//========================================================================================================
float VeinConnectPair::GetCurrLength() const
{
	if(m_Valid)
	{
		if(m_BVNode)
		{
			VeinCollideData * cdata = (VeinCollideData *)m_BVNode->m_UserData;
			if(cdata)
			{
#if SEGMENT_COLLIDE
				if(cdata->m_InContact)
				{
					return (cdata->m_contactinWorld - cdata->m_PointA).Length() + (cdata->m_contactinWorld - cdata->m_PointB).Length();
				}
				else
				{
					return (cdata->m_PointA - cdata->m_PointB).Length();
				}
#else
                if (cdata->m_InContact)
                {
                    return (cdata->m_contactinWorld - cdata->m_PointA0).Length() + (cdata->m_contactinWorld - cdata->m_PointB0).Length();
                }
                else
                {
                    return (cdata->m_PointA0 - cdata->m_PointB0).Length();
                }
#endif
			}
		}
	}
	return 0;
}
GFPhysVector3 VeinConnectPair::GetPosOnA()
{
	GFPhysVector3 pos = m_faceA->m_Nodes[0]->m_CurrPosition * m_weightsA[0]
		+ m_faceA->m_Nodes[1]->m_CurrPosition * m_weightsA[1]
		+ m_faceA->m_Nodes[2]->m_CurrPosition * m_weightsA[2];
	return pos;
}

GFPhysVector3 VeinConnectPair::GetPosOnB()
{
	GFPhysVector3 pos = m_faceB->m_Nodes[0]->m_CurrPosition * m_weightsB[0]
		+ m_faceB->m_Nodes[1]->m_CurrPosition * m_weightsB[1]
		+ m_faceB->m_Nodes[2]->m_CurrPosition * m_weightsB[2];
	return pos;
}

void VeinConnectPair::BuildSolveCachedData()
{
	float facemass[3];

	{
		facemass[0] = m_faceA->m_Nodes[0]->m_Mass;
		facemass[1] = m_faceA->m_Nodes[1]->m_Mass;
		facemass[2] = m_faceA->m_Nodes[2]->m_Mass;

		m_FaceNodeA[0] = m_faceA->m_Nodes[0];
		m_FaceNodeA[1] = m_faceA->m_Nodes[1];
		m_FaceNodeA[2] = m_faceA->m_Nodes[2];

		if (facemass[0] < GP_EPSILON || facemass[1] < GP_EPSILON || facemass[2] < GP_EPSILON)
		{
			m_InvFaceAMass = 0;
		}
		else
		{
			m_InvFaceAMass = 1.0f / (facemass[0] + facemass[1] + facemass[2]);
		}
		m_InvNodeAMass = (m_StripNodeA->m_Mass > GP_EPSILON ? 1.0f / m_StripNodeA->m_Mass : 0.0f);
	}

	{
		facemass[0] = m_faceB->m_Nodes[0]->m_Mass;
		facemass[1] = m_faceB->m_Nodes[1]->m_Mass;
		facemass[2] = m_faceB->m_Nodes[2]->m_Mass;

		m_FaceNodeB[0] = m_faceB->m_Nodes[0];
		m_FaceNodeB[1] = m_faceB->m_Nodes[1];
		m_FaceNodeB[2] = m_faceB->m_Nodes[2];

		if (facemass[0] < GP_EPSILON || facemass[1] < GP_EPSILON || facemass[2] < GP_EPSILON)
		{
			m_InvFaceBMass = 0;
		}
		else
		{
			m_InvFaceBMass = 1.0f / (facemass[0] + facemass[1] + facemass[2]);
		}
		m_InvNodeBMass = (m_StripNodeB->m_Mass > GP_EPSILON ? 1.0f / m_StripNodeB->m_Mass : 0.0f);
	}
}
//========================================================================================================
void VeinConnectPair::SetAttachStateInternal(GFPhysCollideObject * rigidobj, const GFPhysVector3 & worldpoint, const GFPhysVector3 & localHookOffset, const GFPhysVector3 & localHookDir)
{
	if(m_BVNode && m_BVNode->m_UserData)
	{
		VeinCollideData * collideData = (VeinCollideData *)m_BVNode->m_UserData;
		collideData->m_contactRigid = rigidobj;
		collideData->AddHookPoint(worldpoint, localHookOffset, localHookDir);
	}
}
//========================================================================================================================
GFPhysVector3  VeinConnectPair::GetHookPointForce(const GFPhysVector3 & InvHookSuportOffsetdir , bool IsTestDir) const
{
	GFPhysVector3 force(0,0,0);

	GFPhysVector3 partA;

	GFPhysVector3 partB;
	
	GFPhysVector3 stickPoint;

	float RestLength = 0;

#if(0)
	if(m_type & VeinConnectPair::PCT_FF)//m_FaceFaceConnect[0])
	{
		stickPoint = m_WolrdHookPoint;//m_FaceFaceConnect[0]->m_StickPoint;
		RestLength = m_FaceFaceConnect.m_RestLen;
	}
	else if(m_type & VeinConnectPair::PCT_WFF)//m_FaceFaceWeakConnect)
	{
		stickPoint = m_WolrdHookPoint;
		RestLength = m_FaceFaceWeakConnect.m_RestLength[2];
	}

	if(m_BVNode && m_BVNode->m_UserData)
	{
		VeinCollideData * collideData = (VeinCollideData *)m_BVNode->m_UserData;
#if SEGMENT_COLLIDE
		GFPhysVector3 StickPointA = collideData->m_PointA;

		GFPhysVector3 StickPointB = collideData->m_PointB;
#else
        GFPhysVector3 StickPointA = collideData->m_PointA0;

        GFPhysVector3 StickPointB = collideData->m_PointB0;
#endif
		partA = /*m_CurrPointPosA*/StickPointA - stickPoint;
		
		partB = /*m_CurrPointPosB*/StickPointB - stickPoint;

		float lenA = partA.Length();

		float lenB = partB.Length();

		if(lenA < GP_EPSILON || lenB < GP_EPSILON)
		   return GFPhysVector3(0,0,0);

		RestLength = (/*m_CurrPointPosA*/StickPointA-/*m_CurrPointPosB*/StickPointB).Length();

		float currlength = lenA + lenB-0.08f;
		
		float expand = currlength-RestLength;
		
		if(expand < 0)
		   expand = 0;
		
		//partA.Normalize();

		//partB.Normalize();

		GFPhysVector3 direction = (partA+partB).Normalized();

		force = /*(partA+partB)*/direction*expand*0.18f;
		
		if(force.Dot(InvHookSuportOffsetdir) < 0 && IsTestDir)
		   force = GFPhysVector3(0,0,0);
	}
	else
	    force = GFPhysVector3(0,0,0);
	//temply
	//if(m_FaceFaceConnect[0])
	//   force = GFPhysVector3(0,0,0);
#endif
	return force;
}
//========================================================================================================================
void VeinConnectPair::RemovePhysicsPartInternal(GFPhysDBVTree & hostCollideTree)
{
	GFPhysDiscreteDynamicsWorld * physWorld = PhysicsWrapper::GetSingleTon().m_dynamicsWorld;
	
	m_type = PCT_NONE;

	//remove constraint first
	/*
	for(int f = 0 ;f < 2 ; f++)
	{
		if(m_FaceFaceConnect[f])
		{
			if(physWorld)
			   physWorld->RemovePositionConstraint(m_FaceFaceConnect[f]);
			delete m_FaceFaceConnect[f];
			m_FaceFaceConnect[f] = 0;
		}
	}
	
	if(m_TubeTubeConnect)
	{
		if(physWorld)
		   physWorld->RemovePositionConstraint(m_TubeTubeConnect);
		delete m_TubeTubeConnect;
		m_TubeTubeConnect = 0;
	}

	if(m_TubeFaceConnect)
	{
		if(physWorld)
		   physWorld->RemovePositionConstraint(m_TubeFaceConnect);
		delete m_TubeFaceConnect;
		m_TubeFaceConnect = 0;
	}

	if(m_FaceFaceWeakConnect)
	{
		if(physWorld)
		   physWorld->RemovePositionConstraint(m_FaceFaceWeakConnect);
		delete m_FaceFaceWeakConnect;
		m_FaceFaceWeakConnect = 0;
	}	
	*/
	//remove collide data
	if(m_BVNode)
	{
		VeinCollideData * collideData = (VeinCollideData *)m_BVNode->m_UserData;
		
		delete collideData;
		
		m_BVNode->m_UserData = 0;
		
		hostCollideTree.RemoveAABBNode(m_BVNode);//remove aabb node from tree

		m_BVNode = 0;
	}
}
//========================================================================================================================
void VeinConnectCluster::RemovePhysicsPartInternal(GFPhysDBVTree & CollideTree)
{
	VeinConnectPair & MajorPair  = m_pair[0];

	VeinConnectPair & AttachPair = m_pair[1];

	if (MajorPair.m_Valid)
	{
		MajorPair.RemovePhysicsPartInternal(CollideTree);
	}

	if (AttachPair.m_Valid)
	{
		AttachPair.RemovePhysicsPartInternal(CollideTree);
	}

	MajorPair.m_Valid = false;
	
	AttachPair.m_Valid = false;
}
//========================================================================================================================
void VeinConnectCluster::OnAttachFaceChanged()
{
	//constraint only in pair0
	m_pair[0].BuildSolveCachedData();
	m_pair[1].BuildSolveCachedData();

	//if (m_pair[0].m_type == VeinConnectPair::PCT_WFF)
	//{
		/*
		m_pair[0].m_FaceFaceWeakConnect.OnAttachFaceChanged(m_pair[0].m_faceA, m_pair[1].m_faceA,
				                                            m_pair[0].m_faceB, m_pair[1].m_faceB,
				                                            m_pair[0].m_weightsA, m_pair[1].m_weightsA,
				                                            m_pair[0].m_weightsB,m_pair[1].m_weightsB);
															*/
	//}
	//else if (m_pair[0].m_type == VeinConnectPair::PCT_FF)
	//{
		/*
		m_pair[0].m_FaceFaceConnect.OnAttachFaceChanged(m_pair[0].m_faceA, m_pair[1].m_faceA,
			                                            m_pair[0].m_faceB, m_pair[1].m_faceB,
			                                            m_pair[0].m_weightsA, m_pair[1].m_weightsA,
			                                            m_pair[0].m_weightsB, m_pair[1].m_weightsB);
														*/
	//}
}
//========================================================================================================================zYY
void VeinConnectCluster::SetHookOn(GFPhysCollideObject * rigid, const GFPhysVector3 & worldpoint, const GFPhysVector3 & localHookOffset, const GFPhysVector3 & localHookDir)
{
	//if (m_isHooked)
	//	return;
	
	m_pair[0].SetAttachStateInternal(rigid, worldpoint, localHookOffset, localHookDir);

	if(m_mode == VeinConnectCluster::STAY)
	{
		m_ColorBak = m_Color;
		m_SpanScaleBak = m_SpanScale;

		m_ColorOfHookedPart = m_Color; 
		//m_ColorOfHookedPart.a *= 0.9;
		//m_Color.a *= 0.93;//
		m_SpanScaleOfHookedPart = m_SpanScale * 0.8;   
	}
	else if(m_mode == VeinConnectCluster::REDUCE)
	{
		m_ColorBak = m_Color;
		m_SpanScaleBak = m_SpanScale;
		
		m_SpanScaleOfHookedPart = m_SpanScale * 0.8; 
		m_ColorOfHookedPart = m_Color;

		//m_Color.a *= 0.95;
		m_SpanScale *= 0.8;
	}
	else if(m_mode == VeinConnectCluster::DISCONNECT)
	{
		m_SpanScaleOfHookedPart = m_SpanScale;
		m_ColorOfHookedPart = m_Color;
	}

	m_isHooked = true;
}

void VeinConnectCluster::SetHookAndContactOff()
{
	VeinConnectPair & pair = m_pair[0];

	VeinCollideData * cdata = (VeinCollideData *)pair.m_BVNode->m_UserData;

	cdata->SetHookAndContactOff();

	if (m_isHooked)
	{
		if (m_mode == VeinConnectCluster::DISCONNECT)
		{
			m_SpanScale = m_SpanScaleOfHookedPart;
			m_SpanScaleOfHookedPart = 0;
		}
		else if (m_mode == VeinConnectCluster::REDUCE)
		{
			m_SpanScale = m_SpanScaleBak;
			m_Color = m_ColorBak;

			m_SpanScaleOfHookedPart = 0;
		}
		else if (m_mode == VeinConnectCluster::STAY)
		{
			m_SpanScale = m_SpanScaleBak;
			m_Color = m_ColorBak;

			m_SpanScaleOfHookedPart = 0;
		}
		m_isHooked = false;
	}
}

void VeinConnectCluster::RemoveOne()
{
	m_isHooked = false;
	if(m_mode == VeinConnectCluster::STAY)
	{
		m_StayNum--;
		if(m_StayNum <= 0)
			m_mode = m_ReduceNum > 0  ?  VeinConnectCluster::REDUCE : VeinConnectCluster::DISCONNECT;
	}
	else if(m_mode == VeinConnectCluster::REDUCE)
	{
		m_ReduceNum--;
		if(m_ReduceNum == 0)
			m_mode = VeinConnectCluster::DISCONNECT;
	}
	else if(m_mode == VeinConnectCluster::DISCONNECT)
	{
		m_Valid = false;
	}
}
void VeinConnectCluster::TotalyRemove()
{
	m_StayNum = 0;
	
	if (m_ReduceNum > 0)
	{
		m_ReduceNum--;
	}
	
	if (m_ReduceNum <= 0)
	{
		m_mode = VeinConnectCluster::DISCONNECT;
		m_Valid = false;
	}
}
//========================================================================================================================zYY
VeinConnectObject::VeinConnectObject(CBasicTraining * ownertriang)
:MisMedicOrganInterface(DOT_VEINCONNECT ,EODT_VEINCONNECT, 0 , ownertriang)
{
	m_SceneNode = 0;
	//m_pManualObject = 0;
	m_RenderObject = 0;
	m_Stiffness = 0.1f;
	m_connectCount = 0;
	m_HookedCount = 0;
	m_isHookLimitSet = false;
	m_CanBeHooked = true;
	m_IsNewMode = false;
	m_Actived = true;
	m_HookedLimitCount = CANHOOKNUM;
	m_DampingFactor = 10.0f;
	m_SolveFFVolumeCS = true;
	m_PhysBody = 0;
	//m_TextureBloodEffect = new TextureBloodTrackEffect(this);
	//m_TextureBloodEffect->SetBloodSystem(&m_DynBlood);
	SetHookedCount();
	m_CanBlood = true;
	m_TimeSinceLastblood = 0;
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);
}
//========================================================================================================================
VeinConnectObject::~VeinConnectObject()
{
	//if(m_TextureBloodEffect)
	//{
	//	delete m_TextureBloodEffect;
		//m_TextureBloodEffect = 0;
	//}
	if(m_BloodTextureEffect)//put this before blood system
	{
		delete m_BloodTextureEffect;
		m_BloodTextureEffect = 0;
	}
}
//========================================================================================================================
void VeinConnectObject::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
    
}
//========================================================================================================================
void VeinConnectObject::SolveConstraint(Real globalstiffness,Real TimeStep)
{
	if(m_Actived == false)
	   return;

    for (size_t c = 0, nc = m_clusters.size(); c <nc; c++)
	{
		VeinConnectPair & pair0 = m_clusters[c].m_pair[0];
		
		VeinConnectPair & pair1 = m_clusters[c].m_pair[1];
        
		if (m_clusters[c].m_IsDetoryed == false)
		{
			SolveFaceNode_NodeAnchor(pair0.m_FaceNodeA, pair0.m_weightsA, pair0.m_InvFaceAMass, pair0.m_InvNodeAMass, pair0.m_StripNodeA);

			SolveFaceNode_NodeAnchor(pair0.m_FaceNodeB, pair0.m_weightsB, pair0.m_InvFaceBMass, pair0.m_InvNodeBMass, pair0.m_StripNodeB);

			SolveFaceNode_NodeAnchor(pair1.m_FaceNodeA, pair1.m_weightsA, pair1.m_InvFaceAMass, pair1.m_InvNodeAMass, pair1.m_StripNodeA);

			SolveFaceNode_NodeAnchor(pair1.m_FaceNodeB, pair1.m_weightsB, pair1.m_InvFaceBMass, pair1.m_InvNodeBMass, pair1.m_StripNodeB);
		
			VeinCollideData * cdata = (VeinCollideData *)pair0.m_BVNode->m_UserData;
			
			if (cdata->m_contactRigid && (cdata->m_InContact || cdata->m_InHookState))
			{
				float Stiffniss = 0.1f;//hook stiffniss

				GFPhysVector3 CenterA = (pair0.m_StripNodeA->m_CurrPosition + pair1.m_StripNodeA->m_CurrPosition)*0.5f;
				GFPhysVector3 CenterB = (pair0.m_StripNodeB->m_CurrPosition + pair1.m_StripNodeB->m_CurrPosition)*0.5f;

				GFPhysVector3 CenterARest = (pair0.m_StripNodeA->m_UnDeformedPos + pair1.m_StripNodeA->m_UnDeformedPos)*0.5f;
				GFPhysVector3 CenterBRest = (pair0.m_StripNodeB->m_UnDeformedPos + pair1.m_StripNodeB->m_UnDeformedPos)*0.5f;

				float FaceAPartLen = (CenterA - m_clusters[c].m_StickPoint).Length();
				
				float FaceBPartLen = (CenterB - m_clusters[c].m_StickPoint).Length();
				
				float TotalLen = FaceAPartLen + FaceBPartLen;
				
				float RestLen = (CenterARest - CenterBRest).Length();
				
				if (TotalLen > RestLen && TotalLen > GP_EPSILON  && FaceBPartLen > GP_EPSILON && FaceAPartLen > GP_EPSILON)
				{
					float FaceADiff = (TotalLen - RestLen) * FaceAPartLen / TotalLen;

					float FaceBDiff = (TotalLen - RestLen) * FaceBPartLen / TotalLen;

					GFPhysVector3 s = -Stiffniss * FaceADiff * (CenterA - m_clusters[c].m_StickPoint) / FaceAPartLen;

					if (pair0.m_StripNodeA->m_InvM > GP_EPSILON)
					{
						pair0.m_StripNodeA->m_CurrPosition += s;
					}

					if (pair1.m_StripNodeA->m_InvM > GP_EPSILON)
					{
						pair1.m_StripNodeA->m_CurrPosition += s;
					}


			        s = -Stiffniss * FaceBDiff * (CenterB - m_clusters[c].m_StickPoint) / FaceBPartLen;

					if (pair0.m_StripNodeB->m_InvM > GP_EPSILON)
					{
						pair0.m_StripNodeB->m_CurrPosition += s;
					}

					if (pair1.m_StripNodeB->m_InvM > GP_EPSILON)
					{
						pair1.m_StripNodeB->m_CurrPosition += s;
					}
				}
			}
		}
	}
}
//========================================================================================================================
void VeinConnectObject::SolveFaceNode_NodeAnchor(GFPhysSoftBodyNode ** faceNodes, 
	                                             float weights[3], 
												 float InvFaceM,
												 float InvNodeM,
												 GFPhysSoftBodyNode * node)
{
	GFPhysVector3 p0 = faceNodes[0]->m_CurrPosition*weights[0]
		             + faceNodes[1]->m_CurrPosition*weights[1]
		             + faceNodes[2]->m_CurrPosition*weights[2];

	GFPhysVector3 p1 = node->m_CurrPosition;
	
	GFPhysVector3 Diff = (p0 - p1);

	float stiffness = 0.9f;

#if(0)
	float wsum = (InvFaceM + InvNodeM);

	if (wsum > GP_EPSILON)
	{
		GFPhysVector3 correctFace = -Diff * (InvFaceM / wsum) * stiffness;

		GFPhysVector3 correctNode = Diff * (InvNodeM / wsum) * stiffness;

		faceNodes[0]->m_CurrPosition += correctFace;
		faceNodes[1]->m_CurrPosition += correctFace;
		faceNodes[2]->m_CurrPosition += correctFace;

		node->m_CurrPosition += correctNode;
	}
#else
	Real Length = Diff.Length();

	if (Length > FLT_EPSILON)
	{
		float invFaceNodeM = InvFaceM * 3.0f;

		//weighted by invmass 's sum of gradient
		Real sumgrad = (weights[0] * weights[0] + weights[1] * weights[1] + weights[2] * weights[2]) * invFaceNodeM + InvNodeM;

		float w1 = invFaceNodeM / sumgrad;
		
		float w2 = InvNodeM / sumgrad;
		
		GFPhysVector3 delta00 = -Diff * (weights[0] * w1 * stiffness);
		
		GFPhysVector3 delta01 = -Diff * (weights[1] * w1 * stiffness);
		
		GFPhysVector3 delta02 = -Diff * (weights[2] * w1 * stiffness);

		GFPhysVector3 deltaNode = Diff * (w2 * stiffness);

		faceNodes[0]->m_CurrPosition += delta00;
		faceNodes[1]->m_CurrPosition += delta01;
		faceNodes[2]->m_CurrPosition += delta02;

		node->m_CurrPosition += deltaNode;
	}
#endif
}
//========================================================================================================================
void VeinConnectObject::SetStiffness(float stiff)
{
	m_Stiffness = stiff;
}
//========================================================================================================================
void VeinConnectObject::SetDampingFactor(float dampfactor)
{
	m_DampingFactor = dampfactor;
}
//========================================================================================================================
void VeinConnectObject::refreshMaterial(const std::string& materialname)
{
	m_MaterialName = materialname;
	m_EffectRender.InitRenderTexture(m_EffectRender.m_QuantityTexturePtr, Ogre::ColourValue(0,0,0,0));
	ApplyTextureToMaterial(materialname, m_EffectRender.m_QuantityTexturePtr , "BloodMap");
	if(m_RenderObject)
		m_RenderObject->setMaterialName(materialname);
}
//========================================================================================================================
int VeinConnectObject::GetCurrConnectCount()
{
	return m_connectCount;
}
//========================================================================================================================
int VeinConnectObject::GetInitConnectCount()
{
	return m_InitConnectCount;
}
//========================================================================================================================
void VeinConnectObject::SetClusterColor(int clusterid , const Ogre::ColourValue & color)
{
	if(clusterid >= 0 && clusterid < (int)m_clusters.size())
	{
		VeinConnectCluster & cluster = m_clusters[clusterid];

        for (int p = 0; p < 2; p++)
        {
            cluster.m_pair[p].m_PairColor = color;
        }		
	}	
}
//========================================================================================================================
void VeinConnectObject::Create(MisMedicDynObjConstructInfo & cs, DynObjMap & dynmap)
{
	m_CreateInfo = cs;
	
	//put this before create connect
	SetStiffness(cs.m_stiffness);
	SetDampingFactor(cs.m_veldamping);
	m_EffectRender.Create(cs.m_materialname[0], cs.m_effTexWid, cs.m_effTexWid, cs.m_name);//put this before create material
	refreshMaterial(cs.m_materialname[0]);

	m_IsNewMode = cs.m_VeinObjNewMode;
	m_ConnStayNum = cs.m_ConnStayNum;
	m_ConnReduceNum = cs.m_ConnReduceNum;

	if(m_IsNewMode)
	   SetNewMode();

	CreateConnectFromFile(MXOgreWrapper::Get()->GetDefaultSceneManger() , cs.m_s4mfilename , dynmap);
	readVeinConnectTexMapFile(cs.m_t2filename.c_str());
	
	m_BloodTextureEffect = new TextureBloodTrackEffect(this);
	m_BloodTextureEffect->SetBloodSystem(&m_DynBlood);

	//create physics part
	GFPhysVector3 * nodePos = new GFPhysVector3[m_clusters.size()*4];
	
	float * nodeMass = new float[m_clusters.size() * 4];
	
	float massPerNode = cs.m_mass / (m_clusters.size() * 4);
	
	for (int c = 0; c < m_clusters.size(); c++)
	{
		VeinConnectCluster & cluster = m_clusters[c];

		VeinConnectPair & pair0 = cluster.m_pair[0];

		VeinConnectPair & pair1 = cluster.m_pair[1];

		nodePos[4 * c + 0] = pair0.GetPosOnA();
		nodePos[4 * c + 1] = pair0.GetPosOnB();

		nodePos[4 * c + 2] = pair1.GetPosOnA();
		nodePos[4 * c + 3] = pair1.GetPosOnB();

		nodeMass[4 * c + 0] = massPerNode;
		nodeMass[4 * c + 1] = massPerNode;

		nodeMass[4 * c + 2] = massPerNode;
		nodeMass[4 * c + 3] = massPerNode;
	}

	m_PhysBody = new GFPhysSoftBody(nodePos, nodeMass, m_clusters.size() * 4, GFPhysSoftBodyShape::SRCT_NONE, GFPhysSoftBodyShape::SSCT_NONE, false, 0);
	delete[]nodePos;
	delete[]nodeMass;

	for (int t = 0; t < (int)m_clusters.size(); t++)
	{
		 GFPhysSoftBodyNode * NodeA0 = m_PhysBody->GetNode(4 * t + 0);
		 GFPhysSoftBodyNode * NodeB0 = m_PhysBody->GetNode(4 * t + 1);

		 GFPhysSoftBodyNode * NodeA1 = m_PhysBody->GetNode(4 * t + 2);
		 GFPhysSoftBodyNode * NodeB1 = m_PhysBody->GetNode(4 * t + 3);

		 m_clusters[t].m_pair[0].m_StripNodeA = NodeA0;
		 m_clusters[t].m_pair[0].m_StripNodeB = NodeB0;

		 m_clusters[t].m_pair[1].m_StripNodeA = NodeA1;
		 m_clusters[t].m_pair[1].m_StripNodeB = NodeB1;

		 for (int p = 0; p < 2; p++)
		 {
			 VeinConnectPair & pair = m_clusters[t].m_pair[p];
			 pair.BuildSolveCachedData();
		 }

		 GFPhysSoftBodyTetrahedron * tetrahedron = m_PhysBody->AddTetrahedron(NodeA0, NodeB0, NodeA1, NodeB1);
		 m_clusters[t].m_StripTetra = tetrahedron;
	}

	m_PhysBody->SetGravity(GFPhysVector3(0,0,0));//now apply gravity in app OgreToGPVec3(CreateInfo.m_CustomGravityDir*CreateInfo.m_GravityValue));//m_CreateInfo.m_Gravity.x,m_CreateInfo.m_Gravity.y,m_CreateInfo.m_Gravity.z));
	m_PhysBody->SetVelocityDamping(true, cs.m_veldamping, 0);//now apply damp in app //(true , veldamping , 0.00f);
	m_PhysBody->SetVelocityDampingMode(1);
	m_PhysBody->SetStiffness(m_CreateInfo.m_stiffness);
	m_PhysBody->SetInternalForceType(GFPhysSoftBody::IFT_LINK | GFPhysSoftBody::IFT_TETRA);// | GFPhysSoftBody::IFT_TETRA);
	m_PhysBody->m_ElasticParam.m_EdgeInvStiff = cs.m_invEdgePhysStiff;// 1.0f / 5000.0f;
	m_PhysBody->m_ElasticParam.m_EdgeDamping  = cs.m_EdgePhysDamping; //50.0f;

	m_PhysBody->m_ElasticParam.m_TetraInvStiff = cs.m_invTetraPhysStiff;;// 1.0f / 50000.0f;
	m_PhysBody->m_ElasticParam.m_TetraDamping  = cs.m_TetraPhysDamping;// 500.0f;
	m_PhysBody->SetVolumeSor(0.95f);

	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	{
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddSoftBody(m_PhysBody);
	}
}
//========================================================================================================================
const VeinConnectPair & VeinConnectObject::GetConnectPair(int clusterId  , int pairId)
{
    assert(pairId == 0 || pairId == 1);
    return m_clusters[clusterId].m_pair[pairId];
}
//========================================================================================================================
void  VeinConnectObject::SetConnectPairClampByRigid(int clusterId, int pairId, GFPhysCollideObject * rigid, const GFPhysVector3 & worldpoint)
{
	if (m_clusters[clusterId].m_IsDetoryed)
		return;

	m_clusters[clusterId].SetHookOn(rigid, worldpoint, GFPhysVector3(0, 0, 0), GFPhysVector3(0, 0, 0));
}
//========================================================================================================================
void VeinConnectObject::DisconnectPair(int clusterId)
{
	 if (m_clusters[clusterId].m_IsDetoryed)
		return;

	 VeinConnectPair & pair = m_clusters[clusterId].m_pair[0];
	
	 VeinCollideData * cdata = (VeinCollideData *)pair.m_BVNode->m_UserData;
	
	 if(cdata->m_InContact || cdata->m_InHookState)
	 {
	    CreateVeinBlood(pair , cdata->m_contactinWorld , 0 , 1.0f);
	 }

	 m_clusters[clusterId].SetHookAndContactOff();

	 m_clusters[clusterId].TotalyRemove();
	
	 SetHookOffInfo(clusterId);

	 if (m_clusters[clusterId].m_Valid == false)
	 {
		DestoryCluster(clusterId);

		m_Disconnected = true;
		
		MxEvent *  pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_VeinConnectBreak,NULL,&pair);
		
		CMXEventsDump::Instance()->PushEvent(pEvent);
	 }
}
//========================================================================================================================
void VeinConnectObject::ReleaseConnectPair(int clusterId)
{
	if (m_clusters[clusterId].m_IsDetoryed)
		return;

    VeinConnectPair & pair = m_clusters[clusterId].m_pair[0];
	
	if(pair.m_BVNode && pair.m_BVNode->m_UserData)
	{
		VeinCollideData * cdata = (VeinCollideData *)pair.m_BVNode->m_UserData;
		if(cdata->m_InHookState)
		{
			m_clusters[clusterId].SetHookAndContactOff();
			SetHookOffInfo(clusterId);
		}
	}
}

void  VeinConnectObject::ReleaseConnectPairWithRigid(int clusterId , GFPhysCollideObject * collideobj)
{
	if (m_clusters[clusterId].m_IsDetoryed)
		return;

	VeinConnectPair & pair = m_clusters[clusterId].m_pair[0];

	if (pair.m_BVNode && pair.m_BVNode->m_UserData)
	{
		VeinCollideData * cdata = (VeinCollideData *)pair.m_BVNode->m_UserData;
		if (cdata->m_InHookState && cdata->m_contactRigid == collideobj)
		{
			m_clusters[clusterId].SetHookAndContactOff();
			SetHookOffInfo(clusterId);
		}
	}
}
//========================================================================================================================
void VeinConnectObject::RemovePhysicsPart()
{
	MisMedicOrganInterface::RemovePhysicsPart();

	for(size_t c = 0 ; c < m_clusters.size() ; c++)
	{
        m_clusters[c].RemovePhysicsPartInternal(m_CollideTree);		
	}
	SetAllHookOff();
	m_clusters.clear();
	m_CollideTree.Clear();
	m_connectCount = 0;

	if (m_PhysBody)
	{
		if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		{
			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemoveSoftBody(m_PhysBody);
		}
		delete m_PhysBody;
		m_PhysBody = 0;
	}

	if (PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);

}
//========================================================================================================================
void VeinConnectObject::RemoveGraphicPart()
{
	Ogre::SceneManager * scenemgr = MXOgreWrapper::Get()->GetDefaultSceneManger();
	if(scenemgr)
	{
		//if(m_pManualObject)
		//{
		//	((Ogre::SceneNode*)m_pManualObject->getParentNode())->detachObject(m_pManualObject);
			//scenemgr->destroyManualObject(m_pManualObject);
		//	m_pManualObject = 0;
		//}

		if(m_RenderObject)
		{
			((Ogre::SceneNode*)m_RenderObject->getParentNode())->detachObject(m_RenderObject);
			scenemgr->destroyMovableObject(m_RenderObject);
			m_RenderObject = 0;
		}
		if(m_SceneNode)
		{
			scenemgr->getRootSceneNode()->removeAndDestroyChild(m_SceneNode->getName());
			m_SceneNode = 0;
		}
	}
}
//======================================================================================================================
void VeinConnectObject::CreateCollideTree()
{
	m_CollideTree.Clear();

	for(size_t c = 0 ; c < m_clusters.size() ; c++)
	{
        VeinConnectCluster & cluster = m_clusters[c];

        VeinConnectPair & pair = cluster.m_pair[0];

        //insert aabb
        GFPhysVector3 connectPointA = GFPhysVector3(0, 0, 0);

        GFPhysVector3 connectPointB = GFPhysVector3(0, 0, 0);

        GFPhysVector3 minBox = connectPointA;

        GFPhysVector3 maxBox = connectPointA;

        minBox.SetMin(connectPointB);

        maxBox.SetMax(connectPointB);

        //int Index = (c << 16) + p;

        GFPhysDBVNode * TreeNode = m_CollideTree.InsertAABBNode(minBox, maxBox);

#if SEGMENT_COLLIDE
        VeinCollideData * collideData = new VeinCollideData(connectPointA, connectPointB, c, 0);
#else
        VeinCollideData * collideData = new VeinCollideData(connectPointA, connectPointB,connectPointA, connectPointB, c, 0);
#endif
        TreeNode->m_UserData = (void*)(collideData);

        collideData->m_HostObject = this;

        pair.m_BVNode = TreeNode;
	}
}
//=================================================================================================================================
const GFPhysDBVTree & VeinConnectObject::GetCollideTree()
{
	return m_CollideTree;
}
//=================================================================================================================================
void VeinConnectObject::TestCollisionWithBody(GFPhysRigidBody * convexobj , VeinRigidCollisionListener * listener)
{	
	GFPhysCollideShape * convexshape = (GFPhysCollideShape*)convexobj->GetCollisionShape();

	//Collide with strip tree
    GFPhysVector3 aabbmin, aabbmax;

    convexshape->GetAabb(convexobj->GetWorldTransform(), aabbmin, aabbmax);

	VeinTreeCollideConvexCallBack sbconcallback;

	sbconcallback.m_Convexaabbmin = aabbmin;
	sbconcallback.m_Convexaabbmax = aabbmax;

	sbconcallback.m_Convex = convexobj;
	sbconcallback.m_VeinConnectObj = this;

	sbconcallback.m_collisionlistener = listener;

	m_CollideTree.TraverseTreeAgainstAABB(&sbconcallback,aabbmin,aabbmax);

}
//=================================================================================================================================
void VeinConnectObject::TryHookStrips(const GFPhysVector3 & end0, const GFPhysVector3 & end1, const GFPhysVector3 & localoffset, const GFPhysVector3 & localHookDir, float radius, GFPhysRigidBody * convexobj)
{
	VeinTreeCollideSphereCallBack sbconcallback;

	sbconcallback.m_WorldSegmentPointA = end0;
	sbconcallback.m_WorldSegmentPointB = end1;

	sbconcallback.m_Radiusaabbmin = sbconcallback.m_Radiusaabbmax = sbconcallback.m_WorldSegmentPointA;
	
	sbconcallback.m_Radiusaabbmin.SetMin(sbconcallback.m_WorldSegmentPointB);
	
	sbconcallback.m_Radiusaabbmax.SetMax(sbconcallback.m_WorldSegmentPointB);

	sbconcallback.m_Radiusaabbmin = sbconcallback.m_Radiusaabbmin-GFPhysVector3(radius , radius , radius);
	
	sbconcallback.m_Radiusaabbmax = sbconcallback.m_Radiusaabbmax+GFPhysVector3(radius , radius , radius);

	sbconcallback.m_WorldHookDir = QuatRotate(convexobj->GetOrientation() , localoffset.Normalized());
	sbconcallback.m_HookLocalOffset = localoffset;
	sbconcallback.m_HookLocalDir = localHookDir;
	sbconcallback.m_Convex = convexobj;
	sbconcallback.m_VeinConnectObj = this;
	sbconcallback.m_radius = radius;

	
	sbconcallback.m_currHookCount = 0;
	m_CollideTree.TraverseTreeAgainstAABB(&sbconcallback,sbconcallback.m_Radiusaabbmin,sbconcallback.m_Radiusaabbmax);
	if(sbconcallback.m_currHookCount == 0)
		SetHookedCount();
}
//=======================================================================================
void VeinConnectObject::DestoryCluster(int clusterID)
{
	 if (clusterID < 0 || clusterID >= m_clusters.size())
		 return;

	 if (m_clusters[clusterID].m_IsDetoryed)//already destroyed
		 return;

	 VeinConnectCluster & cluster = m_clusters[clusterID];

	 VeinConnectPair & MajorPair  = cluster.m_pair[0];
	
	 VeinConnectPair & AttachPair = cluster.m_pair[1];

	 MisMedicOrgan_Ordinary * organs[2];
		
	 organs[0] = dynamic_cast<MisMedicOrgan_Ordinary*>(m_OwnerTrain->GetOrgan(cluster.m_ObjAID));

	 organs[1] = dynamic_cast<MisMedicOrgan_Ordinary*>(m_OwnerTrain->GetOrgan(cluster.m_ObjBID));

	 int faceid[4];
	 faceid[0] = MajorPair.m_MMo_faceAID;
	 faceid[1] = MajorPair.m_MMo_faceBID;
	 faceid[2] = AttachPair.m_MMo_faceAID;
	 faceid[3] = AttachPair.m_MMo_faceBID;

	 for (int i = 0; i < 4; i++)
	 {
		if (organs[i % 2] != 0)
		{
			MisMedicOrgan_Ordinary * organ = organs[i % 2];

			if (faceid[i] > 0 && faceid[i] < (int)organ->m_OriginFaces.size())
			{
				MMO_Face& mmoface = organ->m_OriginFaces[faceid[i]];
				mmoface.RemoveStripConnectInfo(this, clusterID);
			}
		}
	 }
	 m_clusters[clusterID].RemovePhysicsPartInternal(m_CollideTree);

	 m_clusters[clusterID].m_IsDetoryed = true;

	 m_clusters[clusterID].m_Valid = false;

	 SetHookOffInfo(clusterID);


	 GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> tetrasToRemove;
	 tetrasToRemove.push_back(m_clusters[clusterID].m_StripTetra);

	 GFPhysSoftBodyTetrasRemover tetraRemover(PhysicsWrapper::GetSingleTon().m_dynamicsWorld , m_PhysBody);
	 tetraRemover.RemoveTetrahedrons(tetrasToRemove , false);//do not rebuild face
}
//========================================================================================================
GFPhysVector3 VeinConnectObject::CalculateHookForce(GFPhysRigidBody * hookrigid , const GFPhysVector3 & InvHookSuportOffsetdir)
{
	GFPhysVector3 hookpointforce(0,0,0);

    for (int c = 0, nc = (int)m_clusters.size(); c <nc; c++)
	{
		if (m_clusters[c].m_IsDetoryed == false)
		{
			VeinConnectPair & pair = m_clusters[c].m_pair[0];

			if (pair.m_BVNode)
			{
				VeinCollideData * collidedata = (VeinCollideData*)(pair.m_BVNode->m_UserData);

				if (collidedata && collidedata->m_InHookState && collidedata->m_contactRigid == hookrigid)
				{
					GFPhysVector3 temp = pair.GetHookPointForce(InvHookSuportOffsetdir);
					hookpointforce += temp;
				}
			}
		}	
	}
	return hookpointforce;
}
void VeinConnectObject::CreateVeinBlood(VeinConnectPair & pairToBlood ,
										GFPhysVector3 & BloodPoint , 
										float dt , 
										float booldValue)
{
	if(m_CanBlood == false)
	   return;

	if (m_TimeSinceLastblood < 0.5f)
		return;

	//pairToBlood.m_TimeSinceLastBlood += dt;//add blood time

	float wa = (pairToBlood.m_CurrPointPosA - GPVec3ToOgre(BloodPoint)).length();
	
	float wb = (pairToBlood.m_CurrPointPosB - GPVec3ToOgre(BloodPoint)).length();
	
	float w  = wa+wb;

	float weightInPair = 0;

	if(w > FLT_EPSILON)
	   weightInPair = wa / (wa+wb);

	int randnum = rand()%10;
	
	
	{
		float endWeight = 0;

		m_TimeSinceLastblood = 0;
	
		MisMedicOrgan_Ordinary * organA = dynamic_cast<MisMedicOrgan_Ordinary*> (m_OwnerTrain->GetOrgan(pairToBlood.m_OrganAID));
			
		MisMedicOrgan_Ordinary * organB = dynamic_cast<MisMedicOrgan_Ordinary*> (m_OwnerTrain->GetOrgan(pairToBlood.m_OrganBID));

		//test
		Ogre::TexturePtr bleedTex = Ogre::TextureManager::getSingleton().load("vein_bleed_particle.dds", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		if (organA->IsBleedWhenStripBreak())
		{
			Ogre::Vector2 bTexCoord = organA->GetTextureCoord(pairToBlood.m_faceA, pairToBlood.m_weightsA);
			organA->ApplyEffect_Bleeding(bTexCoord, 0, 1.0, bleedTex);
			endWeight = 0;
			weightInPair = 0.25f;
		}

		else if (organB->IsBleedWhenStripBreak())
		{
			Ogre::Vector2 bTexCoord = organB->GetTextureCoord(pairToBlood.m_faceB, pairToBlood.m_weightsB);
			organB->ApplyEffect_Bleeding(bTexCoord, 0, 1.0, bleedTex);
			endWeight = 1;
			weightInPair = 0.75f;
		}

		VeinConnectBloodTextureTrack * vbt = m_BloodTextureEffect->CreateBloodTrackForVeinConnect(this, pairToBlood, endWeight);
		if (vbt)
		{
			vbt->m_AplhaFade = 1.0f;// booldValue;
		}
	}
	if (randnum < 7)//create blood in strip end
	{
		VeinConnectBloodTextureTrack * vbt = m_BloodTextureEffect->CreateBloodTrackForVeinConnect(this, pairToBlood, weightInPair);
		if (vbt)
		{
			vbt->m_AplhaFade = 1.0f;// booldValue;
		}
	}
}
//========================================================================================================
int VeinConnectObject::BurnHookedAndContactedConnectInternal(std::set<int>& ClampOrganIdSet,GFPhysRigidBody * left, GFPhysRigidBody * right, float dt, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnPairs)
{
	std::vector<int> clusterDestroyed;
	
	int count = 0;

    for (int c = 0, nc = (int)m_clusters.size(); c < nc; c++)
	{
		bool needremove = false;

        VeinConnectPair & pair = m_clusters[c].m_pair[0];
        
		if (pair.m_BVNode && pair.m_Valid)
        {
            VeinCollideData * collidedata = (VeinCollideData*)(pair.m_BVNode->m_UserData);

            if (collidedata 
			    && collidedata->m_InHookState
                && (collidedata->m_contactRigid == left || collidedata->m_contactRigid == right))
            {
                GFPhysVector3 contactpoint = collidedata->m_contactinWorld;

                burnpos.push_back(Ogre::Vector3(contactpoint.x(), contactpoint.y(), contactpoint.z()));
               
				m_clusters[c].m_SpanScaleOfHookedPart -= dt * 1.0;

                if (m_clusters[c].m_SpanScaleOfHookedPart < 0.1f)
                {
					m_clusters[c].SetHookAndContactOff();
                    m_clusters[c].RemoveOne();
                    SetHookOffInfo(c);
                    if (!m_clusters[c].m_Valid)
                    {
                        needremove = true;
                    }
                    CreateVeinBlood(pair, contactpoint, dt, 0.3f);
                }
                else
                {
                    burnPairs.push_back(&pair);
                }
             }
             else if (collidedata
                    && collidedata->m_InContact
                    && (collidedata->m_contactRigid == left || collidedata->m_contactRigid == right))
             {
					int A = m_clusters[c].m_pair[0].m_OrganAID;
					int B = m_clusters[c].m_pair[0].m_OrganBID;
					if (ClampOrganIdSet.find(A) == ClampOrganIdSet.end() && ClampOrganIdSet.find(B) == ClampOrganIdSet.end())
					{
						GFPhysVector3 contactpoint = collidedata->m_contactinWorld;

						burnpos.push_back(Ogre::Vector3(contactpoint.x(), contactpoint.y(), contactpoint.z()));

						m_clusters[c].m_SpanScaleOfHookedPart -= dt * 1.0;

						if (m_clusters[c].m_SpanScaleOfHookedPart < 0.1f)
						{
							m_clusters[c].RemoveOne();
							
							if (!m_clusters[c].m_Valid)
							{
								needremove = true;
							}
							CreateVeinBlood(pair, contactpoint, dt, 0.3f);
						}
					}
               }
         }
		 if(needremove)
			clusterDestroyed.push_back(c);//need destroy
	}
	
	//destroy all burned cluster
    for (size_t c = 0, nc = clusterDestroyed.size(); c <nc; c++)
	{
		VeinConnectCluster & clusterDestory = m_clusters[clusterDestroyed[c]];

		if (clusterDestory.m_IsDetoryed == false)
		{
			DestoryCluster(clusterDestroyed[c]);

			MxEvent *  pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_VeinConnectBurned, NULL, &clusterDestory.m_pair[0]);

			CMXEventsDump::Instance()->PushEvent(pEvent);
		}
    }
	return count;
}
//=================================================================================================================================
Ogre::TexturePtr GetTexture(Ogre::MaterialPtr materialPtr , Ogre::String textureunitname)
{
	if(materialPtr.isNull() == false && materialPtr->getNumTechniques() > 0)
	{
		Ogre::Technique * tech = materialPtr->getTechnique(0);

		if(tech->getNumPasses() > 0)
		{
			Ogre::Pass * pass = tech->getPass(0);
			
			for (int  t = 0; t < pass->getNumTextureUnitStates() ; t++)
			{
				Ogre::TextureUnitState * texunit = pass->getTextureUnitState(t);
				if(texunit->getTextureNameAlias() == textureunitname )
				   return texunit->_getTexturePtr();
			}
		}
	}
	return Ogre::TexturePtr();
}
//=================================================================================================================================
void VeinConnectObject::UpdateScene( float dt , Ogre::Camera * camera)
{
	//SetHookedCount();

	UpdateConnectPairState(dt);

	UpdateCollideTree(dt);

	UpdateMesh(camera);

	Ogre::MaterialPtr connectMat = Ogre::MaterialManager::getSingleton().getByName(m_MaterialName);//"Particle/strip");

	if(connectMat.isNull() == false)
	{   //Ogre::TexturePtr texture = GetTexture(connectMat , "BloodMap");
		m_BloodTextureEffect->UpdateVeinConnectBloodTrack(dt);
		m_EffectRender.RendVeinConnectBlood(*m_BloodTextureEffect , dt);
	}

	m_TimeSinceLastblood += dt;
}
//=================================================================================================================================
void VeinConnectObject::InternalSimulateEnd(int currStep , int TotalStep , Real dt)
{

}
//=================================================================================================================================
void VeinConnectObject::RecalculateHookPointWorldPos()
{
	for(int c = 0; c < (int)m_clusters.size(); c++)
	{
		if (m_clusters[c].m_IsDetoryed || m_clusters[c].m_Valid == false)
			continue;

		VeinConnectPair & pair = m_clusters[c].m_pair[0];

		if (pair.m_BVNode && pair.m_Valid)
		{
			VeinCollideData * cdata = (VeinCollideData *)pair.m_BVNode->m_UserData;

			if (cdata->m_contactRigid && (cdata->m_InContact || cdata->m_InHookState))
			{
				GFPhysVector3 worldcontactPoint = cdata->CalculatStickPointInWorld();

				pair.m_WolrdHookPoint = worldcontactPoint;

				//if(pair.m_TubeFaceConnect)
				//{
				//pair.m_TubeFaceConnect->m_inContactStick = true;
				//pair.m_TubeFaceConnect->m_StickPoint = worldcontactPoint;
				//}
				//if (pair.m_type & VeinConnectPair::PCT_FF)
				//{
					//pair.m_FaceFaceConnect.m_inContactStick = true;
					//pair.m_FaceFaceConnect.m_StickPoint = worldcontactPoint;

					//pair.m_FaceFaceConnect[1].m_inContactStick = true;
					//pair.m_FaceFaceConnect[1].m_StickPoint = worldcontactPoint;
				//}
				//if(pair.m_TubeTubeConnect)
				//{
				//pair.m_TubeTubeConnect->m_inContactStick = true;
				//pair.m_TubeTubeConnect->m_StickPoint = worldcontactPoint;
				//}
				//if (pair.m_type & VeinConnectPair::PCT_WFF)
				//{
					//pair.m_FaceFaceWeakConnect.m_InHookState = true;

					//GFPhysSoftBodyFace * FaceA = pair.m_FaceFaceWeakConnect.m_Face[0];

					//GFPhysSoftBodyFace * FaceB = pair.m_FaceFaceWeakConnect.m_Face[2];

					GFPhysVector3 FaceANormal = pair.m_faceA->m_FaceNormal;//static object should update normal manually once set up

					GFPhysVector3 FaceBNormal = pair.m_faceB->m_FaceNormal;

					GFPhysVector3 NoneOffsetPointA = pair.m_faceA->m_Nodes[0]->m_CurrPosition*pair.m_weightsA[0]
						                           + pair.m_faceA->m_Nodes[1]->m_CurrPosition*pair.m_weightsA[1]
						                           + pair.m_faceA->m_Nodes[2]->m_CurrPosition*pair.m_weightsA[2];
					GFPhysVector3 OffsetPointA = NoneOffsetPointA + FaceANormal*pair.m_SuspendDistInFaceA;


					GFPhysVector3 NoneOffsetPointB = pair.m_faceB->m_Nodes[0]->m_CurrPosition*pair.m_weightsB[0]
						                           + pair.m_faceB->m_Nodes[1]->m_CurrPosition*pair.m_weightsB[1]
						                           + pair.m_faceB->m_Nodes[2]->m_CurrPosition*pair.m_weightsB[2];
					GFPhysVector3 OffsetPointB = NoneOffsetPointB + FaceBNormal*pair.m_SuspendDistInFaceB;


					GFPhysVector3 HookOffset = worldcontactPoint - OffsetPointA;

					m_clusters[c].m_StickPoint = NoneOffsetPointA + HookOffset;//*scale;
					
					if (m_clusters[c].m_StickPoint.Length2() > 1000.0f)
					{
						int k = 0;
					}
				//}

			}
		}
	}
}
//==================================================================================================================================
void VeinConnectObject::UpdateConnectPairState(float dt)
{
    for (size_t c = 0, nc = m_clusters.size(); c < nc; c++)
	{

        for (size_t p = 0; p < 2; p++)
        {
            VeinConnectPair & pair = m_clusters[c].m_pair[p];
            
            if(pair.m_Valid == false)
				continue;

			GFPhysVector3 PointA , PointB;

            if((pair.m_ObjAType == DOT_VOLMESH || pair.m_ObjAType == DOT_MEMBRANE ) &&pair.m_faceA)
			{
				GFPhysSoftBodyFace * faceA = pair.m_faceA;
				PointA = faceA->m_Nodes[0]->m_CurrPosition*pair.m_weightsA[0]
						+faceA->m_Nodes[1]->m_CurrPosition*pair.m_weightsA[1]
						+faceA->m_Nodes[2]->m_CurrPosition*pair.m_weightsA[2];

				GFPhysVector3 facenormal = (faceA->m_Nodes[1]->m_CurrPosition-faceA->m_Nodes[0]->m_CurrPosition).Cross(faceA->m_Nodes[2]->m_CurrPosition-faceA->m_Nodes[0]->m_CurrPosition);
				facenormal.Normalize();
				PointA += facenormal*pair.m_SuspendDistInFaceA;//0.1f;
			}
			
            if((pair.m_ObjBType == DOT_VOLMESH || pair.m_ObjBType == DOT_MEMBRANE) && pair.m_faceB)
			{
				GFPhysSoftBodyFace * faceB = pair.m_faceB;
				PointB = faceB->m_Nodes[0]->m_CurrPosition*pair.m_weightsB[0]
						+faceB->m_Nodes[1]->m_CurrPosition*pair.m_weightsB[1]
						+faceB->m_Nodes[2]->m_CurrPosition*pair.m_weightsB[2];
				
				
				GFPhysVector3 facenormal = (faceB->m_Nodes[1]->m_CurrPosition-faceB->m_Nodes[0]->m_CurrPosition).Cross(faceB->m_Nodes[2]->m_CurrPosition-faceB->m_Nodes[0]->m_CurrPosition);
				facenormal.Normalize();
				PointB += facenormal*pair.m_SuspendDistInFaceB;
			}
			
			pair.m_CurrPointPosA = GPVec3ToOgre(PointA);

			pair.m_CurrPointPosB = GPVec3ToOgre(PointB);

			if(pair.m_BVNode)
			{
				
			}
		}
	}
	//m_CollideTree.RebuildTree();
}
//====================================================================================================================
void VeinConnectObject::NotifyRigidBodyRemovedFromWorld(GFPhysRigidBody * rb)
{
    for (size_t c = 0, nc = m_clusters.size(); c < nc; c++)
	{
         VeinConnectPair & pair = m_clusters[c].m_pair[0];
            
         if (pair.m_BVNode && pair.m_Valid)
		 {
			 VeinCollideData * cdata = (VeinCollideData *)pair.m_BVNode->m_UserData;
			
			if(cdata && cdata->m_contactRigid == rb)
			{
			   m_clusters[c].SetHookAndContactOff();
			   SetHookOffInfo(c);
			}
		}
	}
}
//==================================================================================================================================
void VeinConnectObject::UpdateCollideTree(float dt)
{
	m_RigidHookPairCounts.clear();

	m_Disconnected = false;
	int countValidConnect = 0;
	for (size_t c = 0, nc = m_clusters.size(); c < nc; c++)
	{
		if (m_clusters[c].m_Valid && (m_clusters[c].m_IsDetoryed == false))
		{
			countValidConnect++;
		}
		else
		{
			continue;
		}

		VeinConnectPair & pair = m_clusters[c].m_pair[0];

		if (pair.m_BVNode && pair.m_Valid)
		{
			const VeinConnectPair & Adjpair = m_clusters[c].m_pair[1];

#if SEGMENT_COLLIDE
			GFPhysVector3 PairCenterPosA = OgreToGPVec3(pair.m_CurrPointPosA + Adjpair.m_CurrPointPosA)*0.5f;

			GFPhysVector3 PairCenterPosB = OgreToGPVec3(pair.m_CurrPointPosB + Adjpair.m_CurrPointPosB)*0.5f;

			//Rebuild  aabb box
			GFPhysVector3 AABBMin = PairCenterPosA;//pair.m_CurrPointPosA;

			GFPhysVector3 AABBMax = PairCenterPosA;//pair.m_CurrPointPosA;

			AABBMin.SetMin(PairCenterPosB);//pair.m_CurrPointPosB);

			AABBMax.SetMax(PairCenterPosB);//pair.m_CurrPointPosB);
#else                
			//Rebuild  aabb box
			GFPhysVector3 AABBMin = OgreToGPVec3(pair.m_CurrPointPosA);

			GFPhysVector3 AABBMax = OgreToGPVec3(pair.m_CurrPointPosA);

			AABBMin.SetMin(OgreToGPVec3(pair.m_CurrPointPosB));
			AABBMin.SetMin(OgreToGPVec3(Adjpair.m_CurrPointPosA));
			AABBMin.SetMin(OgreToGPVec3(Adjpair.m_CurrPointPosB));


			AABBMax.SetMax(OgreToGPVec3(pair.m_CurrPointPosB));
			AABBMax.SetMax(OgreToGPVec3(Adjpair.m_CurrPointPosA));
			AABBMax.SetMax(OgreToGPVec3(Adjpair.m_CurrPointPosB));
#endif
			GFPhysDBVNode * DBVNode = pair.m_BVNode;

			VeinCollideData * CollideData = (VeinCollideData *)DBVNode->m_UserData;
#if SEGMENT_COLLIDE
			CollideData->m_PointA = PairCenterPosA;//pair.m_CurrPointPosA;

			CollideData->m_PointB = PairCenterPosB;//pair.m_CurrPointPosB;
#else

			CollideData->m_PointA0 = OgreToGPVec3(pair.m_CurrPointPosA);
			CollideData->m_PointB0 = OgreToGPVec3(pair.m_CurrPointPosB);

			CollideData->m_PointA1 = OgreToGPVec3(Adjpair.m_CurrPointPosA);
			CollideData->m_PointB1 = OgreToGPVec3(Adjpair.m_CurrPointPosB);
#endif
			GFPhysVector3 worldcontactPoint;

			//VeinCollideData * cdata = (VeinCollideData *)pair.m_BVNode->m_UserData;

			CollideData->Update(dt);//update contact point in world space

			//接触后增大包围盒
			if (CollideData->m_InContact || CollideData->m_InHookState)
			{
				AABBMin.SetMin(CollideData->m_contactinWorld);
				AABBMax.SetMax(CollideData->m_contactinWorld);
			}

			/*painting.PushBackPoint(CustomPoint(&AABBMin, Ogre::ColourValue::White, 0.1));
			painting.PushBackPoint(CustomPoint(&AABBMax, Ogre::ColourValue::Blue, 0.1));*/

			DBVNode->SetBound(AABBMin, AABBMax);

			if (CollideData->m_InContact || CollideData->m_InHookState)
			{
				worldcontactPoint = CollideData->m_contactinWorld;

				if (CollideData->m_InContact)
				{
#if SEGMENT_COLLIDE
					float newlen = (worldcontactPoint - CollideData->m_PointA).Length() + (worldcontactPoint - CollideData->m_PointB).Length();

					float dlen = (CollideData->m_PointA - CollideData->m_PointB).Length();
					if (newlen - dlen > 0.1f)// && !cdata->m_WillBeDissected)
#else
					float newlen0, dlen0, newlen1, dlen1;

					newlen0 = (worldcontactPoint - CollideData->m_PointA0).Length() + (worldcontactPoint - CollideData->m_PointB0).Length();
					dlen0 = (CollideData->m_PointA0 - CollideData->m_PointB0).Length();

					newlen1 = (worldcontactPoint - CollideData->m_PointA1).Length() + (worldcontactPoint - CollideData->m_PointB1).Length();
					dlen1 = (CollideData->m_PointA1 - CollideData->m_PointB1).Length();

					if ((newlen0 - dlen0 > 0.2f || newlen1 - dlen1 > 0.2f))// && !cdata->m_WillBeDissected)
#endif
						m_clusters[c].SetHookAndContactOff();//cdata->SetContactOff();
				}
				else if (CollideData->m_InHookState)
				{
#if SEGMENT_COLLIDE
					float newlen = (worldcontactPoint - CollideData->m_PointA).Length() + (worldcontactPoint - CollideData->m_PointB).Length();

					float RestLen = (CollideData->m_PointA - CollideData->m_PointB).Length();
					if (newlen - RestLen > 2.0f)
#else
					float newlen0, RestLen0, newlen1, RestLen1;

					newlen0 = (worldcontactPoint - CollideData->m_PointA0).Length2() + (worldcontactPoint - CollideData->m_PointB0).Length2();

					RestLen0 = (CollideData->m_PointA0 - CollideData->m_PointB0).Length2();

					newlen1 = (worldcontactPoint - CollideData->m_PointA1).Length2() + (worldcontactPoint - CollideData->m_PointB1).Length2();

					RestLen1 = (CollideData->m_PointA1 - CollideData->m_PointB1).Length2();

					if (newlen0 - RestLen0 > 4.0f || newlen1 - RestLen1 > 4.0f)
#endif
					{
						m_clusters[c].SetHookAndContactOff();

						SetHookOffInfo(c);

						m_clusters[c].RemoveOne();

						//create blood
						GFPhysVector3 contactpoint = CollideData->m_contactinWorld;

						CreateVeinBlood(pair, contactpoint, dt, 2);

						if (!m_clusters[c].m_Valid)
						{
							DestoryCluster(c);

							m_Disconnected = true;

							MxEvent *  pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_VeinConnectBreak, NULL, &pair);
							CMXEventsDump::Instance()->PushEvent(pEvent);
						}
					}
					else
					{
						if (CollideData->m_LocalHookDir.Length2() > GP_EPSILON)//if we have valid hook dir
						{
							GFPhysVector3 dir0 = (CollideData->m_PointB0 - CollideData->m_PointA0).Normalized();
							GFPhysVector3 Expand0 = (CollideData->m_contactinWorld - CollideData->m_PointA0) - dir0 * (CollideData->m_contactinWorld - CollideData->m_PointA0).Dot(dir0);

							GFPhysVector3 dir1 = (CollideData->m_PointB1 - CollideData->m_PointA1).Normalized();
							GFPhysVector3 Expand1 = (CollideData->m_contactinWorld - CollideData->m_PointA1) - dir1 * (CollideData->m_contactinWorld - CollideData->m_PointA1).Dot(dir1);

							if (Expand0.Normalized().Dot(CollideData->m_WorldHookDir) < -0.3f && Expand0.Length() > 0.2f && Expand1.Normalized().Dot(CollideData->m_WorldHookDir) < -0.3f && Expand1.Length() > 0.2f)
							{
								m_clusters[c].SetHookAndContactOff();
							}
						}
					}
			     }
	        }

			if (CollideData->m_InHookState)
			{
				std::map<GFPhysCollideObject*, int>::iterator itor = m_RigidHookPairCounts.find(CollideData->m_contactRigid);
				if (itor == m_RigidHookPairCounts.end())
				{
					m_RigidHookPairCounts.insert(std::make_pair(CollideData->m_contactRigid, 1));
				}
				else
				{
					itor->second++;
				}
			}
		}
	}

	m_connectCount = countValidConnect;
	m_CollideTree.RebuildTree();
}
//========================================================================================================================
void VeinConnectObject::UpdateMesh(Ogre::Camera * pCamera)
{
	//static std::vector<VeinConnStrip> stripVecs;
	//stripVecs.clear();
	//stripVecs.reserve(1000);
	VeinConnStrip stripsArray[MAXVEINCONNECTNUM];

	int stripsNum = 0;

	for (size_t v = 0, nv = m_clusters.size(); v < nv; v++)
	{
		VeinConnectCluster & cluster = m_clusters[v];

		VeinConnectPair & pair1 = cluster.m_pair[0];

		VeinConnectPair & pair2 = cluster.m_pair[1];

		if (pair1.m_Valid == false || pair2.m_Valid == false)
			continue;

		if (m_IsNewMode)
		{
			VeinConnStrip RendStrip;

			//position
			RendStrip.m_adhereA[0] = pair1.m_CurrPointPosA;//Ogre::Vector3(pair1.m_CurrPointPosA.x() , pair1.m_CurrPointPosA.y() , pair1.m_CurrPointPosA.z());

			RendStrip.m_adhereB[0] = pair1.m_CurrPointPosB;//Ogre::Vector3(pair1.m_CurrPointPosB.x() , pair1.m_CurrPointPosB.y() , pair1.m_CurrPointPosB.z());

			RendStrip.m_adhereA[1] = pair2.m_CurrPointPosA;//Ogre::Vector3(pair2.m_CurrPointPosA.x() , pair2.m_CurrPointPosA.y() , pair2.m_CurrPointPosA.z()); 

			RendStrip.m_adhereB[1] = pair2.m_CurrPointPosB;//Ogre::Vector3(pair2.m_CurrPointPosB.x() , pair2.m_CurrPointPosB.y() , pair2.m_CurrPointPosB.z()); 

			//texture coordinate
			Ogre::Vector2 * texturecoord = pair1.texcoord;

			RendStrip.m_adherAtext[0] = texturecoord[0];

			RendStrip.m_adherBtext[0] = texturecoord[2];

			RendStrip.m_adherAtext[1] = texturecoord[1];

			RendStrip.m_adherBtext[1] = texturecoord[3];

			RendStrip.m_SpanScale = cluster.m_SpanScale;
			//color
			RendStrip.m_stripValue = pair1.m_PairColor; //Ogre::ColourValue(0, 1, 1, 1);//cluster.m_Color;

			bool StickInConstruct = false;

			GFPhysVector3 worldcontactPoint(0, 0, 0);

			GFPhysVector3 worldHookVerticleDir(0, 0, 0);

			if (cluster.m_mode != VeinConnectCluster::DISCONNECT || !cluster.m_isHooked)
			{
				RendStrip.m_InHookState = false;
				RendStrip.m_HookPosition = Ogre::Vector3(0, 0, 0);
				RendStrip.m_HookVerticleDir = Ogre::Vector3(0, 0, 0);
				RendStrip.m_SpanScale = cluster.m_SpanScale;
				RendStrip.m_stripValue = cluster.m_Color * pair1.m_PairColor;
				stripsArray[stripsNum++] = RendStrip;
			}


			if (pair1.m_BVNode)
			{
				GFPhysDBVNode * DBVNode = pair1.m_BVNode;

				VeinCollideData * CollideData = (VeinCollideData *)DBVNode->m_UserData;

				if (CollideData)
				{
					StickInConstruct = (CollideData->m_InContact || CollideData->m_InHookState);
					if (StickInConstruct)
					{
						worldcontactPoint = CollideData->m_contactinWorld;

						//painting.PushBackPoint(CustomPoint(&worldcontactPoint, Ogre::ColourValue::Green, 0.1f));

						worldHookVerticleDir = CollideData->m_WorldHookOffset;
						RendStrip.m_InHookState = StickInConstruct;
						//for the hooked part

						/*
						此处是一处临时解决方案，以后要彻底改过来。
						原因:当调用过sethookoff()(比如用抓钳抓再释放)后，紧接着进InContact流程，渲染所需要的变量并没有被赋值，导致发生contact的筋膜渲染是空白，看上去像没有碰到一样。
						可能的改法1，统一contact和hook流程；2，单独为contact流程设置渲染变量。
						*/
						RendStrip.m_SpanScale = (cluster.m_SpanScaleOfHookedPart < 0.001f) ? 0.4f:cluster.m_SpanScaleOfHookedPart;
						RendStrip.m_stripValue = cluster.m_ColorOfHookedPart;

						RendStrip.m_HookPosition = Ogre::Vector3(worldcontactPoint.x(), worldcontactPoint.y(), worldcontactPoint.z());
						RendStrip.m_HookVerticleDir = Ogre::Vector3(worldHookVerticleDir.x(), worldHookVerticleDir.y(), worldHookVerticleDir.z());
						RendStrip.m_HookVerticleDir.normalise();
						stripsArray[stripsNum++] = RendStrip;
					}
				}
			}
		}
		else
		{
			VeinConnStrip RendStrip;

			//position
			RendStrip.m_adhereA[0] = pair1.m_CurrPointPosA;//Ogre::Vector3(pair1.m_CurrPointPosA.x() , pair1.m_CurrPointPosA.y() , pair1.m_CurrPointPosA.z());

			RendStrip.m_adhereB[0] = pair1.m_CurrPointPosB;//Ogre::Vector3(pair1.m_CurrPointPosB.x() , pair1.m_CurrPointPosB.y() , pair1.m_CurrPointPosB.z());

			RendStrip.m_adhereA[1] = pair2.m_CurrPointPosA;//Ogre::Vector3(pair2.m_CurrPointPosA.x() , pair2.m_CurrPointPosA.y() , pair2.m_CurrPointPosA.z()); 

			RendStrip.m_adhereB[1] = pair2.m_CurrPointPosB;//Ogre::Vector3(pair2.m_CurrPointPosB.x() , pair2.m_CurrPointPosB.y() , pair2.m_CurrPointPosB.z()); 

			//texture coordinate
			Ogre::Vector2 * texturecoord = pair1.texcoord;

			RendStrip.m_adherAtext[0] = texturecoord[0];

			RendStrip.m_adherBtext[0] = texturecoord[2];

			RendStrip.m_adherAtext[1] = texturecoord[1];

			RendStrip.m_adherBtext[1] = texturecoord[3];

			RendStrip.m_SpanScale = pair1.m_SpanScale;
			//color
			RendStrip.m_stripValue = pair1.m_PairColor;

			bool StickInConstruct = false;

			GFPhysVector3 worldcontactPoint(0, 0, 0);

			GFPhysVector3 worldHookVerticleDir(0, 0, 0);

			if (pair1.m_BVNode)
			{
				GFPhysDBVNode * DBVNode = pair1.m_BVNode;

				VeinCollideData * CollideData = (VeinCollideData *)DBVNode->m_UserData;

				if (CollideData)
				{
					StickInConstruct = (CollideData->m_InContact || CollideData->m_InHookState);
					worldcontactPoint = CollideData->m_contactinWorld;
					worldHookVerticleDir = CollideData->m_WorldHookOffset;
				}
			}
			/*if (worldcontactPoint.Length2() > 10.0f)
			{
			Ogre::LogManager::getSingleton().logMessage(Ogre::String("worldcontactPoint length is  ") + Ogre::StringConverter::toString(worldcontactPoint.Length()));
			}
			Ogre::LogManager::getSingleton().logMessage(Ogre::String("worldcontactPoint is  ")
			+ Ogre::StringConverter::toString(worldcontactPoint.GetX())
			+ Ogre::String(",")
			+Ogre::StringConverter::toString(worldcontactPoint.GetY())
			+ Ogre::String(",")
			+ Ogre::StringConverter::toString(worldcontactPoint.GetZ())
			+ Ogre::String("."));*/

			RendStrip.m_InHookState = StickInConstruct;
			RendStrip.m_HookPosition = Ogre::Vector3(worldcontactPoint.x(), worldcontactPoint.y(), worldcontactPoint.z());
			RendStrip.m_HookVerticleDir = Ogre::Vector3(worldHookVerticleDir.x(), worldHookVerticleDir.y(), worldHookVerticleDir.z());
			RendStrip.m_HookVerticleDir.normalise();
			//stripVecs.push_back(RendStrip);
			stripsArray[stripsNum++] = RendStrip;
		}

	}
	m_RenderObject->BuildStrips(pCamera, stripsArray, stripsNum);

	//painting.Update(0);
}
//============================================================================================================================
void VeinConnectObject::readVeinConnectTexMapFile(const char * texmap)
{
	bool isvalidfile = true;

	Ogre::DataStreamPtr datastream;

	try
	{
		datastream = Ogre::ResourceGroupManager::getSingleton().openResource(texmap);
	}
	catch (...)
	{
		isvalidfile = false;
	}


	if (datastream.isNull() ||  isvalidfile == false)
	{
		isvalidfile = false;
	}
	else
	{
		int srcconnecount;

		datastream->read(&srcconnecount , sizeof(int));

		isvalidfile = (m_clusters.size() == srcconnecount);

		if(isvalidfile)
		{
            for (size_t c = 0, nc = m_clusters.size(); c <nc; c++)
			{
				VeinConnectCluster & cluster = m_clusters[c];

				int stripnum;

				datastream->read(&stripnum , sizeof(int));


                stripnum = 1;

                for (int s = 0; s < stripnum; s++)
                {
                    Ogre::Vector2 * texcoord = cluster.m_pair[s].texcoord;

					//read 4 texture coordinate
					for (int t = 0 ; t < 4 ; t++)
					{
						float tx , ty;

						datastream->read(&tx , sizeof(float));
						datastream->read(&ty , sizeof(float));

						texcoord[t].x = tx;
						texcoord[t].y = ty;
					}
				}
			}
		}
	}

	if(datastream.isNull() == false)
	   datastream->close();
}
//============================================================================================================================
void VeinConnectObject::CreateConnectFromFile(Ogre::SceneManager * scenemgr , Ogre::String filename, DynObjMap & dynmap)
{
	Ogre::DataStreamPtr datastream;

	datastream = Ogre::ResourceGroupManager::getSingleton().openResource(filename);

	int paircount = 0;

	datastream->read(&paircount , sizeof(paircount));

	VeinConnectCluster reference_cluster(m_ConnStayNum , m_ConnReduceNum);

	m_clusters.resize(paircount , reference_cluster);

	int autoincpathid = 0;

	int RealPairIndex = 0;

	for (int i = 0 ; i < paircount ; i++)
	{
		int objAid;

		int objBid;

		int pointcount;

		datastream->read(&objAid , sizeof(objAid) );

		datastream->read(&objBid , sizeof(objBid));

        if (objAid < 0 || objAid > EDOT_ORGAN_LIMIT || objBid > EDOT_ORGAN_LIMIT || objBid < 0)
        {
            continue;
        }

		datastream->read(&pointcount , sizeof(pointcount));

		MisMedicOrganInterface *dynObjA , *dynObjB;

		dynObjA = dynmap.find(objAid)->second;

		dynObjB = dynmap.find(objBid)->second;

		int objTypeA = dynObjA->GetCreateInfo().m_objTopologyType;

		int objTypeB = dynObjB->GetCreateInfo().m_objTopologyType;

		bool hasError = false;

		//read point 
		for (int p = 0 ; p < pointcount ; p++)
		{
			VeinConnectPair pair;

			float suspx , suspy , suspz, susdist;

			int	  Afaceid , Bfaceid;

			float AWeighs[3];

			float BWeighs[3];

			float OriginPosA_X;
			float OriginPosA_Y;
			float OriginPosA_Z;
			datastream->read(&Afaceid ,	sizeof(Afaceid));
			datastream->read(&(AWeighs[0]) , sizeof(AWeighs));
			datastream->read(&suspx , sizeof(float));
			datastream->read(&suspy , sizeof(float));
			datastream->read(&suspz , sizeof(float));
			datastream->read(&susdist , sizeof(float));
			datastream->read(&OriginPosA_X , sizeof(float));
			datastream->read(&OriginPosA_Y , sizeof(float));
			datastream->read(&OriginPosA_Z , sizeof(float));
			//pair.m_tubOffsetA = Ogre::Vector3(suspx , suspy , suspz);
			//pair.m_tubOffsetA.normalise();
			pair.m_SuspendDistInFaceA = susdist;


			float OriginPosB_X;
			float OriginPosB_Y;
			float OriginPosB_Z;
			datastream->read(&Bfaceid ,	sizeof(Bfaceid));
			datastream->read(&(BWeighs[0]) , sizeof(BWeighs));
			datastream->read(&suspx , sizeof(float));
			datastream->read(&suspy , sizeof(float));
			datastream->read(&suspz , sizeof(float));
			datastream->read(&susdist , sizeof(float));

			datastream->read(&OriginPosB_X , sizeof(float));
			datastream->read(&OriginPosB_Y , sizeof(float));
			datastream->read(&OriginPosB_Z , sizeof(float));

			//pair.m_tubOffsetB = Ogre::Vector3(suspx , suspy , suspz);
			//pair.m_tubOffsetB.normalise();
			pair.m_SuspendDistInFaceB = susdist;

			//add constraint
			pair.m_ObjAType = objTypeA;
			pair.m_ObjBType = objTypeB;

			//
			MisMedicOrgan_Ordinary * dynMeshA = 0;
			MisMedicOrgan_Ordinary * dynMeshB = 0;
			
			MisMedicOrgan_Tube * dynTubeA = 0;
			MisMedicOrgan_Tube * dynTubeB = 0;

            pair.m_weightsA[0] = AWeighs[0];
            pair.m_weightsA[1] = AWeighs[1];
            pair.m_weightsA[2] = AWeighs[2];

            pair.m_weightsB[0] = BWeighs[0];
            pair.m_weightsB[1] = BWeighs[1];
            pair.m_weightsB[2] = BWeighs[2];
			if(objTypeA == DOT_VOLMESH  || objTypeA == DOT_MEMBRANE)
			{
				dynMeshA = (MisMedicOrgan_Ordinary*)dynObjA;
				if(Afaceid < 0 || Afaceid >= (int)dynMeshA->m_OriginFaces.size())
				{
					hasError = true;
					break;
				}
                MMO_Face& face = dynMeshA->m_OriginFaces[Afaceid];
                pair.m_MMo_faceAID = Afaceid;
                pair.m_faceA = face.m_physface;
                pair.m_UndeformPointPosA =
                    pair.m_faceA->m_Nodes[0]->m_UnDeformedPos * pair.m_weightsA[0] +
                    pair.m_faceA->m_Nodes[1]->m_UnDeformedPos * pair.m_weightsA[1] +
                    pair.m_faceA->m_Nodes[2]->m_UnDeformedPos * pair.m_weightsA[2];

                //////////////////////////////////////////////////////////////////////////
                MMO_Face::VeinconnPosLocal local;
                if (p==0)
                {
                    local = MMO_Face::VeinconnPosLocal::A;
                }
                else
                {
                    local = MMO_Face::VeinconnPosLocal::C;
                }
                MMO_Face::VeinInfo vinfo = { this, i, local,true};
                face.m_VeinInfoVector.push_back(vinfo);
			}
			

			if(objTypeB == DOT_VOLMESH || objTypeB == DOT_MEMBRANE)
			{
				dynMeshB = (MisMedicOrgan_Ordinary*)dynObjB;
				if(Bfaceid < 0 || Bfaceid >= (int)dynMeshB->m_OriginFaces.size())
				{
					hasError = true;
					break;
				}
                MMO_Face& face = dynMeshB->m_OriginFaces[Bfaceid];
                pair.m_MMo_faceBID = Bfaceid;
                pair.m_faceB = face.m_physface;
                pair.m_UndeformPointPosB =
                    pair.m_faceB->m_Nodes[0]->m_UnDeformedPos * pair.m_weightsB[0] +
                    pair.m_faceB->m_Nodes[1]->m_UnDeformedPos * pair.m_weightsB[1] +
                    pair.m_faceB->m_Nodes[2]->m_UnDeformedPos * pair.m_weightsB[2];
                //////////////////////////////////////////////////////////////////////////
                MMO_Face::VeinconnPosLocal local;
                if (p == 0)
                {
                    local = MMO_Face::VeinconnPosLocal::B;
                }
                else
                {
                    local = MMO_Face::VeinconnPosLocal::D;
                }
                MMO_Face::VeinInfo vinfo = { this, i, local, true };
                face.m_VeinInfoVector.push_back(vinfo);
			}

            m_clusters[RealPairIndex].m_pair[p] = pair;

		}
		if(hasError == false)
		{
			//add physics constraints only for major pair
            VeinConnectPair & DstPair = m_clusters[RealPairIndex].m_pair[0];

            VeinConnectPair & DstPairNext = m_clusters[RealPairIndex].m_pair[1];

            DstPair.m_OwnObject = this;
            DstPairNext.m_OwnObject = this;
			
			MisMedicOrgan_Ordinary * dynMeshA = 0;
			MisMedicOrgan_Ordinary * dynMeshB = 0;

			dynMeshA = dynamic_cast<MisMedicOrgan_Ordinary*>(dynObjA);
			
			dynMeshB = dynamic_cast<MisMedicOrgan_Ordinary*>(dynObjB);

			if (dynMeshA && dynMeshB)
			{
				DstPair.m_OrganAID = dynObjA->m_OrganID;
				DstPair.m_OrganBID = dynObjB->m_OrganID;
			}

			m_connectCount++;

			m_clusters[RealPairIndex].m_ObjAID = objAid;
			m_clusters[RealPairIndex].m_ObjBID = objBid;
			RealPairIndex++;
		}
		else
		{

            memset(m_clusters[RealPairIndex].m_pair, 0, 2 * sizeof(VeinConnectPair));
		}
	}
	m_clusters.resize(RealPairIndex);
	datastream->close();

	m_InitConnectCount = m_connectCount;

	//create collide tree
	CreateCollideTree();

	static int connectid = 0;
	connectid++;

	m_RenderObject = dynamic_cast<VeinConnectStripsObject*>(scenemgr->createMovableObject("VeinConnStripObj"));
	m_RenderObject->setMaterialName(m_MaterialName);

	m_SceneNode = scenemgr->getRootSceneNode()->createChildSceneNode(filename+Ogre::StringConverter::toString(connectid));

	if(m_RenderObject)
	   m_SceneNode->attachObject(m_RenderObject);

}
//============================================================================================================================
void VeinConnectObject::BuildConnectConstraint(DynObjMap & dynmap)
{
	GFPhysParallelPositionConstraintSolver * pbdsolver = PhysicsWrapper::GetSingleTon().m_dynamicsWorld->GetParallelPositionContraintSolver();

    for (size_t c = 0, nc = m_clusters.size(); c <nc; c++)
	{
		 VeinConnectCluster & cluster = m_clusters[c];

		 GFPhysSoftBodyTetrahedron * tetra = cluster.m_StripTetra;

         VeinConnectPair & DstPair0 = cluster.m_pair[0];
       
		 VeinConnectPair & DstPair1 = cluster.m_pair[1];

		 GFPhysVector3 posA = DstPair0.GetPosOnA();

		 GFPhysVector3 posB = DstPair0.GetPosOnB();


		 DstPair0.m_StripNodeA->m_UnDeformedPos = DstPair0.m_StripNodeA->m_CurrPosition = DstPair0.m_StripNodeA->m_LastPosition = DstPair0.m_StripNodeA->m_PosLastStep = posA;
		 DstPair0.m_StripNodeB->m_UnDeformedPos = DstPair0.m_StripNodeB->m_CurrPosition = DstPair0.m_StripNodeB->m_LastPosition = DstPair0.m_StripNodeB->m_PosLastStep = posB;
		 DstPair0.m_StripNodeA->m_Velocity = GFPhysVector3(0, 0, 0);
		 DstPair0.m_StripNodeB->m_Velocity = GFPhysVector3(0, 0, 0);

		 DstPair1.m_StripNodeA->m_UnDeformedPos = DstPair1.m_StripNodeA->m_CurrPosition = DstPair1.m_StripNodeA->m_LastPosition = DstPair1.m_StripNodeA->m_PosLastStep = DstPair1.GetPosOnA();
		 DstPair1.m_StripNodeB->m_UnDeformedPos = DstPair1.m_StripNodeB->m_CurrPosition = DstPair1.m_StripNodeB->m_LastPosition = DstPair1.m_StripNodeB->m_PosLastStep = DstPair1.GetPosOnB();
		 DstPair1.m_StripNodeA->m_Velocity = GFPhysVector3(0, 0, 0);
		 DstPair1.m_StripNodeB->m_Velocity = GFPhysVector3(0, 0, 0);
		 for (int e = 0; e < 6; e++)
		 {
			 GFPhysSoftBodyEdge * edge  = tetra->m_TetraEdge[e];

			 GFPhysSoftBodyNode * Node0 = edge->m_Nodes[0];

			 GFPhysSoftBodyNode * Node1 = edge->m_Nodes[1];

			 Real NewLength = (Node0->m_UnDeformedPos - Node1->m_UnDeformedPos).Length();

			 edge->m_RestLength = NewLength;

			 pbdsolver->EdgeRestValueChanged(edge, NewLength);
		 }

		 tetra->ReCalRestSignedVolume();

		 pbdsolver->TetraRestValueChanged(tetra, tetra->m_RestSignedVolume);
	}
}
//==============================================================================================================================================
void VeinConnectObject::SetHookedCount()
{
	if(!m_isHookLimitSet)
		return;
	m_HookedCount = 0;
	m_CanBeHooked = true;
	m_HookedLimit = rand() % 4 + 1;
}
//==============================================================================================================================================
void VeinConnectObject::AddHookedCount()
{
	if(!m_isHookLimitSet)
		return;
	m_HookedCount++;
	if(m_HookedCount >= m_HookedLimit)
		m_CanBeHooked = false;
}
//==============================================================================================================================================
void VeinConnectObject::ClearHookedCount()
{
	m_HookedCount = 0;
}
//==============================================================================================================================================
//void VeinConnectObject::SetClusterHookedOn(int clusterid)
//{
	//m_clusters[clusterid].SetHookOn();
//}
//==============================================================================================================================================
//void VeinConnectObject::SetClusterHookedOff(int clusterid)
//{
	//m_clusters[clusterid].SetHookOff();
//}
//==============================================================================================================================================
bool VeinConnectObject::SetHookInfo(int clusterid,int pairid)
{
	if(m_IsNewMode)
		return true;
	int valid_index = -1;
	for(int i = 0 ; i < m_HookedLimitCount ; i++)
	{
		if(!m_hook_info[i].m_hooked)
		{
			valid_index = i;
			//m_hook_info[i].m_cluster_id = clusterid;
			//m_hook_info[i].m_pair_id = pairid;
			//m_hook_info[i].m_hooked = true;
			//return true;
		}
		else 
		{
			if(m_hook_info[i].m_cluster_id == clusterid)
				return true;
		}
	}
	if(valid_index != -1)
	{
		m_hook_info[valid_index].m_cluster_id = clusterid;
		//m_hook_info[valid_index].m_pair_id = pairid;
		m_hook_info[valid_index].m_hooked = true;
		return true;
	}
	else
		return false;
}
//==============================================================================================================================================
void VeinConnectObject::SetHookOffInfo(int clusterid)
{
	for(int i = 0 ; i < m_HookedLimitCount ; i++)
	{
		if( m_hook_info[i].m_cluster_id == clusterid)
		{
			m_hook_info[i].m_hooked = false;
			return;
		}
	}
}
//==============================================================================================================================================
void VeinConnectObject::SetAllHookOff()
{
	for(int i = 0 ; i < m_HookedLimitCount ; i++)
		m_hook_info[i].m_hooked = false;
}
//==============================================================================================================================================
void VeinConnectObject::SetNewMode()
{
	m_HookedLimitCount = MAXHOOKNUM;
}
//======================================================================================================
int  VeinConnectObject::GetNumPairRigidHooked(GFPhysCollideObject * rigid)
{
	std::map<GFPhysCollideObject*, int>::iterator itor = m_RigidHookPairCounts.find(rigid);
	if (itor != m_RigidHookPairCounts.end())
	{
		return itor->second;
	}
	else
	{
		return 0;
	}
}