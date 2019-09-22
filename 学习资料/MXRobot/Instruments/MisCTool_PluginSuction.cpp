#include "math/GoPhysTransformUtil.h"
#include "collision/NarrowPhase/GoPhysPrimitiveTest.h"
#include "IObjDefine.h"
#include "MisMedicOrganOrdinary.h"
#include "VeinConnectObject.h"
#include "CustomCollision.h"
#include "PhysicsWrapper.h"
#include "MXOgreGraphic.h"
#include "ITraining.h"
#include "stdafx.h"
#include "BasicTraining.h"
#include "MisNewTraining.h"
#include "MXOgreWrapper.h"

#include "Instruments/Tool.h"
#include "MisCTool_PluginSuction.h"
#include "WaterPool.h"


bool Border2D::IsInside(const Ogre::Vector2 & Point)
{
	float z = (Point - m_Begin).crossProduct(m_Dir);
	return z >= 0 ? true : false;
}


bool Border2D::FindIntersectionWithSeg(const Ogre::Vector2 & a , const Ogre::Vector2 & b , Ogre::Vector2 & result)
{
// 	if(!(IsInside(a) ^ IsInside(b)))
// 		return false;
	Ogre::Vector2 & d0 =  m_Dir;
	Ogre::Vector2 d1 = b - a;
	float length =   d1.normalise();
	float d0xd1 = d0.crossProduct(d1);

	 if(abs(d0xd1) < FLT_EPSILON) //平行or同一直线上
		 return false;

	const Ogre::Vector2 & p0 = m_Begin;
	const Ogre::Vector2 & p1 = a;
	Ogre::Vector2 delta = a - m_Begin;
	float t = delta.crossProduct(d0) / d0xd1;

	if(t >= 0 && t <= length)
	{
		result = p1 + t * d1;
		return true;
	}
	else
		return false;
}

void Clipper2D::SetBorders(const std::vector<Ogre::Vector2> & ClipperPolygonVertices)
{
	m_Borders.clear();
	int edgesNum = ClipperPolygonVertices.size();
	for(int v = 0 ; v < ClipperPolygonVertices.size() ; v++) {
		m_Borders.push_back(Border2D(ClipperPolygonVertices[v] , ClipperPolygonVertices[(v + 1) % edgesNum]));
	}
}

void Clipper2D::Sutherland_Hodgeman(const std::vector<Ogre::Vector2> & SrcVertices , std::vector<Ogre::Vector2> & DestVertices)
{
	DestVertices.clear();

	if(SrcVertices.empty())
		return;

	DestVertices = SrcVertices;

	for(size_t borderN = 0 ; borderN < m_Borders.size() ; borderN++)
	{
		std::vector<Ogre::Vector2> tempVertices;

		Border2D & currBorder = m_Borders[borderN];

		const Ogre::Vector2 & start = DestVertices.back();

		Ogre::Vector2 lastVertex = start;

		bool IsLastInside = currBorder.IsInside(start);

		for(size_t v = 0 ; v < DestVertices.size() ;v++)
		{
			const Ogre::Vector2 & currVertex = DestVertices[v];

			bool currIsInside = currBorder.IsInside(currVertex);

			if(currIsInside)//当前点在边界内侧
			{
				if(!IsLastInside)	//前一个点在外侧
				{
					Ogre::Vector2 intersection(0,0);
					SY_ASSERT(currBorder.FindIntersectionWithSeg(lastVertex , currVertex , intersection) && "what");
					tempVertices.push_back(intersection);
				}
				tempVertices.push_back(currVertex);
			}
			else//当前点在边界外侧
			{
				if(IsLastInside)//前一个点在内侧
				{
					Ogre::Vector2 intersection(0,0);
					SY_ASSERT(currBorder.FindIntersectionWithSeg(lastVertex , currVertex , intersection) && "what");
					tempVertices.push_back(intersection);
				}
			}
			IsLastInside = currIsInside;
			lastVertex = currVertex;
		}
		DestVertices = tempVertices;
		if(DestVertices.empty())	
			break;	
	}
}

