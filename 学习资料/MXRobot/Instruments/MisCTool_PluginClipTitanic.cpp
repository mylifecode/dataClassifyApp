#include "MisCTool_PluginClipTitanic.h"
#include "Tool.h"
#include "ClipApplier.h"
#include "MisMedicOrganOrdinary.h"
#include "MXEvent.h"
#include "MXEventsDump.h"
#include "TipMgr.h"
#include "ITraining.h"
#include "Math/GoPhysTransformUtil.h"
#include "Math/GoPhysSIMDMath.h"
#include "Topology/GoPhysSoftBodyRestShapeModify.h"
#include "PhysicsWrapper.h"
#define ClipGroupMax 12//6
#define CLIPCOUNT


class ClipFaceInterval
{
public:
	ClipFaceInterval(GFPhysSoftBodyFace * face , 
		float t0 , 
		float t1 , 
		const GFPhysVector3 & minPos,
		const GFPhysVector3 & maxPos) : 
	m_face(face) , m_tMin(t0) , m_tMax(t1) , m_PosMin(minPos) ,m_PosMax(maxPos)
	{}
	GFPhysSoftBodyFace * m_face;
	GFPhysVector3 m_PosMin;
	GFPhysVector3 m_PosMax;
	float m_tMin;
	float m_tMax;
};

MisCTool_PluginClipTitanic::MisCTool_PluginClipTitanic(CTool * tool)
										: MisMedicCToolPluginInterface(tool),
										m_bHasNipObject(false),
										m_bLowerHalfPartIsVisiable(true)
{
	m_clipRemain = ClipGroupMax;
	m_ClipBeginCheckShaft = 1;
	m_canClip = true;
	m_TimeElapsedSinceLastClip = 0;
	m_HasEmptyClipToRelease = false;
	m_AppAllowClamp = false;
}

MisCTool_PluginClipTitanic::~MisCTool_PluginClipTitanic()
{

}

void MisCTool_PluginClipTitanic::PhysicsSimulationStart(int currStep , int TotalStep , float dt)
{
	if(m_ClipData.m_InClipState == false)//not in m_InClipState state
	{
		float toolshaft = m_ToolObject->GetShaftAside();
		if(toolshaft <= m_ClipBeginCheckShaft)
		   m_ClipData.m_canCollectClampPoint = true;
		else
		   m_ClipData.m_canCollectClampPoint = false;
	}
}
//==================================================================================================
static int SelectFaceToApplyClip(const std::vector<GFPhysSoftBodyFace*> & LeftClampedFace, 
								  const std::vector<GFPhysSoftBodyFace*> & RightClampedFace, 
								  GFPhysVector3 leftbladepoint[2] , 
								  GFPhysVector3 rightbladepoint[2] ,
								  std::vector<GFPhysSoftBodyFace*> & ResultFaces)
{
	//check left clamp
	GFPhysVector3 CliptriVerts[2][3];
	CliptriVerts[0][0] = leftbladepoint[0];
	CliptriVerts[0][1] = leftbladepoint[1];
	CliptriVerts[0][2] = rightbladepoint[0];

	CliptriVerts[1][0] = rightbladepoint[0];
	CliptriVerts[1][1] = rightbladepoint[1];
	CliptriVerts[1][2] = leftbladepoint[1];
	
	GFPhysAlignedVectorObj<ClipFaceInterval> FClipIntervalLeft;

	for(size_t f = 0 ; f < LeftClampedFace.size() ; f++)
	{
		GFPhysVector3 ResultPoint[2];
		GFPhysVector3 FaceVerts[3];
		GFPhysSoftBodyFace * face = LeftClampedFace[f];
		
        FaceVerts[0] = face->m_Nodes[0]->m_CurrPosition;
		FaceVerts[1] = face->m_Nodes[1]->m_CurrPosition;
		FaceVerts[2] = face->m_Nodes[2]->m_CurrPosition;

		bool intersect0 = TriangleIntersect(CliptriVerts[0], FaceVerts, ResultPoint);
		bool intersect1 = TriangleIntersect(CliptriVerts[1], FaceVerts, ResultPoint);

		if (intersect0 || intersect1)//this face intersect quad
		{
			ResultFaces.push_back(face);
		}
	}
	
	//check right
	for(size_t f = 0 ; f < RightClampedFace.size() ; f++)
	{
		GFPhysVector3 ResultPoint[2];
		GFPhysVector3 FaceVerts[3];
		GFPhysSoftBodyFace * face = RightClampedFace[f];
		FaceVerts[0] = face->m_Nodes[0]->m_CurrPosition;
		FaceVerts[1] = face->m_Nodes[1]->m_CurrPosition;
		FaceVerts[2] = face->m_Nodes[2]->m_CurrPosition;

		bool intersect0 = TriangleIntersect(CliptriVerts[0], FaceVerts, ResultPoint);
		bool intersect1 = TriangleIntersect(CliptriVerts[1], FaceVerts, ResultPoint);
		if (intersect0 || intersect1)//this face intersect quad
		{
			ResultFaces.push_back(face);
		}
	}
	return 1;
}

