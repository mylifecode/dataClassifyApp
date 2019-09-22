#include "MisMedicOrganOrdinary.h"
#include "math/GoPhysTransformUtil.h"
#include "Math/GoPhysMatrixDecompose.h"
#include "Dynamic/Constraint/GoPhysSoftFixpointConstraint.h"
#include "Dynamic/PhysicBody/GoPhysSoftMembrane.h"
#include "PhysicsWrapper.h"
#include "DynamicObjectRenderable.h"
#include "OrganBloodMotionSimulator.h"
#include "TextureBloodEffect.h"
#include "TextureWaterEffect.h"//__/__
#include "XMLWrapperOrgan.h"
#include "XMLWrapperTraining.h"
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "TrainingMgr.h"
#include "Topology/GoPhysPresetCutGeometry.h"
#include "Topology/GoPhysTetraSubdiviser.h"
#include "Topology/GoPhysEdgeCollapse.h"
#include "Instruments/Scissors.h"
#include "MisMedicOrganAttachment.h"
#include "EffectManager.h"
#include "VesselBleedEffect.h"
#include "Instruments/ElectricHook.h"
#include "MisNewTraining.h"
#include "MisMedicObjectSerializer.h"
#include "MisMedicObjectUnion.h"
#include "MisMedicOrganShapeModify.h"
#include <deque>
#include <set>
#include "MXOgreGraphic.h"
#include "AcessoriesCutTrain.h"
#include "ElectrocoagulationTrain.h"
#include "MxSliceOffOrganEvent.h"
#include "Topology/GoPhysEdgeCollapse.h"
#include "MisMedicFasciaTissueOperator.h"
#include "Instruments/MisCTool_PluginClamp.h"
#include "VeinConnectObject.h"

#define ORIGINMATERIALID 1
#define CUTMATERIALID 2

#define BUILDORIGINFACEUSRDATA(FaceIndex) ((ORIGINMATERIALID << 16) | FaceIndex)


#define BUILCUTCROSSFACEUSRDATA(FaceIndex) ((CUTMATERIALID << 16) | FaceIndex)
#define USECUTREFINE 1
void DistributeMassInFace(Real totalmass , GFPhysSoftBody * sb);
struct OrganNodeInfo 
{
	OrganNodeInfo(int index,GFPhysVector3 *position) : m_index(index) , m_postion(position) {}
	int m_index;
	GFPhysVector3 *m_postion;
};

bool TitanicClipInfo::s_clipInValidReg = false;
int TitanicClipInfo::s_clipEmptyCount = 0;
bool ElecCutInfo::s_cutSucceed = false;

bool CompareNodeHeight(const OrganNodeInfo & lhs, const OrganNodeInfo & rhs)
{
	return lhs.m_postion->m_y < rhs.m_postion->m_y;
}

Ogre::Vector2 getGradientTriUV(Ogre::Vector3 triPos[3], Ogre::Vector2 triTex[3])
{
	Ogre::Vector2 gradTex;

	float u0 = triTex[1].x - triTex[0].x;
	float v0 = triTex[1].y - triTex[0].y;

	float u1 = triTex[2].x - triTex[0].x;
	float v1 = triTex[2].y - triTex[0].y;

	Ogre::Vector3 p0 = triPos[1] - triPos[0];
	Ogre::Vector3 p1 = triPos[2] - triPos[0];

	Ogre::Vector3 deltau = (p0*v1 - p1*v0);
	float denomu = u0*v1 - u1*v0;
	if (fabsf(denomu) > GP_EPSILON)
	{
		gradTex.x = fabsf(deltau.length() / denomu);
	}
	else
	{
		gradTex.x = 0;
	}

	Ogre::Vector3 deltav = (p0*u1 - p1*u0);
	float denomv = v0*u1 - v1*u0;
	if (fabsf(denomv) > GP_EPSILON)
	{
		gradTex.y = fabsf(deltav.length() / denomv);
	}
	else
	{
		gradTex.y = 0;
	}



	//test
	float t0 = p0.length() / sqrtf(u0 * u0 + v0 * v0);
	float t1 = p1.length() / sqrtf(u1 * u1 + v1 * v1);


	//
	return gradTex;
}
void ClosetFaceToPoint(MisMedicOrgan_Ordinary * meshordinary,
								 const GFPhysVector3 &testpoint,
								 float &closetDist,
								 GFPhysSoftBodyFace * & closetFace,
								 GFPhysVector3 &closetPoint)
{
	GFPhysSoftBody * attachbody = meshordinary->m_physbody;

	//GFPhysSoftBodyFace * face = attachbody->GetFaceList();

	closetDist = FLT_MAX;
	closetFace = 0;
	//while(face)
	for(size_t f = 0 ; f < attachbody->GetNumFace() ; f++)
	{
		GFPhysSoftBodyFace * face = attachbody->GetFaceAtIndex(f);
		
		GFPhysVector3 closetptface = ClosestPtPointTriangle(testpoint, 
															face->m_Nodes[0]->m_CurrPosition, 
															face->m_Nodes[1]->m_CurrPosition,
															face->m_Nodes[2]->m_CurrPosition);

		float dist = (closetptface-testpoint).Length();

		if(dist < closetDist)
		{
			closetFace = face;
			closetPoint = closetptface;
			closetDist = dist;
		}

		//face = face->m_Next;
	}
}
MMO_Node MMO_Node::s_errorNode(true);
MMO_Face MMO_Face::s_errorFace(true);

PhysNode_Data::PhysNode_Data()
{
	m_PhysNode = 0;
	//m_PosRedirectNode = 0;
	m_ShadowNodeIndex = -1;
	//m_bloodValue = 0;
	//m_burnValue = 0;
	m_TempBuildId = -1;
	//m_NodeInCutEdge  = false;
	m_HasError = false;
	m_ForceFeedScale = 1.0f;
	m_Dist = 0;
	m_AddSMForce = false;
	m_NodeBeFixed = false;
	m_NextFreed = -1;
	//m_NodeTextureCoord = Ogre::Vector2(0,0);
	m_LayerBlendFactor = 0.0f;
}	
//=====================================================================================================
PhysTetra_Data::PhysTetra_Data()
{
	m_PhysTetra = 0;
	m_Layer = -1;
	m_OrganID = -1;
	m_IsMenstary = false;
	m_ContainsVessel = false;
	m_HasError = false;
	//m_IsTextureSetted = false;
	m_NextFreed = -1;
}
//=====================================================================================================
PhysTetra_Data::PhysTetra_Data(GFPhysSoftBodyTetrahedron* phystetra , int organId , bool ismens)
{
	m_PhysTetra = phystetra;
	m_Layer = -1;
	m_OrganID = organId;
	m_IsMenstary = ismens;
	m_ContainsVessel = false;
	m_HasError = false;
	//m_IsTextureSetted = false;
	m_NextFreed = -1;
}
//=====================================================================================================
MMO_Node::MMO_Node(bool haserror)
{
	m_PhysNode = 0;
	//m_InCutEdge = false;
	m_HasError = haserror;//false;
	m_NodeDataIndex = -1;
}
//=====================================================================================================
MMO_Face::MMO_Face(bool haserror)
{
	m_physface = 0;
	m_HasError = haserror;
	m_NeedRend = true;
	m_NextFreed = -1;
	m_BurnValue = 0;    
	m_NeedDoubleFaces = false;
    m_VeinInfoVector.clear();
	m_LayerIndex = 0;
}
//=====================================================================================================
void MMO_Face::BuildConstParameter()
{
	if (m_physface == 0)
		return;

	GFPhysVector3 side0 = (m_NodeUndeformPos[0]-m_NodeUndeformPos[1]).Normalized();
	GFPhysVector3 side1 = (m_NodeUndeformPos[2]-m_NodeUndeformPos[0]).Normalized();
	GFPhysVector3 side2 = (m_NodeUndeformPos[2]-m_NodeUndeformPos[1]).Normalized();

	m_constAngle[0] = acosf(-side0.Dot(side1));
	m_constAngle[1] = acosf(side0.Dot(side2));
	m_constAngle[2] = acosf(side1.Dot(side2));

	Ogre::Vector2  uv1(m_physface->m_TexCoordU[0], m_physface->m_TexCoordV[0]);// m_TextureCoord[0];
	Ogre::Vector2  uv2(m_physface->m_TexCoordU[1], m_physface->m_TexCoordV[1]);// m_TextureCoord[1];
	Ogre::Vector2  uv3(m_physface->m_TexCoordU[2], m_physface->m_TexCoordV[2]);// m_TextureCoord[2];

	m_deltaV0 = uv1.y - uv2.y;
	m_deltaV1 = uv3.y - uv1.y;
	m_deltaU0 = uv1.x - uv2.x;
	m_deltaU1 = uv3.x - uv1.x;
}

//===================================================================================================
MisMedicOrgan_Ordinary::MisMedicOrgan_Ordinary(DynamicObjType organtype,int organId, CBasicTraining * ownertraing)
					  : MisMedicOrganInterface(DOT_VOLMESH , organtype , organId,ownertraing)
{
	m_HasVolTexCoors = false;
	m_FFDObject = 0;
	m_pSceneNode = 0 ;
	m_BloodSystem = NULL;
	m_BloodTextureEffect = NULL;
	//__/__
	m_WaterSystem = NULL;
	m_WaterTextureEffect = NULL;
	m_TotalElapsedTime = 0;
	m_lastBloodEmitTime = 0;
	m_simulateTime = 0.0;
	m_IsThisFrameInElecCogTouch = false;
	m_IsThisFrameInElecCutTouch = false;
	m_TimeNeedToEleCut = 1.0f;
	m_BloodTrackPointDensity = 0.1f;
	m_MaxBloodTrackCount = 5;
	m_MaxBloodTrackExistsTime = 180.0f;
	m_cutActionNum = 0;

	m_bIsResetAction = false;
	m_DragForceRate = 1.0f;

	m_CanBeShrink = true;
	m_BurnShrinkRate = 0.98f;
	m_bUseTransformForMaxData = false;
	
	 
	//m_CutThickNess = 0.02f;
	//m_MaxCutTimeForTetra = 2;

	//m_MaxCutThickNessPercent = 0.1f;//0.02f , 0.1f
	
	m_EleCatogeryCanCut = (~0);

	m_BurnNodeColor = Ogre::ColourValue(0.5f , 0.5f , 0.5f,0);
	m_BloodNodeColor = Ogre::ColourValue(0.387, 0.0022, 0.0007, 0);
	m_ElecCutKeepTime = 0.0f;

	m_ElecBurnKeepTime = 0.0f;
	m_strVesselBleedEffectTemplateName = PT_BLEED_00;

	m_AccmCrossFaceBloodTime = 0.0f;
	
	m_ContiueElecCutTime  = 0.0f;
	
	m_ElecCutThreshold = 1.0f;//0.1f;
	//m_RefBurnFaceID = -1;
	m_FixVertexColor = false;

	m_BloodScatterRadius = 0.01f;
	m_BloddScatterValue  = 0.8f;

	m_isSetGravity = false;
	m_CanBindThread = true;
	m_burnRecord = NULL;
	m_BloodIntervalTime = 1.0f;

	m_OrganIDFurtherStiffness = -1;

	m_IsolatedPartGravityValue = 20.0f;
	m_physDynWorld = 0;
    m_Serializer_NodeNum = 0;    
	m_Serializer_NodeInitPositions_copy = NULL;				

	m_FreeOriginFaceHead = -1;
	m_FreeCutFaceHead = -1;

	m_FreePhysNodeDataHead = -1;
	m_FreePhysTetraDataHead = -1;

	m_ElecCutRadius = 0.2f;
	m_BadTetCollapseParam = std::make_pair(0.2f, 2);

	//m_CutBoundaryShrinkRate = 0.001f;
#if(USECUTREFINE)
	m_MaxCutCount = 10;
#else
	m_MaxCutCount = 3;
#endif

	//m_IsFascia = false;
	m_TopologyDirty = false;

	m_TimeSinceLastElectricMelt = 0;

	//m_MaxSoftNodeSpeed = FLT_MAX;
	m_CutWidthScale = 1.0f;

	m_VolumeBlood = NULL;

	m_frameCount = 0;

	m_BleedRadius = 0.008f;

	m_EndoGiaClips = NULL;

	m_BleedWhenStripBreak = false;
	
	m_IsInnerTexCoordSetted = false;

	m_MinSubPartVolThresHold = 0.1f*0.1f*0.1f;
}
//===================================================================================================
MisMedicOrgan_Ordinary::~MisMedicOrgan_Ordinary()
{
	if(m_BloodTextureEffect)//put this before blood system
	{
		delete m_BloodTextureEffect;
		m_BloodTextureEffect = 0;
	}

	if(m_BloodSystem)
	{
		delete m_BloodSystem;
		m_BloodSystem = 0;
	}

	if(m_WaterTextureEffect)//put this before blood system
	{
		delete m_WaterTextureEffect;
		m_WaterTextureEffect = 0;
	}

	//if(m_WaterSystem)
	//{
	//	delete m_WaterSystem;
		m_WaterSystem = 0;
	//}

	if(m_burnRecord)
		delete[] m_burnRecord;

	//for(size_t c = 0 ; c < m_OrganSubParts.size() ; c++)
	//{
		//delete m_OrganSubParts[c];
	//}
	//m_OrganSubParts.clear();

	stopVesselBleedEffect();
    delete[] m_Serializer_NodeInitPositions_copy;

	/*
	// Already deleted from RemovePhysParts()

	if (m_EndoGiaClips) {
		RemoveOrganAttachment(m_EndoGiaClips);
		delete m_EndoGiaClips;
		m_EndoGiaClips = NULL;
	}
	*/
	

}
//========================================================================================================
int MisMedicOrgan_Ordinary::GetNumSubParts()
{
	return m_physbody->GetSoftBodyShape().GetNumSubParts();
}
//========================================================================================================
GFPhysSubConnectPart * MisMedicOrgan_Ordinary::GetSubPart(int index)
{
	return m_physbody->GetSoftBodyShape().GetSubPart(index);
}
//===================================================================================================
void MisMedicOrgan_Ordinary::ShrinkTetrahedrons(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & TetrasToShrink ,
												const GFPhysVector3 & shrinkDirInMaterialSpace ,
												float shrinkFactor)
{
	if(m_TimeSinceLastElectricMelt < 1.0f / 30.0f)
	   return;

	m_TimeSinceLastElectricMelt = 0;

	GoPhysSoftBodyRestShapeModify restShapeModify;
	//restShapeModify.ShrinkTetrahedrons(PhysicsWrapper::GetSingleTon().m_dynamicsWorld , 
									  // m_physbody, 
									   //TetrasToShrink,
									  // shrinkFactor);

	restShapeModify.SoftShrinkInMaterialSpace(PhysicsWrapper::GetSingleTon().m_dynamicsWorld ,
											  m_physbody , 
											  shrinkDirInMaterialSpace , 
											  TetrasToShrink , 
											  shrinkFactor , 
											  0.4f);

	  
	if(TetrasToShrink.size() > 0)
	{
		GFPhysVector3 aabbMin(FLT_MAX  , FLT_MAX  , FLT_MAX);
		GFPhysVector3 aabbMax(-FLT_MAX , -FLT_MAX , -FLT_MAX);
		for(size_t c = 0 ; c < TetrasToShrink.size() ; c++)
		{
			GFPhysSoftBodyTetrahedron * tetra = TetrasToShrink[c];
			for(int f = 0 ; f < 4 ; f++)
			{
				if(tetra->m_TetraFaces[f])
				{
					GFPhysSoftBodyFace * surFace = tetra->m_TetraFaces[f]->m_surface;
					if(surFace)
					{
					   aabbMin.SetMin(surFace->m_AabbMin);
					   aabbMax.SetMax(surFace->m_AabbMax);
					}
				}
			}
		}
		aabbMin -= GFPhysVector3(0.1f , 0.1f , 0.1f);
		aabbMax += GFPhysVector3(0.1f , 0.1f , 0.1f);

		GFPhysVector3 center = (aabbMin + aabbMax) * 0.5f;
		float radius = (aabbMax-aabbMin).Length() * 0.5f;

		std::vector<GFPhysSoftBodyFace*> refreshFaces;
		SelectPhysFaceAroundPoint(refreshFaces , center , radius , true);

		for(int c = 0 ; c < (int)refreshFaces.size() ; c++)
		{	
			GFPhysSoftBodyFace * physFace = refreshFaces[c];

			int materialid , faceid;

			ExtractFaceIdAndMaterialIdFromUsrData(physFace , materialid , faceid);

			MMO_Face & Organface = (materialid == ORIGINMATERIALID ? GetMMOFace_OriginPart(faceid) : GetMMOFace_CutPart(faceid));

			if(!Organface.m_HasError)
			{
				for(int n = 0 ; n < 3 ; n++)
				{
					GFPhysSoftBodyNode * FaceNode = physFace->m_Nodes[n];

					Organface.m_NodeUndeformPos[n] = FaceNode->m_UnDeformedPos;
				}
				//Organface.BuildConstParameter();
			}
		}
	}
	

		
	/*
	std::vector<GFPhysSoftBodyFace*> facesToRefresh;
	facesToRefresh.reserve(TetrasToShrink.size() * 4);

	for(size_t c = 0 ; c < TetrasToShrink.size() ; c++)
	{
		GFPhysSoftBodyTetrahedron * tetra = TetrasToShrink[c];
		for(int f = 0 ; f < 4 ; f++)
		{
			if(tetra->m_TetraFaces[f])
			{
			   GFPhysSoftBodyFace * surFace = tetra->m_TetraFaces[f]->m_surface;
			   if(surFace)
			      facesToRefresh.push_back(surFace);
			}
		}
	}

	//
	for(int c = 0 ; c < facesToRefresh.size() ; c++)
	{	
		GFPhysSoftBodyFace * physFace = facesToRefresh[c];

		int materialid , faceid;
		
		ExtractFaceIdAndMaterialIdFromUsrData(physFace , materialid , faceid);

		MMO_Face & Organface = (materialid == ORIGINMATERIALID ? GetMMOFace_OriginPart(faceid) : GetMMOFace_CutPart(faceid));

		if(!Organface.m_HasError)
		{
			for(int n = 0 ; n < 3 ; n++)
			{
				GFPhysSoftBodyNode * FaceNode = physFace->m_Nodes[n];

				Organface.m_NodeUndeformPos[n] = FaceNode->m_UnDeformedPos;
			}
		}
	}*/
	//RefreshRendFacesUndeformedPosition();//temply refresh all need optimize

}
//===================================================================================================
void MisMedicOrgan_Ordinary::SetEletricCutParameter(float cutRadius , int maxcutCount)
{
	m_ElecCutRadius = cutRadius;
	m_MaxCutCount = maxcutCount;
}
//===================================================================================================
int MisMedicOrgan_Ordinary::GetOriginFaceIndexFromUsrData(GFPhysSoftBodyFace * face)
{
    if (face == NULL)
    {
        return -1;
    }
	int indexData = (int)face->m_UserData;
	
	int materialid = (indexData >> 16);

	int faceid = (indexData & 0xFFFF);

	if(materialid == ORIGINMATERIALID)
	   return faceid;
	else
	   return -1;
}
int MisMedicOrgan_Ordinary::GetCrossFaceIndexFromUsrData(GFPhysSoftBodyFace * face)
{
	int indexData = (int)face->m_UserData;

	int materialid = (indexData >> 16);

	int faceid = (indexData & 0xFFFF);

	if(materialid == CUTMATERIALID)
	   return faceid;
	else
	   return -1;
}
void  MisMedicOrgan_Ordinary::ExtractFaceIdAndMaterialIdFromUsrData(GFPhysSoftBodyFace * face , int & materialid , int & faceid)
{
	if (face)
	{
		int indexData = (int)face->m_UserData;

		materialid = (indexData >> 16);

		faceid = (indexData & 0xFFFF);
	}
}

void  MisMedicOrgan_Ordinary::ExtractFaceIdAndMaterialIdFromUsrData(int userData , int & materialid , int & faceid)
{
	materialid = (userData >> 16);

	faceid = (userData & 0xFFFF);
}
//===================================================================================================
Ogre::Vector2 MisMedicOrgan_Ordinary::GetTextureCoord(GFPhysSoftBodyFace * face , float weights[3])
{
	if(m_FFDObject == 0)
	{
	   if (face == 0)
		   return Ogre::Vector2(0,0);
	   
	   Ogre::Vector2 texturecoord[3];
	   
	   texturecoord[0] = Ogre::Vector2(face->m_TexCoordU[0], face->m_TexCoordV[0]);//m_texcoords[rendface.a];
	   texturecoord[1] = Ogre::Vector2(face->m_TexCoordU[1], face->m_TexCoordV[1]);//m_texcoords[rendface.b];
	   texturecoord[2] = Ogre::Vector2(face->m_TexCoordU[2], face->m_TexCoordV[2]);//m_texcoords[rendface.c];
	   
	   Ogre::Vector2 texturepoint = texturecoord[0] * weights[0]
		                          + texturecoord[1] * weights[1]
		                          + texturecoord[2] * weights[2];

	   return texturepoint;
	   
	   //Get Origin MMO_Face Index and Material
	   /*
		int materialid , faceid;
		ExtractFaceIdAndMaterialIdFromUsrData(face , materialid , faceid);
		if(materialid == ORIGINMATERIALID)
		{
			MMO_Face rendface = m_OriginFaces[faceid];

			Ogre::Vector2 texturecoord[3];
			texturecoord[0] = rendface.m_TextureCoord[0];//m_texcoords[rendface.a];
			texturecoord[1] = rendface.m_TextureCoord[1];//m_texcoords[rendface.b];
			texturecoord[2] = rendface.m_TextureCoord[2];//m_texcoords[rendface.c];

			Ogre::Vector2 texturepoint = texturecoord[0]*weights[0]
										+texturecoord[1]*weights[1]
										+texturecoord[2]*weights[2];

			return texturepoint;
		}
		else
		{
			return Ogre::Vector2::ZERO;
		}
	  */

		/*for(size_t f = 0 ; f < m_OriginFaces.size() ; f++)
		{
			MMO_Face rendface = m_OriginFaces[f];
			if(rendface.m_physface == face)
			{
				Ogre::Vector2 texturecoord[3];
				texturecoord[0] = rendface.m_TextureCoord[0];//m_texcoords[rendface.a];
				texturecoord[1] = rendface.m_TextureCoord[1];//m_texcoords[rendface.b];
				texturecoord[2] = rendface.m_TextureCoord[2];//m_texcoords[rendface.c];

				Ogre::Vector2 texturepoint = texturecoord[0]*weights[0]
				+texturecoord[1]*weights[1]
				+texturecoord[2]*weights[2];

				return texturepoint;
			}
		}*/
	}
	else
	{
		GFPhysVector3 pointInRoughFace = face->m_Nodes[0]->m_UnDeformedPos * weights[0]
										+face->m_Nodes[1]->m_UnDeformedPos * weights[1]
										+face->m_Nodes[2]->m_UnDeformedPos * weights[2];
		float mindist = FLT_MAX;
		
		int   minfaceIndex = -1;
		
		GFPhysVector3 minPoint;
		
		//iterator of all face
		for(size_t f = 0 ; f < m_FFDObject->m_rendfaces.size() ; f++)
		{
			const DynamicSurfaceFreeDeformObject::RendFaceData & rendface = m_FFDObject->m_rendfaces[f];
			
			GFPhysVector3 manivertPos[3];

			Ogre::Vector3 tempPos[3];

			tempPos[0] = m_FFDObject->m_manivertex[rendface.manivertindex[0]].m_UnDeformedPos;
		
			tempPos[1] = m_FFDObject->m_manivertex[rendface.manivertindex[1]].m_UnDeformedPos;
		
			tempPos[2] = m_FFDObject->m_manivertex[rendface.manivertindex[2]].m_UnDeformedPos;
		
			manivertPos[0] = GFPhysVector3(tempPos[0].x , tempPos[0].y , tempPos[0].z);

			manivertPos[1] = GFPhysVector3(tempPos[1].x , tempPos[1].y , tempPos[1].z);

			manivertPos[2] = GFPhysVector3(tempPos[2].x , tempPos[2].y , tempPos[2].z);

			GFPhysVector3 closetPointInTri = ClosestPtPointTriangle(pointInRoughFace, 
																	manivertPos[0],
																	manivertPos[1], 
																	manivertPos[2]);

			float Dist = (closetPointInTri-pointInRoughFace).Length() ;
			if(Dist < mindist)
			{
				mindist = Dist;
				minfaceIndex = f;
				minPoint = closetPointInTri;
			}
		}

		if(minfaceIndex >= 0)
		{
			const DynamicSurfaceFreeDeformObject::RendFaceData & rendface = m_FFDObject->m_rendfaces[minfaceIndex];

			float weights[3];
			
			Ogre::Vector3 tempPos[3];
			
			GFPhysVector3 faceVertex[3];
			
			tempPos[0] = m_FFDObject->m_manivertex[rendface.manivertindex[0]].m_UnDeformedPos;

			tempPos[1] = m_FFDObject->m_manivertex[rendface.manivertindex[1]].m_UnDeformedPos;

			tempPos[2] = m_FFDObject->m_manivertex[rendface.manivertindex[2]].m_UnDeformedPos;

			faceVertex[0] = GFPhysVector3(tempPos[0].x , tempPos[0].y , tempPos[0].z);

			faceVertex[1] = GFPhysVector3(tempPos[1].x , tempPos[1].y , tempPos[1].z);

			faceVertex[2] = GFPhysVector3(tempPos[2].x , tempPos[2].y , tempPos[2].z);

			CalcBaryCentric(faceVertex[0] , faceVertex[1] , faceVertex[2], minPoint ,weights[0] , weights[1] , weights[2]);

			Ogre::Vector2 texturecoord = rendface.texcoord[0]*weights[0]+rendface.texcoord[1]*weights[1]+rendface.texcoord[2]*weights[2];
			return texturecoord;
		}
	}
	
	return Ogre::Vector2::ZERO;
}

//==================================================================================
void MisMedicOrgan_Ordinary::CreateEffectRender(int Width , int Height , Ogre::String name)
{
	//if(m_OrganID == EODT_UTERUS || m_OrganID == EDOT_APPENDIX)
	//   m_EffectRender.m_BurnWhiteSrcImageName = "UteriaBurn.png";
	
	//else if(m_OrganID == EDOT_LIVER)
	 //  m_EffectRender.m_BurnWhiteSrcImageName = "Liver_Burn.dds";
	
	m_EffectRender.Create(m_materialname , Width, Height, name);
	ApplyTextureToMaterial(m_materialname, m_EffectRender.m_QuantityTexturePtr , "PerminateEffectMap");
	ApplyTextureToMaterial(m_materialname, m_EffectRender.m_DynamicBloodTexturePtr , "DynamicEffectMap");
	//ApplyTextureToMaterial(m_materialname, m_EffectRender.m_BurnWhiteTexturePtr, "BurnTextureMap");

	for (int c = 1; c < m_CreateInfo.m_LayerVecs.size(); c++)
	{
		ApplyTextureToMaterial(m_CreateInfo.m_LayerVecs[c].m_ClonedMatPtr, m_EffectRender.m_QuantityTexturePtr, "PerminateEffectMap");
	}
}

void MisMedicOrgan_Ordinary::InitializeEffectSystem()
{
	if(m_BloodSystem)
	{
	   m_BloodSystem->Initialize(this);
	   if(m_OrganID == EDOT_LIVER)
	   { 
		   m_BloodSystem->setGravityDir(Ogre::Vector3(0,0,-1));
	   }
	   else if(m_OrganID == EDOT_GALLBLADDER)
	   { 
		   m_BloodSystem->setGravityDir(Ogre::Vector3(0,0,-1));
	   }
	}

	if(m_WaterSystem)
	{
		//__/__
		//Ogre::String texName;
		//bool bIsHave = GetMaterialTextureName(this->m_materialname, "NormalMap", texName);
		//if( bIsHave )
		//{
		//	//设置合成法向纹理的背景
		//	Ogre::TexturePtr texNormalMapPtr = Ogre::TextureManager::getSingleton().load(texName,Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		//	if(texNormalMapPtr->isLoaded())
		//		ApplyTextureToMaterial(m_EffectRender.m_ComposedNormalMaterialPtr , texNormalMapPtr ,"BaseNormalMap");
		//	ApplyTextureToMaterial(m_EffectRender.m_ComposedNormalMaterialPtr ,  m_EffectRender.m_DynamicWaterTexturePtr, "DynamicNormalMap");
		//}
		////设置水滴的纹理
		//Ogre::TexturePtr texWaterPatical = Ogre::TextureManager::getSingleton().load("TestWater.png" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		//if(texWaterPatical->isLoaded())
		//	ApplyTextureToMaterial(m_EffectRender.m_DynamicWaterMaterialPtr , texWaterPatical ,"PaticalNormalMap");
		////设置合成的法向纹理作为器官的法向贴图
		//ApplyTextureToMaterial(this->m_materialname ,  m_EffectRender.m_ComposedNormalTexturePtr, "NormalMap");
		////设置法向纹理初始值
		//m_EffectRender.ComposeNormalTexture(Ogre::ColourValue(0,0,0,0));

		//设置水滴的纹理
		//Ogre::TexturePtr texWaterPatical = Ogre::TextureManager::getSingleton().load("TestWater.png" , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		//if(texWaterPatical->isLoaded())
			//ApplyTextureToMaterial(m_EffectRender.m_DynamicWaterMaterialPtr , texWaterPatical ,"PaticalNormalMap");
		//设置动态法向贴图到最终渲染脚本
		//ApplyTextureToMaterial(this->m_materialname ,  m_EffectRender.m_DynamicWaterTexturePtr, "DynamicNormalMap");
		//尝试设置参数
		//SetBaseTextureMix(0.0f);
	}
}

void MisMedicOrgan_Ordinary::SetBaseTextureMix(float fMix)
{
	Ogre::GpuProgramParametersSharedPtr params = GetShaderParamterPtr(m_materialname, FRAGMENT_PROGRAME, 0, 0);
	if (params.isNull() == false)
	{
		//params->setNamedConstant("BaseTextureMix", fMix);
	}
}