//========================================================================================
void MisCTool_PluginSuction::SuctionRegion::UpdateToWorldSpace()
{
	SY_ASSERT(m_pAttachRigid && "m_pAttachRigid is NULL pointer");

	const GFPhysTransform & worldTrans = m_pAttachRigid->GetWorldTransform();
	const GFPhysMatrix3 & worldRotate = worldTrans.GetBasis();

	m_CenterWorld = worldTrans * m_CenterLocal;
	m_Axis0World = worldRotate * m_Axis0Local;
	m_Axis1World = worldRotate * m_Axis1Local;
	m_SuctionInvDirWorld = worldRotate * m_SuctionInvDirLocal;
}
//======================================================================================
bool MisCTool_PluginSuction::SuctionRegion::IsFaceInSuctionRange(GFPhysSoftBodyFace* pFace)
{
    GFPhysVector3 ptOnFace = ClosestPtPointTriangle(m_CenterWorld , pFace->m_Nodes[0]->m_CurrPosition , pFace->m_Nodes[1]->m_CurrPosition , pFace->m_Nodes[2]->m_CurrPosition);
	
	GFPhysVector3 offsetVec = ptOnFace - m_CenterWorld;

    //painting.PushBackPoint(CustomPoint(&ptOnFace, Ogre::ColourValue::Green));
    //painting.PushBackPoint(CustomPoint(&m_CenterWorld, Ogre::ColourValue::Blue));

	
	float NormalOffset  = offsetVec.Dot(m_SuctionInvDirWorld);

	float tangentOffset = (offsetVec-m_SuctionInvDirWorld*NormalOffset).Length();

    //Real temp = m_SuctionInvDirWorld.Dot(offsetVec.Normalized());

	if(tangentOffset < 0.25f && NormalOffset < 0.1f && m_SuctionInvDirWorld.Dot(offsetVec.Normalized()) > 0.7f)
		return true;
	else
		return false;
}
//========================================================================================
float MisCTool_PluginSuction::SuctionRegion::ProjectFace(GFPhysSoftBodyFace* pFace , std::vector<Ogre::Vector2> & results2D , float dist[3])
{
	results2D.clear();
	results2D.resize(3);
	for(int n = 0 ; n < 3 ; n++)
	{
		GFPhysVector3 position = pFace->m_Nodes[n]->m_CurrPosition;
		GFPhysVector3 diff = position - m_CenterWorld;
		results2D[n].x = diff.Dot(m_Axis0World);
		results2D[n].y = diff.Dot(m_Axis1World);
		dist[n] = diff.Dot(m_SuctionInvDirWorld);
	}
	return abs((results2D[1] - results2D[0]).crossProduct(results2D[2] - results2D[0])) * 0.5;
}
//========================================================================================
void MisCTool_PluginSuction::SuctionRegion::ProjectNode(GFPhysSoftBodyNode* pNode , Ogre::Vector2 & results2D)
{
	GFPhysVector3 diff = pNode->m_CurrPosition - m_CenterWorld;
	results2D.x = diff.Dot(m_Axis0World);
	results2D.y = diff.Dot(m_Axis1World);
}
//========================================================================================
void MisCTool_PluginSuction::SuctionRegion::ProjectNode(GFPhysSoftBodyNode* pNode , float component[3])
{
	GFPhysVector3 diff = pNode->m_CurrPosition - m_CenterWorld;
	component[0] = diff.Dot(m_Axis0World);
	component[1] = diff.Dot(m_Axis1World);
	component[2] = diff.Dot(m_SuctionInvDirWorld);
}
//========================================================================================
float MisCTool_PluginSuction::SuctionRegion::GetAreaInSuctionRegion(const std::vector<Ogre::Vector2> & Triangle)
{
	if(Triangle.size() < 3)
		return 0;

	float totalDoubleArea = 0.f;

	std::vector<Ogre::Vector2> VerticesAfterClip;

	m_Clipper.Sutherland_Hodgeman(Triangle , VerticesAfterClip);

	if(VerticesAfterClip.size() < 3)
		return 0;

	Ogre::Vector2 & first = VerticesAfterClip.front();

	for(int v = 1 ; v < VerticesAfterClip.size() - 1 ; v++){
		totalDoubleArea += abs((VerticesAfterClip[v] - first).crossProduct(VerticesAfterClip[v+1] - first));
	}
	return totalDoubleArea * 0.5;
}
//========================================================================================
MisCTool_PluginSuction::MisCTool_PluginSuction(CTool * tool) 
: MisMedicCToolPluginInterface(tool) ,
  m_IsOrganSucked(false) , 
  m_pOrganBeSucked(NULL)  , 
 // m_SuctionAreaRatio(0) ,
  m_CanSuck(false)
{
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this);

	m_DragForce = GFPhysVector3(0,0,0);
}
//========================================================================================
MisCTool_PluginSuction::~MisCTool_PluginSuction()
{
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);
}
//========================================================================================
void MisCTool_PluginSuction::OnOrganBeRemovedFromWorld(MisMedicOrganInterface * organif)
{

}
//========================================================================================
void MisCTool_PluginSuction::ClearSuckedFaces()
{
	uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);

	std::set<GFPhysSoftBodyFace*>::iterator faceItor = m_SuckedFaces.begin();
	for(; faceItor != m_SuckedFaces.end(); ++faceItor)
	{
		GFPhysSoftBodyFace* pFace = *faceItor;
		pFace->m_RSCollisionMask  |= toolcat;
	}
	m_SuckedFaces.clear();
}
//========================================================================================
void MisCTool_PluginSuction::ClearSuckedNodes()
{
	SuckedNodeMap::iterator nodeItor = m_SuckedNodes.begin();
	for(; nodeItor != m_SuckedNodes.end() ; ++nodeItor)
	{
		SoftBodyNodeSucked & nodeInfo = nodeItor->second;
		nodeInfo.m_pNode->m_StateFlag &= (~GPSESF_ATTACHED);
	}
	m_SuckedNodes.clear();
}
//========================================================================================
void MisCTool_PluginSuction::ReleaseSuckedOrgans()
{
	m_IsOrganSucked = false;
	m_pOrganBeSucked->m_FaceMoveIncrementInSuction = 0.0f;
	m_pOrganBeSucked->m_FaceMoveSpeedInSuction=0.f;
	m_pOrganBeSucked->m_IsSucked = false;
	m_pOrganBeSucked = NULL;
	ClearSuckedFaces();
	ClearSuckedNodes();
}
//========================================================================================
bool MisCTool_PluginSuction::isSomethingSucked()
{
	return m_IsOrganSucked;
}
//========================================================================================
MisMedicOrgan_Ordinary * MisCTool_PluginSuction::GetOrganBeSucked()
{
	return m_pOrganBeSucked;
}
//========================================================================================
void MisCTool_PluginSuction::SetCanSuck(bool canSuck)
{
	m_CanSuck = canSuck;
	if(!m_CanSuck && m_IsOrganSucked)
	{
		ReleaseSuckedOrgans();
	}
}
//========================================================================================
void MisCTool_PluginSuction::SetSuctionRegion(GFPhysRigidBody * pAttachRigid, 
											  const GFPhysVector3 & regionCenter, 
											  const GFPhysVector3 & axis0, 
											  const GFPhysVector3 & axis1,
											  const GFPhysVector3 & pointTo , 
											  float suctionRadius , 
											  float suctionDist/* = 0.05f*/)
{
	m_SuctionRegion.m_pAttachRigid = pAttachRigid;
	m_SuctionRegion.m_CenterLocal = regionCenter;
	m_SuctionRegion.m_Axis0Local = axis0;
	m_SuctionRegion.m_Axis1Local = axis1;
	m_SuctionRegion.m_SuctionInvDirLocal = pointTo;
	m_SuctionRegion.m_SuctionRadius = suctionRadius;
	m_SuctionRegion.m_SuctionArea = suctionRadius * suctionRadius * 4;
	m_SuctionRegion.m_SuctionDist = suctionDist;

	std::vector<Ogre::Vector2> suctionConner;
	suctionConner.push_back(Ogre::Vector2(-suctionRadius , -suctionRadius));
	suctionConner.push_back(Ogre::Vector2(-suctionRadius , suctionRadius));
	suctionConner.push_back(Ogre::Vector2( suctionRadius ,  suctionRadius));
	suctionConner.push_back(Ogre::Vector2( suctionRadius , -suctionRadius));
	m_SuctionRegion.m_Clipper.SetBorders(suctionConner);

// 	m_painting.PushBackPoint(CustomPoint(&m_SuctionRegion.m_CenterWorld , Ogre::ColourValue::Green , suctionRadius));
// 	m_painting.PushBackCoordAxis(CustomCoordAxis(&m_SuctionRegion.m_CenterWorld , 
// 		&m_SuctionRegion.m_Axis0World , 
// 		&m_SuctionRegion.m_Axis1World , 
// 		&m_SuctionRegion.m_SuctionInvDirWorld));
}
//========================================================================================
void MisCTool_PluginSuction::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{
	m_DragForce = GFPhysVector3(0 , 0 , 0);
}
//===========================================================================================================
void MisCTool_PluginSuction::SolveConstraint(Real Stiffniss,Real TimeStep)
{
	if(m_pOrganBeSucked)
	{
		SuckedNodeMap::iterator nodeItor = m_SuckedNodes.begin();
		for( ; nodeItor != m_SuckedNodes.end() ; ++nodeItor)
		{
			float component[3];
			
			GFPhysSoftBodyNode *pNode = nodeItor->first;
			
			SoftBodyNodeSucked & nodeInfo = nodeItor->second;
			
			m_SuctionRegion.ProjectNode(pNode , component);
			
			GFPhysVector3 tangentIncrement = (-m_SuctionRegion.m_Axis0World * (component[0] - nodeInfo.m_ProjectPos.x)) + 
											 (-m_SuctionRegion.m_Axis1World * (component[1] - nodeInfo.m_ProjectPos.y));

			GFPhysVector3 normalIncrement = -m_SuctionRegion.m_SuctionInvDirWorld * (component[2] - 0.05f);

			pNode->m_CurrPosition += tangentIncrement * 0.9 + normalIncrement;

			m_DragForce += -(tangentIncrement * 0.9 + normalIncrement);
		}
	}
}
//===========================================================================================================
GFPhysVector3 MisCTool_PluginSuction::GetPluginForceFeedBack()
{
	return m_DragForce * 0.5f;
}
//===========================================================================================================
void MisCTool_PluginSuction::RigidSoftCollisionsSolved(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints)
{
	
	 
}
//========================================================================================
void MisCTool_PluginSuction::PhysicsSimulationStart(int currStep , int TotalStep , float dt)
{
	m_SuctionRegion.UpdateToWorldSpace();
}