//==================================================================================================
//判断钛夹钳夹住时是否与管子垂直
/*
bool IsClampedVertical(std::vector<OrganFaceClamped> & LeftClampedFace, 
					   std::vector<OrganFaceClamped> & RightClampedFace, 
					   GFPhysVector3 leftbladepoint[2])
{
	//计算钛夹钳完全闭合时的朝向
	Ogre::Vector3 direction((leftbladepoint[0]-leftbladepoint[1]).m_x,(leftbladepoint[0]-leftbladepoint[1]).m_y,(leftbladepoint[0]-leftbladepoint[1]).m_z);
	direction.normalise();

	int size = min(LeftClampedFace.size(), RightClampedFace.size());
	if (size >= 5)
	{
		size = 5;
	}
	int count = 0;
	for (int i = 0; i < size; ++i)
	{
		for (int j = 0; j < size; ++j)
		{
			float angle;			
			GFPhysVector3 temp;
			temp = LeftClampedFace[i].m_Face->m_FaceNormal.Cross(RightClampedFace[j].m_Face->m_FaceNormal);
			temp.Normalize();
			//被夹住的绳子部分与被夹面法向量正交的向量
			Ogre::Vector3 ropeBeClampedDirection(temp.m_x, temp.m_y, temp.m_z);		
			
			float value = ropeBeClampedDirection.dotProduct(direction);
			angle = acos(value);
			angle = (angle/3.1415)*180;
			if (fabs(angle - 90.0f) <= 3.0f)
			{
				return true;
			}
		}
	}
	return false;
}
*/

//==================================================================================================

