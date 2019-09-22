#include "MisCTool_TubeClamp.h"
#include "MisCTool_PluginClamp.h"
#include "MisNewTraining.h"
#include "ACTubeShapeObject.h"
#include "Tool.h"
class TubeSegMayClampCallBack : public GFPhysNodeOverlapCallback
{
public:
	TubeSegMayClampCallBack()
	{
		m_TubeSegsToCheck.reserve(20);
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex, void * UserData)
	{
		 int SegIndex = (int)UserData;
		 m_TubeSegsToCheck.push_back(SegIndex);
	}

	std::vector<int>  m_TubeSegsToCheck;
};
//============================================================================================================================
MisCTool_TubeClamp::MisCTool_TubeClamp(MisNewTraining * train,MisCTool_PluginClamp * clampPlugin)
{
	m_train = train;
	m_ClampPlugin = clampPlugin;
}
//============================================================================================================================
MisCTool_TubeClamp::~MisCTool_TubeClamp()
{

}
//========================================================================================================================
void MisCTool_TubeClamp::ReleaseClampedSegment()
{
	uint32 toolcat = (m_ClampPlugin->m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);

	std::set<GFPhysSoftTube*> tubeClamped;
	for (int c = 0; c < m_SegmentsClamped.size(); c++)
	{
		 tubeClamped.insert(m_SegmentsClamped[c].m_Tube);

		 if (toolcat == MMRC_LeftTool)
			 m_SegmentsClamped[c].m_Object->SetClampedByLTool(false);
		 else
			 m_SegmentsClamped[c].m_Object->SetClampedByRTool(false);
	}
	m_SegmentsClamped.clear();
	std::set<GFPhysSoftTube*>::iterator itor = tubeClamped.begin();
	while (itor != tubeClamped.end())
	{
		uint32 originMask = (*itor)->m_MaskBits;
		(*itor)->SetCollisionMask(originMask | toolcat);
		itor++;
	}

}
//========================================================================================================================
void MisCTool_TubeClamp::PrepareSolve(float dt)
{
	for (int c = 0; c < (int)m_SegmentsClamped.size(); c++)
	{
		ClampedTubeSegment & clampSeg = m_SegmentsClamped[c];
		clampSeg.m_RotLambda = GFPhysVector3(0, 0, 0);
	}
}
//====================================================================================================================
void MisCTool_TubeClamp::Solve(float dt, float thickNess)
{
	const MisCTool_PluginClamp::ToolClampRegion * ClampReg = m_ClampPlugin->GetClampRegions();

	for (int c = 0; c < (int)m_SegmentsClamped.size(); c++)
	{
		 ClampedTubeSegment & clampSeg = m_SegmentsClamped[c];


		 //solve frame rotate
		 GFPhysQuaternion frameRot = ClampReg[0].m_AttachRigid->GetWorldTransform().GetRotation() * clampSeg.m_SegQuatRelToTool;
		 GFPhysSoftTube::SolveSegmentWithTarget(*(clampSeg.m_segMent),
			                                    frameRot, GFPhysVector3(1.0f / 10000.0f, 1.0f / 10000.0f, 1.0f / 10000.0f),
			                                    dt, clampSeg.m_RotLambda);

		 //solve points
		 for (int n = 0; n < 2; n++)
		 {
			  GFPhysSoftTubeNode * physNode = (n == 0 ? clampSeg.m_segMent->m_Node0 : clampSeg.m_segMent->m_Node1);

			  if (physNode->GetInvMass() < GP_EPSILON)//exclude static node
				  continue;

			  int RegId = clampSeg.m_Reg[n];
		    
			  float CoordN0 = (physNode->m_CurrPosition - ClampReg[0].m_OriginWorld).Dot(ClampReg[0].m_ClampNormalWorld) - thickNess;
			  if (CoordN0 < 0)
			  {
				  GFPhysVector3 normalCorrect = ClampReg[0].m_ClampNormalWorld*(-CoordN0);
				  physNode->m_CurrPosition += normalCorrect;
			  }

			  float CoordN1 = (physNode->m_CurrPosition - ClampReg[1].m_OriginWorld).Dot(ClampReg[1].m_ClampNormalWorld) - thickNess;
			  if (CoordN1 < 0)
			  {
				  GFPhysVector3 normalCorrect = ClampReg[1].m_ClampNormalWorld*(-CoordN1);
				  physNode->m_CurrPosition += normalCorrect;
			  }
			
			  float tan0 = (physNode->m_CurrPosition - ClampReg[RegId].m_OriginWorld).Dot(ClampReg[RegId].m_Axis0World);
			  float tan1 = (physNode->m_CurrPosition - ClampReg[RegId].m_OriginWorld).Dot(ClampReg[RegId].m_Axis1World);

			  float dt0 = clampSeg.m_RegCoord[n].m_x - tan0;
			  float dt1 = clampSeg.m_RegCoord[n].m_y - tan1;

			  float tanCorrStiff = 0.3f;
			  GFPhysVector3 tanCorrect = (ClampReg[RegId].m_Axis0World * dt0 + ClampReg[RegId].m_Axis1World * dt1) * tanCorrStiff;
			  physNode->m_CurrPosition += tanCorrect;
		}

	
	}
}
//============================================================================================================================
bool MisCTool_TubeClamp::CheckTubeBeClamped(std::vector<ACTubeShapeObject*> & tubeObject)
{
	 if (m_SegmentsClamped.size() > 0)
		 return false;

	 uint32 toolcat = (m_ClampPlugin->m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);

	 GFPhysVector3 clampMin, clampMax;

	 m_ClampPlugin->GetClampSpaceWorldAABB(clampMin, clampMax);

	 const MisCTool_PluginClamp::ToolClampRegion & reg0 = m_ClampPlugin->GetClampRegion(0);
	 
	 const MisCTool_PluginClamp::ToolClampRegion & reg1 = m_ClampPlugin->GetClampRegion(1);

	 for (int c = 0; c < (int)tubeObject.size(); c++)
	 {
		GFPhysSoftTubeCollisionShape * tubeShape = ((GFPhysSoftTubeCollisionShape*)tubeObject[c]->GetPhysicsBody()->GetCollisionShape());
		float checkradius = tubeShape->GetCollisionRadius()*0.5f;//shrink a little

		TubeSegMayClampCallBack callback;
		tubeShape->m_SegDBVTree.TraverseTreeAgainstAABB(&callback, clampMin, clampMax);

		//now iterator all candidate segment to see 
		bool hasClamped = false;
		for (int s = 0; s < callback.m_TubeSegsToCheck.size(); s++)
		{
			 GFPhysSoftTubeSegment & segment = tubeShape->GetSegment(callback.m_TubeSegsToCheck[s]);
			
			 bool beClamped = TestSegmentContactClampReg(segment, checkradius);

			 if (beClamped)
			 {
				 float overlapLen = ClipSegmentByCells(segment);
				 if (overlapLen > 0.01f || overlapLen > (segment.m_Node0->m_CurrPosition - segment.m_Node1->m_CurrPosition).Length() * 0.25f)
				 {
					 beClamped = true;
				 }
				 else
					 beClamped = false;
			 }
			 if (beClamped)
			 {
				 ClampedTubeSegment clampData;
				 GFPhysVector3 segDir = (segment.m_Node0->m_CurrPosition - segment.m_Node1->m_CurrPosition).Normalized();

				 float angle0 = fabsf(segDir.Dot(reg0.m_ClampNormalWorld));
				 float angle1 = fabsf(segDir.Dot(reg1.m_ClampNormalWorld));

				 if (angle0 > 0.5 || angle1 > 0.5)
					 continue;

				 for (int n = 0; n < 2; n++)
				 {
					  GFPhysVector3 nodePos = (n == 0 ? segment.m_Node0 : segment.m_Node1)->m_CurrPosition;
					 
					  float d0 = (nodePos - reg0.m_OriginWorld).Dot(reg0.m_ClampNormalWorld);
					 
					  float d1 = (nodePos - reg1.m_OriginWorld).Dot(reg1.m_ClampNormalWorld);
					  
					  clampData.m_Reg[n] = (d0 < d1 ? 0 : 1);

					  const  MisCTool_PluginClamp::ToolClampRegion & selReg = (d0 < d1 ? reg0 : reg1);
					 
					  clampData.m_RegCoord[n].m_x = (nodePos - selReg.m_OriginWorld).Dot(selReg.m_Axis0World);
					  clampData.m_RegCoord[n].m_y = (nodePos - selReg.m_OriginWorld).Dot(selReg.m_Axis1World);
					  clampData.m_RegCoord[n].m_z = 0;
					  clampData.m_segMent = &segment;
					  clampData.m_Tube = tubeObject[c]->GetPhysicsBody();
					  clampData.m_Object = tubeObject[c];
					  clampData.m_SegQuatRelToTool = reg0.m_AttachRigid->GetWorldTransform().GetRotation().Inverse() * segment.m_CurrRotQuat;//rotation relative to rigid (int rigid local frame)
				 }
				 m_SegmentsClamped.push_back(clampData);
				 hasClamped = true;
			 }
		}

		
		//process node belong clamped faces
		if (hasClamped)
		{
			uint32 originMask = (tubeObject[c]->GetPhysicsBody())->m_MaskBits;
			(tubeObject[c]->GetPhysicsBody())->SetCollisionMask(originMask & (~toolcat));

			if (m_ClampPlugin->m_ToolObject->GetToolSide() == ITool::TSD_LEFT)
				tubeObject[c]->SetClampedByLTool(true);
			else
				tubeObject[c]->SetClampedByRTool(true);
		}
		//
	 }

	 return m_SegmentsClamped.size() > 0 ? true : false;//has clamped something?
}
//============================================================================================================================================
int MisCTool_TubeClamp::TestSegmentContactClampReg(GFPhysSoftTubeSegment & segMent, float segRadius)
{
	GFPhysVector3 segPos[2];
	GFPhysTransform transIdentity;

	segPos[0] = segMent.m_Node0->m_CurrPosition;
	segPos[1] = segMent.m_Node1->m_CurrPosition;
	transIdentity.SetIdentity();

	GFPhysConvexHullShape cellShape;
	cellShape.SetMargin(0);
	
	GFPhysConvexHullShape segShape;
	segShape.SetMargin(segRadius);
	segShape.InitWithPoints(segPos, 2, false);

	for (int c = 0; c < m_ClampPlugin->GetNumClampSpaceCell(); c++)
	{
		MisCTool_PluginClamp::ClampCellData & cellData = m_ClampPlugin->GetClampSpaceCell(c);
		if (c == 0)
		{
			cellShape.InitWithPoints(cellData.m_CellVertsWorldSpace, 6, false);
		}
		else
		{
			for (int p = 0; p < 6; p++)
			{
				cellShape.m_HullVerts[p] = cellData.m_CellVertsWorldSpace[p];
			}
		}

		//first test segment aabb vs cell aabb
		GFPhysVector3 seglocalMin(FLT_MAX, FLT_MAX, FLT_MAX);
		GFPhysVector3 seglocalMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		for (int n = 0; n < 2; n++)
		{
			GFPhysVector3 segLocalVert = cellData.m_InvTrans(segPos[n]);
			seglocalMin.SetMin(segLocalVert);
			seglocalMax.SetMax(segLocalVert);
		}

		if (TestAabbAgainstAabb2(cellData.m_localmin, cellData.m_localmax, seglocalMin, seglocalMax) == false)
		{
			continue;
		}

		GFPhysVector3 closetReg, closetSeg;
		float closetDist = GetConvexsClosetPoint(&cellShape , &segShape,
			                                     transIdentity , transIdentity,
			                                     closetReg, closetSeg,0.001f);


		if (closetDist <= 0)//consider segment in contact with region
		{
			return true;
		}
	}

	return false;
}
//===================================================================================================================
float MisCTool_TubeClamp::ClipSegmentByCells(GFPhysSoftTubeSegment & segMent)
{
	GFPhysVector3 pIn  = segMent.m_Node0->m_CurrPosition;
	
	GFPhysVector3 pOut = segMent.m_Node1->m_CurrPosition;
	
	float minWeight = FLT_MAX;
	
	float maxWeight = -FLT_MAX;
	
	for (int c = 0; c < m_ClampPlugin->GetNumClampSpaceCell(); c++)
	{
		MisCTool_PluginClamp::ClampCellData & cellData = m_ClampPlugin->GetClampSpaceCell(c);

		GFPhysVector3 CellSurrounds[3];

		//use region 0's cell to clip segment
		CellSurrounds[0] = cellData.m_CellVertsWorldSpace[0];
		CellSurrounds[1] = cellData.m_CellVertsWorldSpace[1];
		CellSurrounds[2] = cellData.m_CellVertsWorldSpace[2];

		GFPhysVector3 pNormal = (CellSurrounds[1] - CellSurrounds[0]).Cross(CellSurrounds[2] - CellSurrounds[0]).Normalized();

		bool discard = false;
		for (int t = 0; t < 3; t++)
		{
			GFPhysVector3 sNormal = pNormal.Cross(CellSurrounds[(t + 1)%3] - CellSurrounds[t]).Normalized();//point to inner
			GFPhysVector3 sPoint = CellSurrounds[t];

			float d0 = (pIn  - sPoint).Dot(sNormal);
			float d1 = (pOut - sPoint).Dot(sNormal);

			if (d0 <= 0 && d1 <= 0)
			{
				discard = true;
				break;
			}
		       
			if (d0 * d1 < 0)//need clip
			{
				float t = d0 / (d0 - d1);

				GFPhysVector3 intersectPT = pIn*(1 - t) + pOut*t;
				
				if (d0 > 0)
					pOut = intersectPT;
				else
					pIn  = intersectPT;
			}
			//else not clip total lie inside(d0 >= 0 && d1 > 0 ) or(d1 >= 0 && d0 > 0)
		}

		
		if (discard == false)
		{//update min / max weight if not discarded
			float weight0 = (pIn  - segMent.m_Node0->m_CurrPosition).Dot(segMent.m_Node1->m_CurrPosition - segMent.m_Node0->m_CurrPosition) / (segMent.m_Node1->m_CurrPosition - segMent.m_Node0->m_CurrPosition).Length2();
			float weight1 = (pOut - segMent.m_Node0->m_CurrPosition).Dot(segMent.m_Node1->m_CurrPosition - segMent.m_Node0->m_CurrPosition) / (segMent.m_Node1->m_CurrPosition - segMent.m_Node0->m_CurrPosition).Length2();
            
			if (weight0 > weight1)//numerical error may cause this like (weig0=0.344445 weig1 = 0.34444)
			{
				std::swap(weight0, weight1);
			}
			if (weight0 < minWeight)
				minWeight = weight0;

			if (weight1 > maxWeight)
				maxWeight = weight1;
		}
	}

	if (maxWeight - minWeight > 0)//
	{
		return (pIn - pOut).Length();
	}
	else
	{
		return -1;
	}
}