bool MisCTool_PluginSuction::CheckWhetherOrganBeSucked()
{
	uint32 toolcat = (m_ToolObject->GetToolSide() == ITool::TSD_LEFT ? MMRC_LeftTool : MMRC_RightTool);

	FaceWithOrganMap faceWithOrganMap;

	ITraining * currtrain = m_ToolObject->GetOwnerTraining();

	for(size_t f = 0 ; f < m_ToolObject->m_ToolColliedFaces.size() ; f++)
	{
		const ToolCollidedFace & datacd = m_ToolObject->m_ToolColliedFaces[f];
		GFPhysSoftBody * softbody = (GFPhysSoftBody*)datacd.m_collideSoft;
		MisMedicOrgan_Ordinary * pCurrOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(currtrain->GetOrgan(softbody));
		FaceWithOrganMap::iterator Itor = faceWithOrganMap.find(pCurrOrgan);
		if(Itor != faceWithOrganMap.end())
		{
			std::list<GFPhysSoftBodyFace*> & faces = Itor->second;
			faces.push_back(datacd.m_collideFace);
		}
		else
		{
			std::list<GFPhysSoftBodyFace*> faces;
			faces.push_back(datacd.m_collideFace);
			faceWithOrganMap.insert(std::make_pair(pCurrOrgan , faces));
		}
	}

	MisMedicOrgan_Ordinary * pOrganSelected = SelectWhichOrganToBeSucked(faceWithOrganMap);
	
	if(pOrganSelected != NULL) 
	{
		m_pOrganBeSucked = pOrganSelected;
		m_IsOrganSucked = true;
		m_pOrganBeSucked->m_IsSucked = true;
		m_pOrganBeSucked->m_FaceMoveIncrementInSuction = 0.f;
		m_pOrganBeSucked->m_FaceMoveSpeedInSuction=0.f;

		//加入吸起的Node
		FaceWithOrganMap::iterator organItor = faceWithOrganMap.find(m_pOrganBeSucked);
		if(organItor != faceWithOrganMap.end())
		{
			std::list<GFPhysSoftBodyFace*> & faces = organItor->second;
			std::list<GFPhysSoftBodyFace*>::iterator faceItor = faces.begin();
			for(; faceItor != faces.end() ; ++faceItor)
			{
				GFPhysSoftBodyFace * pFace  = *faceItor;
				for(int n = 0 ; n < 3 ; n++)
				{
					GFPhysSoftBodyNode *pNode = pFace->m_Nodes[n];
					SuckedNodeMap::iterator nodeItor = m_SuckedNodes.find(pNode);
					if(nodeItor != m_SuckedNodes.end())
					{
						SoftBodyNodeSucked & nodeInfo = nodeItor->second;
						nodeInfo.m_FaceRefCount++;
					}
					else
					{
						SoftBodyNodeSucked nodeInfo(pNode);
						m_SuckedNodes.insert(std::make_pair(pNode , nodeInfo));
					}
					pNode->m_StateFlag |= GPSESF_ATTACHED;
				}
				pFace->m_RSCollisionMask &= (~toolcat);
				m_SuckedFaces.insert(pFace);
			}
		}
		ProcessSuckedNode();
	}

	return m_IsOrganSucked;
}
//========================================================================================
bool MisCTool_PluginSuction::CheckSuckWater()
{
	Ogre::Vector3 headPos  = m_ToolObject->GetKernelNode()->_getDerivedPosition();
	Ogre::Vector3 pivotPos = m_ToolObject->GetPivotPosition();

	ITraining * train =  m_ToolObject->GetOwnerTraining();
	if(train)
	{
		MisNewTraining * newTraining = dynamic_cast<MisNewTraining*>(train);
		if(newTraining)
		{
			int num = newTraining->GetNumOfWaterPools();
			for(size_t p = 0 ; p < num ; p++)
			{
				WaterPool * pPool = newTraining->GetWaterPools(p);
				Ogre::Vector2 planePos;
				if (pPool->IsSegementInsectWaterPool(pivotPos , headPos))
				{
					pPool->Reduce(3 ,planePos);
					return true;
				}
			}
		}
	}
	return false;
}
//========================================================================================
void MisCTool_PluginSuction::PhysicsSimulationEnd(int currStep , int TotalStep , float dt)
{
	m_SuctionRegion.UpdateToWorldSpace();

	UpdateMoveInfo(dt);

	if(m_CanSuck)
	{
		if(!m_IsOrganSucked)
			CheckWhetherOrganBeSucked();
		//if(!m_IsOrganSucked)
			CheckSuckWater();
	}
	else
	{
		if(m_IsOrganSucked)
			ReleaseSuckedOrgans();
	}
}
//========================================================================================
void MisCTool_PluginSuction::OneFrameUpdateStarted(float timeelapsed)
{
	//m_painting.Update(timeelapsed);
}
//========================================================================================
void MisCTool_PluginSuction::OneFrameUpdateEnded()
{

}
//===============================================================================================
void MisCTool_PluginSuction::OnSoftBodyFaceBeDeleted(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face)
{

}
//===============================================================================================
void MisCTool_PluginSuction::OnSoftBodyFaceBeAdded(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face)
{
	
}
//===============================================================================================
MisMedicOrgan_Ordinary * MisCTool_PluginSuction::SelectWhichOrganToBeSucked(FaceWithOrganMap & facesWithOrgan)
{
	float maxTotalFacesProjectArea = 0.0f;
	MisMedicOrgan_Ordinary *pOrganSelected = NULL;
	FaceWithOrganMap::iterator organItor = facesWithOrgan.begin();
	for(; organItor != facesWithOrgan.end() ; ++organItor)
	{
		MisMedicOrgan_Ordinary *pCurrOrgan = organItor->first;
		float currTotalArea = 0.f;
		float AreaInSuctionRegion = 0.0f;
		//bool IsFull = false;
		std::list<GFPhysSoftBodyFace*> & faces = organItor->second;
		std::list<GFPhysSoftBodyFace*>::iterator faceItor = faces.begin();
		for( ; faceItor != faces.end() ; )
		{
			bool IsInSuctionRange = false;
			if(m_SuctionRegion.IsFaceInSuctionRange(*faceItor))
			{
			    std::vector<Ogre::Vector2> projectVertics2d;
				float dists[3];
			    float area = m_SuctionRegion.ProjectFace(*faceItor , projectVertics2d , dists);
				float clipArea = m_SuctionRegion.GetAreaInSuctionRegion(projectVertics2d);
				if(clipArea > 0)
				{
					currTotalArea += area;
					AreaInSuctionRegion += clipArea;
					//if(AreaInSuctionRegion >= m_SuctionRegion.m_SuctionArea)
					 //  IsFull = true;
					IsInSuctionRange = true;
				}
			}

			if(IsInSuctionRange)
			   ++faceItor;
			else
			   faceItor = faces.erase(faceItor);
		}
		if(currTotalArea > maxTotalFacesProjectArea)
		{
			pOrganSelected = pCurrOrgan;
			maxTotalFacesProjectArea = currTotalArea;
			//m_SuctionAreaRatio = m_SuctionRegion.m_SuctionArea / currTotalArea;
		}
	}
	return pOrganSelected;
}
//===============================================================================================
void MisCTool_PluginSuction::ProcessSuckedNode()
{
	//float scaleFactor = 3.5f / m_SuctionAreaRatio;
	//float doubleRadius =  m_SuctionRegion.m_SuctionRadius * 2;

	SuckedNodeMap::iterator nodeItor = m_SuckedNodes.begin();
	for(; nodeItor != m_SuckedNodes.end() ; ++nodeItor)
	{
		SoftBodyNodeSucked & nodeInfo = nodeItor->second;
		m_SuctionRegion.ProjectNode(nodeItor->first , nodeInfo.m_ProjectPos);
	}
}
//===============================================================================================
void MisCTool_PluginSuction::UpdateMoveInfo(float dt)
{
	//after m_SuctionRegion.UpdateToWorldSpace();
	m_MoveIncrementInSuctionInvDir = (m_SuctionRegion.m_CenterWorld - m_SuctionRegion.m_LastCenterWorld).Dot(m_SuctionRegion.m_LastSuctionInvDirWorld);
	
	m_SuctionMoveSpeed = m_SuctionRegion.m_CenterWorld.Distance(m_SuctionRegion.m_LastCenterWorld)  / dt ;

	m_SuctionRegion.m_LastCenterWorld = m_SuctionRegion.m_CenterWorld;
	m_SuctionRegion.m_LastSuctionInvDirWorld = m_SuctionRegion.m_SuctionInvDirWorld;

	if(m_pOrganBeSucked)
	{
		m_pOrganBeSucked->m_FaceMoveIncrementInSuction = m_MoveIncrementInSuctionInvDir;
		m_pOrganBeSucked->m_FaceMoveSpeedInSuction = m_SuctionMoveSpeed;
	}

    //m_SuctionRegion.painting.Update(dt);
}