void MisMedicOrgan_Ordinary::SetBloodAndBurnColorForMaterial()
{
	//set material program parameter
	Ogre::MaterialPtr matr = Ogre::MaterialManager::getSingleton().getByName(m_materialname);
	if(matr.isNull() == false)
	{
		if(matr->getNumTechniques() > 0)
		{
			Ogre::Technique * maintech = matr->getTechnique(0);
			if(maintech && maintech->getNumPasses() > 0)
			{
				Ogre::Pass * pass = maintech->getPass(0);
				
				if(pass->hasFragmentProgram())
				{
					Ogre::GpuProgramParametersSharedPtr gpuParams = pass->getFragmentProgramParameters();

					try
					{
						//gpuParams->setNamedConstant("BurnColor", m_BurnNodeColor);
						if(gpuParams->_findNamedConstantDefinition("OrganBloodColor"))
						   gpuParams->setNamedConstant("OrganBloodColor", m_BloodNodeColor);
					}
					catch (...)
					{}		

				}
			}
		}
	}
	//set cut material
	/*
	if(m_CreateInfo.m_CutMaterialName != m_materialname)
	{
		matr = Ogre::MaterialManager::getSingleton().getByName(m_CreateInfo.m_CutMaterialName);
		if(matr.isNull() == false)
		{
			if(matr->getNumTechniques() > 0)
			{
				Ogre::Technique * maintech = matr->getTechnique(0);
				if(maintech && maintech->getNumPasses() > 0)
				{
					Ogre::Pass * pass = maintech->getPass(0);

					if(pass->hasFragmentProgram())
					{
						Ogre::GpuProgramParametersSharedPtr gpuParams = pass->getFragmentProgramParameters();

						try
						{
							//gpuParams->setNamedConstant("BurnColor", m_BurnNodeColor);
							if(gpuParams->_findNamedConstantDefinition("OrganBloodColor"))
							   gpuParams->setNamedConstant("OrganBloodColor", m_BloodNodeColor);
						}
						catch (...)
						{}		

					}
				}
			}
		}
	}*/
	
}
//==================================================================================
void MisMedicOrgan_Ordinary::ReadOrganObjectFile(MisMedicDynObjConstructInfo & cs)
{
	m_IsAlreadyReadFile = true;
	if (m_bIsResetAction == false)//only first time need we read file from serializer
	{
		cs.m_LayerVecs.push_back(OrganLayer(cs.m_LayerName, cs.m_materialname[0], 0));
		
		if(cs.m_MergedObjIDS.size() == 0)
        {
			m_Serializer.ReadFromOrganObjectFile(cs.m_OrganType , cs.m_s3mfilename , cs.m_s4mfilename , cs.m_t2filename ,  cs.m_Position);
			
			if (cs.m_objTopologyType == DOT_VOLMESH)
			    m_Serializer.GenerateInnerTexture();

			///for Adhesion after inflation
            if (fabs(cs.m_ExpandValue) > GP_EPSILON)
            {
                m_Serializer_NodeNum = m_Serializer.m_NodeInitPositions.size();             
                m_Serializer_NodeInitPositions_copy = new GFPhysVector3[m_Serializer_NodeNum];
                for (int i = 0;i<m_Serializer_NodeNum;i++)
                {
                    m_Serializer_NodeInitPositions_copy[i] = m_Serializer.m_NodeInitPositions[i];
                }                
            }
        }
		else
		{
			std::vector<int> CombinedFixPointIndex;

			MisMedicObjectUnion objMerge;

			m_Serializer.ReadFromOrganObjectFile(cs.m_MergedObjIDS[0], cs.m_MergedObjMMSFileNames[0], "", "", cs.m_Position);
			m_Serializer.GenerateInnerTexture();

			for (int c = 1; c < cs.m_MergedObjIDS.size(); c++)
			{
				std::string layerName = cs.m_MergedObjLayerNames[c];

				int layerIndex = -1;

				for (int t = 0; t < (int)cs.m_LayerVecs.size(); t++)
				{
					if (cs.m_LayerVecs[t].m_LayerName == layerName)
					{
						layerIndex = t;
						break;
					}
				}

				if (layerIndex < 0)
				{
					layerIndex = cs.m_LayerVecs.size();

					std::string layerMaterial = cs.m_MergedObjLayerMaterials[c];

					cs.m_LayerVecs.push_back(OrganLayer(layerName, layerMaterial, layerIndex));
				}
				std::vector<int> FixPointsIndex = cs.m_FixPointsIndex;
				cs.m_FixPointsIndex.clear();

				objMerge.MergeObjectToExist(layerIndex, m_Serializer, cs.m_MergedObjIDS[c], cs.m_MergedObjMMSFileNames[c], cs.m_Position, 0.005f, FixPointsIndex, cs.m_FixPointsIndex);
			}

			//temp
#if(0)
			if (cs.m_LayerVecs.size() > 1)
			{
				std::vector<MisMedicObjetSerializer::MisSerialTetra> reserveTetra = m_Serializer.m_InitTetras;
				for (int t = 0; t < m_Serializer.m_InitTetras.size(); t++)
				{
					m_Serializer.m_InitTetras[t].m_IsTextureSetted = false;
				}

				m_Serializer.GenerateInnerTexture();

				for (int t = 0; t < reserveTetra.size(); t++)
				{
					if (m_Serializer.m_InitTetras[t].m_LayerIndex == 0)
					{
						m_Serializer.m_InitTetras[t].m_TextureCoord[0] = reserveTetra[t].m_TextureCoord[0];
						m_Serializer.m_InitTetras[t].m_TextureCoord[1] = reserveTetra[t].m_TextureCoord[1];
						m_Serializer.m_InitTetras[t].m_TextureCoord[2] = reserveTetra[t].m_TextureCoord[2];
						m_Serializer.m_InitTetras[t].m_TextureCoord[3] = reserveTetra[t].m_TextureCoord[3];
					}
				}
			}
#endif
			//m_Serializer.GenerateInnerTexture();
			//objMerge.CreateMergedObject(m_Serializer , cs.m_unionedobjid , cs.m_OrganType , cs.m_unionedmmsfilename , cs.m_s3mfilename ,  cs.m_Position , 0.005f , std::vector<int>() , cs.m_FixPointsIndex , CombinedFixPointIndex);
			//m_Serializer.m_MergedObjectStiffness.insert(std::make_pair(cs.m_OrganType , cs.m_stiffness));
			//m_Serializer.m_MergedObjectStiffness.insert(std::make_pair(cs.m_unionedobjid , cs.m_unionedobjstiffness));

			//m_CreateInfo.m_FixPointsIndex.clear();
			//m_CreateInfo.m_FixPointsIndex = CombinedFixPointIndex;
		}

		//for(int t = 0 ; t < m_Serializer.m_InitTetras.size() ; t++)
		//{
			///bool isMenstary = false;

			//if( m_Serializer.m_InitTetras[t].m_unionObjectID == cs.m_unionedobjid && cs.m_unionobjMenstary)
			//	isMenstary = true;

			//if( m_Serializer.m_InitTetras[t].m_unionObjectID == cs.m_OrganType && cs.m_IsMensentary)
			//	isMenstary = true;

			//m_Serializer.m_InitTetras[t].m_IsMenstary = isMenstary;
		//}

		MisNewTraining * newTrain = dynamic_cast<MisNewTraining*>(m_OwnerTrain);
		if(newTrain)
		{
			newTrain->SerializerReadFinish(this , m_Serializer);
		}
	}

	
}
void MisMedicOrgan_Ordinary::Create(MisMedicDynObjConstructInfo & cs)
{
	m_CreateInfo = cs;
	m_Visible = cs.m_Visible;
	m_CanBeGrasp = cs.m_bCanBeGrasp;
	m_CanBePuncture = cs.m_bCanPuncture;
	m_ForceFeedBackRation = cs.m_ForceFeedBackRation;
	m_ConnectMass = cs.m_ConnectMass;
	m_congulateradius = cs.m_BurnRadius;
	
	m_BloodSystem = new OrganBloodMotionSimulator();
	m_BloodTextureEffect = new TextureBloodTrackEffect(this);
	m_BloodTextureEffect->SetBloodSystem(m_BloodSystem);

	//SetBloodAndBurnColorForMaterial();
	
	//__/__
	m_WaterSystem = m_BloodSystem;//new OrganBloodMotionSimulator();
	m_WaterTextureEffect = new TextureWaterTrackEffect(this);
	m_WaterTextureEffect->SetBloodSystem(m_WaterSystem);

	
	if(!m_IsAlreadyReadFile)
		ReadOrganObjectFile(cs);

	if (m_CreateInfo.m_LayerVecs.size() > 1)//multilayer
	{
		for (int c = 0; c < m_CreateInfo.m_LayerVecs.size(); c++)
		{
			OrganLayer & layer = m_CreateInfo.m_LayerVecs[c];

			layer.m_CloneMaterialName = layer.m_MaterialName + cs.m_name + "layer_" + layer.m_LayerName;

			Ogre::MaterialManager::getSingleton().load(layer.m_MaterialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

			Ogre::MaterialPtr originMatPtr = Ogre::MaterialManager::getSingleton().getByName(layer.m_MaterialName);

			layer.m_ClonedMatPtr = originMatPtr->clone(layer.m_CloneMaterialName);

			if (layer.m_layerIndex == 0)
			{
				m_materialname = layer.m_CloneMaterialName;
				m_OwnerMaterialPtr = layer.m_ClonedMatPtr;
			}
		}
	}
	else
	{
		//clone owner material
		Ogre::String Orignmaterialname = cs.m_materialname[0];

		m_materialname = Orignmaterialname + cs.m_name;

		Ogre::MaterialManager::getSingleton().load(Orignmaterialname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		Ogre::MaterialPtr originMatPtr = Ogre::MaterialManager::getSingleton().getByName(Orignmaterialname);

		m_OwnerMaterialPtr = originMatPtr->clone(m_materialname);

		//hack to cs
		//if (m_CreateInfo.m_CutMaterialName == Orignmaterialname)//if they are same point cut material to clone material
		//{
		//	m_CreateInfo.m_CutMaterialName = m_materialname;
		//}
	}
	
//dy change
// 	if (m_bIsResetAction == false)//only first time need we read file from serializer
// 	{
// 		if(cs.m_unionedmmsfilename == "")
// 		   m_Serializer.ReadFromOrganObjectFile(cs.m_OrganType , cs.m_s3mfilename , cs.m_s4mfilename , cs.m_t2filename ,  cs.m_Position);
// 		else
// 		{
// 		   std::vector<int> CombinedFixPointIndex;
// 
// 		   MisMedicObjectUnion objMerge;
// 		  
// 		   objMerge.CreateMergedObject(m_Serializer , cs.m_unionedobjid , cs.m_OrganType , cs.m_unionedmmsfilename , cs.m_s3mfilename ,  cs.m_Position , 0.005f , std::vector<int>() , cs.m_FixPointsIndex , CombinedFixPointIndex);
// 		   m_Serializer.m_MergedObjectStiffness.insert(std::make_pair(cs.m_OrganType , cs.m_stiffness));
// 		   m_Serializer.m_MergedObjectStiffness.insert(std::make_pair(cs.m_unionedobjid , cs.m_unionedobjstiffness));
// 
// 		   m_CreateInfo.m_FixPointsIndex.clear();
// 		   m_CreateInfo.m_FixPointsIndex = CombinedFixPointIndex;
// 		}
// 
// 		for(int t = 0 ; t < m_Serializer.m_TetraInitNum ; t++)
// 		{
// 			bool isMenstary = false;
// 			
// 			if( m_Serializer.m_InitTetras[t].m_unionObjectID == cs.m_unionedobjid && cs.m_unionobjMenstary)
// 				isMenstary = true;
// 
// 			if( m_Serializer.m_InitTetras[t].m_unionObjectID == cs.m_OrganType && cs.m_IsMensentary)
// 				isMenstary = true;
// 
// 			m_Serializer.m_InitTetras[t].m_IsMenstary = isMenstary;
// 		}
// 
// 		MisNewTraining * newTrain = dynamic_cast<MisNewTraining*>(m_OwnerTrain);
// 		if(newTrain)
// 		{
// 			newTrain->SerializerReadFinish(this , m_Serializer);
// 		}
// 	}

	if(m_bUseTransformForMaxData)
		transformForModelData();
	
	CreateOrganObjectFromSerialize(m_Serializer  , cs.m_s3mfilename);

	if(cs.m_FFDFile != "")
	   CreateFFDObject(cs.m_name+"FFDObj" , cs.m_FFDFile);
	
	CreateEffectRender(cs.m_effTexWid , cs.m_effTexWid , cs.m_name);
	
	InitializeEffectSystem();

	m_bleedEffectFlag = 0;
	
	//@some face may not correct in mms file when merge 2 object
	//here let it auto create surface to see if we miss any surface
	if(m_physbody && m_CreateInfo.m_MergedObjIDS.size() > 0)
	{
		m_physbody->AutoCreateSurface();

		//GFPhysSoftBodyFace * face = m_physbody->GetFaceList();
		//while(face)
		for(size_t f = 0 ; f < m_physbody->GetNumFace() ; f++)
		{
			GFPhysSoftBodyFace * face = m_physbody->GetFaceAtIndex(f);

			if(face->m_UserData == 0)//new formed face;
			{
				int ResultIndex;
				AddNewCuttedMMO_FaceInternal(face , ResultIndex);
			}
			//face = face->m_Next;
		}
	}

	//rebuild Rend Vertex And Face Vertex Index
	RebuildRendVertexAndIndex();

	if (cs.m_pulsePoints != "") {
		Ogre::vector<Ogre::String>::type strs = Ogre::StringUtil::split(cs.m_pulsePoints, ",");
		pulseForce = Ogre::StringConverter::parseInt(strs[0]);
		for (size_t f = 1; f < strs.size(); f++)
		{
			Ogre::String str = strs[f];
			int index = Ogre::StringConverter::parseInt(str);

			pulseNodes.push_back(m_physbody->GetNode(index));

		}

	}
	
	//printf("%s %s\n", getName().c_str(), cs.m_pulsePoints.c_str());

}
void MisMedicOrgan_Ordinary::PostLoadScene()
{
	for (int c = 0; c < m_physbody->GetSoftBodyShape().GetNumSubParts(); c++)
	{
		GFPhysSubConnectPart * subPart = m_physbody->GetSoftBodyShape().GetSubPart(c);// m_OrganSubParts[c];

		subPart->m_PartStateFlag = 0;

		for (int n = 0; n < (int)subPart->m_Nodes.size(); n++)
		{
			//velocity
			GFPhysSoftBodyNode * pnode = subPart->m_Nodes[n];
			subPart->m_PartStateFlag |= pnode->m_StateFlag;
		}
	}
}
//==================================================================================
void MisMedicOrgan_Ordinary::transformForModelData()
{
	for(int v = 0 ;v  < m_Serializer.m_NodeInitPositions.size();++v)
	{
		Ogre::Vector4 originVertex(m_Serializer.m_NodeInitPositions[v].GetX(),m_Serializer.m_NodeInitPositions[v].GetY(),m_Serializer.m_NodeInitPositions[v].GetZ(),1);
		Ogre::Vector4 newVertex = m_transformMatrix * originVertex;
		m_Serializer.m_NodeInitPositions[v].SetValue(newVertex.x,newVertex.y,newVertex.z);
	}
}
//=====================================================================================
void MisMedicOrgan_Ordinary::SetBloodColor(Ogre::ColourValue & colorValue)
{
	m_BloodNodeColor = colorValue;
	SetBloodAndBurnColorForMaterial();
}
//=====================================================================================
void MisMedicOrgan_Ordinary::SetBurnNodeColor(Ogre::ColourValue & colorValue)
{
	m_BurnNodeColor = colorValue;
	SetBloodAndBurnColorForMaterial();
}
//=====================================================================================
/*void MisMedicOrgan_Ordinary::Reset()
{
	m_bIsResetAction = true;
	RemoveFromWorld();
	
	if (m_bIsResetAction == false)//only first time need we read file from serializer
	{
		m_Serializer.ReadFromOrganObjectFile(m_CreateInfo.m_OrganType , m_CreateInfo.m_s3mfilename , m_CreateInfo.m_s4mfilename , m_CreateInfo.m_t2filename , m_CreateInfo.m_Position);
	}
	CreateOrganObjectFromSerialize(m_Serializer  , m_CreateInfo.m_s3mfilename);

	
	if(m_CreateInfo.m_FFDFile != "")
		CreateFFDObject(m_CreateInfo.m_name+"FFDObj" , m_CreateInfo.m_FFDFile);

	CreateEffectRender(m_CreateInfo.m_effTexWid , m_CreateInfo.m_effTexWid , m_CreateInfo.m_name);

	InitializeEffectSystem();

	m_bleedEffectFlag = 0;


	for(size_t f = 0 ; f < m_OriginFaces.size() ; f++)
	{
		MMO_Face & organface = m_OriginFaces[f];

		organface.m_physface->m_UserData = (void*)BUILDORIGINFACEUSRDATA(f);
	}

	RefreshRendFacesUndeformedPosition();

	RebuildRendVertexAndIndex();
	m_bIsResetAction = false;
}
*/
PhysNode_Data & MisMedicOrgan_Ordinary::GetPhysNodeData(GFPhysSoftBodyNode * node)
{
	int index = (int)node->m_UserPointer;
	if(index > 0 && index < (int)m_PhysNodeData.size())
	   return m_PhysNodeData[index];
	else
	   return m_PhysNodeData[0];//return error reference
}
PhysTetra_Data & MisMedicOrgan_Ordinary::GetPhysTetraAppData(GFPhysSoftBodyTetrahedron * tera)
{
	int index = (int)tera->m_UserPointer;

	if(index > 0 && index < (int)m_PhysTetraData.size())
	   return m_PhysTetraData[index];
	else
	   return m_PhysTetraData[0];//return error reference
}
//===================================================================================
MMO_Face & MisMedicOrgan_Ordinary::GetMMOFace_OriginPart(int FaceIndex)
{
	if(FaceIndex >= 0 && FaceIndex < (int)m_OriginFaces.size())
	{
		return m_OriginFaces[FaceIndex];
	}
	else
	{
		return MMO_Face::s_errorFace;
	}
}
//===================================================================================
MMO_Face & MisMedicOrgan_Ordinary::GetMMOFace_CutPart(int FaceIndex)
{
	if(FaceIndex >= 0 && FaceIndex < (int)m_CutCrossfaces.size())
	{
		return m_CutCrossfaces[FaceIndex];
	}
	else
	{
		return MMO_Face::s_errorFace;
	}
}
//=============================================================================================
MMO_Face & MisMedicOrgan_Ordinary::GetMMOFace(GFPhysSoftBodyFace * physface)
{
     int materialid, faceid;

     ExtractFaceIdAndMaterialIdFromUsrData(physface, materialid, faceid);

     MMO_Face & Organface = (materialid == ORIGINMATERIALID ? GetMMOFace_OriginPart(faceid) : GetMMOFace_CutPart(faceid));

     return Organface;
 }
//==================================================================================
const MMO_Node & MisMedicOrgan_Ordinary::GetMMONode(int NodeIndex)
{
	if(NodeIndex >= 0 && NodeIndex < (int)m_OrganRendNodes.size())
	{
		return m_OrganRendNodes[NodeIndex];
	}
	else
	{
		return MMO_Node::s_errorNode;
	}
}
//==================================================================================
void MisMedicOrgan_Ordinary::SetTimeNeedToEletricCut(float timeneed)
{
	m_TimeNeedToEleCut = timeneed;
}
//==================================================================================
void MisMedicOrgan_Ordinary::SetBloodPointDensity(float density)
{
	m_BloodTrackPointDensity = density;
}
//==================================================================================
void MisMedicOrgan_Ordinary::SetMaxBloodTrackCount(int cmax)
{
	m_MaxBloodTrackCount = cmax;
}
//==================================================================================
int  MisMedicOrgan_Ordinary::GetBloodTrackCount()
{
	if(m_BloodTextureEffect)
	   return m_BloodTextureEffect->GetAllBloodTrack().size();
	else
	   return 0;
}
void MisMedicOrgan_Ordinary::SetBloodScatterValue(float radius)
{
	m_BloodScatterRadius = radius;
}
void MisMedicOrgan_Ordinary::SetBloodScatterRadius(float value)
{
	m_BloddScatterValue = value;
}
//==================================================================================
OrganSurfaceBloodTextureTrack * MisMedicOrgan_Ordinary::GetBloodTrack(int streamIndex)
{
	if(m_BloodTextureEffect == 0)
	   return 0;
	const std::vector<OrganSurfaceBloodTextureTrack*> & vecBloods = m_BloodTextureEffect->GetAllBloodTrack();

	if(streamIndex >= 0 && streamIndex < (int)vecBloods.size())
	   return vecBloods[streamIndex];
	else
	   return 0;
}
//========================================================================================
int MisMedicOrgan_Ordinary::GetBloodTrackStartFaceID(int streamIndex)
{
	OrganSurfaceBloodTextureTrack * btrack = GetBloodTrack(streamIndex);
	
	if(btrack)
	{
		int faceid = btrack->m_StartFaceID;
		
		if(faceid >= 0 && faceid < (int)m_OriginFaces.size())
		{
			return faceid;
		}
	}
	return -1;
}
//==================================================================================

bool MisMedicOrgan_Ordinary::AddBloodTrackAtFace(int faceID)
{
	if(faceID < 0 || faceID >= (int)m_OriginFaces.size())
		return false;
	if(m_OriginFaces[faceID].m_physface == NULL)
		return false;
	float weight[3] = {0.3333f,0.3333f,0.3333f};
	m_BloodTextureEffect->createBloodTrackUseForOrganBeCutFace(this, weight, 1.0f, faceID, m_CreateInfo.m_BloodRadius + 0.002f, 2.0f, 2.5f);
	return true;
}

int MisMedicOrgan_Ordinary::GetNumOfDynamicBlood()
{
	if(m_BloodTextureEffect)
		return m_BloodTextureEffect->GetNumOfDynamicBlood();
	else
		return 0;
}
void MisMedicOrgan_Ordinary::SetBleedRadius(float radius)
{
	m_BleedRadius = radius;
}
//=============================================================================================================================
void MisMedicOrgan_Ordinary::AddInjuryPoint(GFPhysSoftBodyFace * Face, float weights[3], VolumeBlood * vblood)
{
	if (m_CreateInfo.m_CanBlood == false)
		return;

	if (m_InjuryPoints.size() > 10)
	{
		for (int c = 0; c < m_InjuryPoints.size(); c++)
		{
			if (m_InjuryPoints[c].m_VolumeBlood == 0)//remove injury point with out volume blood
			{
				std::swap(m_InjuryPoints[c], m_InjuryPoints[m_InjuryPoints.size()-1]);
				m_InjuryPoints.resize(m_InjuryPoints.size() - 1);
			}
		}	
	}
	if (vblood)//new volume blood mark old as null
	{
		for (int c = 0; c < m_InjuryPoints.size(); c++)
		{
			m_InjuryPoints[c].m_VolumeBlood = 0;
		}
	}
	m_InjuryPoints.push_back(OrganInjuryPoint(Face, weights, vblood));
}
//=============================================================================================================================
/*void MisMedicOrgan_Ordinary::RemoveInjuryPoint(int index)
{

}*/
//=============================================================================================================================
void MisMedicOrgan_Ordinary::RemoveInjuryPointsOnFaces(GFPhysVectorObj<GFPhysSoftBodyFace*> & faces)
{
	int lastIndex = m_InjuryPoints.size() - 1;

	int validIndex = 0;
	
	while (validIndex <= lastIndex)
	{
		int FaceUID = m_InjuryPoints[validIndex].m_FaceUID;
		
		bool finded = false;
		
		for (int f = 0; f < faces.size(); f++)
		{
			if (FaceUID == faces[f]->m_uid)
			{
				finded = true;
				break;
			}
		}

		if (finded)
		{
			if (m_InjuryPoints[validIndex].m_VolumeBlood)
			{
				m_InjuryPoints[validIndex].m_VolumeBlood = 0;
				StopEffect_VolumeBlood();
			}
			std::swap(m_InjuryPoints[validIndex], m_InjuryPoints[lastIndex]);
			lastIndex--;
		}
		else
		{
			validIndex++;
		}
	}
	m_InjuryPoints.resize(validIndex);
}
//==============================================================================
void MisMedicOrgan_Ordinary::RemoveInjuryPoints(const Ogre::Vector2 & tex, float range)
{
	int lastIndex = m_InjuryPoints.size() - 1;

	int validIndex = 0;

	while (validIndex <= lastIndex)
	{
		float dist = (m_InjuryPoints[validIndex].m_BleedTexCoord - tex).length();
		if (dist < range)
		{
			if (m_InjuryPoints[validIndex].m_VolumeBlood)
			{
				m_InjuryPoints[validIndex].m_VolumeBlood = 0;
				StopEffect_VolumeBlood();
			}
			std::swap(m_InjuryPoints[validIndex], m_InjuryPoints[lastIndex]);
			lastIndex--;
		}
		else
		{
			validIndex++;
		}
	}
	m_InjuryPoints.resize(validIndex);
}
//==================================================================================
void MisMedicOrgan_Ordinary::RemovePhysicsPart()
{
	MisMedicOrganInterface::RemovePhysicsPart();

	for(size_t t = 0 ; t < m_OrganAttachments.size();t++)
	{
		MisMedicOrganAttachment* attch = m_OrganAttachments[t];
		m_OrganAttachments[t] = NULL;
        delete attch;
	}
	m_OrganAttachments.clear();

	std::vector<SBleedPoint>::iterator iter;
	for(iter = m_bleednodes.begin(); iter!= m_bleednodes.end(); iter++)
	{
		SBleedPoint& sbp = *iter;
		EffectManager::Instance()->removeVesselBleedEffect(sbp.effect);
		sbp.effect = NULL;
	}

	if(m_physbody)
	{
		if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		{
		   m_OwnerTrain->OrganBeginRemoveFromDynamicWorld(this);
		   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemoveSoftBody(m_physbody);
		}
		delete m_physbody;
		m_physbody = 0;
	}
}

void MisMedicOrgan_Ordinary::RemoveGraphicPart()
{
	if(m_pManualObject)
	{
		((Ogre::SceneNode*)m_pManualObject->getParentNode())->detachObject(m_pManualObject);
#if USEOLDRENDOBJECT
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyManualObject(m_pManualObject);
#else
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyMovableObject(m_pManualObject);
#endif
		m_pManualObject = 0;
	}

	if(m_FFDObject)
	{
		((Ogre::SceneNode*)m_FFDObject->getParentNode())->detachObject(m_FFDObject);
		MXOgreWrapper::Get()->GetDefaultSceneManger()->destroyMovableObject(m_FFDObject);
		m_FFDObject = 0;
	}

	if(m_pSceneNode)
	{
		MXOgreWrapper::Get()->GetDefaultSceneManger()->getRootSceneNode()->removeAndDestroyChild(m_pSceneNode->getName());
		m_pSceneNode = 0;
	}

	m_EffectRender.Destory();
	
	//remove volume blood
	StopEffect_VolumeBlood();
}

void MisMedicOrgan_Ordinary::CreateFFDObject(/*Ogre::SceneManager * scenemgr,*/ Ogre::String name , Ogre::String ffdfile)
{
	Ogre::SceneManager * pSMG =  MXOgre_SCENEMANAGER;

	m_FFDObject = (DynamicSurfaceFreeDeformObject*)pSMG->createMovableObject(name, "DynamicObjRendable");
	
	if(ffdfile != "")
	{
		//destroy old one first
		if(m_pManualObject)
		{
			if(m_pManualObject)
				((Ogre::SceneNode*)m_pManualObject->getParentNode())->detachObject(m_pManualObject);
#if USEOLDRENDOBJECT
			pSMG->destroyManualObject(m_pManualObject);
#else			
			pSMG->destroyMovableObject(m_pManualObject);
#endif
			m_pManualObject = 0;

			if(m_pSceneNode)
			{
				pSMG->getRootSceneNode()->removeAndDestroyChild(m_pSceneNode->getName());
				m_pSceneNode = 0;
			}
		}

		m_FFDObject->LoadFFDFile(ffdfile.c_str());
		
		m_FFDObject->m_useffd = true;
		
		m_FFDObject->InitalizeUndeformedPos(m_physbody);
		//create manual mesh
		static int ffdid = 0;
		ffdid++;

		m_pSceneNode = pSMG->getRootSceneNode()->createChildSceneNode("ffdobj" + Ogre::StringConverter::toString(ffdid));
		m_pSceneNode->attachObject(m_FFDObject);
	}

}

static void GetProjectWeighsInTriangle(const GFPhysVector3 & faceVert0,
	                            const GFPhysVector3 & faceVert1,
	                            const GFPhysVector3 & faceVert2,
								const GFPhysVector3 & extpos,
								Real weights[3])
{
	GFPhysVector3 faceNorml = (faceVert1 - faceVert0).Cross(faceVert2 - faceVert0).Normalized();
	GFPhysVector3 prjPos = extpos + faceNorml * (faceVert0 - extpos).Dot(faceNorml);
	
	//prjPos = ClosestPtPointTriangle(prjPos, faceVert0, faceVert1, faceVert2);

	CalcBaryCentric(faceVert0,
		            faceVert1,
		            faceVert2,
		            extpos,
		            weights[0],
		            weights[1],
		            weights[2]);
	/*GFPhysVector3 u = (faceVert1 - faceVert0);
	GFPhysVector3 v = (faceVert2 - faceVert0);
	GFPhysVector3 p = (prjPos - faceVert0);
	float uv = u.Dot(v);
	float uu = u.Dot(u);
	float vv = v.Dot(v);
	float pv = p.Dot(v);
	float pu = p.Dot(u);*/

}
void MisMedicOrgan_Ordinary::BuildTetrahedronNodeTextureCoord(const GFPhysVector3 & posFaceDir, bool usePosFaceDir)
{
	if (m_IsInnerTexCoordSetted)
		return;

	std::vector<GFPhysSoftBodyTetrahedron*> tetraQueue;
	
	for (int c = 0; c < (int)m_OriginFaces.size(); c++)
	{
		GFPhysSoftBodyFace * face  = m_OriginFaces[c].m_physface;
		
		if (usePosFaceDir)
		{
			if (face->m_RestFaceNormal.Dot(/*GFPhysVector3(0, 1, 0)*/posFaceDir) < 0)
				continue;
		}

		GFPhysGeneralizedFace * genFace = face->m_GenFace;

		GFPhysSoftBodyTetrahedron * tetra = genFace->m_ShareTetrahedrons[0].m_Hosttetra;

		//Ogre::Vector2 * faceTexCoord = m_OriginFaces[c].m_TextureCoord;
		Ogre::Vector2 faceTexCoord[3];
		
		faceTexCoord[0] = Ogre::Vector2(face->m_TexCoordU[0], face->m_TexCoordV[0]);
		faceTexCoord[1] = Ogre::Vector2(face->m_TexCoordU[1], face->m_TexCoordV[1]);
		faceTexCoord[2] = Ogre::Vector2(face->m_TexCoordU[2], face->m_TexCoordV[2]);

		for (int n = 0; n < 4; n++)
		{
			if (tetra->m_TetraNodes[n] == face->m_Nodes[0])
			{
				tetra->m_texCoordU[n] = faceTexCoord[0].x;
				tetra->m_texCoordV[n] = faceTexCoord[0].y;
			}
			
			else if(tetra->m_TetraNodes[n] == face->m_Nodes[1])
			{
				tetra->m_texCoordU[n] = faceTexCoord[1].x;
				tetra->m_texCoordV[n] = faceTexCoord[1].y;
			}
			
			else if(tetra->m_TetraNodes[n] == face->m_Nodes[2])
			{
				tetra->m_texCoordU[n] = faceTexCoord[2].x;
				tetra->m_texCoordV[n] = faceTexCoord[2].y;
			}
			else
			{
				//project
				//physTetraData.m_NodeTexture[n] = m_OriginFaces[c].m_TextureCoord[0];//temp test
				float weights[3];
				GetProjectWeighsInTriangle(face->m_Nodes[0]->m_UnDeformedPos, 
					                       face->m_Nodes[1]->m_UnDeformedPos, 
					                       face->m_Nodes[2]->m_UnDeformedPos, 
					                       tetra->m_TetraNodes[n]->m_UnDeformedPos,
										   weights);

				tetra->m_texCoordU[n] = faceTexCoord[0].x*weights[0] + faceTexCoord[1].x*weights[1] + faceTexCoord[2].x*weights[2];
				tetra->m_texCoordV[n] = faceTexCoord[0].y*weights[0] + faceTexCoord[1].y*weights[1] + faceTexCoord[2].y*weights[2];
			}
		}
		//physTetraData.m_IsTextureSetted = true;
		tetra->m_bTexSetted = true;
		tetraQueue.push_back(tetra);
	}

	while (tetraQueue.size() > 0 )
	{
		int numt = (int)tetraQueue.size();
		
		for (int c = 0; c < numt; c++)
		{
			GFPhysSoftBodyTetrahedron * ParentTetra = tetraQueue[c];

			//PhysTetra_Data & physParentTetra = GetPhysTetraAppData(ParentTetra);

			GFPhysSoftBodyTetrahedron * neighborTetra;
			
			for (int f = 0; f < 4; f++)
			{
				GFPhysGeneralizedFace * genFace = ParentTetra->m_TetraFaces[f];
				
				if (genFace->m_surface)
					continue;

				if (genFace->m_ShareTetrahedrons[0].m_Hosttetra == ParentTetra)
					neighborTetra = genFace->m_ShareTetrahedrons[1].m_Hosttetra;
				else
					neighborTetra = genFace->m_ShareTetrahedrons[0].m_Hosttetra;

				//PhysTetra_Data & physNeighborTetra = GetPhysTetraAppData(neighborTetra);

				if (neighborTetra->m_bTexSetted == false)
				{
					GFPhysVector3 faceNodes[3];
					Ogre::Vector2 facetexcoord[3];
					int nodeIndex = -1;
					int cc = 0;
					for (int n = 0; n < 4; n++)
					{
						if (neighborTetra->m_TetraNodes[n] == ParentTetra->m_TetraNodes[0])
						{
							neighborTetra->m_texCoordU[n] = ParentTetra->m_texCoordU[0];
							neighborTetra->m_texCoordV[n] = ParentTetra->m_texCoordV[0];
							
							faceNodes[cc] = ParentTetra->m_TetraNodes[0]->m_UnDeformedPos;
							facetexcoord[cc].x = ParentTetra->m_texCoordU[0];
							facetexcoord[cc].y = ParentTetra->m_texCoordV[0];
							cc++;
						}
						else if (neighborTetra->m_TetraNodes[n] == ParentTetra->m_TetraNodes[1])
						{
							neighborTetra->m_texCoordU[n] = ParentTetra->m_texCoordU[1];
							neighborTetra->m_texCoordV[n] = ParentTetra->m_texCoordV[1];

							faceNodes[cc] = ParentTetra->m_TetraNodes[1]->m_UnDeformedPos;
							facetexcoord[cc].x = ParentTetra->m_texCoordU[1];
							facetexcoord[cc].y = ParentTetra->m_texCoordV[1];
							cc++;
						}
						else if(neighborTetra->m_TetraNodes[n] == ParentTetra->m_TetraNodes[2])
						{
							neighborTetra->m_texCoordU[n] = ParentTetra->m_texCoordU[2];
							neighborTetra->m_texCoordV[n] = ParentTetra->m_texCoordV[2];

							faceNodes[cc] = ParentTetra->m_TetraNodes[2]->m_UnDeformedPos;
							facetexcoord[cc].x = ParentTetra->m_texCoordU[2];
							facetexcoord[cc].y = ParentTetra->m_texCoordV[2];
							cc++;
						}
						else if (neighborTetra->m_TetraNodes[n] == ParentTetra->m_TetraNodes[3])
						{
							neighborTetra->m_texCoordU[n] = ParentTetra->m_texCoordU[3];
							neighborTetra->m_texCoordV[n] = ParentTetra->m_texCoordV[3];

							faceNodes[cc] = ParentTetra->m_TetraNodes[3]->m_UnDeformedPos;
							facetexcoord[cc].x = ParentTetra->m_texCoordU[3];
							facetexcoord[cc].y = ParentTetra->m_texCoordV[3];
							cc++;
						}
						else
						{
							nodeIndex = n;
							//physNeighborTetra.m_NodeTexture[n] = physParentTetra.m_NodeTexture[0];//temp
						}
					}
					if (nodeIndex >=0)
					{
						float weights[3];
						GetProjectWeighsInTriangle(faceNodes[0],
							                       faceNodes[1],
							                       faceNodes[2],
												   neighborTetra->m_TetraNodes[nodeIndex]->m_UnDeformedPos,
							                       weights);

						neighborTetra->m_texCoordU[nodeIndex] = facetexcoord[0].x*weights[0] + facetexcoord[1].x*weights[1] + facetexcoord[2].x*weights[2];
						neighborTetra->m_texCoordV[nodeIndex] = facetexcoord[0].y*weights[0] + facetexcoord[1].y*weights[1] + facetexcoord[2].y*weights[2];
					}

					neighborTetra->m_bTexSetted = true;
					tetraQueue.push_back(neighborTetra);
				}
				
			}
		}

		tetraQueue.erase(tetraQueue.begin(), tetraQueue.begin() + numt);
	}

	m_IsInnerTexCoordSetted = true;
}
void MisMedicOrgan_Ordinary::CreateOrganObjectFromSerialize(const MisMedicObjetSerializer & serializer  , Ogre::String organObjName)
{
	//give the train a chance to chance to change to parameter
	if(m_OwnerTrain)
	{
	   MisNewTraining * newtrain = dynamic_cast<MisNewTraining*>(m_OwnerTrain);
	   newtrain->SetCustomParamBeforeCreatePhysicsPart(this);
	}

	//
	CreatePhysicsPart(m_CreateInfo , m_CreateInfo.m_objTopologyType == DOT_VOLMESH ? 0 : 1);
	
	//if(m_Serializer.m_NodeTexCoord.size() > 0)
	//{
	   //m_HasVolTexCoors = true;
	   //for(size_t n = 1 ; n < m_PhysNodeData.size() ; n++)//first node is error node
	   //{
		//   m_PhysNodeData[n].m_NodeTextureCoord = m_Serializer.m_NodeTexCoord[n-1];
	  // }
	//}

	m_physbody->SetUserPointer(this);
		//m_physbody->SetVelocityDamping(true , m_CreateInfo.m_veldamping ,m_CreateInfo.m_perelementdamping);//m_lineardamping ,m_CreateInfo.m_angulardampint);

	((GFPhysSoftBodyShape*)m_physbody->GetCollisionShape())->SetMargin(m_CreateInfo.m_collideRSMargin);
	if(m_CreateInfo.m_enableSSCollide == false)
		((GFPhysSoftBodyShape*)m_physbody->GetCollisionShape())->SetSSCollideTag(GFPhysSoftBodyShape::SSCT_NONE);

	//create manual mesh
	static int meshid = 0;
	meshid++;

	Ogre::SceneManager * pSMG =  MXOgre_SCENEMANAGER;

#if USEOLDRENDOBJECT
	m_pManualObject = pSMG->createManualObject(organObjName + Ogre::StringConverter::toString(meshid));
	m_pManualObject->setDynamic(true);
#else
	m_pManualObject = (MisMedicOrganRender*)(pSMG->createMovableObject("MisMedicOrganRender"));
#endif
	m_pManualObject->setVisible(m_Visible);
	m_pManualObject->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);

	if (m_OwnerMaterialPtr->getTechnique(0)->getPass(0)->isTransparent())
	{
		m_pManualObject->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN + 1);//强制透明的器官 比如粘连排序在血池后面
	}
	m_pSceneNode = pSMG->getRootSceneNode()->createChildSceneNode(organObjName + Ogre::StringConverter::toString(meshid));
	if(m_pManualObject)
	   m_pSceneNode->attachObject(m_pManualObject);
	

#if USEOLDRENDOBJECT
	//create section give some empty data
	m_pManualObject->setDynamic(true);
	m_pManualObject->begin(m_materialname , Ogre::RenderOperation::OT_TRIANGLE_LIST);
	for(size_t v = 0 ; v < 3 ; v++)
	{
		m_pManualObject->position(Ogre::Vector3::ZERO);
		m_pManualObject->normal(Ogre::Vector3::UNIT_Z);	
		m_pManualObject->tangent(Ogre::Vector3::UNIT_X);
		m_pManualObject->colour(Ogre::ColourValue::White);
		m_pManualObject->textureCoord(Ogre::Vector2::ZERO);
		//m_pManualObject->textureCoord(Ogre::Vector3::UNIT_Y);
		//m_pManualObject->textureCoord(Ogre::Vector3::UNIT_X);
	}
	m_pManualObject->triangle(0 , 1 , 2);
	m_pManualObject->end();


	m_pManualObject->begin(m_materialname, Ogre::RenderOperation::OT_TRIANGLE_LIST);
	for(int v = 0 ; v < 3 ; v++)
	{	
		m_pManualObject->position(Ogre::Vector3::ZERO);
		m_pManualObject->normal(Ogre::Vector3::UNIT_Z);
		m_pManualObject->tangent(Ogre::Vector3::UNIT_X);
		m_pManualObject->colour(Ogre::ColourValue::White);
		m_pManualObject->textureCoord(Ogre::Vector2::ZERO);
	}
	m_pManualObject->end();
#else
	//if (m_CreateInfo.m_LayerVecs.size() <= 1)
	{
		m_pManualObject->setMaterialName(0, 0, m_materialname);
		m_pManualObject->setMaterialName(0, 1, m_materialname);
	}
	//else
	{
		for (int i = 1; i < m_CreateInfo.m_LayerVecs.size(); i++)//addtional layer material
		{
			int resultIndex = m_pManualObject->AddLayer(m_CreateInfo.m_LayerVecs[i].m_CloneMaterialName);
			if (resultIndex != m_CreateInfo.m_LayerVecs[i].m_layerIndex)
			{
				MessageBoxA(0, "error index", "error", 0);
			}
		}
	}
#endif

	m_pDebugMO = pSMG->createManualObject(organObjName + Ogre::StringConverter::toString(meshid) + "Debug");
	m_pDebugMO->setDynamic(true);
	m_pDebugMO->setVisible(false);
	m_pDebugMO->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
	m_pSceneNodeDebug = pSMG->getRootSceneNode()->createChildSceneNode(organObjName + Ogre::StringConverter::toString(meshid) + "Debug");
	if(m_pDebugMO)
		m_pSceneNodeDebug->attachObject(m_pDebugMO);
// 	m_pDebugMO->begin("default_template_debug" , Ogre::RenderOperation::OT_LINE_LIST);
// 	for(int z = 0 ; z < 3 ; z++)
// 	{	
// 		m_pDebugMO->position(Ogre::Vector3::ZERO);
// 		m_pDebugMO->normal(Ogre::Vector3::UNIT_Z);
// 		m_pDebugMO->colour(Ogre::ColourValue::White);
// 		m_pDebugMO->textureCoord(Ogre::Vector2::ZERO);
// 	}
// 	m_pDebugMO->end();

    if (m_OwnerTrain && m_CreateInfo.m_AttachStaticMesh != "")
    {
        MisNewTraining * newtrain = dynamic_cast<MisNewTraining*>(m_OwnerTrain);
        
        Ogre::Entity * domeStatic = newtrain->m_pOms->GetEntity(m_CreateInfo.m_AttachStaticMesh + "$1", true);

        if (domeStatic)
        {
            Ogre::SceneNode * nodestatic = domeStatic->getParentSceneNode();

            Ogre::MeshPtr& StaticDomeMeshPtr = newtrain->GetStaticDomeMeshPtr();

            if (StaticDomeMeshPtr.isNull() == false && StaticDomeMeshPtr != domeStatic->getMesh())
            {
                Ogre::LogManager::getSingleton().logMessage("Failed, We Temporarily support only one static object to participate in the static vs dynamic connection!");
                exit(EXIT_FAILURE);
            }

            StaticDomeMeshPtr = domeStatic->getMesh();

            MisMedicObjectUnion& StaticDynDomeUnion = newtrain->GetStaticDynDomeUnion();

            StaticDynDomeUnion.AttachStaticMeshToDynamicOrgan(StaticDomeMeshPtr,
                nodestatic,
                nodestatic->getPosition(),
                nodestatic->getOrientation(),
                nodestatic->getScale(),
                this, m_CreateInfo.m_AttachStaticMeshThresHold);
        }
    }
}