void MisCTool_PluginClipTitanic::CustomClipOrgan(MisMedicOrgan_Ordinary * organclip , 
												 std::vector<GFPhysSoftBodyFace*> & faceInRegL ,
												 std::vector<GFPhysSoftBodyFace*> & faceInRegR ,
												 const GFPhysVectorObj<GFPhysSoftBodyTetrahedron*> & CompressTetras,
												 const GFPhysVector3 & direInMaterialSpace)
{
	if(m_canClip == false)
	   return;
	float toolshaft = m_ToolObject->GetShaftAside();
	int leftNum  = faceInRegL.size();
	int rightNum = faceInRegR.size();

	float clipLength = 0.5f;//to do dynamic get clip length from mesh
	if(rightNum > 0 && leftNum > 0)
	{
		Ogre::SceneNode * kernalnode = m_ToolObject->GetKernelNode();

		GFPhysVector3 leftClipLine[2];
		GFPhysVector3 rightClipLine[2];

		leftClipLine[0] = m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[0];
		leftClipLine[1] = m_ToolObject->m_CutBladeLeft.m_LinePointsWorld[1];

		rightClipLine[0] = m_ToolObject->m_CutBladeRight.m_LinePointsWorld[0];
		rightClipLine[1] = m_ToolObject->m_CutBladeRight.m_LinePointsWorld[1];


		Ogre::Vector3 clipPosition = GPVec3ToOgre(leftClipLine[1] + rightClipLine[1])*0.5f;
		
		Ogre::Vector3 clipz = GPVec3ToOgre((leftClipLine[0]  - leftClipLine[1]).Normalized());
		Ogre::Vector3 clipy = GPVec3ToOgre((rightClipLine[1] - leftClipLine[1]).Normalized());
		Ogre::Vector3 clipx = clipy.crossProduct(clipz).normalisedCopy();

		Ogre::Quaternion clipOrient;
		clipOrient.FromAxes(clipx, clipy, clipz);

		std::vector<GFPhysSoftBodyFace*> ResultFaces;
		
		int selSide = SelectFaceToApplyClip(faceInRegL , 
											faceInRegR ,
											leftClipLine ,
											rightClipLine,
											ResultFaces
											);

		if (ResultFaces.size() > 0)
		{
			  if (m_clipRemain > 0)	//判断是否还有钛夹多余
			  {
				  CClipApplier *pClipApplier = dynamic_cast<CClipApplier*>(m_ToolObject);
				  
				  if (pClipApplier->GetOpenCloseStatus() == CLIPAPLLIER_COMPLETELY_OPEN_STATUS)
				  {
					  //compress for deformed effect
					  float invalidClipLen;
					  GFPhysVector3 resultClipCoordAxis[3];
					  GFPhysVector3 resultClipBound[3];
					  
					  GoPhysSoftBodyRestShapeModify restmodify;
					  restmodify.HardCompressInMaterialSpace(PhysicsWrapper::GetSingleTon().m_dynamicsWorld , 
						                                     organclip->m_physbody , 
															 leftClipLine, 
															 rightClipLine, 
															 0.04f, 
															 invalidClipLen,
															 resultClipCoordAxis, resultClipBound);

					  Ogre::Vector3 clipAxisRestFrame[3];
					  Ogre::Vector2 clipBoundRestFrame[3];
					  for (int dim = 0; dim < 3; dim++)
					  {
						  clipAxisRestFrame[dim] = GPVec3ToOgre(resultClipCoordAxis[dim]);
						  clipBoundRestFrame[dim].x = resultClipBound[dim].m_x;
						  clipBoundRestFrame[dim].y = resultClipBound[dim].m_y;

					  }

					  float centerCoord = resultClipBound[2].m_x + clipLength*0.5f;
					
					  float maxAngleDot  = -FLT_MAX;
					  
					  float minCenterOff = FLT_MAX;
					 
					  int   selectFaceIndex = -1;

					  for(size_t c = 0 ; c < ResultFaces.size() ; c++)
					  {
						  GFPhysSoftBodyFace * physFace = ResultFaces[c];
						  
						  GFPhysVector3 undeformedNorm = (physFace->m_Nodes[1]->m_UnDeformedPos-physFace->m_Nodes[0]->m_UnDeformedPos).Cross(physFace->m_Nodes[2]->m_UnDeformedPos-physFace->m_Nodes[0]->m_UnDeformedPos);
						  
						  undeformedNorm.Normalize();
						  
						  GFPhysVector3 center = (physFace->m_Nodes[0]->m_UnDeformedPos
							  + physFace->m_Nodes[1]->m_UnDeformedPos
							  + physFace->m_Nodes[2]->m_UnDeformedPos)*0.33333f;

						  float centeroff = fabsf(center.Dot(resultClipCoordAxis[2]) - centerCoord);

						  if (centeroff < minCenterOff)
						  {
							  minCenterOff = centeroff;
							  selectFaceIndex = c;
						  }
						 // float dotv = fabsf(physFace->m_FaceNormal.Dot(resultClipCoordAxis[1]));
						
						 // if (dotv > maxAngleDot)
						 // {	
							 // maxAngleDot = dotv;
							//  selectFaceIndex = c;
						//  }
					  }

					  if (selectFaceIndex >= 0)
					  {
						  MisMedicTitaniumClampV2 * TianClip =organclip->CreateTitanic(invalidClipLen,
							                                                           clipAxisRestFrame,
							                                                           clipBoundRestFrame,
							                                                           ResultFaces[selectFaceIndex],
							                                                           m_ToolObject->GetToolSide());

						  pClipApplier->SetOpenCloseStatus(CLIPAPLLIER_COMPLETELY_CLOSE_STATUS);
						  m_bHasNipObject = true;

						  MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_AddHemoClip, m_ToolObject, TianClip);
						 
						  if (pEvent)
						  {
							  pEvent->SetOrgan(organclip);
							  CMXEventsDump::Instance()->PushEvent(pEvent);
						  }
					  }
				  }

					m_ToolObject->ReleaseOneTitanicClip();
					m_canClip = false;
					m_ToolObject->SetMinShaftAside(toolshaft);
					m_TimeElapsedSinceLastClip = 0;
					m_MaxShaftSinceLastClip = m_LastClipShaft = toolshaft;
			}
			else if (0 == m_clipRemain)
			{
			   if (m_ToolObject->GetToolSide() == ITool::TSD_LEFT)
			   {
				   CTipMgr::Instance()->ShowTip("NoClipInHolder_Left");
			   }
			   else
			   {
				   CTipMgr::Instance()->ShowTip("NoClipInHolder_Right");
			   }
			   return;
		   }
	   }
   }
}