#define USEPREBUILDBTNDATA 1
void MisMedicOrgan_Ordinary::UpdateBTNs()
{	
	//
	//set zero
#if(0)
	GFPhysVector3 camPos = OgreToGPVec3(m_OwnerTrain->m_pLargeCamera->getDerivedPosition());

	GFPhysSoftBodyNode * SoftNode = m_physbody->GetSoftBodyShape().GetNodeList();
	while (SoftNode)
	{
		if (SoftNode->m_insurface == true)
		{
			SoftNode->m_Normal = GFPhysVector3(0, 0, 0);//temp
		}
		SoftNode = SoftNode->m_Next;
	}
	
	GFPhysSoftBodyFace * Face = m_physbody->GetSoftBodyShape().GetFaceList();
	while (Face)
	{
		//update face normal first
			float weight0 = Face->m_NodeAngleWeight[0];
			float weight1 = Face->m_NodeAngleWeight[1];
			float weight2 = Face->m_NodeAngleWeight[2];

			GFPhysVector3 viewDir = (camPos - Face->m_Nodes[0]->m_CurrPosition).Normalized();
			float dotv = GPClamped(viewDir.Dot(Face->m_FaceNormal), -1.0f, 1.0f);
			if (dotv < 0)
			{
				float fadeout = 0.65f;// (1 + dotv);
				weight0 *= fadeout;
				weight1 *= fadeout;
				weight2 *= fadeout;
			}

			//add to node normal
			Face->m_Nodes[0]->m_Normal += Face->m_FaceNormal * weight0;
			Face->m_Nodes[1]->m_Normal += Face->m_FaceNormal * weight1;
			Face->m_Nodes[2]->m_Normal += Face->m_FaceNormal * weight2;
		
		Face = Face->m_Next;
	}
	

	//normalize node normal
	SoftNode = m_physbody->GetSoftBodyShape().GetNodeList();
	while (SoftNode)
	{
		if (SoftNode->m_insurface == true)
		{
			Real norLen = SoftNode->m_Normal.Length();
			if (norLen > GP_EPSILON)
				SoftNode->m_Normal /= norLen;
			else
				SoftNode->m_Normal = GFPhysVector3(0, 0, 1);//temp
		}
		SoftNode = SoftNode->m_Next;
	}
	
#endif
	//
	
	for(size_t n = 0 ; n < m_OrganRendNodes.size() ; n++)
	{
		m_OrganRendNodes[n].m_Tangent = Ogre::Vector3::ZERO;
		PhysNode_Data & physNodeData = m_PhysNodeData[m_OrganRendNodes[n].m_NodeDataIndex];
		physNodeData.m_AvgNormal = GPVec3ToOgre(physNodeData.m_PhysNode->m_Normal);
	}

	

	int OriginFaceNum = m_OriginFaces.size();
	int CutFaceNum = m_CutCrossfaces.size(); 

	for (int f = 0; f < OriginFaceNum + CutFaceNum; ++f)
	{
		const MMO_Face & face = (f < OriginFaceNum ? m_OriginFaces[f] : m_CutCrossfaces[f - OriginFaceNum]);
		
		int ia = face.vi[0];
		
		int ib = face.vi[1];
		
		int ic = face.vi[2];

		bool needRend = face.m_NeedRend;

		if (ia < 0 || ib < 0 || ic < 0 || (face.m_physface == 0) || needRend == false)//skip invalid face
		{
			continue;
		}

		Ogre::Vector3 v1 = m_OrganRendNodes[ia].m_CurrPos;
				
		Ogre::Vector3 v2 = m_OrganRendNodes[ib].m_CurrPos;
				
		Ogre::Vector3 v3 = m_OrganRendNodes[ic].m_CurrPos;

		Ogre::Vector3 side0 = v1-v2;

		Ogre::Vector3 side1 = v3-v1;

		//update bionormal and tangent
#if(USEPREBUILDBTNDATA)
		Ogre::Real deltaV0 = face.m_deltaV0;

		Ogre::Real deltaV1 = face.m_deltaV1;

		Ogre::Real deltaU0 = face.m_deltaU0;

		Ogre::Real deltaU1 = face.m_deltaU1;
#else
		Ogre::Vector2  uv1 = face.m_TextureCoord[0];//m_texcoords[face.a];
				
		Ogre::Vector2  uv2 = face.m_TextureCoord[1];//m_texcoords[face.b];
				
		Ogre::Vector2  uv3 = face.m_TextureCoord[2];//m_texcoords[face.c];

		Ogre::Real deltaV0 = uv1.y - uv2.y;

		Ogre::Real deltaV1 = uv3.y - uv1.y;

		Ogre::Real deltaU0 = uv1.x - uv2.x;

		Ogre::Real deltaU1 = uv3.x - uv1.x;
#endif
		Ogre::Vector3 tangent = deltaV1 * side0 - deltaV0 * side1;
		//float coeff = -(deltaU0 * deltaV1 - deltaV0*deltaU1);
		//tangent = Ogre::Vector3(tangent.x/coeff, tangent.y/coeff, tangent.z/coeff);
		tangent.normalise();

		m_OrganRendNodes[ia].m_Tangent += tangent;
		m_OrganRendNodes[ib].m_Tangent += tangent;
		m_OrganRendNodes[ic].m_Tangent += tangent;
	}

#if(0)
	//update tagent of cut cross node
	for (size_t f = 0; f < m_CutCrossfaces.size(); ++f)
	{
		const MMO_Face & face = m_CutCrossfaces[f];

		int ia = face.vi[0];

		int ib = face.vi[1];

		int ic = face.vi[2];

		bool needRend = face.m_NeedRend;

		if (ia < 0 || ib < 0 || ic < 0 || (face.m_physface == 0) || needRend == false)//skip invalid face
		{
			continue;
		}
	}
	//
#endif
}

void MisMedicOrgan_Ordinary::UpdateMesh(const std::vector<ShadowNodeForLinkage> & LinkNodes)
{
	if(m_FFDObject)
	{
		m_FFDObject->updateS4mFFD(m_physbody , m_materialname);
		return;
	}
	
	Ogre::SceneManager * pSMG =  MXOgre_SCENEMANAGER;
	Ogre::Camera * pCamera = CTrainingMgr::Instance()->GetCurTraining()->m_pLargeCamera;
	int dislayMode = pCamera->getPolygonMode();


	/*
	int materialId = getFlag_MaterialId();
	Ogre::String newmaterialname = m_CreateInfo.m_materialname[materialId];
#if 1
	if (newmaterialname == "")
	{
		newmaterialname = m_CreateInfo.m_materialname[0];
	}

	if (newmaterialname!= materialname)
	{
		materialname = newmaterialname;
		m_pManualObject->setMaterialName(0, materialname);
	}
#else
	if (newmaterialname!= materialname && newmaterialname != "")
	{
		materialname = newmaterialname;
		m_pManualObject->setMaterialName(0, materialname);
	}

#endif
	*/
	
	//normalize BTN and copy normal to rend node
	int NumPhysNodeData = (int)m_PhysNodeData.size();

	for (size_t n = 0; n < m_OrganRendNodes.size(); n++)
	{
		//GFPhysVector3 PhysNodeNormal = m_OrganRendNodes[n].m_PhysNode->m_Normal;
		int NodeDataIndex = m_OrganRendNodes[n].m_NodeDataIndex;

		if(NodeDataIndex >= 0 && NodeDataIndex < NumPhysNodeData)
		{
			PhysNode_Data & nodeData = m_PhysNodeData[NodeDataIndex];

			if (nodeData.m_ShadowNodeIndex >= 0)//this is a node linked with other node in scene
			{
				m_OrganRendNodes[n].m_CurrPos = LinkNodes[nodeData.m_ShadowNodeIndex].m_AvgPosition;
				m_OrganRendNodes[n].m_Normal = LinkNodes[nodeData.m_ShadowNodeIndex].m_AvgNormal;
			}
            else
            {
                m_OrganRendNodes[n].m_CurrPos = GPVec3ToOgre(nodeData.m_PhysNode->m_CurrPosition);
                m_OrganRendNodes[n].m_Normal = nodeData.m_AvgNormal;
            }
			
			m_OrganRendNodes[n].m_Color.a = nodeData.m_LayerBlendFactor;

			m_OrganRendNodes[n].m_Color.g = 0.0f;// GPClamped(nodeData.m_burnValue, 0.0f, 1.0f);

			Ogre::Real tannormlength  = m_OrganRendNodes[n].m_Tangent.length();

			if (tannormlength < 1e-06)
			{
				m_OrganRendNodes[n].m_Tangent = m_OrganRendNodes[n].m_Normal.perpendicular();//Ogre::Vector3(0, 1 , 0);
			}
			else
			{
				tannormlength = 1.0f / tannormlength;
				m_OrganRendNodes[n].m_Tangent *= tannormlength;
			}
		}
		else
		{
			int i = 0;
			int j = i+1;
		}

	}

	Ogre::ColourValue vertexColorBurned(m_BurnNodeColor.r , m_BurnNodeColor.g , m_BurnNodeColor.g , 1);
	Ogre::ColourValue vertexColorNotBurn(m_BurnNodeColor.r , m_BurnNodeColor.g , m_BurnNodeColor.g , 0);

	//Update Dynamic Object
#if USEOLDRENDOBJECT
	m_pManualObject->beginUpdate(0);
	m_pManualObject->estimateVertexCount(m_OrganRendNodes.size()+100);
	for(size_t v = 0 ; v  < m_OrganRendNodes.size() ; v++)
	{
		GFPhysSoftBodyNode * physNode = m_OrganRendNodes[v].m_PhysNode;//PhysNode_Data & nodeData = m_PhysNodeData[m_OrganRendNodes[v].m_NodeDataIndex];

		m_pManualObject->position(m_OrganRendNodes[v].m_CurrPos);
		
		m_pManualObject->normal(m_OrganRendNodes[v].m_Normal);
		
		m_pManualObject->textureCoord(m_OrganRendNodes[v].m_TextureCoord);
		
		m_pManualObject->tangent(m_OrganRendNodes[v].m_Tangent);

		m_pManualObject->colour(m_OrganRendNodes[v].m_Color);
	}

	for (size_t f = 0 ; f < m_OriginFaces.size(); f++)
	{
		int vid[3];
		vid[0] = m_OriginFaces[f].vi[0];
		vid[1] = m_OriginFaces[f].vi[1];
		vid[2] = m_OriginFaces[f].vi[2];
		if(m_OriginFaces[f].m_physface && m_OriginFaces[f].m_NeedRend)
		{
		   m_pManualObject->triangle(vid[0] , vid[1] , vid[2]);
		}
		else if(m_OriginFaces[f].m_physface && !m_OriginFaces[f].m_NeedRend)
		{
			m_pManualObject->triangle(vid[0] , vid[0] , vid[0]);		//update buffer
		}
	}

	//test
	for (size_t f = 0 ; f < m_CutCrossfaces.size(); f++)
	{
		int vid[3];
		const MMO_Face & face = m_CutCrossfaces[f];
		vid[0] = face.vi[0];
		vid[1] = face.vi[1];
		vid[2] = face.vi[2];
		if(face.m_physface)
		{
			m_pManualObject->triangle(vid[0] , vid[1] , vid[2]);
		}
	}
	m_pManualObject->end();
#else
	GFPhysVector3 aabbMin, aabbMax;

	m_physbody->GetSoftBodyShape().GetAabb(m_physbody->GetWorldTransform() , aabbMin , aabbMax);

	m_pManualObject->UpdateVertexBuffer(m_OrganRendNodes, GPVec3ToOgre(aabbMin) , GPVec3ToOgre(aabbMax));
	
	if (m_pManualObject->IsIndexBufferDirty())
	{
		m_pManualObject->UpdateIndexBuffer(m_OriginFaces, m_CutCrossfaces);
	}
#endif
	//m_pManualObject->beginUpdate(1);
	//if(m_CutCrossfaces.size() > 0)
	{
		//m_pManualObject->begin(m_CreateInfo.m_CutMaterialName , Ogre::RenderOperation::OT_TRIANGLE_LIST);

		/*
		for(size_t f = 0 ; f < m_CutCrossfaces.size() ; f++)
		{
			const MMO_Face & face = m_CutCrossfaces[f];

			if(face.m_physface)
			{	
				GFPhysSoftBodyNode * NodesFace[3];
				NodesFace[0] = face.m_physface->m_Nodes[0];
				NodesFace[1] = face.m_physface->m_Nodes[1];
				NodesFace[2] = face.m_physface->m_Nodes[2];

				Ogre::Vector2 texturecoord[3];

				//if(m_HasVolTexCoors)
				//{
				texturecoord[0] = face.m_TextureCoord[0];
				texturecoord[1] = face.m_TextureCoord[1];
				texturecoord[2] = face.m_TextureCoord[2];
				//}
				//else
				//{
					//texturecoord[0] = Ogre::Vector2(0,0);//testface.m_TextureCoord[0];

					//texturecoord[1] = Ogre::Vector2(0,0);//testface.m_TextureCoord[1];

					//texturecoord[2] = Ogre::Vector2(0,0);//testface.m_TextureCoord[2];

				//}
				
				for(int v = 0 ; v < 3 ; v++)
				{
					int RendIndex = face.vi[v];

					Ogre::Vector3 vertex    = m_OrganRendNodes[RendIndex].m_CurrPos;//NodesFace[v]->m_CurrPosition;

					Ogre::Vector3 vernormal = m_OrganRendNodes[RendIndex].m_Normal;//NodesFace[v]->m_Normal;

					Ogre::Vector3 tangent = vernormal.perpendicular();

					m_pManualObject->position(vertex);//(vertex.x(), vertex.y(), vertex.z());

					m_pManualObject->normal(vernormal);//Ogre::Vector3(vernormal.x() , vernormal.y() , vernormal.z()));

					m_pManualObject->tangent(tangent);

					Ogre::ColourValue VertexColor(0 , 0 , 0 , 0);

					PhysNode_Data & nodeData = GetPhysNodeData(NodesFace[v]);
					
					float bloodv = GPClamped(nodeData.m_bloodValue , 0.0f , 1.0f);
					
					float burnv  = GPClamped(nodeData.m_burnValue  , 0.0f , 1.0f);

					VertexColor.g = burnv;

					VertexColor.r = bloodv;

					VertexColor.a = nodeData.m_LayerBlendFactor;
					
					m_pManualObject->colour(VertexColor);
					
					m_pManualObject->textureCoord(texturecoord[v]);
				}
			}
		}*/
		
	}
	//m_pManualObject->end();

	if (2 == dislayMode)
	{
#if(0)
		m_pDebugMO->setVisible(true);
		m_pDebugMO->clear();
		m_pDebugMO->begin("default_template_debug" , Ogre::RenderOperation::OT_LINE_LIST);

		GFPhysSoftBodyEdge * edge = m_physbody->GetEdgeList();
		while(edge)
		{
			if (edge->m_Nodes[0]->m_Flag!= 0 || edge->m_Nodes[1]->m_Flag!= 0 )
			{
			Ogre::Vector3 vec_0 = Ogre::Vector3(edge->m_Nodes[0]->m_CurrPosition.GetX(), edge->m_Nodes[0]->m_CurrPosition.GetY(), edge->m_Nodes[0]->m_CurrPosition.GetZ());
			Ogre::Vector3 vec_1 = Ogre::Vector3(edge->m_Nodes[1]->m_CurrPosition.GetX(), edge->m_Nodes[1]->m_CurrPosition.GetY(), edge->m_Nodes[1]->m_CurrPosition.GetZ());
			m_pDebugMO->position(vec_0);
			m_pDebugMO->position(vec_1);
			}
			edge = edge->m_Next;
		}

// 		GFPhysSoftBodyTetrahedron * tet = m_physbody->GetTetrahedronList();
// 		while(tet)
// 		{
// 			//int ss = tet->getUserIndex(0xff0000);
// 			//if (ss == 0x10000)
// 			{
// 				Ogre::Vector3 vec_0 = Ogre::Vector3(tet->m_TetraNodes[0]->m_CurrPosition.GetX(), tet->m_TetraNodes[0]->m_CurrPosition.GetY(), tet->m_TetraNodes[0]->m_CurrPosition.GetZ());
// 				Ogre::Vector3 vec_1 = Ogre::Vector3(tet->m_TetraNodes[1]->m_CurrPosition.GetX(), tet->m_TetraNodes[1]->m_CurrPosition.GetY(), tet->m_TetraNodes[1]->m_CurrPosition.GetZ());
// 				Ogre::Vector3 vec_2 = Ogre::Vector3(tet->m_TetraNodes[2]->m_CurrPosition.GetX(), tet->m_TetraNodes[2]->m_CurrPosition.GetY(), tet->m_TetraNodes[2]->m_CurrPosition.GetZ());
// 				Ogre::Vector3 vec_3 = Ogre::Vector3(tet->m_TetraNodes[3]->m_CurrPosition.GetX(), tet->m_TetraNodes[3]->m_CurrPosition.GetY(), tet->m_TetraNodes[3]->m_CurrPosition.GetZ());
// 				m_pDebugMO->position(vec_0);
// 				m_pDebugMO->position(vec_1);
// 				m_pDebugMO->position(vec_0);
// 				m_pDebugMO->position(vec_2);
// 				m_pDebugMO->position(vec_0);
// 				m_pDebugMO->position(vec_3);
// 				m_pDebugMO->position(vec_1);
// 				m_pDebugMO->position(vec_2);
// 				m_pDebugMO->position(vec_1);
// 				m_pDebugMO->position(vec_3);
// 				m_pDebugMO->position(vec_2);
// 				m_pDebugMO->position(vec_3);
// 			}
// 			tet = tet->m_Next;
// 		}
		m_pDebugMO->end();
#endif
	}
	else
	{
		m_pDebugMO->setVisible(false);
	}
}
//======================================================================================================
void MisMedicOrgan_Ordinary::UpdateBurn(float dt)
{
    //m_painting.Update(dt);
	CTool * leftTool = (CTool*)(m_OwnerTrain->m_pToolsMgr->GetLeftTool());
	CTool * rightTool = (CTool*)(m_OwnerTrain->m_pToolsMgr->GetRightTool());

	if (leftTool && leftTool->IsInElecCutting(this))
	{
		m_IsThisFrameInElecCutTouch = true;
	}
	else if (rightTool && rightTool->IsInElecCutting(this))
	{
		m_IsThisFrameInElecCutTouch = true;
	}
	else if (leftTool && leftTool->IsInElecCogulation(this))
	{
		m_IsThisFrameInElecCogTouch = true;
	}
	else if (rightTool && rightTool->IsInElecCogulation(this))
	{
		m_IsThisFrameInElecCogTouch = true;
	}

	if(m_IsThisFrameInElecCogTouch == false)
	{
		if(m_ElecBurnKeepTime > 0)//发送电凝结束事件
		{
		   MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_ElecCoagEnd, 0, 0);
		   pEvent->SetOrgan(this);
		   pEvent->m_DurationTime = m_ElecBurnKeepTime;
		   
		   CMXEventsDump::Instance()->PushEvent(pEvent , true);
		}
		m_ElecBurnKeepTime = 0;
	}

	if(m_IsThisFrameInElecCutTouch == false)
	{
		if( m_ElecCutKeepTime > 0)//发送电切结束事件
		{
			MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_ElecCutEnd , 0 , 0);
			pEvent->SetOrgan(this);
			pEvent->m_DurationTime = m_ElecCutKeepTime;
			CMXEventsDump::Instance()->PushEvent(pEvent , true);
		}
		m_ElecCutKeepTime = 0;

		m_ContiueElecCutTime = 0;

		//m_BurnCreatedBlood = 0;
	}
	
	//ReSet state to false
	m_IsThisFrameInElecCogTouch = false;
	m_IsThisFrameInElecCutTouch = false;
}
void MisMedicOrgan_Ordinary::PreUpdateScene(float dt , Ogre::Camera * camera)
{
	MisMedicOrganInterface::PreUpdateScene(dt , camera);
	if(m_FFDObject)
	   return;
	//Get Rend Node position From Physics Node first
	for(size_t n = 0 ; n < m_OrganRendNodes.size() ; n++)
	{
		GFPhysVector3 tempPos = m_OrganRendNodes[n].m_PhysNode->m_CurrPosition;
		m_OrganRendNodes[n].m_CurrPos = Ogre::Vector3(tempPos.x(), tempPos.y() , tempPos.z());
	}
	
	//Update BioNormal Tangent and Normal
	UpdateBTNs();
}
//======================================================================================================

void MisMedicOrgan_Ordinary::UpdateScene(float dt, Ogre::Camera * camera)
{
	MisMedicOrganInterface::UpdateScene(dt, camera);
	
	m_FrameElapsedTime = dt;

	m_TimeSinceLastElectricMelt += dt;
	if(m_Visible == true)
	{
		UpdateMesh(((MisNewTraining*)m_OwnerTrain)->GetNodesLinkData());
	}
	
	

	UpdateBurn(dt);

	//new organ blood
	if (m_VolumeBlood) {
		if (m_VolumeBlood->updateCutInfoByOrganFace()) {
			m_VolumeBlood->step(0.002);
			m_VolumeBlood->draw(this->m_OwnerTrain);
		}
		else {
			StopEffect_VolumeBlood();
		}
	}
	if (m_InjuryPoints.size() > 0)
	{  
	    std::vector<Ogre::Vector2> bleedTexCoords;
		for (int c = 0; c < (int)m_InjuryPoints.size(); c++)
		{
			bleedTexCoords.push_back(m_InjuryPoints[c].m_BleedTexCoord);
		}
	    Ogre::TexturePtr bleedTex = Ogre::TextureManager::getSingleton().load("surface_bleed2.dds", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

		m_EffectRender.ApplyBleedings(bleedTexCoords, m_BleedRadius, 5.0, bleedTex);
		
		/*
		Ogre::TexturePtr m_Tex = Ogre::TextureManager::getSingleton().load("partcl_bleed_new.dds", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		m_frameCount++;
		//if (m_frameCount % 50 < 25) {
			for (int c = 0; c < m_InjuryPoints.size(); c++)
			{   // (m_frameCount % 50) / 3.0   
				this->ApplyEffect_Bleeding(m_InjuryPoints[c].m_BleedTexCoord, 0, m_frameCount % 100 * 0.005f, m_Tex);

			}
		//}
		*/


	}

	if (m_pManualObject)
	    m_EffectRender.BloodBurnSpread(m_pManualObject, this->GetCreateInfo().m_CrossDir);
	//

	//old blood system
	if(m_BloodSystem)
	{
	   m_BloodSystem->Update(dt);
	}
	//if(m_WaterSystem)
	//{
	//	m_WaterSystem->Update(dt);
	//}


	//BloodScatterInCutCrossFace(dt);

	

	std::vector<int> removeTrackIDs;
	m_BloodTextureEffect->StopBloodsTooOlder(m_MaxBloodTrackExistsTime);
	bool LiquidBloodUpdated = m_BloodTextureEffect->Update(dt , removeTrackIDs);
	OnStopBleeding(removeTrackIDs);

	bool LiquidWaterUpdated = m_WaterTextureEffect->Update(dt);

	bool needCompose = false;
	


	if(LiquidBloodUpdated || LiquidWaterUpdated)
	{
	   needCompose = needCompose||m_EffectRender.RendLiquidParticles(*m_BloodTextureEffect , *m_WaterTextureEffect , dt);
	}
	
	//__/__
	
	//if(updated)
	//{
	   //needCompose = needCompose||m_EffectRender.RendWaterParticles(*m_WaterTextureEffect,dt);
	//}
	//if(needCompose)
	//   m_EffectRender.ComposeEffectTexture(Ogre::ColourValue(1.0f , 1.0f ,1.0f ,1.0f));

	m_TotalElapsedTime += dt;

	if (m_CreateInfo.m_CutActionParticleParam[2] != -1)
	{
		std::vector<SBleedPoint>::iterator iter;
		for(iter = m_bleednodes.begin(); iter!= m_bleednodes.end(); )
		{
			SBleedPoint& sbp = *iter;
			if (sbp.effect == NULL)
			{
				int bleedTime;

				int attachnum = 0;
				for(size_t i = 0 ; i < m_OrganAttachments.size() ; i++)
				{
					MisMedicTitaniumClampV2* attch = dynamic_cast<MisMedicTitaniumClampV2*>(m_OrganAttachments[i]);
					if (attch)
						attachnum++;
				}

				if(attachnum == 0)
				{
					bleedTime = m_CreateInfo.m_CutActionParticleParam[0];
				}
				else
				{
					bleedTime = m_CreateInfo.m_CutActionParticleParam[1];
				}

				CVesselBleedEffect* eff = EffectManager::Instance()->createVesselBleedEffect(this);
				eff->initOneBleedPoint(bleedTime, MXOgre_SCENEMANAGER, sbp.node, m_CreateInfo.m_CutActionParticleParam[2], m_strVesselBleedEffectTemplateName);
				sbp.effect = eff;
				iter++;
			}
			else
			{
				bool alive = sbp.effect->Update(dt);
				if (!alive)
				{
					EffectManager::Instance()->removeVesselBleedEffect(sbp.effect);
					sbp.effect = NULL;
					iter = m_bleednodes.erase(iter);
				}
				else
					iter++;
			}
		}
	}

	//UpdateNonWoundBlood(dt);


	//if (m_fToolsActionTime < 0)
	//{
		//m_fToolsActionTime = 0;
	//}

	//if (m_bCutAction)
	//{

		//m_fToolsActionTime += dt;
		//m_bCutAction = false;
	//}
	//else
		//m_fToolsActionTime -=dt;


}
//===================================================================================================================
void MisMedicOrgan_Ordinary::HeatTetrahedrons(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & tetrasToHeat , float deltavalue)
{
	std::set<GFPhysSoftBodyNode*> NodesToApplyHeat;

	for(size_t t = 0 ; t < tetrasToHeat.size(); t++)
	{
		GFPhysSoftBodyTetrahedron * tetra = tetrasToHeat[t];
		
		for(int n = 0 ; n < 4 ; n++)
		{
			NodesToApplyHeat.insert(tetra->m_TetraNodes[n]);
		}
	}

	std::set<GFPhysSoftBodyNode*>::iterator itor = NodesToApplyHeat.begin();
	
	while(itor != NodesToApplyHeat.end())
	{
		GFPhysSoftBodyNode * sNode = (*itor);
		
		PhysNode_Data & physData = GetPhysNodeData(sNode);

		//physData.m_burnValue  += deltavalue;
		
		//physData.m_bloodValue -= deltavalue;

		//give some max value
		//GPClamp(physData.m_burnValue , 0.0f , 100.0f);
		
		//GPClamp(physData.m_bloodValue , 0.0f , 100.0f);

		itor++;
	}
}
//===================================================================================================================
void MisMedicOrgan_Ordinary::StopAroundTexBloodGradual(const Ogre::Vector2 & centerTexCoord)
{
	Ogre::Vector2 tracklocateInTex;

	OrganSurfaceBloodTextureTrack * bs = m_BloodTextureEffect->GetNearestTextureBloodTrackIndex(centerTexCoord , tracklocateInTex);

	if(bs && (tracklocateInTex-centerTexCoord).length() < m_CreateInfo.m_BurnRadius)
	{
		bool isStop = bs->ScaleBloodDropRadius(0.96f);
		if(isStop)
			OnStopBleeding(bs->m_BloodTrackId);

		//if scorch opacity is small than 2.0 then continuous 
		float maxopacity = 3.0f;
		if(bs->m_scorchTransparent < maxopacity-FLT_EPSILON)
		{
			float old   = bs->m_scorchTransparent;

			float delta = 0.1f;//BurnValue*2.0f;

			bs->m_scorchTransparent += delta;

			if(bs->m_scorchTransparent > maxopacity)
			   bs->m_scorchTransparent = maxopacity;

			delta = bs->m_scorchTransparent-old;

			//m_EffectRender.ApplyScorchEffect(tracklocateInTex , m_congulateradius , delta);
		}
	}
}
//===================================================================================================================
void MisMedicOrgan_Ordinary::Tool_InElec_TouchFacePoint(ITool * tool , GFPhysSoftBodyFace * TouchedFace , float TouchWeights[3] , int touchtype  , float dt)
{
	if(touchtype == 0)
	   m_IsThisFrameInElecCogTouch = true;
	
	else if(touchtype == 1)
	   m_IsThisFrameInElecCutTouch = true;
	
	int IdInOrigidnFace = GetOriginFaceIndexFromUsrData(TouchedFace);
		
	float BurnValue = dt*m_CreateInfo.m_BurnRation;
	
	GPClamp(BurnValue , 0.0f , 1.0f);
	
	GFPhysVector3 BurnPosUndeformSpace;
	
	Ogre::Vector2 BurnTextureCoord;

	bool succedBurnWhite = ApplyEffect_BurnWhite(dynamic_cast<CTool*>(tool) , 
												 TouchedFace , 
												 TouchWeights , 
												 BurnValue , 
												 BurnTextureCoord ,
												 m_IsThisFrameInElecCogTouch ? m_ElecBurnKeepTime : m_ElecCutKeepTime);
	BurnPosUndeformSpace = TouchedFace->m_Nodes[0]->m_UnDeformedPos*TouchWeights[0]
						  +TouchedFace->m_Nodes[1]->m_UnDeformedPos*TouchWeights[1]
						  +TouchedFace->m_Nodes[2]->m_UnDeformedPos*TouchWeights[2];

	
	
	
	RemoveInjuryPoints(BurnTextureCoord , m_CreateInfo.m_BurnRadius);

	
	std::vector<GFPhysSoftBodyTetrahedron*> tetrasAround;

	if (CanBeCut())
	{
		uint32 tetraLayerCategory = TouchedFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra->m_ElementCategory;

		CollectTetrasAroundPoint(BurnPosUndeformSpace, m_ElecCutRadius, tetrasAround , tetraLayerCategory, false);

		ShrinkTetrahedrons(tetrasAround, GFPhysVector3(0, 0, 1), m_BurnShrinkRate);
	}
	//logic process start -- blood etc
	if(touchtype == 0)//电凝止血
	{
		if(succedBurnWhite)
			StopAroundTexBloodGradual(BurnTextureCoord);
		
		CElectricHook *elechook = dynamic_cast<CElectricHook*>(tool);
		if(elechook != NULL)
		{
// 			Ogre::Vector3 sparkPos = GPVec3ToOgre(TouchedFace->m_Nodes[0]->m_CurrPosition * TouchWeights[0] +
// 				TouchedFace->m_Nodes[1]->m_CurrPosition * TouchWeights[1] +
// 				TouchedFace->m_Nodes[2]->m_CurrPosition * TouchWeights[2]);

			elechook->EmitSpark(true , 20);
		}

		//发送电凝开始事件
		if(m_ElecBurnKeepTime < FLT_EPSILON)
		{
			MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_ElecCoagStart, tool, 0);
			pEvent->SetOrgan(this);
			pEvent->m_OperatorFaces.push_back(TouchedFace);
			pEvent->m_DurationTime = dt;
			
			CMXEventsDump::Instance()->PushEvent(pEvent , true);
		}
		else//发送持续电凝事件
		{
			MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_ElecCoagKeep, tool, 0);
			pEvent->SetOrgan(this);
			pEvent->m_OperatorFaces.push_back(TouchedFace);
			pEvent->m_DurationTime = dt;
			
			CMXEventsDump::Instance()->PushEvent(pEvent , true);
		}
		m_ElecBurnKeepTime += dt;

		if (m_ElecBurnKeepTime > m_TimeNeedToEleCut * 2.0f)
		{
			PerformElectricCut(tool, TouchedFace, TouchWeights);
		}
	}
	else if(touchtype == 1)//电切
	{
		if (m_ContiueElecCutTime == 0 || (BurnPosUndeformSpace - m_RefBurnPoint).Length() > m_ElecCutThreshold)//start burn or too long dist with base point
		{
			m_RefBurnPoint = BurnPosUndeformSpace;
			m_ContiueElecCutTime = dt;//new point start reset to dt
		}
		else
		{
			m_ContiueElecCutTime += dt;
		}

		if (m_ContiueElecCutTime > m_TimeNeedToEleCut)
		{
			//不同的电切形式
			if(CanBeCut() == false || tool->GetType() == "DissectingForceps")//流血 //用弯分离钳尖端电只出血
			{
				if(IdInOrigidnFace >= 0)
				{
					//GFPhysSoftBodyFace * face = m_OriginFaces[IdInOrigidnFace].m_physface;
					//if(face)
					//  createBloodTrack(face , TouchWeights);
				}
			}
			else//断离组织
			{
				PerformElectricCut(tool, TouchedFace , TouchWeights);
			}
				
			m_ContiueElecCutTime = 0;
		}
		
		if(m_ElecCutKeepTime < FLT_EPSILON)//发送电切开始事件
		{
			MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_ElecCutStart, tool, 0);
			pEvent->SetOrgan(this);
			pEvent->m_OperatorFaces.push_back(TouchedFace);
			pEvent->m_DurationTime = dt;
			
			CMXEventsDump::Instance()->PushEvent(pEvent , true);
		}
		else//发送电切持续事件
		{
			MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_ElecCutKeep, tool, 0);
			pEvent->SetOrgan(this);
			pEvent->m_OperatorFaces.push_back(TouchedFace);
			pEvent->m_DurationTime = dt;
			
			CMXEventsDump::Instance()->PushEvent(pEvent , true);
		}
		m_ElecCutKeepTime += dt;
	}
}
//===================================================================================================================
void MisMedicOrgan_Ordinary::InjectSomething(ITool *tool , GFPhysSoftBodyFace * face , float weights[3] , float dt , std::vector<Ogre::Vector2> & resultUv)
{
	float whiteValue = dt*m_CreateInfo.m_BurnRation * 0.5;
	GPClamp(whiteValue , 0.0f , 1.0f);
	//	ApplyEffect_InjectWhite(dynamic_cast<CTool*>(tool) , face , weights , whiteValue ,dt , resultUv);
}
//===================================================================================================================
void MisMedicOrgan_Ordinary::ToolElectricClampedFaces(ITool * tool, const std::vector<Ogre::Vector2> & touchfaces, const std::vector<Ogre::Vector2> & TFUV, float dt)
{
	std::vector<int> removeTrackIds;
	m_BloodTextureEffect->RemoveBloodTrack(touchfaces , removeTrackIds);
	OnStopBleeding(removeTrackIds);
	m_EffectRender.ApplyCongulate_type_2(touchfaces, TFUV, tool);

	//发送电凝开始事件
	{
		MxToolEvent * pEvent = 0;//
		if (m_ElecBurnKeepTime < FLT_EPSILON)
		{
			pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_ElecCoagStart, tool, 0);
		}
		else//发送持续电凝事件
		{
			pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_ElecCoagKeep, tool, 0);
		}

		pEvent->SetOrgan(this);
		pEvent->m_DurationTime = dt;
		CMXEventsDump::Instance()->PushEvent(pEvent, true);

		m_ElecBurnKeepTime += dt;

		m_IsThisFrameInElecCogTouch = true;
	}
}

//void MisMedicOrgan_Ordinary::ToolWithElectricTouched_uv_3(ITool * tool , const std::vector<Ogre::Vector2> & touchfaces, const std::vector<Ogre::Vector2> & TFUV , float dt , int n , Ogre::Real dU , Ogre::Real dV)
//{
	
//}

//=============================================================================================================
void MisMedicOrgan_Ordinary::ToolPunctureSurface(ITool * tool , GFPhysSoftBodyFace * face , const float weights[3])
{
	if (!m_CanBePuncture || !m_CreateInfo.m_CanBlood)
	{
		return;
	}

	int useGPUSurfaceBlood;

	useGPUSurfaceBlood = 0;

	if (useGPUSurfaceBlood) {
		float tx1[2], tx2[2], tx3[2], tx[2];
		tx1[0] = face->m_TexCoordU[0];
		tx1[1] = face->m_TexCoordV[0];
		tx2[0] = face->m_TexCoordU[1];
		tx2[1] = face->m_TexCoordV[1];
		tx3[0] = face->m_TexCoordU[2];
		tx3[1] = face->m_TexCoordV[2];
		tx[0] = weights[0] * tx1[0] + weights[1] * tx2[0] + weights[2] * tx3[0];
		tx[1] = weights[0] * tx1[1] + weights[1] * tx2[1] + weights[2] * tx3[1];

		//Ogre::TexturePtr m_Tex = Ogre::TextureManager::getSingleton().load("partcl_bleed_new.dds", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		//this->ApplyEffect_Bleeding(Ogre::Vector2(tx[0], tx[1]), 0.003, 1.0, m_Tex);
		float tweighs[3];
		tweighs[0] = weights[0];
		tweighs[1] = weights[1];
		tweighs[2] = weights[2];
		//ApplyEffect_VolumeBlood(face->m_uid, tweighs);

		AddInjuryPoint(face, tweighs, NULL);		
	}
	else	
		createBloodTrack(face , weights);

	MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_PunctureSurface, tool, this);
	pEvent->m_OperatorFaces.push_back(face);
	pEvent->SetOrgan(this);
	pEvent->SetWeights(weights);
	
	CMXEventsDump::Instance()->PushEvent(pEvent , true);
}
//======================================================================================================
void MisMedicOrgan_Ordinary::SetCanClamp()
{
		//MedInstructManager::GetSingleton().m_sbcanclamp = m_physbody;
}

void MisMedicOrgan_Ordinary::SetCanBeGrasp(bool canBeGrasp)
{
	m_CanBeGrasp = canBeGrasp;
}

bool MisMedicOrgan_Ordinary::createBloodTrack(GFPhysSoftBodyFace * face , const float Roughweights[3], int nMaxNum /* = 3  */, bool IsWound /* = true  */, float bleedingTime /* = 2.0f */)
{
	static int nRecentFace = -1;
	int forigin = GetOriginFaceIndexFromUsrData(face);
	if( nRecentFace == forigin )
		return false;
	nRecentFace = forigin;
	/*if( ::rand()%2 )
	{
		if( createBloodTrack(face , Roughweights, -0.5f*(::rand()%1000)*0.001f, true) )
			createBloodTrack(face , Roughweights, 0.5f*(::rand()%1000)*0.001f, false);
	}
	else*/
	
#if 1
	createBloodTrack(face , Roughweights, 0.0f, true , IsWound ,  bleedingTime);
#else
//__/__血流替换成水流做测试
	static bool g_bFlag = true;
	if(g_bFlag)
		createWaterTrack(face , Roughweights, 0.0f, true);
	else
		createBloodTrack(face , Roughweights, 0.0f, true);
	//g_bFlag = !g_bFlag;
#endif
	return true;
}
 
OrganSurfaceBloodTextureTrack *  MisMedicOrgan_Ordinary::createBloodTrack(GFPhysSoftBodyFace * face , const float weights[3], float fDir, bool bPos , bool IsWound /* = true  */, float bleedingTime /* = 2.0f */)
{
	OrganSurfaceBloodTextureTrack * trackcreated = 0;
	if (face== NULL)
	{
		return 0;
	}

	GFPhysVector3 temp = face->m_Nodes[0]->m_UnDeformedPos*weights[0]
						+face->m_Nodes[1]->m_UnDeformedPos*weights[1]
						+face->m_Nodes[2]->m_UnDeformedPos*weights[2];
	
	Ogre::Vector3 bloodPosUndeformed(temp.x() , temp.y() , temp.z());

	const std::vector<OrganSurfaceBloodTextureTrack*> & existsTracks = m_BloodTextureEffect->GetAllBloodTrack();

	if( bPos && IsWound)
	{
		for(size_t i = 0 ; i < existsTracks.size() ; i++)
		{
			Ogre::Vector3 startPosUndeformed = existsTracks[i]->m_StartPosUndeformedFrame;

			float dist = (bloodPosUndeformed-startPosUndeformed).length();

			if(dist < m_BloodTrackPointDensity)
				return 0;
		}
	}

	if(!bPos || m_TotalElapsedTime-m_lastBloodEmitTime > m_BloodIntervalTime || !IsWound)//2.0f)
	{
		if((int)existsTracks.size() >= m_MaxBloodTrackCount && IsWound)//too many blood track remove 
		{
			int removeTrackId =  m_BloodTextureEffect->StopFirstBloodTrack();
			OnStopBleeding(removeTrackId , true);
		}

		if(m_FFDObject != 0)
		{
		    trackcreated = m_BloodTextureEffect->createBloodTrackInRoughFace(this , face , weights , m_CreateInfo.m_BloodRadius);
		}
		else
		{
			int forigin = GetOriginFaceIndexFromUsrData(face);
			
			if(forigin >= 0 && forigin < (int)m_OriginFaces.size())
			{
				float onwerweights[3];
				onwerweights[0] = weights[0];
				onwerweights[1] = weights[1];
				onwerweights[2] = weights[2];
				trackcreated = m_BloodTextureEffect->createBloodTrack(this, onwerweights, 1.0f, forigin, m_CreateInfo.m_BloodRadius,2.0f, 2.5f , fDir);
			}
			/*for(size_t f = 0 ; f < m_OriginFaces.size() ; f++)
			{
				if(m_OriginFaces[f].m_physface == face)
				{
					float onwerweights[3];
					onwerweights[0] = weights[0];
					onwerweights[1] = weights[1];
					onwerweights[2] = weights[2];
					trackcreated = m_BloodTextureEffect->createBloodTrack(this , onwerweights , 1.0f , f , m_CreateInfo.m_BloodRadius);
				}
			}*/
		}

		//m_BloodedPoints.push_back(bloodPos);

		if(IsWound)
			m_lastBloodEmitTime = m_TotalElapsedTime;

	}
	if(trackcreated != NULL)
	{
		if(IsWound)
			AddBleedingRecord(trackcreated->m_BloodTrackId);

		if(!IsWound)
		{
			m_NonWoundedBloodLimitTime[trackcreated] = bleedingTime;
			m_NonWoundedBloodTimeRecord[trackcreated] = 0.f;
		}
	}
	return trackcreated;
}

//__/__创建器官表面的一个水流
OrganSurfaceBloodTextureTrack *  MisMedicOrgan_Ordinary::createWaterTrack(GFPhysSoftBodyFace * face , const float weights[3], float fDir, bool bPos )
{
	OrganSurfaceBloodTextureTrack * trackcreated = 0;
	if (face== NULL)
	{
		return 0;
	}

	GFPhysVector3 temp = face->m_Nodes[0]->m_UnDeformedPos*weights[0]
						+face->m_Nodes[1]->m_UnDeformedPos*weights[1]
						+face->m_Nodes[2]->m_UnDeformedPos*weights[2];
	
	Ogre::Vector3 bloodPosUndeformed(temp.x() , temp.y() , temp.z());

	const std::vector<OrganSurfaceBloodTextureTrack*> & existsTracks = m_WaterTextureEffect->GetAllBloodTrack();

	if( bPos )
	{
		for(size_t i = 0 ; i < existsTracks.size() ; i++)
		{
			Ogre::Vector3 startPosUndeformed = existsTracks[i]->m_StartPosUndeformedFrame;

			float dist = (bloodPosUndeformed-startPosUndeformed).length();

			if(dist < m_BloodTrackPointDensity)
				return 0;
		}
	}

	if(!bPos || m_TotalElapsedTime-m_lastBloodEmitTime > m_BloodIntervalTime)//2.0f)
	{
		if((int)existsTracks.size() >= m_MaxBloodTrackCount)//too many blood track remove 
		{
			int removeTrackId =  m_WaterTextureEffect->StopFirstBloodTrack();
			//StopBleeding(removeTrackId , true);
		}

		if(m_FFDObject != 0)
		{
		    trackcreated = m_WaterTextureEffect->createBloodTrackInRoughFace(this , face , weights , m_CreateInfo.m_WaterRadius);
		}
		else
		{
			int forigin = GetOriginFaceIndexFromUsrData(face);
			
			if(forigin >= 0 && forigin < (int)m_OriginFaces.size())
			{
				float onwerweights[3];
				onwerweights[0] = weights[0];
				onwerweights[1] = weights[1];
				onwerweights[2] = weights[2];
				trackcreated = m_WaterTextureEffect->createWaterTrack(this , onwerweights , 1.0f , forigin , m_CreateInfo.m_WaterRadius, fDir);
			}
		}

		//m_BloodedPoints.push_back(bloodPos);

		m_lastBloodEmitTime = m_TotalElapsedTime;
	}
	//if(trackcreated != NULL)
	//{
	//	AddBleedingRecord(trackcreated->m_BloodTrackId);
	//}
	return trackcreated;
}

DynamicBloodPoint* MisMedicOrgan_Ordinary::createDynamicBloodPoint(GFPhysSoftBodyFace * pFace,const float weights[3])
{
	DynamicBloodPoint * pBloodPoint = m_BloodTextureEffect->CreateDynamicBloodPoint(pFace,weights);
	return pBloodPoint;
}

bool MisMedicOrgan_Ordinary::removeDynamicBloodPoint(GFPhysSoftBodyFace * pFace)
{
	return m_BloodTextureEffect->RemoveDynamicBloodPoint(pFace);
}

void MisMedicOrgan_Ordinary::SetMaxBloodTrackExistsTime(float time)
{
	m_MaxBloodTrackExistsTime = time;
}
/*
void MisMedicOrgan_Ordinary::EliminateTetrasInternal(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron *> & tetrasToRemove)
{
	for(size_t t = 0 ; t < tetrasToRemove.size(); t++)
	{
		m_physbody->GetSoftBodyShape().RemoveTetra(tetrasToRemove[t]);
	}
}
*/
class TetraAroundPointCB : public GFPhysNodeOverlapCallback
{
public:
	TetraAroundPointCB(int Catageory , 
					   const GFPhysVector3 & pointPos , 
					   float range ,
					   std::vector<GFPhysSoftBodyTetrahedron*> & tetraInrange) : m_TetrasInRage(tetraInrange)
	{
		m_PointPos = pointPos;
		m_Range = range;
		m_Categeroy = Catageory;
	}
	
	~TetraAroundPointCB()
	{

	}

	virtual void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
		GFPhysSoftBodyTetrahedron * tetra = (GFPhysSoftBodyTetrahedron *)UserData;
		