//==================================================================================================
void MisCTool_PluginClipTitanic::PhysicsSimulationEnd(int currStep , int TotalStep , float dt)
{

}
//==================================================================================================
void MisCTool_PluginClipTitanic::OneFrameUpdateStarted(float timeelapsed)
{
	float toolshaft = m_ToolObject->GetShaftAside();

	if(m_canClip == false)
	{
		m_TimeElapsedSinceLastClip += timeelapsed;

		if(toolshaft > m_MaxShaftSinceLastClip)
		   m_MaxShaftSinceLastClip = toolshaft;

		if(m_TimeElapsedSinceLastClip > 1.0f && (m_MaxShaftSinceLastClip > m_LastClipShaft+2))
		{
		   m_canClip = true;
		   m_ToolObject->SetMinShaftAside(0);
		}
	}
	else if(toolshaft == m_ToolObject->GetMinShaftAside())										//完全闭合
	{
		m_HasEmptyClipToRelease = true;
	}
	else if(toolshaft == m_ToolObject->GetMaxShaftAside() && m_HasEmptyClipToRelease)			//完全张开
	{
#ifdef CLIPCOUNT
		if (!m_bHasNipObject)
		{
			if (m_clipRemain > 0)
			{
				((CClipApplier*)m_ToolObject)->CreateEmptyClip();
				m_clipRemain --;
				//m_ToolObject->ReleaseOneTitanicClip();

				MxToolEvent * pEvent = CMXEventsDump::Instance()->CreateEventNew(MxEvent::MXET_AddHemoClip, m_ToolObject, NULL);
				if (pEvent != NULL)
				{
					pEvent->SetOrgan(0);
					CMXEventsDump::Instance()->PushEvent(pEvent);
				}

			}
			if (0 == m_clipRemain)
			{
				if (m_ToolObject->GetToolSide() == ITool::TSD_LEFT)
				{
					CTipMgr::Instance()->ShowTip("NoClipInHolder_Left");
				}
				else
				{
					CTipMgr::Instance()->ShowTip("NoClipInHolder_Right");
				}
			}
		}
		else
			m_bHasNipObject = false;
#else
		((CClipApplier*)m_ToolObject)->CreateEmptyClip();
#endif
		m_HasEmptyClipToRelease = false;
	}
}
//==================================================================================================
// void MisCTool_PluginClipTitanic::OneFrameUpdateEnded()
// {
// 
// }