		if(tetra->m_ElementCategory & m_Categeroy)
		{
		   GFPhysVector3 closetPt = ClosetPtPointTetrahedron(m_PointPos, 
															 tetra->m_TetraNodes[0]->m_UnDeformedPos,
															 tetra->m_TetraNodes[1]->m_UnDeformedPos,
															 tetra->m_TetraNodes[2]->m_UnDeformedPos,
															 tetra->m_TetraNodes[3]->m_UnDeformedPos);

			if((closetPt-m_PointPos).Length() < m_Range)
			{
				m_TetrasInRage.push_back(tetra);
			}
		}
	};
	std::vector<GFPhysSoftBodyTetrahedron*> & m_TetrasInRage;
	GFPhysVector3 m_PointPos;
	float m_Range;
	int m_Categeroy;
};
//==================================================================================================
void MisMedicOrgan_Ordinary::CollectTetrasAroundPoint(const GFPhysVector3 & point , float radius , std::vector<GFPhysSoftBodyTetrahedron*> & tetraInrange , int Catageory , bool indeformspace)
{
	if(m_physbody)
	{
		if(indeformspace == false)
		{
		   TetraAroundPointCB callBack(Catageory , point , radius , tetraInrange);
		   const GFPhysDBVTree & tetraTree = m_physbody->GetSoftBodyShape().GetTetrahedronBVTree(false);
		   tetraTree.TraverseTreeAgainstAABB(&callBack , point-GFPhysVector3(radius , radius , radius) , point+GFPhysVector3(radius , radius , radius));
		}
		else
		{
			//need a lot of optimize这段很耗时需要优化比如先河AABB测试等！！
			for(size_t t = 0 ; t < m_physbody->GetNumTetrahedron() ; t++)
			{
				GFPhysSoftBodyTetrahedron * tetra = m_physbody->GetTetrahedronAtIndex(t);

				if (tetra->m_ElementCategory & Catageory)
				{
					GFPhysVector3 closetPt = ClosetPtPointTetrahedron(point,
						tetra->m_TetraNodes[0]->m_CurrPosition,
						tetra->m_TetraNodes[1]->m_CurrPosition,
						tetra->m_TetraNodes[2]->m_CurrPosition,
						tetra->m_TetraNodes[3]->m_CurrPosition);

					if ((closetPt - point).Length() < radius)
					{
						tetraInrange.push_back(tetra);
					}
				}
			}
			
		}
	}
}
//==================================================================================================
void MisMedicOrgan_Ordinary::HeatAroundUndeformedPoint(const GFPhysVector3 & point , float radius , float deltaValue)
{
	std::vector<GFPhysSoftBodyTetrahedron*> tetraInrange;
	CollectTetrasAroundPoint(point , radius , tetraInrange , (~0) , false);
	HeatTetrahedrons(tetraInrange , deltaValue);
}
//=======================================================================================================================
/*
void MisMedicOrgan_Ordinary::ElectricCutMensetaryImp(CTool * toolcut)
{
	GFPhysVector3 cutQuadVert[4];
	toolcut->GetToolCutPlaneVerts(cutQuadVert);

	GFPhysVector3 QuadMin = cutQuadVert[0];
	QuadMin.SetMin(cutQuadVert[1]);
	QuadMin.SetMin(cutQuadVert[2]);
	QuadMin.SetMin(cutQuadVert[3]);

	GFPhysVector3 QuadMax = cutQuadVert[0];
	QuadMax.SetMax(cutQuadVert[1]);
	QuadMax.SetMax(cutQuadVert[2]);
	QuadMax.SetMax(cutQuadVert[3]);

	std::vector<GFPhysSoftBodyTetrahedron*> MesntaryTetras;

	std::set<GFPhysSoftBodyNode*> hollowCenters;

	GFPhysSoftBodyNode * Node = m_physbody->GetNodeList();
	while(Node)
	{
		Node->m_StateFlag &= (~EMMT_InElecHollowCenter);
		Node = Node->m_Next;
	}
	//GFPhysSoftBodyTetrahedron * tetra = m_physbody->GetTetrahedronList();

	//while(tetra)
	for(size_t th = 0 ; th < m_physbody->GetNumTetrahedron() ; th++)
	{
		GFPhysSoftBodyTetrahedron * tetra = m_physbody->GetTetrahedronAtIndex(th);

		if(tetra->m_ElementCategory & EMMT_BeMesentary)//getUserIndex(0xff) == EDOT_APPENDMENSTORY)
		{
			MesntaryTetras.push_back(tetra);

			GFPhysVector3 TetrahedronVerts[4];
			GFPhysVector3 triangelverts[3];

			TetrahedronVerts[0] = tetra->m_TetraNodes[0]->m_CurrPosition;
			TetrahedronVerts[1] = tetra->m_TetraNodes[1]->m_CurrPosition;
			TetrahedronVerts[2] = tetra->m_TetraNodes[2]->m_CurrPosition;
			TetrahedronVerts[3] = tetra->m_TetraNodes[3]->m_CurrPosition;

			GFPhysVector3 TetraMin = TetrahedronVerts[0];
			TetraMin.SetMin(TetrahedronVerts[1]);
			TetraMin.SetMin(TetrahedronVerts[2]);
			TetraMin.SetMin(TetrahedronVerts[3]);

			GFPhysVector3 TetraMax = TetrahedronVerts[0];
			TetraMax.SetMax(TetrahedronVerts[1]);
			TetraMax.SetMax(TetrahedronVerts[2]);
			TetraMax.SetMax(TetrahedronVerts[3]);

			bool isaabboverlap = TestAabbAgainstAabb2(QuadMin, QuadMax,TetraMin, TetraMax);

			if(isaabboverlap)
			{
				Real signedvoulme = (TetrahedronVerts[1]-TetrahedronVerts[0]).Cross(TetrahedronVerts[2]-TetrahedronVerts[0]).Dot(TetrahedronVerts[3]-TetrahedronVerts[0]);
				if(signedvoulme < 0)//invert to keep [v1-v0 , v2-v0 , v3-v0] > 0
				   GPSwap(TetrahedronVerts[0] , TetrahedronVerts[1]);

				triangelverts[0] = cutQuadVert[0];
				triangelverts[1] = cutQuadVert[2];
				triangelverts[2] = cutQuadVert[3];
				bool isintersect = TriangleTetrahedronIntersection(TetrahedronVerts , triangelverts );

				if(isintersect == false)
				{
					triangelverts[0] = cutQuadVert[0];
					triangelverts[1] = cutQuadVert[3];
					triangelverts[2] = cutQuadVert[1];
					isintersect = TriangleTetrahedronIntersection(TetrahedronVerts , triangelverts);
				}
				if(isintersect)
				{
					int minIndex = -1;
					
					float minDist = FLT_MAX;
					
					for(size_t n = 0 ; n < 4 ; n++)
					{
						GFPhysVector3 nodePos = tetra->m_TetraNodes[n]->m_CurrPosition;
						GFPhysVector3 closetPt0 = ClosestPtPointTriangle(nodePos , cutQuadVert[0] , cutQuadVert[2] , cutQuadVert[3]);
						GFPhysVector3 closetPt1 = ClosestPtPointTriangle(nodePos , cutQuadVert[0] , cutQuadVert[3] , cutQuadVert[1]);

						float distSqr0 = (nodePos-closetPt0).Length2();
						float distSqr1 = (nodePos-closetPt1).Length2();

						float distSqr = (distSqr0 < distSqr1 ? distSqr0 : distSqr1);

						if(distSqr < minDist)
						{
							minDist = distSqr;
							minIndex = n;
						}
					}
					tetra->m_TetraNodes[minIndex]->m_StateFlag |= EMMT_InElecHollowCenter;
					hollowCenters.insert(tetra->m_TetraNodes[minIndex]);
				}
			}
		}
		
		//tetra = tetra->m_Next;
	}
	if(MesntaryTetras.size() == 0)//early filter out
	   return;
	//
	std::set<GFPhysSoftBodyNode*> nodeCutThisTime;

	GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> tetrasToRemove;
	std::set<GFPhysSoftBodyTetrahedron*> removeTetraSet;
	for(size_t c = 0 ; c < MesntaryTetras.size() ; c++)
	{
		GFPhysSoftBodyTetrahedron * tetra = MesntaryTetras[c];
		if((tetra->m_TetraNodes[0]->m_StateFlag & EMMT_InElecHollowCenter)
		 ||(tetra->m_TetraNodes[1]->m_StateFlag & EMMT_InElecHollowCenter)
		 ||(tetra->m_TetraNodes[2]->m_StateFlag & EMMT_InElecHollowCenter)
		 ||(tetra->m_TetraNodes[3]->m_StateFlag & EMMT_InElecHollowCenter))
		{
			//tetrasToRemove.push_back(tetra);
			removeTetraSet.insert(tetra);

			nodeCutThisTime.insert(tetra->m_TetraNodes[0]);
			nodeCutThisTime.insert(tetra->m_TetraNodes[1]);
			nodeCutThisTime.insert(tetra->m_TetraNodes[2]);
			nodeCutThisTime.insert(tetra->m_TetraNodes[3]);
		}
	}
	
	//tetrasToRemove.clear();
	
	for(size_t c = 0 ; c < MesntaryTetras.size() ; c++)
	{
		GFPhysSoftBodyTetrahedron * tetra = MesntaryTetras[c];

		int NumNodeBurned = 0;

		int NumSurface = 0;

		GFPhysSoftBodyNode * * TetraNodes = tetra->m_TetraNodes;

		for(int f = 0 ; f < 4 ; f++)
		{
			GFPhysSoftBodyNode * tetraNode = TetraNodes[f]; 

			if(tetraNode->m_StateFlag & EMMP_InCrossFace)
			   NumNodeBurned++;

			if(tetra->m_TetraFaces[f] != 0 && tetra->m_TetraFaces[f]->m_surface != 0)
			   NumSurface++;
		}

		bool needEliminate = false;

		if(NumNodeBurned >= 2 && NumSurface >= 2)
		{
			int NodeCutPreviousTime = 0;

			int NodeCutThisTime = 0;

			for(int f = 0 ; f < 4 ; f++)
			{
				GFPhysSoftBodyNode * tetraNode = TetraNodes[f]; 

				bool isNodeBurnThisTime = (nodeCutThisTime.find(tetraNode) == nodeCutThisTime.end() ? false : true);

				if((tetraNode->m_StateFlag & EMMP_InCrossFace) && (isNodeBurnThisTime == false))
					NodeCutPreviousTime++;

				else if(isNodeBurnThisTime)
					NodeCutThisTime++;
			}
			if(NodeCutPreviousTime > 0 && NodeCutThisTime > 0)
			   needEliminate = true;
		}
		if(needEliminate)
		{
			removeTetraSet.insert(MesntaryTetras[c]);
		   //tetrasToRemove.push_back(MesntaryTetras[c]);
		}
	}
	std::set<GFPhysSoftBodyTetrahedron*>::iterator itor = removeTetraSet.begin();
	while(itor != removeTetraSet.end())
	{
		tetrasToRemove.push_back(*itor);
		itor++;
	}
	
	if(tetrasToRemove.size() > 0)
	{
		GFPhysSoftBodyTetrasRemover tetraRemover(m_physDynWorld , m_physbody);
		tetraRemover.RemoveTetrahedrons(tetrasToRemove);
		onTopologyChanged(tetraRemover);
	}
}
*/
void MisMedicOrgan_Ordinary::CreatePhysicsPart(const MisMedicDynObjConstructInfo & CreateInfo , int topologytype)//float mass , bool distributemass , const std::vector<int> & fixpoints , float stiffness , int contactmode, float veldamping,float collidefricoeff , float collideharness, bool hardfixpoint, float surfacemassmult)
{	
	m_physDynWorld = PhysicsWrapper::GetSingleTon().m_dynamicsWorld;

	float mass = CreateInfo.m_mass; 
	const std::vector<int> & fixpoints = CreateInfo.m_FixPointsIndex;
    const std::vector<int> & expandpoints = CreateInfo.m_ExpandPointsIndex;    
	float stiffness = CreateInfo.m_stiffness;
    float poissonrate = CreateInfo.m_poissonrate;
	int contactmode = CreateInfo.m_contactmode; 
	float veldamping = CreateInfo.m_veldamping; 
	float collidefricoeff = CreateInfo.m_frictcoeff;
	float collideharness = CreateInfo.m_collidehardness; 
	bool  hardfixpoint = CreateInfo.m_hardfixpoint;     
	float surfacemassmult = CreateInfo.m_surfacemassmultifactor;

	int  NodeInitNum = m_Serializer.m_NodeInitPositions.size();

	float masspernode = mass / NodeInitNum;

	float * MassNode = new float[NodeInitNum];

	for (int m = 0; m < NodeInitNum; m++)
	{
		MassNode[m] = mass / NodeInitNum;//0.1f;
	}

	//see how many multi-collide tree we need
	int facenum = m_Serializer.m_InitFaces.size();//m_OriginFaces.size();
	int multicollidetreenum = 0;
	if(facenum > 1250)
	{
	   multicollidetreenum = facenum / 1250+1;
	   if(multicollidetreenum > 3)
	      multicollidetreenum = 3;
	}

	m_physbody = 0;
	
	if(topologytype == 0)
	   m_physbody = new GFPhysSoftBody(&(m_Serializer.m_NodeInitPositions[0]), MassNode, m_Serializer.m_NodeInitPositions.size(), GFPhysSoftBodyShape::SRCT_FACE, GFPhysSoftBodyShape::SSCT_TETRAVSNODE, false, multicollidetreenum);
	else
	{
	   int * FaceIndexArray = new int[facenum*3];
	   for(int f = 0 ; f < facenum ; f++)
	   {
		   FaceIndexArray[f*3+0] = m_Serializer.m_InitFaces[f].m_Index[0];
		   FaceIndexArray[f*3+1] = m_Serializer.m_InitFaces[f].m_Index[1];
		   FaceIndexArray[f*3+2] = m_Serializer.m_InitFaces[f].m_Index[2];
	   }
	   m_physbody = new GPhysSoftMembrane(&(m_Serializer.m_NodeInitPositions[0]),
										  MassNode , 
										  m_Serializer.m_NodeInitPositions.size(),
										  FaceIndexArray ,
										  facenum ,
										  stiffness);

	   delete []FaceIndexArray;

	}
	delete []MassNode;
	m_physbody->GetSoftBodyShape().m_RestScale = CreateInfo.m_RestScale;

	//create node application data
	m_PhysNodeData.clear();
	PhysNode_Data errorData;
	errorData.m_HasError = true;
	m_PhysNodeData.push_back(errorData);

	GFPhysSoftBodyNode * nodephys = m_physbody->GetNodeList();
	int nodeIndex = 0;
	while(nodephys)
	{
		PhysNode_Data nodeData;
		nodeData.m_PhysNode = nodephys;

		//set attribute
 		std::map<int,std::vector<std::pair<std::string,int>>>::iterator itr;
 		itr = m_Serializer.m_NodeAttributeMap.find(nodeIndex);
 		if(itr != m_Serializer.m_NodeAttributeMap.end())
 		{
 			std::vector<std::pair<std::string,int>>& attributes = itr->second;
 			for(std::size_t a = 0;a < attributes.size();++a)
 			{
 				nodeData.m_AttributeMap.insert(attributes[a]);
 			}
 		}
 		++nodeIndex;

		m_PhysNodeData.push_back(nodeData);

		nodephys->m_UserPointer = (void*)((int)m_PhysNodeData.size() - 1);

		nodephys = nodephys->m_Next;
	}

	//copy origin face to MMO_Face first
	m_OriginFaces.clear();
	m_OriginFaces.reserve(m_Serializer.m_InitFaces.size());

	if(topologytype == 0)
	{
		//create all tetrahedron and their application data
		m_PhysTetraData.clear();
		PhysTetra_Data TerrorData;
		TerrorData.m_HasError = true;
		m_PhysTetraData.push_back(TerrorData);

		for(int t = 0 ; t < m_Serializer.m_InitTetras.size() ; t++)
		{
			const MisMedicObjetSerializer::MisSerialTetra & teraelement = m_Serializer.m_InitTetras[t];
			
			GFPhysSoftBodyTetrahedron * tetrahedron = m_physbody->AddTetrahedron(teraelement.m_Index[0], teraelement.m_Index[1], teraelement.m_Index[2], teraelement.m_Index[3]);
			
			if (teraelement.m_IsTextureSetted)
			{
				for (int k = 0; k < 4; k++)
				{
					tetrahedron->m_texCoordU[k] = teraelement.m_TextureCoord[k].x;
					tetrahedron->m_texCoordV[k] = teraelement.m_TextureCoord[k].y;
				}
				//internal tetrahedron has swapped hack here to refactory
				GPSwap(tetrahedron->m_texCoordU[0], tetrahedron->m_texCoordU[1]);
				GPSwap(tetrahedron->m_texCoordV[0], tetrahedron->m_texCoordV[1]);

				m_IsInnerTexCoordSetted = true;
			}
			if(tetrahedron->m_UserPointer == 0)//prevent mms file contains redundant tetrahedron,this if ensure tetrahedron is new created
			{
				//if(teraelement.m_IsMenstary)
				   //tetrahedron->m_ElementCategory |= EMMT_BeMesentary;
				//else
				//   tetrahedron->m_ElementCategory |= EMMT_BeTissue;

				//tetrahedron->m_DensityFactor = teraelement.massScale;
				
				//add to application data
				PhysTetra_Data tetraData(tetrahedron , teraelement.m_unionObjectID , teraelement.m_IsMenstary);
				tetraData.m_Layer = teraelement.m_LayerIndex;

				//temp need re factory
				if (teraelement.m_LayerIndex == 0)
				    tetrahedron->m_ElementCategory |= EMMT_LayerTissue;
				else
			        tetrahedron->m_ElementCategory |= EMMT_LayerMembrane;
				//

				m_PhysTetraData.push_back(tetraData);
				tetrahedron->m_UserPointer = (void*)((int)m_PhysTetraData.size() - 1);
				tetrahedron->m_LayerIndex = teraelement.m_LayerIndex;
				//multi-layer tetra set the node of tetra layer factor
				//if(tetraData.m_Layer > 0)
				//{
					//for(int n = 0 ; n < 4 ; n++)
					//{
					//	GetPhysNodeData(tetrahedron->m_TetraNodes[n]).m_LayerBlendFactor = 1.0f;
					//}
				//}
			}
		}

		//create all faces and set pointer to MMO_Face Object
		for(int f = 0 ; f < m_Serializer.m_InitFaces.size() ; f++)
		{
			const MisMedicObjetSerializer::MisSerialFace & serialface = m_Serializer.m_InitFaces[f];
			GFPhysSoftBodyFace * physFace = m_physbody->AddFace(serialface.m_Index[0],
															    serialface.m_Index[1], 
															    serialface.m_Index[2]);

			if(physFace->m_UserData == 0)//prevent mms file contains redundant tetrahedron
			{
				GFPhysVector3 vertUndeformPos[3];
				vertUndeformPos[0] = (physFace->m_Nodes[0]->m_UnDeformedPos);
				vertUndeformPos[1] = (physFace->m_Nodes[1]->m_UnDeformedPos);
				vertUndeformPos[2] = (physFace->m_Nodes[2]->m_UnDeformedPos);
				
				const MMO_Face & face0 = AddOriginMMO_FaceInternal( physFace ,
																   serialface.m_Index , 
																   vertUndeformPos , 
																   serialface.m_TextureCoord
																  );
 				MMO_Face& mmoFace = const_cast<MMO_Face&>(face0);
	 
 				//set attribute
  				std::map<int,std::vector<std::pair<std::string,int>>>::iterator itr;
  				itr = m_Serializer.m_FaceAttributeMap.find(f);
  				if(itr != m_Serializer.m_FaceAttributeMap.end())
  				{
  					std::vector<std::pair<std::string,int>>& attributes = itr->second;
  					for(std::size_t a = 0;a < attributes.size();++a)
  					{
  						mmoFace.m_AttributeMap.insert(attributes[a]);
  					}
  				}
			}
		}

		//distribute mass over all soft body volume
		/*if(CreateInfo.m_distributemass)
		   GFPhysSoftBodyUtility::DistributeMassInTetra(mass , SoftBody);

		//add surface Node Multiply
		std::set<GFPhysSoftBodyNode*> faceNodes;
		for(size_t f = 0 ; f < m_OriginFaces.size() ; f++)
		{
			MMO_Face &face = m_OriginFaces[f];
			faceNodes.insert(face.m_physface->m_Nodes[0]);
			faceNodes.insert(face.m_physface->m_Nodes[1]);
			faceNodes.insert(face.m_physface->m_Nodes[2]);
		}
		std::set<GFPhysSoftBodyNode*>::iterator itor = faceNodes.begin();
		while(itor != faceNodes.end())
		{
			GFPhysSoftBodyNode * softnode = (*itor);
			softnode->SetMass(softnode->m_Mass*surfacemassmult);//1.8f
			itor++;
		}*/
		ReDistributeMass();
	}
	else
	{
		//temp process
		int fid = 0;
		//GFPhysSoftBodyFace * physFace = SoftBody->GetFaceList();

		//while(physFace)
		for (size_t f = 0; f < m_physbody->GetNumFace(); f++)
		{
			GFPhysSoftBodyFace * physFace = m_physbody->GetFaceAtIndex(f);

			const MisMedicObjetSerializer::MisSerialFace & serialface = m_Serializer.m_InitFaces[fid];

			GFPhysVector3 vertUndeformPos[3];
			vertUndeformPos[0] = (physFace->m_Nodes[0]->m_UnDeformedPos);
			vertUndeformPos[1] = (physFace->m_Nodes[1]->m_UnDeformedPos);
			vertUndeformPos[2] = (physFace->m_Nodes[2]->m_UnDeformedPos);

			const MMO_Face& face0 = AddOriginMMO_FaceInternal( physFace ,
															   serialface.m_Index , 
															   vertUndeformPos , 
															   serialface.m_TextureCoord
															  );

 			MMO_Face& mmoFace = const_cast<MMO_Face&>(face0);
 
 			//set attribute
  			std::map<int,std::vector<std::pair<std::string,int>>>::iterator itr;
  			itr = m_Serializer.m_FaceAttributeMap.find(f);
  			if(itr != m_Serializer.m_FaceAttributeMap.end())
  			{
  				std::vector<std::pair<std::string,int>>& attributes = itr->second;
  				for(std::size_t a = 0;a < attributes.size();++a)
  				{
  					mmoFace.m_AttributeMap.insert(attributes[a]);
  				}
  			}

			fid++;
			//physFace = physFace->m_Next;
		}
		//DistributeMassInFace(mass , SoftBody);
	}

	//fix point set
	if(hardfixpoint == false)
	{
		std::vector<GFPhysSoftBodyNode*> FixNodes;
		for(size_t i = 0 ; i < fixpoints.size() ; i++)
		{
			GFPhysSoftBodyNode * sbnode = m_physbody->GetNode(fixpoints[i]);
			//sbnode->m_RSCollisionMask = 0;
			sbnode->m_StateFlag |= GPSESF_CONNECTED;
			PhysNode_Data & nData = GetPhysNodeData(sbnode);
			nData.m_NodeBeFixed = true;

			FixNodes.push_back(sbnode);
		}
		
		SetNodeFixForce(CreateInfo.m_FixPointStiffness, FixNodes);
	}
	else
	{
		for(size_t i = 0 ; i < fixpoints.size() ; i++)
		{
			int nid = fixpoints[i];
			GFPhysSoftBodyNode * sbnode = m_physbody->GetNode(nid);
			sbnode->m_RSCollisionMask = 0;
			sbnode->m_StateFlag |= GPSESF_CONNECTED;

			PhysNode_Data & nData = GetPhysNodeData(sbnode);
			nData.m_NodeBeFixed = true;
			
			m_physbody->SetMass(nid, 0);
		}
	}
	
	//stiffness contact mode etc
	m_RSCollideAlgoType = CreateInfo.m_collidealgo;
	//m_IsolatedPartGravityValue = CreateInfo.m_GravityValue;
	m_physbody->SetGravity(OgreToGPVec3(CreateInfo.m_CustomGravityDir*CreateInfo.m_GravityValue));//now apply gravity in app OgreToGPVec3(CreateInfo.m_CustomGravityDir*CreateInfo.m_GravityValue));//m_CreateInfo.m_Gravity.x,m_CreateInfo.m_Gravity.y,m_CreateInfo.m_Gravity.z));
	m_physbody->SetVelocityDamping(true, CreateInfo.m_veldamping, 0);//now apply damp in app //(true , veldamping , 0.00f);
	m_physbody->SetVelocityDampingMode(1);
	m_physbody->SetRSCollideFrictionCoff(CreateInfo.m_frictcoeff);//collidefricoeff);//(0.5f);
	m_physbody->SetRSCollideStiffness(collideharness);//(0.6f);
	((GFPhysSoftBodyShape*)m_physbody->GetCollisionShape())->SetSSCollideTag(GFPhysSoftBodyShape::SSCT_FACEVSNODE);
	m_physbody->SetStiffness(stiffness);
    //SoftBody->SetPoissonrate(poissonrate);
	m_physbody->SetRSContactMode(contactmode);
	m_physbody->SetRSMaxPenetrate(m_CreateInfo.m_RSMaxPenetrate);

	//if(m_OrganID == EDOT_APPENDIX)
	   //SoftBody->SetInternalForceType(GFPhysSoftBody::IFT_LINK | GFPhysSoftBody::IFT_TETRA | GFPhysSoftBody::IFT_FACE);
	//else
	m_physbody->SetInternalForceType(GFPhysSoftBody::IFT_LINK | GFPhysSoftBody::IFT_TETRA);
	m_physbody->m_ElasticParam.m_EdgeInvStiff = CreateInfo.m_invEdgePhysStiff;// 1.0f / 5000.0f;
	m_physbody->m_ElasticParam.m_EdgeDamping = CreateInfo.m_EdgePhysDamping; //50.0f;

	m_physbody->m_ElasticParam.m_TetraInvStiff = CreateInfo.m_invTetraPhysStiff;// 1.0f / 50000.0f;
	m_physbody->m_ElasticParam.m_TetraDamping = CreateInfo.m_TetraPhysDamping;// 500.0f;

	m_physbody->SetVolumeSor(0.95f);
	  //if (m_OrganID == EDOT_SIGMOIDCUTPART)
	  //{
		  //SoftBody->m_ElasticParam.m_EdgeCompressDamping = 0.2f;
		 // SoftBody->m_ElasticParam.m_TetraCompressDamping = 0.3f;
		  //SoftBody->m_ElasticParam.m_WarmStartFactor = 0.5f;
	 // }
	//
	//if(CreateInfo.m_DoubleFaceCollide)
	//	m_physbody->EnableDoubleFaceCollision();
	//else
		m_physbody->DisableDoubleFaceCollision();

	m_physbody->ConstructConstParameter(0, 0);
	if(CreateInfo.m_HomingForce > 0.0001f)
	{
	    SetHomingForce(CreateInfo.m_HomingForce , m_physbody);
	}

	//calculate separate part
	CalculateSeperatePart();

	//create shape match force
	if(m_CreateInfo.m_FurtherStiffness > 0)
	{
		m_OrganIDFurtherStiffness = -1;
		CreatePoseRigidForce();
	}
	//create additional bend force if configured
	//CreateAdditionalBendingForce();

	//SoftBody->GetSoftBodyShape().EnableRefitBVTree(true);
	if(CreateInfo.m_IsStaticObject)
	{
		m_physbody->SetCollisionFlags(m_physbody->GetCollisionFlags() | GFPhysCollideObject::CF_STATIC_OBJECT);
		((GFPhysSoftBodyShape*)m_physbody->GetCollisionShape())->SetSSCollideTag(GFPhysSoftBodyShape::SSCT_NONE);
		((GFPhysSoftBodyShape*)m_physbody->GetCollisionShape())->SetSRCollideTag(GFPhysSoftBodyShape::SRCT_NONE);
		
		//manually update normal once for static object
		for (int n = 0; n < m_physbody->GetNodesNum(); n++)
		{
			m_physbody->GetNode(n)->SetMass(0);
			m_physbody->GetNode(n)->m_Normal = GFPhysVector3(0, 0, 0);
		}
		for (size_t f = 0; f < m_physbody->GetNumFace(); f++)
		{
			GFPhysSoftBodyFace * physface = m_physbody->GetFaceAtIndex(f);

			GFPhysVector3 facenormal = (physface->m_Nodes[1]->m_CurrPosition-physface->m_Nodes[0]->m_CurrPosition).Cross(physface->m_Nodes[2]->m_CurrPosition-physface->m_Nodes[0]->m_CurrPosition);
			facenormal.Normalize();
			physface->m_FaceNormal = facenormal;
			physface->m_Nodes[0]->m_Normal += facenormal;
			physface->m_Nodes[1]->m_Normal += facenormal;
			physface->m_Nodes[2]->m_Normal += facenormal;

		}

		for (int n = 0; n < m_physbody->GetNodesNum(); n++)
		{
			m_physbody->GetNode(n)->m_Normal.Normalize();
		}
	}

	for(size_t i = 0 ; i < m_OrganActionListeners.size(); ++i)
	{
		m_OrganActionListeners[i]->OnOrganPhysicsPartCreated(this);
	}
	//
	if( PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	{
		m_OwnerTrain->OrganBeginAddToDynamicWorld(this);
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddSoftBody(m_physbody);
	}
	

	//
	m_TexMaxGrad = Ogre::Vector2(0, 0);
	std::vector<float> texGradU;
	std::vector<float> texGradV;

	for (int c = 0; c < m_OriginFaces.size(); c++)
	{
		Ogre::Vector3 triPos[3];
		
		Ogre::Vector2 triTex[3];

		GFPhysSoftBodyFace * physface = m_OriginFaces[c].m_physface;

		float * textureU = physface->m_TexCoordU;

		float * textureV = physface->m_TexCoordV;

		for (int n = 0; n < 3; n++)
		{
			GFPhysSoftBodyNode * FaceNodePhys = physface->m_Nodes[n];
			
			triPos[n] = GPVec3ToOgre(FaceNodePhys->m_UnDeformedPos);
			
			triTex[n].x = textureU[n];
			
			triTex[n].y = textureV[n];
		}
		Ogre::Vector2 grad = getGradientTriUV(triPos, triTex);
		
		texGradU.push_back(grad.x);
		
		texGradV.push_back(grad.y);
	}
	std::sort(texGradU.begin(), texGradU.end());
	std::sort(texGradV.begin(), texGradV.end());

	int middleIndex = texGradU.size() / 2;
	m_TexMaxGrad.x = texGradU[middleIndex];

	middleIndex = texGradV.size() / 2;
	m_TexMaxGrad.y = texGradV[middleIndex];
}
//=======================================================================
MisMedicTitaniumClampV2 * MisMedicOrgan_Ordinary::CreateTitanic(float invalidClipLen,
	                                       Ogre::Vector3 clipAxis[3],
	                                       Ogre::Vector2 clipBound[3], 
										   GFPhysSoftBodyFace * attachFace,
										   int toolFlag)
{
	MisMedicTitaniumClampV2 * tianumclamp = new MisMedicTitaniumClampV2(invalidClipLen, clipAxis, clipBound, attachFace, this, toolFlag);
	m_OrganAttachments.push_back(tianumclamp);

	RefreshRendFacesUndeformedPosition();//rest position modified refresh

 	setAttchmentFlag(EMMOI_WithTitanic_On_Ordinary, false);
	
#if(1)
	Ogre::Vector3 intersecPt[3];

	TitanicClipInfo::s_clipInValidReg = false;

	for(size_t i = 0; i < m_titanicClipInfos.size() ; i++)
	{
		TitanicClipInfo & clipInfo = m_titanicClipInfos[i];

		//if(!clipInfo.m_IsClip)
		//{
		std::vector<GFPhysSoftBodyFace *> & faces =  clipInfo.m_facesSatisfied;

		for (int c = 0; c < faces.size(); c++)
		{
			 Ogre::Vector3 triVerts[3];
			
			 triVerts[0] = GPVec3ToOgre(faces[c]->m_Nodes[0]->m_UnDeformedPos);
			 triVerts[1] = GPVec3ToOgre(faces[c]->m_Nodes[1]->m_UnDeformedPos);
			 triVerts[2] = GPVec3ToOgre(faces[c]->m_Nodes[2]->m_UnDeformedPos);
			
			 int numIntersectPt = tianumclamp->GetTriEdgesIntersectClip(triVerts, intersecPt);
			
			 if (numIntersectPt > 1)
			 {
				 clipInfo.m_IsClip = true;
				 TitanicClipInfo::s_clipInValidReg = true;
				 break;
			 }
		}
			
		if (TitanicClipInfo::s_clipInValidReg)
			break;

		//}
	}
#endif

	return tianumclamp;
}

void MisMedicOrgan_Ordinary::AddOrganAttachment(MisMedicOrganAttachment * attach)
{
    std::vector<MisMedicOrganAttachment*>::iterator result = find( m_OrganAttachments.begin( ), m_OrganAttachments.end( ), attach ); //be sure not repeat.
    if ( result == m_OrganAttachments.end( ) ) //not find
    {
        m_OrganAttachments.push_back(attach);
    }    	
}

void MisMedicOrgan_Ordinary::RemoveOrganAttachment(MisMedicOrganAttachment * attach)
{
	for(size_t c = 0 ; c < m_OrganAttachments.size(); c++)
	{
		if(m_OrganAttachments[c] == attach)
		{
			m_OrganAttachments.erase(m_OrganAttachments.begin()+c);
			break;
		}
	}
}
//===========================================================================================================================
void MisMedicOrgan_Ordinary::ReDistributeMass()
{
	//m_CreateInfo.m_GravityValue
	float massPerNode = m_CreateInfo.m_mass / m_Serializer.m_NodeInitPositions.size();

	if (m_CreateInfo.m_distributemass)
	{
		GFPhysSoftBodyUtility::DistributeMassInTetra(m_CreateInfo.m_mass, m_physbody);

		//@make surface node the same mass this make a more good effect
		GFPhysSoftBodyNode * softnode = m_physbody->GetNodeList();
		while (softnode)
		{
			if (softnode->m_insurface && softnode->m_InvM > GP_EPSILON)
			{
				softnode->SetMass(massPerNode * m_CreateInfo.m_surfacemassmultifactor);
			}
			softnode = softnode->m_Next;
		}
	}
	else
	{
		//@add surface Node Multiply
		GFPhysSoftBodyNode * softnode = m_physbody->GetNodeList();
		while (softnode)
		{
			if (softnode->m_InvM > GP_EPSILON)
			{
				if (softnode->m_insurface)
					softnode->SetMass(massPerNode * m_CreateInfo.m_surfacemassmultifactor);
				else
					softnode->SetMass(massPerNode);
			}
			softnode = softnode->m_Next;
		}
		//
	}
}
//=============================================================================================================================
void MisMedicOrgan_Ordinary::RefreshDirtyData()
{
	if(m_TopologyDirty)
	{
		//calculate separate part
		CalculateSeperatePart();

		//@ distribute mass again
		ReDistributeMass();

		//@create pose-frame via SM
		CreatePoseRigidForce();

		//rebuild texture surface blood system, 
		//@@ !! to do optimize prevent rebuild , use partial build
		if(m_BloodSystem)
		   m_BloodSystem->Initialize(this , m_isSetGravity);

		//rebuild rend vertex and index
		RebuildRendVertexAndIndex();

		//
		for (int c = 0; c < m_OriginFaces.size(); c++)
		{
			GFPhysSoftBodyFace * face = m_OriginFaces[c].m_physface;
			if (face == 0)
				continue;
			if (face->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra->m_IsFlipped)
				m_OriginFaces[c].m_NeedDoubleFaces = true;
			else
				m_OriginFaces[c].m_NeedDoubleFaces = false;
		}
		for (int c = 0; c < m_CutCrossfaces.size(); c++)
		{
			GFPhysSoftBodyFace * face = m_CutCrossfaces[c].m_physface;
			if (face == 0)
				continue;
			if (face->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra->m_IsFlipped)
				m_CutCrossfaces[c].m_NeedDoubleFaces = true;
			else
				m_CutCrossfaces[c].m_NeedDoubleFaces = false;
		}

		m_TopologyDirty = false;

		m_OwnerTrain->onOrganTopologyRefreshed(this);
	}
}
//=============================================================================================================================
void MisMedicOrgan_Ordinary::InternalSimulateStart(int currStep , int TotalStep , Real dt)
{
	m_simulateTime += dt;
	//MisNewTraining::InternalSimulateStart(currStep, TotalStep, dt);
	if (((int)(m_simulateTime * 1400)) % 1000 < 300)
	{
		for (int i = 0; i < pulseNodes.size(); i++) {
			pulseNodes.at(i)->m_ExtForce = pulseNodes.at(i)->m_Normal*pulseForce;
		}
	}
}
//=============================================================================================================================
void MisMedicOrgan_Ordinary::InternalSimulateEnd(int currStep , int TotalStep , Real dt)
{
	GFPhysVector3 GravityDir = OgreToGPVec3(m_CreateInfo.m_CustomGravityDir);
	
	//if isolated part gravity is greater than whole object gravity , we calculate delta gravity
	float DeltaIsolateGravity = (m_IsolatedPartGravityValue > m_CreateInfo.m_GravityValue ? (m_IsolatedPartGravityValue - m_CreateInfo.m_GravityValue) : 0);
	
	for(int c = 0 ; c < (int)m_physbody->GetSoftBodyShape().GetNumSubParts() ; c++)
	{
		GFPhysSubConnectPart * subPart = m_physbody->GetSoftBodyShape().GetSubPart(c);// m_OrganSubParts[c];
		
		bool IsolatedPart = ((subPart->m_PartStateFlag & (GPSESF_CONNECTED)) == 0);
	
		//total isolated part (i.e not connect to any other object)
		if(IsolatedPart)
		{
		    for(size_t n = 0 ; n < subPart->m_Nodes.size() ; n++)
			{
				//velocity
				GFPhysSoftBodyNode * pnode = subPart->m_Nodes[n];
				if (pnode->m_InvM > 0)
					pnode->m_Velocity += GravityDir * (DeltaIsolateGravity * dt);
			}
		}
	}
}
//=============================================================================================================================
const MMO_Face & MisMedicOrgan_Ordinary::AddOriginMMO_FaceInternal(GFPhysSoftBodyFace * face , const int vid[3] , const GFPhysVector3 vertUndeformPos[3] , const Ogre::Vector2 texCoord[3])
{
	//now add new Rend Face
	MMO_Face NewFace;
	NewFace.m_physface = face;

	NewFace.m_NodeUndeformPos[0] = vertUndeformPos[0];
	NewFace.m_NodeUndeformPos[1] = vertUndeformPos[1];
	NewFace.m_NodeUndeformPos[2] = vertUndeformPos[2];

	//NewFace.m_TextureCoord[0] = texCoord[0];
	//NewFace.m_TextureCoord[1] = texCoord[1];
	//NewFace.m_TextureCoord[2] = texCoord[2];
	face->m_TexCoordU[0] = texCoord[0].x;
	face->m_TexCoordV[0] = texCoord[0].y;

	face->m_TexCoordU[1] = texCoord[1].x;
	face->m_TexCoordV[1] = texCoord[1].y;

	face->m_TexCoordU[2] = texCoord[2].x;
	face->m_TexCoordV[2] = texCoord[2].y;

	NewFace.vi[0] = vid[0];
	NewFace.vi[1] = vid[1];
	NewFace.vi[2] = vid[2];
	NewFace.BuildConstParameter();

	int ResultIndex = -1;

	if(m_FreeOriginFaceHead >= 0)
	{
		ResultIndex = m_FreeOriginFaceHead;
		int NextFreedIndex = m_OriginFaces[ResultIndex].m_NextFreed;

		m_OriginFaces[ResultIndex] = NewFace;
		m_OriginFaces[ResultIndex].m_NextFreed = -1;
		NewFace.m_physface->m_UserData = (void*)BUILDORIGINFACEUSRDATA(ResultIndex);

		m_FreeOriginFaceHead = NextFreedIndex;
	}
	else
	{
		NewFace.m_physface->m_UserData = (void*)BUILDORIGINFACEUSRDATA(m_OriginFaces.size());
		m_OriginFaces.push_back(NewFace);
		ResultIndex = m_OriginFaces.size()-1;
	}

	return m_OriginFaces[ResultIndex];
}
//=============================================================================================================================
const MMO_Face & MisMedicOrgan_Ordinary::AddNewCuttedMMO_FaceInternal(GFPhysSoftBodyFace * face , int & ResultIndex)
{
	MMO_Face NewFace;
	NewFace.m_physface = face;

	//save undeformed position and texture coordinate
	NewFace.m_NodeUndeformPos[0] = face->m_Nodes[0]->m_UnDeformedPos;
	NewFace.m_NodeUndeformPos[1] = face->m_Nodes[1]->m_UnDeformedPos;
	NewFace.m_NodeUndeformPos[2] = face->m_Nodes[2]->m_UnDeformedPos;

	if(m_HasVolTexCoors)
	{
	   //NewFace.m_TextureCoord[0] = GetPhysNodeData(face->m_Nodes[0]).m_NodeTextureCoord;
	  // NewFace.m_TextureCoord[1] = GetPhysNodeData(face->m_Nodes[1]).m_NodeTextureCoord;
	  // NewFace.m_TextureCoord[2] = GetPhysNodeData(face->m_Nodes[2]).m_NodeTextureCoord;
	}
	else
	{
	   //NewFace.m_TextureCoord[0] = Ogre::Vector2(0,0);
	  // NewFace.m_TextureCoord[1] = Ogre::Vector2(1,0);
	  // NewFace.m_TextureCoord[2] = Ogre::Vector2(1,1);
	}

	//test
	//GFPhysSoftBodyTetrahedron * tetra = face->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra;
	//PhysTetra_Data & physTetraData = GetPhysTetraAppData(tetra);

	//GFPhysVector3 tetraNodePos[4];
	//tetraNodePos[0] = tetra->m_TetraNodes[0]->m_UnDeformedPos;
	//tetraNodePos[1] = tetra->m_TetraNodes[1]->m_UnDeformedPos;
	//tetraNodePos[2] = tetra->m_TetraNodes[2]->m_UnDeformedPos;
	//tetraNodePos[3] = tetra->m_TetraNodes[3]->m_UnDeformedPos;

	/*
	for (int n = 0; n < 3; n++)
	{
		float weights[4];
		bool  succed = GetPointBarycentricCoordinate(tetraNodePos[0], tetraNodePos[1], tetraNodePos[2], tetraNodePos[3],
			                                         face->m_Nodes[n]->m_UnDeformedPos,
			                                         weights);

		if (succed)
		{
			NewFace.m_TextureCoord[n].x = tetra->m_texCoordU[0] * weights[0]
				                        + tetra->m_texCoordU[1] * weights[1]
				                        + tetra->m_texCoordU[2] * weights[2]
				                        + tetra->m_texCoordU[3] * weights[3];
		
			NewFace.m_TextureCoord[n].y = tetra->m_texCoordV[0] * weights[0]
				                        + tetra->m_texCoordV[1] * weights[1]
				                        + tetra->m_texCoordV[2] * weights[2]
				                        + tetra->m_texCoordV[3] * weights[3];
		}
		else
		{
			NewFace.m_TextureCoord[n] = Ogre::Vector2(0, 0);
		}
	}*/

	//
	
	NewFace.vi[0] = NewFace.vi[1] = NewFace.vi[2] = -1;//none need currently
	NewFace.BuildConstParameter();

	if(m_FreeCutFaceHead >= 0)
	{
		ResultIndex = m_FreeCutFaceHead;
		int NextFreedIndex = m_CutCrossfaces[ResultIndex].m_NextFreed;
		
		m_CutCrossfaces[ResultIndex] = NewFace;
		m_CutCrossfaces[ResultIndex].m_NextFreed = -1;
		NewFace.m_physface->m_UserData = (void*)BUILCUTCROSSFACEUSRDATA(ResultIndex);

		m_FreeCutFaceHead = NextFreedIndex;
	}
	else
	{
		NewFace.m_physface->m_UserData = (void*)BUILCUTCROSSFACEUSRDATA(m_CutCrossfaces.size());
		m_CutCrossfaces.push_back(NewFace);
		ResultIndex = (int)m_CutCrossfaces.size()-1;
	}
	
	
	return m_CutCrossfaces[ResultIndex];
}
//=============================================================================================================================
const MMO_Face & MisMedicOrgan_Ordinary::AddSplittedMMO_FaceInternal(const MMO_Face & splitFromData , 
														 GFPhysSoftBodyFace * splitface ,
														 bool InOriginPart)
{
		//now add new Rend Face
		MMO_Face NewFace;
		NewFace.m_physface = splitface;

		//GFPhysVector3 parentNode0 = splitFromData.m_physface->m_Nodes[0]->m_UnDeformedPos;
		//GFPhysVector3 parentNode1 = splitFromData.m_physface->m_Nodes[1]->m_UnDeformedPos;
		//GFPhysVector3 parentNode2 = splitFromData.m_physface->m_Nodes[2]->m_UnDeformedPos;

		//calculate texture coordinate etc from old face data
		for(int v = 0 ; v < 3 ; v++)
		{
			GFPhysSoftBodyNode * FaceNode = splitface->m_Nodes[v];

			//save undeformed position
			NewFace.m_NodeUndeformPos[v] = FaceNode->m_UnDeformedPos;

			//calculate texture coordinate
			/*
			float weights[3];

			CalcBaryCentric(splitFromData.m_NodeUndeformPos[0],
							splitFromData.m_NodeUndeformPos[1],
							splitFromData.m_NodeUndeformPos[2],
							NewFace.m_NodeUndeformPos[v],
							weights[0],
							weights[1],
							weights[2]);

			float wsum = 0;

			for(int w = 0 ; w < 3 ; w++)
			{
				GPClamp(weights[w] , 0.0f , 1.0f);
				wsum += weights[w];
			}

			if(wsum > FLT_EPSILON)
			{
				float InvSum = 1.0f / wsum;
				weights[0] *= InvSum;
				weights[1] *= InvSum;
				weights[2] *= InvSum;
			}

			NewFace.m_TextureCoord[v] = splitFromData.m_TextureCoord[0]*weights[0]
									   +splitFromData.m_TextureCoord[1]*weights[1]
									   +splitFromData.m_TextureCoord[2]*weights[2];
									   */
		}
		NewFace.vi[0] = NewFace.vi[1] = NewFace.vi[2] = -1;//will be reconstruct in function "RebuildRendVertexAndIndex"
		NewFace.BuildConstParameter();
		NewFace.m_BurnValue = splitFromData.m_BurnValue;
		
		if(InOriginPart)
		{
			int ResultIndex = -1;

			if(m_FreeOriginFaceHead >= 0)
			{
				ResultIndex = m_FreeOriginFaceHead;
				int NextFreedIndex = m_OriginFaces[ResultIndex].m_NextFreed;

				m_OriginFaces[ResultIndex] = NewFace;
				m_OriginFaces[ResultIndex].m_NextFreed = -1;
				NewFace.m_physface->m_UserData = (void*)BUILDORIGINFACEUSRDATA(ResultIndex);

				m_FreeOriginFaceHead = NextFreedIndex;
			}
			else
			{
				NewFace.m_physface->m_UserData = (void*)BUILDORIGINFACEUSRDATA(m_OriginFaces.size());
				m_OriginFaces.push_back(NewFace);
				ResultIndex = m_OriginFaces.size()-1;
			}

			return m_OriginFaces[ResultIndex];
		}
		else
		{
			int ResultIndex = -1;

			if(m_FreeCutFaceHead >= 0)
			{
				ResultIndex = m_FreeCutFaceHead;
				int NextFreedIndex = m_CutCrossfaces[ResultIndex].m_NextFreed;

				m_CutCrossfaces[ResultIndex] = NewFace;
				m_CutCrossfaces[ResultIndex].m_NextFreed = -1;
				NewFace.m_physface->m_UserData = (void*)BUILCUTCROSSFACEUSRDATA(ResultIndex);

				m_FreeCutFaceHead = NextFreedIndex;
			}
			else
			{
				NewFace.m_physface->m_UserData = (void*)BUILCUTCROSSFACEUSRDATA(m_CutCrossfaces.size());
				m_CutCrossfaces.push_back(NewFace);
				ResultIndex = (int)m_CutCrossfaces.size()-1;
			}

			return m_CutCrossfaces[ResultIndex];
		}
}
void MisMedicOrgan_Ordinary::ExtractCutInformation(std::vector<Ogre::Vector2>  & cuttedTexCoords, GFPhysVector3 cutQuadVert[4])
{
	//extract "m_CutCrossFacesInLastCut" and cut texture coordinate
	//std::set<GFPhysSoftBodyNode*> NodesInCutCrossFaces;
	m_CutCrossFacesInLastCut.clear();
	m_SplitFacesInLastCut.clear();
	std::vector<GFPhysSoftBodyFace*> FacesNearCut;
	GFPhysVector3 center = (cutQuadVert[0] + cutQuadVert[1] + cutQuadVert[2] + cutQuadVert[3])*0.25f;
	float range = 0;
	for (int c = 0; c < 4; c++)
	{
		if ((cutQuadVert[c] - center).Length() > range)
			range = (cutQuadVert[c] - center).Length();
	}

	//select face new cutted 
	SelectPhysFaceAroundPoint(FacesNearCut, center, range*1.25f, true);
	int currentEpoch = m_physbody->GetCurrentEpoch();

	for (int c = 0; c < (int)FacesNearCut.size(); c++)
	{
		GFPhysSoftBodyFace * face = FacesNearCut[c];
		//face cutted by this time
		if (face->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra->m_CurrentEpoch >= currentEpoch)
		{
			if (face->m_IsCutCrossFace)
			{
				m_CutCrossFacesInLastCut.insert(face);
			}
			else
			{
				for (int v = 0; v < 3; v++)
				{
					GFPhysSoftBodyNode * node = face->m_Nodes[v];
					if (node->m_CurrentEpoch >= currentEpoch)
					{
						cuttedTexCoords.push_back(Ogre::Vector2(face->m_TexCoordU[v], face->m_TexCoordV[v]));
					}
				}
				m_SplitFacesInLastCut.insert(face);

			}
		}
	}
}
//=============================================================================================================================
void MisMedicOrgan_Ordinary::PostModifyTopology(const GFPhysAlignedVectorObj<GFPhysCuttedNewFace> & splitfaceData , 
											    const GFPhysAlignedVectorObj<GFPhysCuttedNewFace> & newCutfaceData , 
											    const GFPhysAlignedVectorObj<GFPhysNodeCreatedBySplit> & nodesCreateData,
											    std::vector<FaceSplitByCut> & FacesSplitted)//,
											    //std::vector<Ogre::Vector2>  & organTexCoord)
{	
	//used for receiveCheckPointList , just save the texcoord from the origin organ

	//interpolated value like blood burn etc for new created physics node data
	/*for (size_t c = 0; c < nodesCreateData.size(); c++)
	{
		const GFPhysNodeCreatedBySplit & splitNoda = nodesCreateData[c];

		int NewNodeIndex = (int)(splitNoda.m_NewNode->m_UserPointer);
		
		if( NewNodeIndex > 0 && NewNodeIndex < (int)m_PhysNodeData.size())
		{
			float BurnValue = 0;
			float BloodValue = 0;
			Ogre::Vector2 texCoord(0,0);
			for(int n = 0; n < splitNoda.m_NumSuperNode ; n++)
			{
				int SuperIndex = (int)(splitNoda.m_SuperNode[n]->m_UserPointer);
				float SuperWeight = splitNoda.m_Weights[n];
				BurnValue  += m_PhysNodeData[SuperIndex].m_burnValue  * SuperWeight;
				BloodValue += m_PhysNodeData[SuperIndex].m_bloodValue * SuperWeight;
				texCoord   += m_PhysNodeData[SuperIndex].m_NodeTextureCoord * SuperWeight;
			}
			m_PhysNodeData[NewNodeIndex].m_burnValue = BurnValue;
			m_PhysNodeData[NewNodeIndex].m_bloodValue = BloodValue;
			m_PhysNodeData[NewNodeIndex].m_NodeTextureCoord = texCoord;
		}
	}*/
	

	
	//Add splatted face - face Generated by split on last exists face
	for(size_t f = 0 ; f < splitfaceData.size() ; f++)
	{
		const GFPhysCuttedNewFace * newFace = &splitfaceData[f];

		GFPhysSoftBodyFace * splitface = splitfaceData[f].m_NewFace;

		void * splitFromData = splitfaceData[f].m_SplittedUserData;

		bool isNewCutFace = splitfaceData[f].m_IsNewCutFace;

		if(isNewCutFace == false)
		{
			int SplitFromIndexData = (int)splitFromData;

			int SplitFromMatID, SplitFromFaceid;

			//Get Origin MMO_Face Index and Material
			ExtractFaceIdAndMaterialIdFromUsrData(SplitFromIndexData , SplitFromMatID , SplitFromFaceid);

			MMO_Face SplitFromFaceData;

			if(SplitFromFaceData.m_physface != 0)
			{
				GFPhysGlobalConfig::GetGlobalConfig().GetLog()->logMessage("error splatted face not clear phys face in MMO_Face!!!");
			}
			//splatted face is split from origin face
			if(SplitFromMatID == ORIGINMATERIALID)
				SplitFromFaceData = m_OriginFaces[SplitFromFaceid];

			//splatted face is split from cut face
			else if(SplitFromMatID == CUTMATERIALID)
				SplitFromFaceData = m_CutCrossfaces[SplitFromFaceid];

			//now add new Rend Face
			const MMO_Face & NewRendFace = AddSplittedMMO_FaceInternal(SplitFromFaceData , splitface , SplitFromMatID == ORIGINMATERIALID ? true : false);

			FacesSplitted.push_back(FaceSplitByCut(SplitFromFaceid , SplitFromMatID , splitface));
		}	
		else
		{
			assert(0 && "error returned split face");
		}
	}

	//Add Cut Cross Face - Face New Generated add them to "m_CutCrossfaces" array
	for(size_t f = 0 ; f < newCutfaceData.size() ; f++)
	{
		const GFPhysCuttedNewFace * newcutFaceT = &newCutfaceData[f];

		GFPhysSoftBodyFace * newCutFace = newcutFaceT->m_NewFace;

		int ResultIndex;

		AddNewCuttedMMO_FaceInternal(newCutFace , ResultIndex);
	}

	
}
//===========================================================================================================================
/*
void MisMedicOrgan_Ordinary::CreateBloodScatterPointInCutCrossFace()
{
	std::set<GFPhysSoftBodyFace*>::iterator itor = m_CutCrossFacesInLastCut.begin();
	while(itor != m_CutCrossFacesInLastCut.end())
	{
		//切割面渗血效果
		GFPhysSoftBodyFace * CutCrossFace = (*itor);
			
		if(CutCrossFace && CutCrossFace->m_GenFace)
		{
			GFPhysGeneralizedFace * genFace = CutCrossFace->m_GenFace;

			PhysNode_Data & nodeData0 = GetPhysNodeData(CutCrossFace->m_Nodes[0]);
			PhysNode_Data & nodeData1 = GetPhysNodeData(CutCrossFace->m_Nodes[1]);
			PhysNode_Data & nodeData2 = GetPhysNodeData(CutCrossFace->m_Nodes[2]);

			float burnValue = nodeData0.m_burnValue + nodeData1.m_burnValue + nodeData2.m_burnValue;

			if(genFace && burnValue <= 0.1f && genFace->m_ShareTetrahedrons.size() > 0)
			{
			   GFPhysSoftBodyTetrahedron * tetraBlood = genFace->m_ShareTetrahedrons[0].m_Hosttetra;

			   if(GetPhysTetraAppData(tetraBlood).m_ContainsVessel == true)
			   {
				   m_BloodPointCutCrossFaces.insert(CutCrossFace);
			   }
			}
		}
		itor++;
	}
}
*/
//=======================================================================
void MisMedicOrgan_Ordinary::PerformElectricCut(ITool * itool , GFPhysSoftBodyFace * face  , float weights[3])
{
	if(m_CreateInfo.m_objTopologyType == DOT_MEMBRANE)
		return;

	CTool* tool = dynamic_cast<CTool*>(itool);
	if (tool == NULL)
		return;

	//do real cut organ
	if(tool->ElectricCutOrgan(this , face , weights))
	{
		//notify train
		m_OwnerTrain->OnOrganCutByTool(this , true);

		MisNewTraining * pMisNewTraining = dynamic_cast<MisNewTraining*>(m_OwnerTrain);
		if(pMisNewTraining != NULL)
		{
			int faceId = GetOriginFaceIndexFromUsrData(face);
			//only in origin face
			if(faceId >= 0  && faceId < (int)m_OriginFaces.size())
			{
				MMO_Face originface = m_OriginFaces[faceId];
				if(originface.m_HasError == false)
				{
					/*Ogre::Vector2 texturecoord = originface.m_TextureCoord[0] * weights[0]
					+originface.m_TextureCoord[1] * weights[1]
					+originface.m_TextureCoord[2] * weights[2];*/
					Ogre::Vector2 texturecoord = originface.GetTextureCoord(0) * weights[0]
						                       + originface.GetTextureCoord(1) * weights[1]
						                       + originface.GetTextureCoord(2) * weights[2];

					std::vector<Ogre::Vector2> uv;
					std::vector<Ogre::Vector2> unused;
					uv.push_back(texturecoord);

					pMisNewTraining->receiveCheckPointList(MisNewTraining::OCPT_ElecCut, uv , unused , tool , this);

					//make tear organ event
					GFPhysVector3 tearPoint = originface.m_NodeUndeformPos[0] * weights[0]
												+ originface.m_NodeUndeformPos[1] * weights[1]
												+ originface.m_NodeUndeformPos[2] * weights[2];
					MxSliceOffOrganEvent * pEvent = (MxSliceOffOrganEvent*)MxEvent::CreateEvent(MxEvent::MXET_SliceOffOrgan);
					pEvent->SetOrgan(this);
					pEvent->SetSlicePoint(tearPoint);
				}
			}
		}
	}

	m_lastEleCutFace = face;
	ElecCutInfo::s_cutSucceed = false;
	for(size_t i = 0; i < m_elecCutInfos.size() ; i++)
	{
		ElecCutInfo & cutInfo = m_elecCutInfos[i];
		if(!cutInfo.m_isCut)
		{
			std::vector<GFPhysSoftBodyFace*> &faces = cutInfo.m_facesSatisfied;
			for(size_t f = 0; f < faces.size() ; f++)
			{
				if(face == faces[f])
				{
					cutInfo.m_isCut = true;
					ElecCutInfo::s_cutSucceed = true;
					break;
				}
			}
			if(ElecCutInfo::s_cutSucceed)
				break;
		}
	}
}
//===========================================================================================
void MisMedicOrgan_Ordinary::GetLastTimeCutCrossFaces(std::vector<GFPhysSoftBodyFace*> & result)
{
	std::set<GFPhysSoftBodyFace*>::iterator itor = m_CutCrossFacesInLastCut.begin();
	while(itor != m_CutCrossFacesInLastCut.end())
	{
		GFPhysSoftBodyFace * physFace = (*itor);
		result.push_back(physFace);
		itor++;
	}
}
//================================================================================================
void MisMedicOrgan_Ordinary::SetTetraCollapseParam(float ratio, int minCutTimes)
{
	m_BadTetCollapseParam.first = ratio;
	m_BadTetCollapseParam.second = minCutTimes;
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::CutOrganByTool(CTool * tool)
{
	//@Step1 Cut mensetary PartFirst
	//ElectricCutMensetaryImp(tool);//cut mesentary part first

	//@Step2 Cut other part construct cut geometry and cut
    GFPhysVector3 cutQuadVert[4];
    tool->GetToolCutPlaneVerts(cutQuadVert);
    if (m_CreateInfo.m_IsMensentary == false)//if this organ has tissue part
    {
        float cutWidth = tool->GetCutWidth() * m_CutWidthScale;
        CutBySemiInfinitQuadImp(cutWidth, cutQuadVert, true);
    }
	m_physbody->ManuallyUpdateNodeNormals();

	//@step3 create blood stream on cut face
	std::vector<Ogre::Vector2> cuttedTexCoords;
	ExtractCutInformation(cuttedTexCoords, cutQuadVert);

	MisNewTraining * misntrain = dynamic_cast<MisNewTraining*>(m_OwnerTrain);
	if (misntrain)
	{
		std::vector<Ogre::Vector2>  unused;
		misntrain->receiveCheckPointList(MisNewTraining::OCPT_Cut, cuttedTexCoords, unused, NULL, this);
	}

	GFPhysVector3 cutPlaneNormal = (cutQuadVert[1] - cutQuadVert[0]).Cross(cutQuadVert[2] - cutQuadVert[0]).Normalized();
	CreateBloodStreamInCutFace(cutPlaneNormal);//particle blood stream will be deprecated

	//
	RefreshDirtyData();

	//create injury volume blood
	float minDist = FLT_MAX;
	
	GFPhysSoftBodyFace * selectface = 0;
	GFPhysVector3 selectFacePt;

	GFPhysVector3 cutCentLinePt[2];
	cutCentLinePt[0] = (cutQuadVert[0] + cutQuadVert[2])*0.5f;
	cutCentLinePt[1] = (cutQuadVert[1] + cutQuadVert[3])*0.5f;

	std::set<GFPhysSoftBodyFace*>::iterator litor = m_SplitFacesInLastCut.begin();
	GFPhysVector3 AppromCenter(0,0,0);
	int numNode = 0;
	while (litor != m_SplitFacesInLastCut.end())
	{
		GFPhysSoftBodyFace * physFace = (*litor);
		for (int n = 0; n < 3; n++)
		{
			if (physFace->m_Nodes[n]->m_CurrentEpoch >= m_physbody->GetCurrentEpoch())
			{
				AppromCenter += physFace->m_Nodes[n]->m_UnDeformedPos;
				numNode++;
			}
		}
		litor++;
	}
	AppromCenter /= numNode;

	litor = m_SplitFacesInLastCut.begin();
	while (litor != m_SplitFacesInLastCut.end())
	{
		GFPhysSoftBodyFace * physFace = (*litor);
		GFPhysVector3 triVertPos[3];
		triVertPos[0] = physFace->m_Nodes[0]->m_UnDeformedPos;
		triVertPos[1] = physFace->m_Nodes[1]->m_UnDeformedPos;
		triVertPos[2] = physFace->m_Nodes[2]->m_UnDeformedPos;

		GFPhysVector3 pointOnTri;
		GFPhysVector3 pointOnSeg;
		GFPhysVector3 collideNormOnTri;

		ClosetPtSegmentTriangle(triVertPos,cutCentLinePt,pointOnTri,pointOnSeg,collideNormOnTri);

		pointOnTri = ClosestPtPointTriangle(AppromCenter, triVertPos[0], triVertPos[1], triVertPos[2]);

		Real distSqr = (AppromCenter - pointOnTri).Length();

		if (distSqr < minDist)
		{
			minDist = distSqr;
			selectface = physFace;
			selectFacePt = pointOnTri;
		}
		litor++;
	}
	if (selectface)
	{
		int faceID = GetOriginFaceIndexFromUsrData(selectface);

		const MMO_Face & FaceData = GetMMOFace_OriginPart(faceID);

		if (FaceData.m_HasError == false && FaceData.m_BurnValue <= 0.0001f)
		{
			float pFaceWeight[3];
			CalcBaryCentric(selectface->m_Nodes[0]->m_UnDeformedPos,
				selectface->m_Nodes[1]->m_UnDeformedPos,
				selectface->m_Nodes[2]->m_UnDeformedPos,
				selectFacePt,
				pFaceWeight[0], pFaceWeight[1], pFaceWeight[2]
				);

			
			for (size_t c = 0; c < m_InjuryPoints.size(); c++)
			{
				m_InjuryPoints[c].m_VolumeBlood = 0;
			}
			ApplyEffect_VolumeBlood(selectface->m_uid, pFaceWeight);

			AddInjuryPoint(selectface, pFaceWeight, m_VolumeBlood);
		}
	}
	
	
	

	//@Step4 call train call back
	m_OwnerTrain->OnOrganCutByTool(this , false);

	//@call all attachments attach cut event
	for(size_t t = 0 ; t < m_OrganAttachments.size();t++)
	{
		m_OrganAttachments[t]->OnCutByToolFinish();
	}
	
	MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_Cut, tool, 0);
	pEvent->SetOrgan(this);

	CMXEventsDump::Instance()->PushEvent(pEvent , true);

}
//===========================================================================================================
void MisMedicOrgan_Ordinary::TearOrganBySemiInfinteQuad(GFPhysVector3 quadVerts[4] , bool InDeformSpace)
{
	CutBySemiInfinitQuadImp(0.1f , quadVerts , InDeformSpace);
	//
	RefreshDirtyData();
}
//===========================================================================================================
void MisMedicOrgan_Ordinary::TearOrganByCustomedGeomtry(GFPhysISoftBodyCutSurface * cutsurface , bool InDeformSpace)
{
	GFPhysSoftBodyCutter cutter(m_physDynWorld , m_physbody);
	cutter.CutBodyByCustomedGeomtry(m_physbody , 
									cutsurface , 
									m_MaxCutCount,
									m_CreateInfo.m_CutThreshold);

	onTopologyChanged(cutter);
	//
	RefreshDirtyData();
}
//==================================================================================================
void MisMedicOrgan_Ordinary::EliminateTetras(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron *> & tetrasToRemove)
{
	GFPhysSoftBodyTetrasRemover tetraRemover(m_physDynWorld , m_physbody);
	tetraRemover.RemoveTetrahedrons(tetrasToRemove);
	onTopologyChanged(tetraRemover);
	RefreshDirtyData();
}
//==================================================================================================
void MisMedicOrgan_Ordinary::RemoveSubParts(int subPartIndex)
{
	if (subPartIndex >= 0 && subPartIndex < GetNumSubParts())
	{
		GFPhysSubConnectPart * subpart = GetSubPart(subPartIndex);
		EliminateTetras(subpart->m_Tetras);
	}
}
//==================================================================================================
void MisMedicOrgan_Ordinary::DestroyTissueAroundNode(GFPhysSoftBodyFace * physFace, float weights[3], bool indeformspace)
{
	//int destCutLayer = physFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra->m_LayerIndex;
	//GFPhysVector3 destoryCenter = hollowcenter->m_UnDeformedPos;
	std::vector<GFPhysSoftBodyTetrahedron *> tetrasAround;
	GFPhysVector3  SeledDestoryCenter;

	//cut tissue tetrahedron apart first
	uint32 destCat = physFace->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra->m_ElementCategory;
	destCat &= m_EleCatogeryCanCut;

	float finalelecradius = m_ElecCutRadius;
	if (destCat)
	{
		GFPhysVector3  UndeformedFacePos[3];
		GFPhysVector3  DeformedFacePos[3];

		GFPhysVector3  UndeformedTouchPos;

		GFPhysVector3  DeformedTouchPos;

		UndeformedFacePos[0] = physFace->m_Nodes[0]->m_UnDeformedPos;
		UndeformedFacePos[1] = physFace->m_Nodes[1]->m_UnDeformedPos;
		UndeformedFacePos[2] = physFace->m_Nodes[2]->m_UnDeformedPos;

		DeformedFacePos[0] = physFace->m_Nodes[0]->m_CurrPosition;
		DeformedFacePos[1] = physFace->m_Nodes[1]->m_CurrPosition;
		DeformedFacePos[2] = physFace->m_Nodes[2]->m_CurrPosition;

		UndeformedTouchPos = UndeformedFacePos[0] * weights[0]
			               + UndeformedFacePos[1] * weights[1]
						   + UndeformedFacePos[2] * weights[2];

		DeformedTouchPos = DeformedFacePos[0] * weights[0]
			             + DeformedFacePos[1] * weights[1]
			             + DeformedFacePos[2] * weights[2];

		int selectNode = -1;
		
		float minDist = FLT_MAX;
		
		for (int c = 0; c < 3; c++)
		{
			float dist = (UndeformedTouchPos - UndeformedFacePos[c]).Length();
			
			if(dist < minDist)
			{
				SeledDestoryCenter = (indeformspace ? physFace->m_Nodes[c]->m_CurrPosition : UndeformedFacePos[c]);
				
				minDist = dist;
				
				if (dist < m_ElecCutRadius*0.75f)
				{
					selectNode = c;
				}
			}
		}

		//all 3 nodes dist from destroy pos is large than thres hold
		if (selectNode < 0)
		{
			GFPhysVector3 ptOnEdgeUndeformed;

			GFPhysVector3 ptOnEdgeDeformspace;

			float minDist = FLT_MAX;

			int selectedEdge = -1;
			
			for (int c = 0; c < 3; c++)
			{
				GFPhysVector3 e0 = DeformedFacePos[c];
				
				GFPhysVector3 e1 = DeformedFacePos[(c + 1) % 3];

				GFPhysVector3 temp = CloasetPtToSegment(DeformedTouchPos , e0 , e1);

				float dist = (temp - DeformedTouchPos).Length();

				if (dist < minDist)
				{
					selectedEdge = c;
					
					float w = (temp - e0).Dot(e1 - e0) / (e1 - e0).Length2();

					ptOnEdgeDeformspace = temp;

					ptOnEdgeUndeformed = UndeformedFacePos[c] * (1 - w) + UndeformedFacePos[(c + 1) % 3] * w;
					
					minDist = dist;
				}
			}

			if (selectedEdge >= 0)
			{//check another intersect pt
				GFPhysVector3 ptOtherEdge;
				
				int OtherEdge = -1;
				
				GFPhysVector3 normal = (UndeformedFacePos[selectedEdge] - UndeformedFacePos[(selectedEdge + 1) % 3]).Normalized();
				
				for (int c = 0; c < 3; c++)
				{
					if (c != selectedEdge)
					{
						GFPhysVector3 e0 = UndeformedFacePos[c];

						GFPhysVector3 e1 = UndeformedFacePos[(c + 1) % 3];

						float dist0 = (e0 - UndeformedTouchPos).Dot(normal);
						
						float dist1 = (e1 - UndeformedTouchPos).Dot(normal);

						if (dist0 * dist1 < 0)
						{
							OtherEdge = c;
							
							float w = dist0 / (dist0 - dist1);

							ptOtherEdge = e0 * (1 - w) + e1 * w;
							
							break;
						}
					}
				 }

				 if (OtherEdge >= 0)
				 {
					 GFPhysVector3 faceNormal = (UndeformedFacePos[1] - UndeformedFacePos[0]).Cross(UndeformedFacePos[2] - UndeformedFacePos[0]).Normalized();

					 GFPhysVector3 tan = (ptOnEdgeUndeformed - ptOtherEdge).Normalized();

					 GFPhysVector3 splitQuadVert[4];
					
					 splitQuadVert[0] = ptOnEdgeUndeformed + tan * 0.05f;
					 splitQuadVert[1] = ptOtherEdge - tan * 0.05f;

					 splitQuadVert[2] = splitQuadVert[0] - faceNormal*m_ElecCutRadius;
					 splitQuadVert[3] = splitQuadVert[1] - faceNormal*m_ElecCutRadius;

					 splitQuadVert[0] += faceNormal*m_ElecCutRadius;
					 splitQuadVert[1] += faceNormal*m_ElecCutRadius;


					 GFPhysSoftBodyEdgeSplitDiviser subDiviser(m_physDynWorld, m_physbody, 0, 0);
					 subDiviser.m_InDeformedSpace = false;
					 subDiviser.m_CutCategory = destCat;
					 subDiviser.SplitTetrasSpanGap(m_physbody, &splitQuadVert[0], &splitQuadVert[2], 0);
					 onTopologyChanged(subDiviser);

					 SeledDestoryCenter = (indeformspace ? ptOnEdgeDeformspace : ptOnEdgeUndeformed);
                   
                  #if(0)
					 float dist = (SeledDestoryCenter - (indeformspace ? DeformedTouchPos : UndeformedTouchPos)).Length();
					 if (dist > finalelecradius * 1.2f)
					 {
						 finalelecradius = dist;
					 }
                  #endif
				}
			}
		}
		

		//
		GFPhysSoftBodyCutter cutter(m_physDynWorld , m_physbody , 0.001f , 0.001f , indeformspace);
		cutter.m_CutCategory = destCat;
		
		SphereCutSurface spherecutface(this, SeledDestoryCenter, finalelecradius, finalelecradius*0.1f, indeformspace);
		cutter.CutBodyByCustomedGeomtry(m_physbody , &spherecutface , m_MaxCutCount , 0);
		onTopologyChanged(cutter);

		//eliminate fat tetrahedron and cut tissue tetrahedron now
		tetrasAround.clear();
		CollectTetrasAroundPoint(SeledDestoryCenter, finalelecradius, tetrasAround, destCat, indeformspace);
		spherecutface.ExtractInnerTetras(tetrasAround);
	}
	/*
	else
	{
		//eliminate fat tetrahedron and cut tissue tetrahedron now
		tetrasAround.clear();
		CollectTetrasAroundPoint(SeledDestoryCenter, 0.01f, tetrasAround, EMMT_BeTissue | EMMT_BeMesentary, indeformspace);
	}
	*/

	std::vector<GFPhysSoftBodyTetrahedron *> TetraToRemove;

	for(size_t t = 0 ; t < tetrasAround.size() ; t++)
	{
		GFPhysSoftBodyTetrahedron * tetra = tetrasAround[t];

		if (tetra)
		{
			TetraToRemove.push_back(tetra);
		}
	}

	if(TetraToRemove.size() > 0)
	{
		{//send event
			MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_StartElectricHollow, 0 , this);//send electric hollow event first
			for(size_t t = 0 ; t < TetraToRemove.size() ; t++)
				pEvent->m_OperatorTetras.push_back(TetraToRemove[t]);
			CMXEventsDump::Instance()->PushEvent(pEvent , true);
		}

		GFPhysSoftBodyTetrasRemover tetraRemover(m_physDynWorld , m_physbody);
		tetraRemover.RemoveTetrahedrons(TetraToRemove);
		onTopologyChanged(tetraRemover);
	}

#if(USECUTREFINE)
	//if (m_EleCatogeryCanCut & EMMT_BeTissue)
	{
		GFPhysEdgeCollapse edgeCollapse(m_physDynWorld, m_physbody, m_BadTetCollapseParam.first, m_BadTetCollapseParam.second);
		edgeCollapse.ApplyEdgeCollapse(m_physbody->GetSoftBodyShape(), m_physbody->GetCurrentEpoch());
		onTopologyChanged(edgeCollapse);
	}
#endif

	m_physbody->ManuallyUpdateNodeNormals();

	for(size_t t = 0 ; t < m_OrganAttachments.size();t++)
	{
		m_OrganAttachments[t]->OnCutByToolFinish();
	}
	return;
}

//=======================================================================================================
void MisMedicOrgan_Ordinary::CutBySemiInfinitQuadImp(float cutWidth , GFPhysVector3 cutQuadVert[4] , bool InDeformSpace)
{
	MisNewTraining * misNewTrain = dynamic_cast<MisNewTraining*>(CTrainingMgr::Get()->GetCurTraining());
	
	//ensure the Cut Quad lie same plane
	GFPhysVector3 QuadPlaneNorm = (cutQuadVert[1]-cutQuadVert[0]).Cross(cutQuadVert[2]-cutQuadVert[0]).Normalized();
	float correctDist = (cutQuadVert[3]-cutQuadVert[0]).Dot(QuadPlaneNorm);
	cutQuadVert[3] -= QuadPlaneNorm*correctDist;
	
	//if cut width great than zero
	if(cutWidth > 0.001f)
	{
		//for correct cut we must first split edges which is span the cut width region
		GFPhysVector3 splitQuadVert[4];
		for(int c = 0 ; c < 4 ; c++)
		{
			splitQuadVert[c] = cutQuadVert[c];
		}
		splitQuadVert[0] += (cutQuadVert[0]-cutQuadVert[1]).Normalized()*0.05f;
		splitQuadVert[2] += (cutQuadVert[2]-cutQuadVert[3]).Normalized()*0.05f;

		GFPhysSoftBodyEdgeSplitDiviser subDiviser(m_physDynWorld , m_physbody , 0 , 0);
	    subDiviser.SplitTetrasSpanGap(m_physbody , &splitQuadVert[0] , &splitQuadVert[2] , cutWidth);
	    onTopologyChanged(subDiviser);
		
		cutQuadVert[1] = splitQuadVert[1];//copy back tail vertex back because "GFPhysSoftBodyEdgeSplitDiviser" may revision this
		cutQuadVert[3] = splitQuadVert[3];

		//now do real cut us cut surface surround the cut Region
	    CutQuadWithThickNess cutsurface(cutQuadVert[0] , cutQuadVert[1] , cutQuadVert[2] , cutQuadVert[3] , cutWidth);
	    GFPhysSoftBodyCutter cutter(m_physDynWorld , m_physbody , 0.0001f , 0.0001f);
	    cutter.CutBodyByCustomedGeomtry(m_physbody , &cutsurface, m_MaxCutCount , 0);        
	    onTopologyChanged(cutter);

			
		//remove tetras inner the cut width region
	    GFPhysVectorObj<GFPhysSoftBodyTetrahedron *> tetrasToRemove;
	    tetrasToRemove.reserve(cutter.m_TetrasCreated.size());

	    int sbepoch = m_physbody->GetCurrentEpoch();

	    for(size_t c = 0 ; c < cutter.m_TetrasDeleted.size() ; c++)
	    {
		    cutsurface.m_IntersectTetras.erase(cutter.m_TetrasDeleted[c]);
	    }

	    for(size_t c = 0 ; c < cutter.m_TetrasCreated.size() ; c++)
	    {
		    GFPhysSoftBodyTetrahedron * tetra = cutter.m_TetrasCreated[c];
			GFPhysVector3 centPos = (tetra->m_TetraNodes[0]->m_CurrPosition 
				                    +tetra->m_TetraNodes[1]->m_CurrPosition
				                    +tetra->m_TetraNodes[2]->m_CurrPosition
				                    +tetra->m_TetraNodes[3]->m_CurrPosition)*0.25f;

			if (!cutsurface.IsInSideEnvelope(centPos))
				continue;

			bool inner = true;
		    for(int n = 0 ; n < 4 ; n++)
		    {
				if(tetra->m_TetraNodes[n]->m_CurrentEpoch < sbepoch)
				{
					if(cutsurface.IsInSideEnvelope(tetra->m_TetraNodes[n]->m_CurrPosition))
					{
						tetrasToRemove.push_back(tetra);
						break;
					}
				}
		    }
	    }
		
		std::set<GFPhysSoftBodyTetrahedron*>::iterator itor = cutsurface.m_IntersectTetras.begin();
	    while(itor != cutsurface.m_IntersectTetras.end())
	    {
			GFPhysSoftBodyTetrahedron * tetra = (*itor);

			//GFPhysVector3 centPos = (tetra->m_TetraNodes[0]->m_CurrPosition
								   // +tetra->m_TetraNodes[1]->m_CurrPosition
								   // +tetra->m_TetraNodes[2]->m_CurrPosition
								   // +tetra->m_TetraNodes[3]->m_CurrPosition)*0.25f;


			//if(cutsurface.IsInSideEnvelope(centPos))
			bool inner = true;
			for (int n = 0; n < 4; n++)
			{
				if (!cutsurface.IsInSideEnvelope(tetra->m_TetraNodes[n]->m_CurrPosition))
				{
					inner = false;
					break;
				}
			}
			if (inner)
			   tetrasToRemove.push_back(tetra);
			itor++;
	    }

	   GFPhysSoftBodyTetrasRemover tetraRemover(m_physDynWorld , m_physbody);
	   tetraRemover.RemoveTetrahedrons(tetrasToRemove);       
	   onTopologyChanged(tetraRemover);
#if(USECUTREFINE)
	   GFPhysEdgeCollapse edgeCollapse(m_physDynWorld, m_physbody, m_BadTetCollapseParam.first, m_BadTetCollapseParam.second);
	   int NumRefined = edgeCollapse.ApplyEdgeCollapse(m_physbody->GetSoftBodyShape(), m_physbody->GetCurrentEpoch());
	   onTopologyChanged(edgeCollapse);
#endif
	}
    else
    {
		//not supported
		assert(0 && "not support unwidth cut");
    }
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::onTopologyChanged(GFPhysSoftBodyTopologyModifier & cData)
{
    //GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> NewTetraMapOld;

    std::map<GFPhysSoftBodyTetrahedron *, std::vector<GFPhysSoftBodyTetrahedron *>> DelToSplitTetraMap;

    for (int c = 0; c < (int)cData.m_TetrasDeleted.size(); c++)
    {
        DelToSplitTetraMap.insert(std::make_pair(cData.m_TetrasDeleted[c], std::vector <GFPhysSoftBodyTetrahedron *>()));
    }

    for (int c = 0; c < (int)cData.m_TetrasCreated.size(); c++)
    {
        GFPhysSoftBodyTetrahedron * NewTetra = cData.m_TetrasCreated[c];

        PhysTetra_Data & tetraData = GetPhysTetraAppData(NewTetra);//in this time user pointer point to the be cutted tetras

        GFPhysSoftBodyTetrahedron * DelTetra = tetraData.m_PhysTetra;

        std::map<GFPhysSoftBodyTetrahedron *, std::vector<GFPhysSoftBodyTetrahedron *>>::iterator itor = DelToSplitTetraMap.find(DelTetra);

        if (itor == DelToSplitTetraMap.end())
        {
            int i = 0;//shouldn't move there
            int j = i + 1;
        }
        itor->second.push_back(NewTetra);
    }


    NewTetrasAdded(cData.m_TetrasCreated);
    NewNodesAdded(cData.m_NodesCreated);

    std::vector<FaceSplitByCut> FacesSplitted;

    PostModifyTopology(cData.m_SplitFaces, cData.m_NewCuttedFaces, cData.m_NewSplittedNodes, FacesSplitted);// , organTexCoord);

    RebuildVeinConnect(FacesSplitted, cData.m_FacesDeleted);
    //////////////////////////////////////////////////////////////////////////
    MisNewTraining * newtrain = dynamic_cast<MisNewTraining*>(m_OwnerTrain);
    if (newtrain)
    {
        GFPhysEdgeCollapse * edgeCollapser = dynamic_cast<GFPhysEdgeCollapse*>(&cData);
        if (edgeCollapser)
            newtrain->RefreshVeinconnectOnFacesBeModified(edgeCollapser->m_FacesBeModified, this);
    }
    //////////////////////////////////////////////////////////////////////////

	//put remove behind post cut operation because split face may use removed face data
	TetrasRemoved(cData.m_TetrasDeleted);
	FacesRemoved(cData.m_FacesDeleted);
	NodesRemoved(cData.m_NodesDeleted);

	//@@ Now Call Back Train
	//call all attachments face slitted
	for(size_t t = 0 ; t < m_OrganAttachments.size();t++)
	{
		m_OrganAttachments[t]->OnFaceSplittedByCut(FacesSplitted ,cData.m_FacesDeleted, this);
	}

	//call new train
	//MisNewTraining * newtrain = dynamic_cast<MisNewTraining*>(m_OwnerTrain);
	if(newtrain)
	{
	   newtrain->UpdateNodeToTetraLinks(DelToSplitTetraMap, cData.m_TetrasCreated, this);
	   newtrain->FacesBeAdded(cData.m_FacesCreated , this);

	   GFPhysEdgeCollapse * edgeCollapser = dynamic_cast<GFPhysEdgeCollapse*>(&cData);
	   if (edgeCollapser)
           newtrain->FacesBeModified(edgeCollapser->m_FacesBeModified, cData.m_FacesCreated, this);

       newtrain->FacesBeRemoved(cData.m_FacesDeleted, cData.m_FacesCreated, this);
	   newtrain->NodesBeRemoved(cData.m_NodesDeleted , this);
	   newtrain->TetrasBeRemoved(cData.m_TetrasDeleted , this); 
	}

	m_TopologyDirty = true;
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::RebuildVeinConnect(std::vector<FaceSplitByCut> & FacesSplitted , 
                                                GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & FacesDeleted)
{
    std::set<MMO_Face::VeinClusterInfo> clusterToProcess;

    for (int c = 0; c < FacesDeleted.size(); c++)
    {
         MMO_Face & mmoface = GetMMOFace(FacesDeleted[c]);
       
         if (mmoface.m_HasError)
             continue;

         for (int t = 0; t < mmoface.m_VeinInfoVector.size(); t++)
         {
             const MMO_Face::VeinInfo & temp = mmoface.m_VeinInfoVector[t];

             MMO_Face::VeinconnPosLocal LocalLocate = temp.local;

             //mark pair point as negative represent deleted face
             VeinConnectCluster & cluster = (temp.veinobj)->m_clusters[temp.clusterId];

             VeinConnectPair & Pair = ((LocalLocate == MMO_Face::VeinconnPosLocal::A || LocalLocate == MMO_Face::VeinconnPosLocal::B) ?
                                        cluster.m_pair[0] : cluster.m_pair[1]);
             int * pairFaceId;
             if (LocalLocate == MMO_Face::VeinconnPosLocal::A || LocalLocate == MMO_Face::VeinconnPosLocal::C)
             {
                 pairFaceId = &Pair.m_MMo_faceAID;
             }
             else
             {
                 pairFaceId = &Pair.m_MMo_faceBID;
             }
             (*pairFaceId) = -(*pairFaceId);
           
             //add cluster to process set
             MMO_Face::VeinClusterInfo info = { temp.veinobj, temp.clusterId };
             clusterToProcess.insert(info);
         }
    }

    //rebuild connection
    for (int c = 0; c < FacesSplitted.size(); c++)
    {
        GFPhysSoftBodyFace * newface = FacesSplitted[c].m_NewFace;

        MMO_Face & SplitFromFaceData = GetMMOFace_OriginPart(FacesSplitted[c].m_OldFaceID);

        if (SplitFromFaceData.m_HasError)
            continue;

        for (int t = 0; t < (int)SplitFromFaceData.m_VeinInfoVector.size(); t++)
        {
            MMO_Face::VeinInfo & info = SplitFromFaceData.m_VeinInfoVector[t];
           
            if (info.valid)
            {
                VeinConnectObject * veinobj = info.veinobj;
                if (veinobj)
                {
                    Real facePtWeights[3];
                    Real * pairweights = 0;
                    GFPhysSoftBodyFace ** pairLocateFace = 0;
                    GFPhysVector3 * ptUndeformedPos;

                    MMO_Face::VeinconnPosLocal LocalLocate = info.local;

                    VeinConnectCluster & cluter = veinobj->m_clusters[info.clusterId];

                    VeinConnectPair & Pair = ((LocalLocate == MMO_Face::VeinconnPosLocal::A || LocalLocate == MMO_Face::VeinconnPosLocal::B) ?
                                               cluter.m_pair[0] : cluter.m_pair[1]);

                    if (Pair.m_rebuildcount < MAXREBUILDCOUNT)
                    {
                        int * pairFaceId;
                        if (LocalLocate == MMO_Face::VeinconnPosLocal::A || LocalLocate == MMO_Face::VeinconnPosLocal::C)
                        {
                            pairLocateFace = &Pair.m_faceA;
                            ptUndeformedPos = &Pair.m_UndeformPointPosA;
                            pairweights = Pair.m_weightsA;
                            pairFaceId = &Pair.m_MMo_faceAID;
                        }
                        else
                        {
                            pairLocateFace = &Pair.m_faceB;
                            ptUndeformedPos = &Pair.m_UndeformPointPosB;
                            pairweights = Pair.m_weightsB;
                            pairFaceId = &Pair.m_MMo_faceBID;
                        }

                        CalcBaryCentric(newface->m_Nodes[0]->m_UnDeformedPos,
                                        newface->m_Nodes[1]->m_UnDeformedPos,
                                        newface->m_Nodes[2]->m_UnDeformedPos, (*ptUndeformedPos),
                                        facePtWeights[0],
                                        facePtWeights[1],
                                        facePtWeights[2]);

                        if (0.0f < facePtWeights[0] && facePtWeights[0] < 1.0f &&
                            0.0f < facePtWeights[1] && facePtWeights[1] < 1.0f &&
                            0.0f < facePtWeights[2] && facePtWeights[2] < 1.0f)
                        {
                            int faceId = GetOriginFaceIndexFromUsrData(newface);
                            //only in origin face
                            if (faceId >= 0 && faceId < (int)m_OriginFaces.size())
                            {
                                (*pairFaceId) = faceId;
                                (*pairLocateFace) = newface;
                                pairweights[0] = facePtWeights[0];
                                pairweights[1] = facePtWeights[1];
                                pairweights[2] = facePtWeights[2];
                                (*ptUndeformedPos) = newface->m_Nodes[0]->m_UnDeformedPos * facePtWeights[0]
                                                   + newface->m_Nodes[1]->m_UnDeformedPos * facePtWeights[1]
                                                   + newface->m_Nodes[2]->m_UnDeformedPos * facePtWeights[2];
                                Pair.m_rebuildcount++;

                                MMO_Face& splitface = m_OriginFaces[faceId];

                                MMO_Face::VeinInfo newinfo = { veinobj, info.clusterId, LocalLocate, true };
                                splitface.m_VeinInfoVector.push_back(newinfo);
                                //需要从SplitFromFaceData.m_VeinconnectOnFace中删除这一条
                                info.valid = false;
                            }
                        }
                    }
                }
            }
        }
    }

    //check any cluster has negative face id then delete this cluster
    std::set<MMO_Face::VeinClusterInfo>::iterator itor = clusterToProcess.begin();
    while (itor != clusterToProcess.end())
    {
        bool rebuildSucced = false; 

        //check rebuild succed or not
        MMO_Face::VeinClusterInfo info = (*itor);
        VeinConnectCluster & cluster = info.veinobj->m_clusters[info.clusterId];
        if (cluster.m_pair[0].m_MMo_faceAID < 0 || cluster.m_pair[0].m_MMo_faceBID < 0
         || cluster.m_pair[1].m_MMo_faceAID < 0 || cluster.m_pair[1].m_MMo_faceBID < 0)

        {
            rebuildSucced = false;
        }
        else
        {
            rebuildSucced = true;
            ((MisNewTraining*)m_OwnerTrain)->OnVeinconnectChanged(info.veinobj, info.clusterId);
        }
        //

        if (rebuildSucced == false)
        {
			info.veinobj->DestoryCluster(info.clusterId);//
            //((MisNewTraining*)m_OwnerTrain)->RemoveClustbyID(info.veinobj, info.clusterId);//
        }
        itor++;
    }
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::RefreshRendFacesUndeformedPosition()
{
	//refresh faces in origin part
	for(size_t f = 0 ; f < m_OriginFaces.size() ; f++)
	{
		MMO_Face & Organface = m_OriginFaces[f];

		if(Organface.m_physface)
		{
			for(int n = 0 ; n < 3 ; n++)
			{
				GFPhysSoftBodyNode * FaceNode = Organface.m_physface->m_Nodes[n];

				Organface.m_NodeUndeformPos[n] = FaceNode->m_UnDeformedPos;
			}
			//Organface.BuildConstParameter();
		}
	}

	//refresh faces in cut part
	for(size_t f = 0 ; f < m_CutCrossfaces.size() ; f++)
	{
		MMO_Face & Organface = m_CutCrossfaces[f];

		if(Organface.m_physface)
		{
			for(int n = 0 ; n < 3 ; n++)
			{
				GFPhysSoftBodyNode * FaceNode = Organface.m_physface->m_Nodes[n];

				Organface.m_NodeUndeformPos[n] = FaceNode->m_UnDeformedPos;
			}
			//Organface.BuildConstParameter();
		}
	}
}
//=======================================================================================================
//void MisMedicOrgan_Ordinary::onRemoveOldTetrahedron(const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & removedtetras)
//{
	//do nothing
//}
//=======================================================================================================
void MisMedicOrgan_Ordinary::NewTetrasAdded(const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> & tetras)
{
	for(size_t c = 0 ; c < tetras.size() ;c++)
	{
		GFPhysSoftBodyTetrahedron * tetra = tetras[c];
		if(m_PhysTetraData.size() == 0)//first one is for error reference
		{
			PhysTetra_Data errorData;
			errorData.m_HasError = true;
			m_PhysTetraData.push_back(errorData);
		}

		PhysTetra_Data dstData;

		int SrcIndex = (int)tetra->m_UserPointer;//split tetra
		if(SrcIndex > 0)//split tetrahedron has data
		{  
			//copy splatted tetrahedron data to this one
			PhysTetra_Data & srcData = m_PhysTetraData[SrcIndex];
			dstData = srcData;

			//reconstruct texture coordinate for 4 nodes of the new tetra
			/*
			GFPhysVector3 oldPos[4];
			
			oldPos[0] = srcData.m_PhysTetra->m_TetraNodes[0]->m_UnDeformedPos;
			oldPos[1] = srcData.m_PhysTetra->m_TetraNodes[1]->m_UnDeformedPos;
			oldPos[2] = srcData.m_PhysTetra->m_TetraNodes[2]->m_UnDeformedPos;
			oldPos[3] = srcData.m_PhysTetra->m_TetraNodes[3]->m_UnDeformedPos;

			Real * srcTexCoordU = srcData.m_PhysTetra->m_texCoordU;
			Real * srcTexCoordV = srcData.m_PhysTetra->m_texCoordV;

			for (int n = 0; n < 4; n++)
			{
				float   weights[4];
				
				bool succed = GetPointBarycentricCoordinate(oldPos[0],oldPos[1],oldPos[2],oldPos[3],
					                                        tetra->m_TetraNodes[n]->m_UnDeformedPos,
					                                        weights);

				if (succed)
				{
					tetra->m_texCoordU[n] = srcTexCoordU[0] * weights[0]
						                  + srcTexCoordU[1] * weights[1]
						                  + srcTexCoordU[2] * weights[2]
						                  + srcTexCoordU[3] * weights[3];

					tetra->m_texCoordV[n] = srcTexCoordV[0] * weights[0]
						                  + srcTexCoordV[1] * weights[1]
						                  + srcTexCoordV[2] * weights[2]
						                  + srcTexCoordV[3] * weights[3];
				}
				else
				{
					tetra->m_texCoordU[n] = tetra->m_texCoordV[n] = 0;
				}
			}*/
			//
		}
		dstData.m_PhysTetra = tetra;

		//add to data and set new user pointer
		if(m_FreePhysTetraDataHead > 0)
		{
		   int nextFreed = m_PhysTetraData[m_FreePhysTetraDataHead].m_NextFreed;
		   
		   m_PhysTetraData[m_FreePhysTetraDataHead] = dstData;
		  
		   tetra->m_UserPointer = (void*)(m_FreePhysTetraDataHead);

		   m_FreePhysTetraDataHead = nextFreed;
		}
		else
		{
		   m_PhysTetraData.push_back(dstData);
		   tetra->m_UserPointer = (void*)((int)m_PhysTetraData.size()-1);
		}
	}
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::NewNodesAdded(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode *> & nodes)
{
	for(size_t c = 0 ; c < nodes.size() ;c++)
	{
		GFPhysSoftBodyNode * node = nodes[c];
		if(m_PhysNodeData.size() == 0)//first one is for error reference
		{
			PhysNode_Data errorData;
			errorData.m_HasError = true;
			m_PhysNodeData.push_back(errorData);
		}

		int Index = (int)node->m_UserPointer;
		if(Index == 0)//no data
		{
			PhysNode_Data nData;
			nData.m_PhysNode = node;

			if(m_FreePhysNodeDataHead > 0)
			{
				int nextFreed = m_PhysNodeData[m_FreePhysNodeDataHead].m_NextFreed;

				m_PhysNodeData[m_FreePhysNodeDataHead] = nData;

				node->m_UserPointer = (void*)(m_FreePhysNodeDataHead);

				m_FreePhysNodeDataHead = nextFreed;
			}
			else
			{
				m_PhysNodeData.push_back(nData);

				node->m_UserPointer = (void*)((int)m_PhysNodeData.size()-1);
			}
		}
	}
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::NodesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode *> & nodes)
{
	for(size_t c = 0 ; c < nodes.size() ;c++)
	{
		GFPhysSoftBodyNode * node = nodes[c];

		int Index = (int)node->m_UserPointer;
		if(Index > 0 && Index < (int)m_PhysNodeData.size())//use > 0 because 0 element is error element
		{
			if(m_PhysNodeData[Index].m_PhysNode == node)
			{
			   m_PhysNodeData[Index].m_PhysNode = 0;//mark as removed
			   //add to free list
			   m_PhysNodeData[Index].m_NextFreed = m_FreePhysNodeDataHead;
			   m_FreePhysNodeDataHead = Index;
			}
		}

		//移除流血点 todo：optimize
		std::vector<SBleedPoint>::iterator iter = m_bleednodes.begin();
		while(iter != m_bleednodes.end())
		{
			if((*iter).node == node)
			{
				SBleedPoint& sbp = *iter;
				EffectManager::Instance()->removeVesselBleedEffect(sbp.effect);
				sbp.effect = NULL;
				iter = m_bleednodes.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::TetrasRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> & tetras)
{
	for(size_t c = 0 ; c < tetras.size() ;c++)
	{
		GFPhysSoftBodyTetrahedron * tetra = tetras[c];

		int Index = (int)tetra->m_UserPointer;
		if(Index > 0 && Index < (int)m_PhysTetraData.size())//use > 0 because 0 element is error element
		{
			if(m_PhysTetraData[Index].m_PhysTetra == tetra)
			{
			   m_PhysTetraData[Index].m_PhysTetra = 0;//mark as removed
			}
			//.add to free list
			m_PhysTetraData[Index].m_NextFreed = m_FreePhysTetraDataHead;
			m_FreePhysTetraDataHead = Index;
		}
		

		//notify all attachments
		for(size_t c = 0 ; c < m_OrganAttachments.size() ; c++)
		{
			if(m_OrganAttachments[c])
			   m_OrganAttachments[c]->OnRemoveTetrahedron(tetra);
		}
	}
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::FacesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace *> & faces)
{
	for(size_t c = 0 ; c < faces.size() ;c++)
	{
		GFPhysSoftBodyFace * faceToRemove = faces[c];
		//set MMO_Face as invalid face
		void * faceData = faceToRemove->m_UserData;
		if(faceData != 0)
		{
			int indexData = (int)faceData;

			int materialid = (indexData >> 16);

			int faceid = (indexData & 0xFFFF);

			if(materialid == ORIGINMATERIALID)
			{
			   m_OriginFaces[faceid].m_physface = 0; 
			   FreeOrginFace(faceid);
			}
			
			else if(materialid == CUTMATERIALID)
			{
			   m_CutCrossfaces[faceid].m_physface = 0;
			   FreeCutFace(faceid);
			}

			//remove unreferenced face
			m_CutCrossFacesInLastCut.erase(faceToRemove);
			m_BloodPointCutCrossFaces.erase(FaceInScatterBlood(faceToRemove));
		}
	}
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::FreeOrginFace(int index)
{
	m_OriginFaces[index].m_NextFreed = m_FreeOriginFaceHead;
    m_OriginFaces[index].m_VeinInfoVector.clear();
	m_FreeOriginFaceHead = index;
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::FreeCutFace(int index)
{
	m_CutCrossfaces[index].m_NextFreed = m_FreeCutFaceHead;
	m_CutCrossfaces[index].m_VeinInfoVector.clear();
	m_FreeCutFaceHead = index;
}
//=======================================================================================================
bool MisMedicOrgan_Ordinary::RemoveAndDestoryChild(const Ogre::String& name)
{
	if (m_pSceneNode)
	{
		m_pSceneNode->removeAndDestroyChild(name);
		return true;
	}
	return false;
}
//=======================================================================================================
Ogre::SceneNode* MisMedicOrgan_Ordinary::getSceneNode()
{
	return m_pSceneNode;
}

//////////////////////////////////////////////////////////////////////////
Ogre::String MisMedicOrgan_Ordinary::getMaterialName()
{
	return m_materialname;
}
//=======================================================================================================
#define MAXPTORINDEX 20
void MisMedicOrgan_Ordinary::RebuildRendVertexAndIndex()
{
	class PhysicsToRendNodeIndex
	{
	public:
		PhysicsToRendNodeIndex(GFPhysSoftBodyNode * physicsnode)
		{
			m_physNode = physicsnode;
			m_RendIndexNum = 0;
		}
		GFPhysSoftBodyNode * m_physNode;
		int m_RendNodesIndex[MAXPTORINDEX];
		int m_RendIndexNum;
	};

	int MaxFaceRendNodesNum = m_OriginFaces.size()*3;

	std::vector <PhysicsToRendNodeIndex> PhysToRendIndex;
	PhysToRendIndex.reserve(MaxFaceRendNodesNum);
	
	m_OrganRendNodes.reserve(MaxFaceRendNodesNum);
	m_OrganRendNodes.clear();

	//reset or index to -1
	GFPhysSoftBodyNode * physnode = m_physbody->GetNodeList();
	while(physnode)
	{
		int DataIndex = (int)physnode->m_UserPointer;
		if(DataIndex > 0)
		{
		   m_PhysNodeData[DataIndex].m_TempBuildId = -1;
		}//physnode->m_UserIndex = -1;
		physnode = physnode->m_Next;
	}

	for(size_t f = 0 ; f < m_CutCrossfaces.size() ; f++)
	{
		GFPhysSoftBodyFace * physFace = m_CutCrossfaces[f].m_physface;
		if(physFace)
		{
			physFace->m_Nodes[0]->m_StateFlag |= EMMP_InCrossFace;
			physFace->m_Nodes[1]->m_StateFlag |= EMMP_InCrossFace;
			physFace->m_Nodes[2]->m_StateFlag |= EMMP_InCrossFace;
		}
	}
	//only build origin face + cur face rend node
	size_t originFaceCount = m_OriginFaces.size();
	size_t cutFaceCount = m_CutCrossfaces.size();

	for(size_t f = 0 ; f < originFaceCount + cutFaceCount ; f++)
	{
		MMO_Face & organface = (f < originFaceCount ? m_OriginFaces[f] : m_CutCrossfaces[f-originFaceCount]);
		
		GFPhysSoftBodyFace * physface = organface.m_physface;

		if(physface == 0)
		   continue;
		
		int layerid = physface->m_GenFace->m_ShareTetrahedrons[0].m_Hosttetra->m_LayerIndex;

		organface.m_LayerIndex = layerid;
		//create or index to an exists rend node for 3 node of the face
		//rend node is considered as the same if they point to same SoftbodyNode
		//and have same texture coordinate
		float * textureU = physface->m_TexCoordU;
		
		float * textureV = physface->m_TexCoordV;

		for(int v = 0 ; v < 3 ; v++)
		{
			GFPhysSoftBodyNode * FaceNodePhys = physface->m_Nodes[v];
			
			Ogre::Vector2 FaceNodeTexCoord(textureU[v], textureV[v]);
			
			int NodeDataIndex = (int)FaceNodePhys->m_UserPointer;

			if(NodeDataIndex <= 0)
			{
				throw ("Error invalid Node Data index when Rebuild Rend vertex andIndex");
				break;
			}
			int BuildIndex = m_PhysNodeData[NodeDataIndex].m_TempBuildId;

			//bool nodeinCutedge = ((FaceNodePhys->m_StateFlag & EMMP_InCrossFace) == 0 ? false : true);//organface.m_NodeInCutEdge[v];

			PhysicsToRendNodeIndex * PRNodeIndex = 0;
			
			bool addNewRendNode = false;
			
			if(BuildIndex >= 0)//this physics node already added to the index map
			{
				PRNodeIndex = &PhysToRendIndex[BuildIndex];

				bool exists = false;//check any rend node is the same as this node in the face

				for(int b = 0 ; b < PRNodeIndex->m_RendIndexNum ; b++)
				{
					int RendNodeIndex = PRNodeIndex->m_RendNodesIndex[b];

					Ogre::Vector2 ExistTexCoord = m_OrganRendNodes[RendNodeIndex].m_TextureCoord;

					float texcoorddeviate = (ExistTexCoord-FaceNodeTexCoord).length();

					if(texcoorddeviate < 0.0001f)
					{
						organface.vi[v] = RendNodeIndex;//index this face node to exists node index
						exists = true;
						break;
					}
				}
				if(exists == false)//new
				{
					addNewRendNode = true;
				}
				else
				{
					addNewRendNode = false;
				}
			}
			else//physics node not have rend index map add this 
			{
				PhysToRendIndex.push_back(PhysicsToRendNodeIndex(physnode));
				int TempBuildId = (int)PhysToRendIndex.size()-1;
				m_PhysNodeData[NodeDataIndex].m_TempBuildId = TempBuildId;
				//m_PhysNodeData[NodeDataIndex].m_NodeInCutEdge = nodeinCutedge;
				//m_PhysNodeData[NodeDataIndex].m_TextureCoord = FaceNodeTexCoord;
				PRNodeIndex = &PhysToRendIndex[TempBuildId];
				addNewRendNode = true;
			}

			if(addNewRendNode)
			{
				MMO_Node NewRendNode;
				NewRendNode.m_Color = Ogre::ColourValue(0,0,0,0);//(nodeinCutedge ? m_BurnNodeColor : Ogre::ColourValue(1,1,1,0));
				NewRendNode.m_PhysNode = FaceNodePhys;
				NewRendNode.m_TextureCoord = FaceNodeTexCoord;
				//NewRendNode.m_InCutEdge = nodeinCutedge;
				NewRendNode.m_NodeDataIndex = NodeDataIndex;//cache data index for convient

				m_OrganRendNodes.push_back(NewRendNode);
				int RendNodeIndex = m_OrganRendNodes.size()-1;

				if(PRNodeIndex->m_RendIndexNum < MAXPTORINDEX)
				{
					PRNodeIndex->m_RendNodesIndex[PRNodeIndex->m_RendIndexNum] = RendNodeIndex;//.push_back(RendNodeIndex);
					PRNodeIndex->m_RendIndexNum++;
					organface.vi[v] = RendNodeIndex;
				}
				else
				{//too many select the first one
					organface.vi[v] = PRNodeIndex->m_RendNodesIndex[0];
					Ogre::LogManager::getSingleton().getDefaultLog()->logMessage("too many face conincide node ! error when build rend face and node!!!\n");
				}
			}
		}
	}

	m_pManualObject->DirtyIndexBuffer();
}
//=======================================================================================================
void MisMedicOrgan_Ordinary::CreateBloodStreamInCutFace( const GFPhysVector3 & cutPlaneNorm )
{
	if(m_CutCrossFacesInLastCut.size() == 0)
	{
		return;
	}

	if (m_CreateInfo.m_CutActionParticleParam[2] == -1)
	{
		return;
	}

	GFPhysSoftBodyFace * NegativeFace = 0;
	GFPhysSoftBodyFace * PositiveFace = 0;
	
	float minNegativeDot = FLT_MAX;
	float maxPositiveDot = -FLT_MAX;
	
	std::set<GFPhysSoftBodyFace*>::iterator itor = m_CutCrossFacesInLastCut.begin();
	
	while(itor != m_CutCrossFacesInLastCut.end())
	{
		GFPhysSoftBodyFace * physFace = (*itor);

		GFPhysVector3 facenormal = (physFace->m_Nodes[1]->m_CurrPosition-physFace->m_Nodes[0]->m_CurrPosition).Cross(physFace->m_Nodes[2]->m_CurrPosition-physFace->m_Nodes[0]->m_CurrPosition);
		
		float dotv = facenormal.Dot(cutPlaneNorm);
		
		if(dotv < 0 && dotv < minNegativeDot)
		{
			minNegativeDot = dotv;
			NegativeFace = physFace;
		}
		else if(dotv > 0 && dotv > maxPositiveDot)
		{
			maxPositiveDot = dotv;
			PositiveFace = physFace;
		}

		itor++;
	}
	if(PositiveFace)
	   m_bleednodes.push_back(SBleedPoint(PositiveFace->m_Nodes[0], NULL));

	if(NegativeFace)
	   m_bleednodes.push_back(SBleedPoint(NegativeFace->m_Nodes[0], NULL));

	if(m_bleednodes.size() > 2)
	{
		for(int i = 0; i < (int)m_bleednodes.size() - 2; i++)
		{
			if (m_bleednodes[i].effect)
			{
				m_bleednodes[i].effect->StopBleed();
			}
		}
	}
}
void DistributeMassInFace(Real totalmass , GFPhysSoftBody * sb)
{
	//GFPhysSoftBodyFace * face = sb->GetFaceList();

	GFPhysVectorObj<Real> faceArea;

	std::map<GFPhysSoftBodyNode * , Real> m_NodesMass;

	Real totalArea = 0;

	//while(face)
	for(size_t f = 0 ; f < sb->GetNumFace() ; f++)
	{
		GFPhysSoftBodyFace * face = sb->GetFaceAtIndex(f);

		Real AreaValue = GFPhysMath::GPFabs(face->m_RestAreaMult2);

		faceArea.push_back(AreaValue);

		totalArea += AreaValue;

		//face = face->m_Next;
	}

	Real density = totalmass / totalArea;

	//face = sb->GetFaceList();
	//while(face)
	for(size_t f = 0 ; f < sb->GetNumFace() ; f++)
	{
		GFPhysSoftBodyFace * face = sb->GetFaceAtIndex(f);

		Real area = GFPhysMath::GPFabs(face->m_RestAreaMult2);

		Real areaMass = area*density;

		Real nodeMass = areaMass / 3.0f;

		for(int n = 0 ; n < 3 ; n++)
		{
			GFPhysSoftBodyNode * node = face->m_Nodes[n];

			std::map<GFPhysSoftBodyNode * , Real>::iterator itor = m_NodesMass.find(node);
			if(itor == m_NodesMass.end())
				m_NodesMass.insert(std::make_pair(node , nodeMass));
			else
				itor->second += nodeMass;
		}

		//face = face->m_Next;
	}

	Real totalmassnode = 0;
	std::map<GFPhysSoftBodyNode * , Real>::iterator itor = m_NodesMass.begin();
	while(itor != m_NodesMass.end())
	{
		GFPhysSoftBodyNode * noda = itor->first;
		Real mass = itor->second;
		totalmassnode += mass;

		if(noda->m_Mass == 0)//zero mass
		{}
		else
			noda->SetMass(mass);
		itor++;
	}
	return;
}
//=====================================================================================================
void MisMedicOrgan_Ordinary::CreatePoseRigidForce()
{	
	if(m_CreateInfo.m_FurtherStiffness > 0)
	{
	   if (m_physbody->IsRigidPoseForceEnabled() == false)
	   {
		   m_physbody->EnableRigidPoseForce();
		   m_physbody->SetRigidPoseForceStiffness(m_CreateInfo.m_FurtherStiffness);
	   }
	   m_physbody->RebuildRigidPoseForces();
	}
}
//=====================================================================================================
void MisMedicOrgan_Ordinary::TestLinkingArea(int MarkFlag , std::vector<Ogre::Vector2> & reginInfo)
{	
	for(int c = 0 ; c < m_physbody->GetSoftBodyShape().GetNumSubParts() ; c++)
	{
		GFPhysSubConnectPart * subPart = m_physbody->GetSoftBodyShape().GetSubPart(c);

		int nodeNum_Mark = 0;

		for(size_t n = 0 ; n < subPart->m_Nodes.size() ; n++)
		{
			GFPhysSoftBodyNode * node = subPart->m_Nodes[n];
			
			if ((node->m_Flag & MarkFlag)!=0)
			{
				nodeNum_Mark++;
			}
		}
		reginInfo.push_back(Ogre::Vector2(subPart->m_Nodes.size() , nodeNum_Mark));
	}

	return;
}


//=======================================================================================================
void MisMedicOrgan_Ordinary::setAttchmentFlag( int flag , bool isRemove)
{
// 	MisMedicOrganInterface::setAttchmentFlag(flag, isRemove);

	if (m_CreateInfo.m_CutActionParticleParam[2] != -1)
	{
		std::vector<SBleedPoint>::iterator iter;
		for(iter = m_bleednodes.begin(); iter!= m_bleednodes.end(); iter++)
		{
			SBleedPoint& sbp = *iter;
			if (sbp.effect)
			{
				sbp.effect->resetLastTime(m_CreateInfo.m_CutActionParticleParam[1]);
			}
		}
	}

}
//=======================================================================================
Ogre::Vector3 MisMedicOrgan_Ordinary::getMMO_NODE_Pos( int index )
{
	if (index < (int)m_OrganRendNodes.size())
	{
		return m_OrganRendNodes[index].m_CurrPos;
	}
	else
		return Ogre::Vector3(100000,100000,100000);

}
//=======================================================================================
bool MisMedicOrgan_Ordinary::GetMMOFaceNodesTextureCoord(int faceindex , Ogre::Vector2 textureCoord[3])
{
	if(faceindex >= 0 && faceindex < (int)m_OriginFaces.size())
	{
		GFPhysSoftBodyFace * physFace = m_OriginFaces[faceindex].m_physface;
		float * textureU = physFace->m_TexCoordU;
		float * textureV = physFace->m_TexCoordV;

		textureCoord[0] = Ogre::Vector2(textureU[0], textureV[0]);
		textureCoord[1] = Ogre::Vector2(textureU[1], textureV[1]);
		textureCoord[2] = Ogre::Vector2(textureU[2], textureV[2]);
		return true;
	}
	else
		return false;
}


void MisMedicOrgan_Ordinary::ApplyEffect_Bleeding(Ogre::Vector2 texturecoord, float radius, float heatvalue, Ogre::TexturePtr bleedingPointTex) {
	m_EffectRender.ApplyBleeding(texturecoord, radius > GP_EPSILON ? radius : m_BleedRadius, heatvalue, bleedingPointTex);
}


//=====================================================================================================================
bool MisMedicOrgan_Ordinary::ApplyEffect_BurnWhite(CTool * hostTool, 
												   GFPhysSoftBodyFace * face ,
												   float weights[3] , 
												   float heatvalue , 
												   Ogre::Vector2 & resultTexCoord ,
												   float keepTime)
{
	if(hostTool == 0)
	   return false;

	Ogre::Vector2 texturecoord = GetTextureCoord(face, weights);

	//auto calculate texture radius for burn
	//float weights0[3] = { 1, 0, 0 };
	//float weights1[3] = { 0, 1, 0 };
	//Ogre::Vector2 nodeTex0 = GetTextureCoord(face, weights0);
	//Ogre::Vector2 nodeTex1 = GetTextureCoord(face, weights1);
	//float texDist = (nodeTex0 - nodeTex1).length();
	float posDist = (face->m_Nodes[0]->m_UnDeformedPos - face->m_Nodes[1]->m_UnDeformedPos).Length();

	float texRadius   = 0.00001f;
	//float spaceRadius = 0.08f;
	//if (texDist > 0.00001f)
	//{
		//texRadius = m_CreateInfo.m_BurnRadius * texDist / posDist;
	//}

	{
		texRadius = (m_CreateInfo.m_BurnRadius / m_TexMaxGrad.x + m_CreateInfo.m_BurnRadius / m_TexMaxGrad.y)*0.5f;
	}

	m_EffectRender.ApplyHeat(texturecoord, texRadius, heatvalue, hostTool->GetToolBrandTexture());

	resultTexCoord = texturecoord;

	return true;
}

void MisMedicOrgan_Ordinary::ApplyEffect_Soak(Ogre::Vector3 porigin, Ogre::Vector3 pnormal)
{
	Ogre::GpuProgramParametersSharedPtr params = GetShaderParamterPtr("MarkTextureDipBlood", FRAGMENT_PROGRAME, 0, 0);
	if (params.isNull() == false)
	{
		if (params->_findNamedConstantDefinition("PlaneOrigin"))
		{
			params->setNamedConstant("PlaneOrigin", porigin);
			params->setNamedConstant("PlaneNormal", pnormal);
		}
	}
	m_EffectRender.ApplySoakEffect(m_pManualObject, "MarkTextureDipBlood");
}

void MisMedicOrgan_Ordinary::stopVesselBleedEffect()
{
	for (int i = 0; i != m_bleednodes.size(); ++i)
	{
		if (m_bleednodes[i].effect)
		{
			m_bleednodes[i].effect->StopBleed();
			EffectManager::Instance()->removeVesselBleedEffect(m_bleednodes[i].effect);
			m_bleednodes[i].effect = NULL;
		}
	}
	m_bleednodes.clear();
}

void MisMedicOrgan_Ordinary::resumeVesselBleedEffect()
{
	for (int i = 0; i != m_bleednodes.size(); ++i)
	{
		if (m_bleednodes[i].effect)
		{
			m_bleednodes[i].effect->ResumeBleed();
		}
	}
}

//bool MisMedicOrgan_Ordinary::addBleedPoint(GFPhysSoftBodyNode * pNode)
//{
	//if(!pNode)
	//	return false;
	//m_bleednodes.push_back(SBleedPoint(pNode,NULL));
	//return true;
//}

void MisMedicOrgan_Ordinary::setVesselBleedEffectTempalteName(Ogre::String templateName)
{
	m_strVesselBleedEffectTemplateName = templateName;
}

void MisMedicOrgan_Ordinary::GetSortedNodeIndex(std::vector<int> &results,int &nodeNum)
{
	nodeNum = m_Serializer.m_NodeInitPositions.size();
	std::vector<OrganNodeInfo> nodes_info;
	for(int n = 0 ; n < nodeNum ; n++)
	{
		OrganNodeInfo info(n , &(m_Serializer.m_NodeInitPositions[n]));
		nodes_info.push_back(info);
	}
	std::sort(nodes_info.begin(),nodes_info.end(),CompareNodeHeight);
	results.clear();
	for(int n = 0; n < (int)nodes_info.size(); n++)
		results.push_back(nodes_info[n].m_index);
}

void MisMedicOrgan_Ordinary::SetOrdinaryMatrial(const Ogre::String& matiralName, int layer)
{
	m_pManualObject->setMaterialName(layer, 0, matiralName);
}

void MisMedicOrgan_Ordinary::ChangeTexture(Ogre::TexturePtr texture,const Ogre::String& textureunitname)
{
	ApplyTextureToMaterial(m_materialname,texture,textureunitname);
}

void MisMedicOrgan_Ordinary::ChangeTexture(const Ogre::String & texname , const Ogre::String& textureunitname)
{
	Ogre::TexturePtr srctexPtr = Ogre::TextureManager::getSingleton().load(texname , Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	if(srctexPtr.isNull() == false)
	   ChangeTexture(srctexPtr,textureunitname);
}

void MisMedicOrgan_Ordinary::CopyBurnTexture()
{
	Ogre::TexturePtr QuantityTexturePtr = m_EffectRender.m_QuantityTexturePtr;
	if(QuantityTexturePtr.isNull())
		assert(0);

	int width = QuantityTexturePtr->getWidth();
	int height = QuantityTexturePtr->getHeight();

	if(!m_burnRecord)
	{	
		m_burnRecord = new Ogre::uint32[width * height];
	}
	Ogre::PixelBox tmpBox = Ogre::PixelBox(width , 
											height , 
											QuantityTexturePtr->getDepth() ,
											QuantityTexturePtr->getFormat() ,
											m_burnRecord);

	Ogre::HardwarePixelBufferSharedPtr pixelBufferPtr = QuantityTexturePtr->getBuffer();

	pixelBufferPtr->blitToMemory(tmpBox);

}

uint32 MisMedicOrgan_Ordinary::GetBurnTexPixel(const Ogre::Vector2 & texCoord)
{
	if(!m_burnRecord)
		return 0;
	
	//Ogre::uint * pDest = static_cast<Ogre::uint*>(pixelBufferPtr->getCurrentLock().data);
	

	//assert(pDest != NULL);

	Ogre::TexturePtr QuantityTexturePtr = m_EffectRender.m_QuantityTexturePtr;

	if(QuantityTexturePtr.isNull())
		assert(0);

	int width = QuantityTexturePtr->getWidth();
	int height = QuantityTexturePtr->getHeight();

	int tcx = texCoord.x * (width - 1);

	int tcy = texCoord.y * (height - 1);

	uint32 pixelValue = *(m_burnRecord + tcy * width + tcx);

	return pixelValue;
}

void MisMedicOrgan_Ordinary::SetBloodSystemGravityDir(Ogre::Vector3 & gravity)
{
	m_BloodSystem->setGravityDir(gravity);
	m_isSetGravity = true;
}

void MisMedicOrgan_Ordinary::SetIsolatedPartGravity(float gvalue)
{
	m_IsolatedPartGravityValue = gvalue;
}
void MisMedicOrgan_Ordinary::CalcCurrentComAndOrient(GFPhysVector3 & com , GFPhysMatrix3 & rotateMat , GFPhysVector3 & restcom)
{
	//center of mass 
	GFPhysVector3 UndeformCom = GFPhysVector3(0,0,0);
	
	GFPhysVector3 CurrentCom  = GFPhysVector3(0,0,0);

	Real TotalMass = 0;
	GFPhysSoftBodyNode * Node = m_physbody->GetNodeList();
	while(Node)
	{
		Real wi = 1.0f / (Node->m_OriginInvMass + FLT_EPSILON);

		UndeformCom += Node->m_UnDeformedPos * wi;
		CurrentCom  += Node->m_CurrPosition * wi;
		
		TotalMass += wi;

		Node = Node->m_Next;
	}

	UndeformCom /= TotalMass;
	CurrentCom /= TotalMass;
	
	// A
	GFPhysMatrix3 A_pq;
	A_pq.SetZero();

	Node = m_physbody->GetNodeList();
	while(Node)
	{
		GFPhysVector3 q = Node->m_UnDeformedPos - UndeformCom;
		GFPhysVector3 p = Node->m_CurrPosition - CurrentCom;

		float w = 1.0f / (Node->m_OriginInvMass + FLT_EPSILON);
		p *= w;

		A_pq[0][0] += p[0] * q[0]; A_pq[0][1] += p[0] * q[1]; A_pq[0][2] += p[0] * q[2];
		A_pq[1][0] += p[1] * q[0]; A_pq[1][1] += p[1] * q[1]; A_pq[1][2] += p[1] * q[2];
		A_pq[2][0] += p[2] * q[0]; A_pq[2][1] += p[2] * q[1]; A_pq[2][2] += p[2] * q[2];

		Node = Node->m_Next;
	}

	GFPhysMatrix3 R;
	R = A_pq;
	polarDecomposition2(A_pq , 1e-6f, R);

	com = CurrentCom;
	restcom = UndeformCom;
	rotateMat = R;
	//else
	//m_isValid = false;
}

void MisMedicOrgan_Ordinary::CalcCurrentComAndOrient(GFPhysVector3 & com , GFPhysMatrix3 & rotateMat , GFPhysVector3 & restcom , std::vector<GFPhysSoftBodyNode*> & nodeCluods)
{
	//center of mass 
	GFPhysVector3 UndeformCom = GFPhysVector3(0,0,0);

	GFPhysVector3 CurrentCom  = GFPhysVector3(0,0,0);

	Real TotalMass = 0;
	
	for(size_t n = 0 ; n < nodeCluods.size() ; n++)
	{
		GFPhysSoftBodyNode * Node = nodeCluods[n];

		Real wi = 1.0f / (Node->m_OriginInvMass + FLT_EPSILON);

		UndeformCom += Node->m_UnDeformedPos * wi;
		CurrentCom  += Node->m_CurrPosition * wi;

		TotalMass += wi;
	}

	UndeformCom /= TotalMass;
	CurrentCom /= TotalMass;

	// A
	GFPhysMatrix3 A_pq;
	A_pq.SetZero();

	for(size_t n = 0 ; n < nodeCluods.size() ; n++)
	{
		GFPhysSoftBodyNode * Node = nodeCluods[n];

		GFPhysVector3 q = Node->m_UnDeformedPos - UndeformCom;
		GFPhysVector3 p = Node->m_CurrPosition - CurrentCom;

		float w = 1.0f / (Node->m_OriginInvMass + FLT_EPSILON);
		p *= w;

		A_pq[0][0] += p[0] * q[0]; A_pq[0][1] += p[0] * q[1]; A_pq[0][2] += p[0] * q[2];
		A_pq[1][0] += p[1] * q[0]; A_pq[1][1] += p[1] * q[1]; A_pq[1][2] += p[1] * q[2];
		A_pq[2][0] += p[2] * q[0]; A_pq[2][1] += p[2] * q[1]; A_pq[2][2] += p[2] * q[2];

		Node = Node->m_Next;
	}

	GFPhysMatrix3 R;
	R = A_pq;
	polarDecomposition2(A_pq , 1e-6f, R);

	com = CurrentCom;
	restcom = UndeformCom;
	rotateMat = R;
}

#define CHECKSEPALGOPERFORMANCE 0
void MisMedicOrgan_Ordinary::CalculateSeperatePart()
{
#if(CHECKSEPALGOPERFORMANCE)
	LARGE_INTEGER  countFreq;
	LARGE_INTEGER m_litmp;
    QueryPerformanceFrequency(&m_litmp);

	QueryPerformanceCounter(&countFreq) ; 
	double TimeStart = (double)(countFreq.QuadPart) / (double)m_litmp.QuadPart;
#endif
    
	m_physbody->GetSoftBodyShape().CalculateSubConnectParts();

	//bool needRecalculate = false;
	std::vector<int> NeedRemoveParts;
	NeedRemoveParts.reserve(m_physbody->GetSoftBodyShape().GetNumSubParts());

	for (int c = 0; c < m_physbody->GetSoftBodyShape().GetNumSubParts(); c++)
	{
		GFPhysSubConnectPart * subPart = m_physbody->GetSoftBodyShape().GetSubPart(c);// m_OrganSubParts[c];

	    subPart->m_PartStateFlag = 0;

		for (int n = 0; n < (int)subPart->m_Nodes.size(); n++)
		{
			//velocity
			GFPhysSoftBodyNode * pnode = subPart->m_Nodes[n];
			subPart->m_PartStateFlag |= pnode->m_StateFlag;
		}

		float subPartVol = 0;
		for (int t = 0; t < (int)subPart->m_Tetras.size(); t++)
		{
			GFPhysSoftBodyTetrahedron * tetra = subPart->m_Tetras[t];
			subPartVol += fabsf(tetra->m_RestSignedVolume);
		}

		if (subPartVol < m_MinSubPartVolThresHold)
		{
			NeedRemoveParts.push_back(c);
		}
	}

	if (NeedRemoveParts.size() > 0)
	{
		GFPhysSoftBodyTetrasRemover tetraRemover(m_physDynWorld, m_physbody);
		tetraRemover.RemoveSubParts(NeedRemoveParts);
		onTopologyChanged(tetraRemover);
	}

#if(CHECKSEPALGOPERFORMANCE)
	QueryPerformanceCounter(&countFreq) ; 
	double TimeEnd = (double)(countFreq.QuadPart) / (double)m_litmp.QuadPart;
	double Timesecond = TimeEnd-TimeStart;

	char totaltime[100];
	sprintf(totaltime , "use time = %f" , Timesecond);
	OutputDebugStringA(totaltime);
#endif
}

#if(0)
void MisMedicOrgan_Ordinary::EnableFakeGravity(float gvalue)
{
	m_physbody->SetGravity(GFPhysVector3(0,0,0));
	
	m_FakeGravityValue = gvalue;//true;
	for(size_t p = 0 ; p < m_OrganSubParts.size() ; p++)
	{
		m_OrganSubParts[p]->m_GravityMoveDist = OgreToGPVec3(m_CreateInfo.m_CustomGravityDir);
	}

	for(size_t n = 1 ; n < m_PhysNodeData.size() ; n++)//zero index is error reference
	{
		PhysNode_Data & NodaData = m_PhysNodeData[n];
		if(NodaData.m_SepPartID >= 0 
		&& NodaData.m_NodeBeFixed 
		&&(!NodaData.m_HasError) 
		&& NodaData.m_PhysNode)
		{
			OrganSubPart * subpart = m_OrganSubParts[NodaData.m_SepPartID];
			subpart->m_GravityMoveDist = GFPhysVector3(0,0,0);//disable part which has fixed node
		}
	}
}
#endif

void MisMedicOrgan_Ordinary::AddBleedingRecord(int trackId)
{
	BleedingRecord record(trackId , m_OrganID);
	record.m_RemainderTimeAtBleed = Inception::Instance()->m_remainTime;
	m_BleedingRecords.push_back(record);

	for(size_t i = 0 ; i < m_OrganActionListeners.size(); ++i)
	{
		m_OrganActionListeners[i]->OnOrganBleeding(this->m_OrganID , trackId);
	}
}

void MisMedicOrgan_Ordinary::OnStopBleeding(int trackId , bool isAutoStopped)
{
	for(size_t b = 0 ; b < m_BleedingRecords.size() ; b++)
	{
		BleedingRecord &record = m_BleedingRecords[b];
		if(record.m_TrackId == trackId)
		{
			if(record.m_IsStopped)
				continue;
			else
			{
				record.m_RemainderTimeAtStop = Inception::Instance()->m_remainTime;
				record.m_BleedingTime = record.m_RemainderTimeAtBleed - record.m_RemainderTimeAtStop;
				record.m_IsStopped = true;
				record.m_IsAutoStopped = isAutoStopped;

				//
				for(size_t i =0; i < m_OrganActionListeners.size(); ++i)
				{
					m_OrganActionListeners[i]->OnOrganStopBleeding(this->m_OrganID , trackId);
				}
			}
		}
	}
}


void MisMedicOrgan_Ordinary::OnStopBleeding(std::vector<int>& trackIds , bool isAutoStopped /* = false */)
{
	for(size_t i = 0 ; i < trackIds.size() ; i++)
	{
		int trackId = trackIds[i];
		for(size_t b = 0 ; b < m_BleedingRecords.size() ; b++)
		{
			BleedingRecord &record = m_BleedingRecords[b];
			if(record.m_TrackId == trackId)
			{
				if(record.m_IsStopped)
					continue;
				else
				{
					record.m_RemainderTimeAtStop = Inception::Instance()->m_remainTime;
					record.m_IsStopped = true;
					record.m_BleedingTime = record.m_RemainderTimeAtBleed - record.m_RemainderTimeAtStop;
					record.m_IsAutoStopped = isAutoStopped;

					//
					for(size_t i = 0 ; i < m_OrganActionListeners.size(); ++i)
					{
						m_OrganActionListeners[i]->OnOrganStopBleeding(this->m_OrganID , trackId);
					}
				}
			}
		}
	}
}

float MisMedicOrgan_Ordinary::GetBleedingVolume()
{
	float bleedingSpeed = m_CreateInfo.m_bleedingSpeed;
	if(bleedingSpeed <= 0 || m_BleedingRecords.size() == 0)
		return 0.f;

	float bleedingTime = 0.f;
	for(std::size_t b = 0;b < m_BleedingRecords.size();++b)
	{
		BleedingRecord& record = m_BleedingRecords[b];
		if(record.m_IsStopped)
			bleedingTime += record.m_BleedingTime;
		else
		{
			bleedingTime += record.m_RemainderTimeAtBleed - Inception::Instance()->m_remainTime;
		}
	}

	return bleedingTime * bleedingSpeed;
}

void MisMedicOrgan_Ordinary::ScaleSerializerNodeByDir(const float factor , const GFPhysVector3 & dir)
{
	GFPhysVector3 center = GFPhysVector3(0,0,0);
	int numNode = m_Serializer.m_NodeInitPositions.size();

	for (int n = 0; n < numNode; n++)
	{
		center	+= m_Serializer.m_NodeInitPositions[n];
	}
	center /= numNode;

	GFPhysVector3 normDir = dir.Normalized();
	for (int n = 0; n < numNode; n++)
	{
		GFPhysVector3 & pos = m_Serializer.m_NodeInitPositions[n];
		GFPhysVector3 diff = pos - center;
		GFPhysVector3 projectVec = diff.Dot(normDir) * normDir;
		GFPhysVector3 verticalVec = diff - projectVec;
		pos = center + projectVec * factor + verticalVec;
	}
}

void MisMedicOrgan_Ordinary::CreateSerializerNodeTree(const float extend)
{
	GFPhysDBVTree & tree = m_SerializerDBVTrees.m_NodeTree;
	tree.Clear();
	float absex = fabs(extend);
	GFPhysVector3 ex = GFPhysVector3(absex , absex , absex);
	for(int n = 0 ; n < m_Serializer.m_NodeInitPositions.size() ; n++)
	{
		GFPhysDBVNode * pDBVNode = tree.InsertAABBNode(m_Serializer.m_NodeInitPositions[n] - ex , m_Serializer.m_NodeInitPositions[n] + ex);
		pDBVNode->m_UserData = (void*)n;
	}
	m_SerializerDBVTrees.m_IsCreateNodeTree = true;
}

const GFPhysDBVTree & MisMedicOrgan_Ordinary::GetSerializerNodeTree()
{
	if(!m_SerializerDBVTrees.m_IsCreateNodeTree)
		CreateSerializerNodeTree(0.05);
	return m_SerializerDBVTrees.m_NodeTree;
}

const GFPhysDBVTree & MisMedicOrgan_Ordinary::GetSerializerFaceTree()
{
	if(!m_SerializerDBVTrees.m_IsCreateFaceTree)
	{
		GFPhysDBVTree & tree = m_SerializerDBVTrees.m_FaceTree;
		tree.Clear();

		for(int f = 0 ; f < m_Serializer.m_InitFaces.size() ; f++)
		{
			GFPhysVector3 aabbMin , aabbMax;
			MisMedicObjetSerializer::MisSerialFace & face = m_Serializer.m_InitFaces[f];
			for(int n = 0 ; n < 3 ; n++)
			{
				aabbMin.SetMin(m_Serializer.m_NodeInitPositions[face.m_Index[n]]);
				aabbMax.SetMax(m_Serializer.m_NodeInitPositions[face.m_Index[n]]);
			}
			GFPhysDBVNode * pDBVNode = tree.InsertAABBNode(aabbMin , aabbMax);
			pDBVNode->m_UserData = (void*)(f);//pTetra;
		}

		m_SerializerDBVTrees.m_IsCreateFaceTree = true;
	}
	return m_SerializerDBVTrees.m_FaceTree;
}

const GFPhysDBVTree & MisMedicOrgan_Ordinary::GetSerializerTetraTree()
{
	if(!m_SerializerDBVTrees.m_IsCreateTetraTree)
	{
		GFPhysDBVTree & tree = m_SerializerDBVTrees.m_TetraTree;
		tree.Clear();

		for(int t = 0 ; t < m_Serializer.m_InitTetras.size() ; t++)
		{
			GFPhysVector3 aabbMin , aabbMax;
			MisMedicObjetSerializer::MisSerialTetra & tetra = m_Serializer.m_InitTetras[t];
			for(int n = 0 ; n < 4 ; n++)
			{
				aabbMin.SetMin(m_Serializer.m_NodeInitPositions[tetra.m_Index[n]]);
				aabbMax.SetMax(m_Serializer.m_NodeInitPositions[tetra.m_Index[n]]);
			}
			GFPhysDBVNode * pDBVNode = tree.InsertAABBNode(aabbMin , aabbMax);
			pDBVNode->m_UserData = (void*)(t);//pTetra;
		}
		m_SerializerDBVTrees.m_IsCreateTetraTree = true;
	}
	return m_SerializerDBVTrees.m_TetraTree;
}

GFPhysVector3 MisMedicOrgan_Ordinary::GetSerializerNodePos(int index)
{
	if(index >= m_Serializer.m_NodeInitPositions.size() && index < 0)
		return GFPhysVector3(0,0,0);
	else
	    return m_Serializer.m_NodeInitPositions[index];
}

const MisMedicObjetSerializer::MisSerialFace & MisMedicOrgan_Ordinary::GetSerializerFace(int index)
{
	if(index >= m_Serializer.m_InitFaces.size() && index < 0)
		return MisMedicObjetSerializer::MisSerialFace::sInvalidFace;
	return m_Serializer.m_InitFaces[index];
}

const MisMedicObjetSerializer::MisSerialTetra & MisMedicOrgan_Ordinary::GetSerializerTetra(int index)
{
	if(index >= m_Serializer.m_InitTetras.size() && index < 0)
		return MisMedicObjetSerializer::MisSerialTetra::sInvalidTetra;
	return m_Serializer.m_InitTetras[index];
}

float MisMedicOrgan_Ordinary::GetCurMaxBleedTime()
{
	float maxTime = 0.f;
	for(size_t b = 0;b < m_BleedingRecords.size();++b)
	{
		BleedingRecord& record = m_BleedingRecords[b];
		float time;
		if(record.m_IsStopped)
			time = record.m_BleedingTime;
		else
			time = record.m_RemainderTimeAtBleed - Inception::Instance()->m_remainTime;

		if(maxTime < time)
			maxTime = time;
	}
	return maxTime;
}

void MisMedicOrgan_Ordinary::TranslateUndeformedPosition(GFPhysVector3 & offset)
{
	if(m_physbody)
	{
		GFPhysVector3 velocity(0,0,0);
		Real masspernode = m_CreateInfo.m_mass / m_physbody->GetNodesNum();
		GFPhysSoftBodyNode *pNode = m_physbody->GetNodeList();
		while (pNode)
		{
			pNode->Reset(pNode->m_UnDeformedPos+offset , velocity , masspernode);
			pNode = pNode->m_Next;
		}
		RefreshRendFacesUndeformedPosition();
	}
}

//======================================================================================================================
class SoftFaceAroundPointCallBack : public GFPhysNodeOverlapCallback
{
public:
	SoftFaceAroundPointCallBack(std::vector<GFPhysSoftBodyFace*> & resultFace ,
		const GFPhysVector3 & point , 
		float range,
		bool  deformedSpace)
		: m_ResultFaces(resultFace) ,
		m_PointPos(point) ,
		m_DeformedSpace(deformedSpace),
		m_Range(range)
	{

	}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
	{
		GFPhysSoftBodyFace * face = (GFPhysSoftBodyFace*)UserData;

		GFPhysVector3 trianglePos[3];

		for(int v = 0 ; v < 3; v++)
		{
			trianglePos[v] = (m_DeformedSpace ? face->m_Nodes[v]->m_CurrPosition : face->m_Nodes[v]->m_UnDeformedPos);
		}

		GFPhysVector3 closetPoint = ClosestPtPointTriangle(  m_PointPos, 
			trianglePos[0],
			trianglePos[1], 
			trianglePos[2]);

		float dist  = (closetPoint-m_PointPos).Length();

		if(dist <= m_Range)
		{
			m_ResultFaces.push_back(face);
		}
	}


	std::vector<GFPhysSoftBodyFace*> & m_ResultFaces;
	GFPhysVector3 m_PointPos;
	bool m_DeformedSpace;
	float m_Range;
};
//======================================================================================================================
void MisMedicOrgan_Ordinary::SelectPhysFaceAroundPoint(std::vector<GFPhysSoftBodyFace*> & ResultFaces , const GFPhysVector3 & pointPos , float radius , bool deformedspace)
{
	GFPhysSoftBodyShape & sbShape = m_physbody->GetSoftBodyShape();

	GFPhysVector3 extend(radius , radius , radius);

	SoftFaceAroundPointCallBack cb(ResultFaces , pointPos , radius , deformedspace);

	sbShape.TraverseFaceTreeAgainstAABB(&cb , pointPos-extend ,pointPos+extend , deformedspace);
}
//======================================================================================================================
void MisMedicOrgan_Ordinary::DisRigidCollAroundPoint(std::vector<GFPhysSoftBodyFace*>& ResultFaces, const GFPhysVector3 & pointPos, Real threshold, bool deformedspace)
{
    GFPhysSoftBodyShape & sbShape = m_physbody->GetSoftBodyShape();

    GFPhysVector3 extend(threshold, threshold, threshold);    

    SoftFaceAroundPointCallBack cb(ResultFaces, pointPos, threshold, deformedspace);

    sbShape.TraverseFaceTreeAgainstAABB(&cb, pointPos - extend, pointPos + extend, deformedspace);

    for (std::vector<GFPhysSoftBodyFace*>::iterator itor = ResultFaces.begin();
        itor != ResultFaces.end();
        ++itor)
    {
        GFPhysSoftBodyFace* face = *itor;

        //m_painting.PushBackFace(CustomFace(face));

        face->DisableCollideWithRigid();
    }
}
//======================================================================================================================
void MisMedicOrgan_Ordinary::CollectFacesAroundPoint(std::vector<GFPhysSoftBodyFace*>& ResultFaces, const GFPhysVector3 & pointPos, Real threshold, bool deformedspace)
{
	GFPhysSoftBodyShape & sbShape = m_physbody->GetSoftBodyShape();

	GFPhysVector3 extend(threshold, threshold, threshold);

	SoftFaceAroundPointCallBack cb(ResultFaces, pointPos, threshold, deformedspace);

	sbShape.TraverseFaceTreeAgainstAABB(&cb, pointPos - extend, pointPos + extend, deformedspace);
}
//======================================================================================================================
void MisMedicOrgan_Ordinary::UpdateNonWoundBlood( float dt )
{
	std::map<OrganSurfaceBloodTextureTrack*,float>::iterator bloodItor = m_NonWoundedBloodTimeRecord.begin();
	while(bloodItor != m_NonWoundedBloodTimeRecord.end())
	{
		std::map<OrganSurfaceBloodTextureTrack*,float>::iterator limitTimeItor = m_NonWoundedBloodLimitTime.find(bloodItor->first);
		if(limitTimeItor != m_NonWoundedBloodLimitTime.end())
		{
			float limitTime = limitTimeItor->second;
			float & currTime = bloodItor->second;
			currTime += dt;
			if(currTime >= limitTime)
			{
				OrganSurfaceBloodTextureTrack * pBloodTrack = bloodItor->first;
				pBloodTrack->Stop();
				m_NonWoundedBloodLimitTime.erase(limitTimeItor);
				m_NonWoundedBloodTimeRecord.erase(bloodItor++);
			}
			else
				bloodItor++;
		}
		else
			m_NonWoundedBloodTimeRecord.erase(bloodItor++);
	}
}
//======================================================================================================================
class RayIntersectFaceCallback : public GFPhysNodeOverlapCallback
{
public:
    RayIntersectFaceCallback(const GFPhysVector3 & raySource , const GFPhysVector3 & rayTarget)
    {       
        m_raySource = raySource;
        m_rayTarget = rayTarget;
        m_bingo = false;
        m_face = 0;
		m_MinWeight = FLT_MAX;
    }

    void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
    {        
        GFPhysSoftBodyFace  * face = (GFPhysSoftBodyFace*)UserData;        
        Real weight;

        Real TriangleWeight[3];

        GFPhysVector3 instersectpt;

        bool succed = false;

        succed = LineIntersectTriangle(
            face->m_Nodes[0]->m_CurrPosition ,
            face->m_Nodes[1]->m_CurrPosition ,
            face->m_Nodes[2]->m_CurrPosition ,
            m_raySource , 
            m_rayTarget , 
            weight ,
            instersectpt , 
            TriangleWeight);
		//float dist = (instersectpt - m_raySource).Length();
		if (succed && weight >= 0.0001f && weight <= 1 && weight < m_MinWeight)
        {     
            m_face = face;
            m_bingo = true;
			m_MinWeight = weight;
        }
    }

    GFPhysVector3       m_raySource;
    GFPhysVector3       m_rayTarget;
    bool                m_bingo;
    GFPhysSoftBodyFace* m_face;
    Real                m_MinWeight;//we select closet intersect point
};
//======================================================================================================================
GFPhysSoftBodyFace * MisMedicOrgan_Ordinary::GetRayIntersectFace(const Ogre::Vector3 rayStart, const Ogre::Vector3 rayEnd, Real& dist)
{
    RayIntersectFaceCallback nodeCallback(OgreToGPVec3(rayStart), OgreToGPVec3(rayEnd/*rayStart + 1000.0f * rayDir*/));

    if(m_physbody)
    {
        GFPhysVectorObj<GFPhysDBVTree*> bvTrees = m_physbody->GetSoftBodyShape().GetFaceBVTrees();
        for (size_t i = 0 ; i < bvTrees.size(); ++i)
        {
            GFPhysDBVTree * bvTree = bvTrees[i];
            bvTree->TraverseTreeAgainstRay(&nodeCallback, OgreToGPVec3(rayStart), OgreToGPVec3(rayEnd/*rayStart + 1000.0f * rayDir)*/));
        }
    }
	if (nodeCallback.m_bingo)
	{
        dist = (rayEnd - rayStart).length() * nodeCallback.m_MinWeight;
		return nodeCallback.m_face;
	}
    return 0;
}
//======================================================================================================================
void MisMedicOrgan_Ordinary::ApplyEffect_VolumeBlood(int faceID, float* pFaceWeight)
{
	if (m_CreateInfo.m_CanBlood == false)
		return;

	if (m_OwnerTrain->OrganShouldBleed(this, faceID, pFaceWeight)) {
		if (m_VolumeBlood == NULL) {
			float gravity[3];
			cw_vec3(m_CreateInfo.m_CustomGravityDir.x*9.8f, m_CreateInfo.m_CustomGravityDir.y*9.8f, m_CreateInfo.m_CustomGravityDir.z*9.8f, gravity);
			m_VolumeBlood = new VolumeBlood(gravity, 100, 10, 0.1);

		}

		m_VolumeBlood->setCutInfo(0.001, 0.01*0.01*0.01*1.25, this, faceID, pFaceWeight);
	}

}

void MisMedicOrgan_Ordinary::StopEffect_VolumeBlood() {

	if (m_VolumeBlood)
	{
		delete m_VolumeBlood;
		m_VolumeBlood = NULL;
	}
}

void MisMedicOrgan_Ordinary::SetVolumeBloodParameter(float radius, float flow)
{
	if(m_VolumeBlood){
		m_VolumeBlood->setCutInfo(radius, flow);
	}
}

MisMedicEndoGiaClips* MisMedicOrgan_Ordinary::GetEndoGiaClips() {
	if (m_EndoGiaClips == NULL) {
		char tempName[200];
		sprintf(tempName, "%ld_endoGiaClips", (unsigned long)this);
		m_EndoGiaClips = new MisMedicEndoGiaClips(tempName);
		AddOrganAttachment(m_EndoGiaClips);
	}
	return m_EndoGiaClips;
}

class TexTri2D
{
public:
	GFPhysVector3 aabbMin;
	GFPhysVector3 aabbMax;
	GFPhysVector3 m_TexCoords[3];
	int m_globalIndex[3];
};


class MapCallBack : public GFPhysNodeOverlapCallback
{
public:
	MapCallBack(GFPhysAlignedVectorObj<TexTri2D>&vecs, const TexTri2D & triToMap) : m_ReferceTris(vecs)
	{
		m_TriToMap = triToMap;
		m_DstTriIndex = -1;
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
	{
		 int index = (int)UserData;
		 
		 TexTri2D & tri_Ref = m_ReferceTris[index];
		
		 bool isMapped[3] = { false, false, false };
		 
		 Ogre::Vector2 ToMapTex[3];
		 ToMapTex[0] = Ogre::Vector2(m_TriToMap.m_TexCoords[0].m_x, m_TriToMap.m_TexCoords[0].m_y);
		 ToMapTex[1] = Ogre::Vector2(m_TriToMap.m_TexCoords[1].m_x, m_TriToMap.m_TexCoords[1].m_y);
		 ToMapTex[2] = Ogre::Vector2(m_TriToMap.m_TexCoords[2].m_x, m_TriToMap.m_TexCoords[2].m_y);

		 for (int c = 0; c < 3; c++)
		 {
			  for (int t = 0; t < 3; t++)
			  {
				  Ogre::Vector2 refTex(tri_Ref.m_TexCoords[t].m_x, tri_Ref.m_TexCoords[t].m_y);
				 
				  if ((ToMapTex[c] - refTex).length() < 0.001f)
				  {
					   isMapped[c] = true;
					   m_ResultIndex[c] = tri_Ref.m_globalIndex[t];
					   break;
				  }
			  }
		 }
		 if (isMapped[0] && isMapped[1] && isMapped[2])
		 {
			 m_DstTriIndex = index;
		 }
	}
	int m_DstTriIndex;
	GFPhysAlignedVectorObj<TexTri2D> & m_ReferceTris;
	TexTri2D m_TriToMap;
	int m_ResultIndex[3];
};

void MisMedicOrgan_Ordinary::MapSurfaceToMorphedMesh(Ogre::MeshPtr & meshToMap)
{
	std::vector<Ogre::Vector3> vertices;
	std::vector<Ogre::Vector2> textures;
	std::vector<unsigned int>  indices;
	ExtractOgreMeshInfo(meshToMap, vertices, textures, indices);

	GFPhysAlignedVectorObj<TexTri2D> tri2DVecs;

	GFPhysDBVTree dynTree;

	//build reference tree
	for (int i = 0; i < indices.size(); i += 3)
	{
		int index0 = indices[i];
		int index1 = indices[i + 1];
		int index2 = indices[i + 2];

		TexTri2D triangle;
		triangle.m_globalIndex[0] = index0;
		triangle.m_globalIndex[1] = index1;
		triangle.m_globalIndex[2] = index2;

		triangle.m_TexCoords[0] = GFPhysVector3(textures[index0].x, textures[index0].y, 0);
		triangle.m_TexCoords[1] = GFPhysVector3(textures[index1].x, textures[index1].y, 0);
		triangle.m_TexCoords[2] = GFPhysVector3(textures[index2].x, textures[index2].y, 0);
		
		triangle.aabbMin = triangle.aabbMax = triangle.m_TexCoords[0];
		triangle.aabbMin.SetMin(triangle.m_TexCoords[1]);
		triangle.aabbMin.SetMin(triangle.m_TexCoords[2]);

		triangle.aabbMax.SetMax(triangle.m_TexCoords[1]);
		triangle.aabbMax.SetMax(triangle.m_TexCoords[2]);

		tri2DVecs.push_back(triangle);

		GFPhysDBVNode * bvNode = dynTree.InsertAABBNode(triangle.aabbMin, triangle.aabbMax);
		bvNode->m_UserData = (void*)(tri2DVecs.size()-1);
	}

	//map organ faces
	for (int c = 0; c < m_OriginFaces.size(); c++)
	{
		 Ogre::Vector2 tex0 = m_OriginFaces[c].GetTextureCoord(0);
		 Ogre::Vector2 tex1 = m_OriginFaces[c].GetTextureCoord(1);
		 Ogre::Vector2 tex2 = m_OriginFaces[c].GetTextureCoord(2);

		 GFPhysVector3 tex3d0(tex0.x, tex0.y, 0);
		 GFPhysVector3 tex3d1(tex1.x, tex1.y, 0);
		 GFPhysVector3 tex3d2(tex2.x, tex2.y, 0);

		 GFPhysVector3 aabbMin, aabbMax;
		 aabbMin = aabbMax = tex3d0;
		 aabbMin.SetMin(tex3d1);
		 aabbMin.SetMin(tex3d2);

		 aabbMax.SetMax(tex3d1);
		 aabbMax.SetMax(tex3d2);

		 aabbMin.m_z = -FLT_MAX;
		 aabbMax.m_z = FLT_MAX;
		 TexTri2D triToMap;
		 triToMap.m_TexCoords[0] = tex3d0;
		 triToMap.m_TexCoords[1] = tex3d1;
		 triToMap.m_TexCoords[2] = tex3d2;

		 MapCallBack callback(tri2DVecs, triToMap);
		 dynTree.TraverseTreeAgainstAABB(&callback , aabbMin, aabbMax);

		 if (callback.m_DstTriIndex >= 0)
		 {
			 GFPhysSoftBodyFace * physFace = m_OriginFaces[c].m_physface;
			 physFace->m_Nodes[0]->SetDeformedPos(OgreToGPVec3(vertices[callback.m_ResultIndex[0]]));// m_CurrPosition = physFace->m_Nodes[0]->m_LastPosition = OgreToGPVec3(vertices[callback.m_ResultIndex[0]]);
			 physFace->m_Nodes[1]->SetDeformedPos(OgreToGPVec3(vertices[callback.m_ResultIndex[1]]));// m_CurrPosition = physFace->m_Nodes[1]->m_LastPosition = OgreToGPVec3(vertices[callback.m_ResultIndex[1]]);
			 physFace->m_Nodes[2]->SetDeformedPos(OgreToGPVec3(vertices[callback.m_ResultIndex[2]]));// m_CurrPosition = physFace->m_Nodes[2]->m_LastPosition = OgreToGPVec3(vertices[callback.m_ResultIndex[2]]);
			 physFace->m_Nodes[2]->SetMass(0);
		 }
	}
}



