#include "CallBackForUnion.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicObjectSerializer.h"
#include "CustomConstraint.h"
#include "MisNewTraining.h"
#include "XMLWrapperAdhere.h"
#include "Math/GoPhysTransformUtil.h"
#include "MisMedicObjectSerializer.h"
#include "MXOgreGraphic.h"

using namespace GoPhys;

//=======================================================================================================================
//SoftBodyAdhersionCallBack
//=======================================================================================================================
void LinkedOrganNodeCallBack::ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
{
	GFPhysSoftBodyNode * NodeA = (GFPhysSoftBodyNode*)dynNodeA->m_UserData;
	GFPhysSoftBodyNode * NodeB = (GFPhysSoftBodyNode*)dynNodeB->m_UserData;

	GFPhysVector3 PoisiontA = NodeA->m_UnDeformedPos;
	GFPhysVector3 PoisiontB = NodeB->m_UnDeformedPos;

	float dist = (PoisiontA-PoisiontB).Length();

	if(dist < m_UnionThreasHold)
	{
		std::map<GFPhysSoftBodyNode * , NearestNode>::iterator itor = m_NearestNodes.find(NodeA);

		if(itor != m_NearestNodes.end())//already remapping 
		{
			NearestNode & nNode = itor->second;

			if(dist < nNode.m_MinDist)
			{
				nNode.m_MinDist = dist;
				nNode.m_Node = NodeB;
			}
		}
		else
		{
			NearestNode nNode(NodeB , dist);
			m_NearestNodes.insert(std::make_pair(NodeA , nNode));
		}
	}
}
//=======================================================================================================================
//=======================================================================================================================
//NodeNearestCallBack
//=======================================================================================================================
void NodeNearestCallBack::ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
{
	int indexA = (int)dynNodeA->m_UserData;
	int indexB = (int)dynNodeB->m_UserData;

	GFPhysVector3 PoisiontA = m_NodesPosA[indexA];
	GFPhysVector3 PoisiontB = m_NodesPosB[indexB];

	if(m_BRemappedTag[indexB] <= 0)
	{
		float dist = (PoisiontA-PoisiontB).Length();

		if(dist < m_UnionThreasHold)
		{
			int ExistsRemapIndex = m_AToBRemap[indexA];
			if(ExistsRemapIndex >= 0)//already remapping 
			{
				GFPhysVector3 remapPosInB = m_NodesPosB[ExistsRemapIndex];
				float existsDist = (remapPosInB-PoisiontA).Length();
				if(dist < existsDist)
				{
					m_AToBRemap[indexA] = indexB;
				}
			}
			else
			{
				m_AToBRemap[indexA] = indexB;
				m_BRemappedTag[indexB] = 1;
			}
		}
	}
	else
	{
		int i = 0;
		int j = i+1;
	}
}
//=======================================================================================================================
//S2DNearestCallBack
//=======================================================================================================================
void S2DNearestCallBack::ProcessOverlappedNodes(GFPhysDBVNode * NodeForStatic , GFPhysDBVNode * NodeForDynamic)
{
	int indexStatic = (int)NodeForStatic->m_UserData;
	GFPhysSoftBodyNode * dynNode = (GFPhysSoftBodyNode*)NodeForDynamic->m_UserData;

	SubMeshVertex & meshvert = m_StaticPos[indexStatic];
	GFPhysVector3 PosStatic  = meshvert.m_Position;
	GFPhysVector3 PosDynamic = dynNode->m_UnDeformedPos;

	float dist = (PosStatic-PosDynamic).Length();

	if(dist < m_UnionThreasHold)
	{
		bool exists = false;

		for(size_t n = 0; n < m_SharedVertex.size() ; n++)
		{
			if(m_SharedVertex[n].m_DynamicNode == dynNode)
			{
				m_SharedVertex[n].m_NodesInStatic.push_back(meshvert);
				exists = true;
				break;
			}
		}

		if(exists == false)
		{
			m_SharedVertex.push_back(DynStaticSharedVertex());
			m_SharedVertex[m_SharedVertex.size()-1].m_DynamicNode = dynNode;
			m_SharedVertex[m_SharedVertex.size()-1].m_NodesInStatic.push_back(meshvert);
		}
	}
}
//=======================================================================================================================

//=======================================================================================================================

void UniverseLinkCallBack::ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
{
	GFPhysSoftBodyNode * NodeA = (GFPhysSoftBodyNode*)dynNodeA->m_UserData;

	if (m_ElementType == 0)
	{
		GFPhysSoftBodyTetrahedron * TetraB = (GFPhysSoftBodyTetrahedron*)dynNodeB->m_UserData;

		GFPhysVector3 closetPoint = ClosetPtPointTetrahedron(NodeA->m_UnDeformedPos,
			TetraB->m_TetraNodes[0]->m_UnDeformedPos,
			TetraB->m_TetraNodes[1]->m_UnDeformedPos,
			TetraB->m_TetraNodes[2]->m_UnDeformedPos,
			TetraB->m_TetraNodes[3]->m_UnDeformedPos);


		float dist = (closetPoint - NodeA->m_UnDeformedPos).Length();

		if (dist < m_UnionThreasHold)
		{
			std::map<GFPhysSoftBodyNode *, NodeLieInTetra>::iterator itor = m_StickNodes.find(NodeA);

			if (itor != m_StickNodes.end())
			{
				NodeLieInTetra & nNode = itor->second;

				if (dist < nNode.m_MinDist)
				{
					nNode.m_MinDist = dist;
					nNode.m_Node = NodeA;
					nNode.m_tetra = TetraB;
					nNode.m_ClosetPoint = closetPoint;
				}
			}
			else
			{
				NodeLieInTetra nNode(NodeA, dist);
				nNode.m_tetra = TetraB;
				nNode.m_ClosetPoint = closetPoint;
				m_StickNodes.insert(std::make_pair(NodeA, nNode));
			}
		}
	}
	else
	{
		GFPhysSoftBodyFace * FaceB = (GFPhysSoftBodyFace*)dynNodeB->m_UserData;

		GFPhysVector3 closetPoint = ClosestPtPointTriangle(NodeA->m_UnDeformedPos,
			                                               FaceB->m_Nodes[0]->m_UnDeformedPos,
			                                               FaceB->m_Nodes[1]->m_UnDeformedPos,
			                                               FaceB->m_Nodes[2]->m_UnDeformedPos);


		float dist = (closetPoint - NodeA->m_UnDeformedPos).Length();

		if (dist < m_UnionThreasHold)
		{
			std::map<GFPhysSoftBodyNode *, NodeLieInTetra>::iterator itor = m_StickNodes.find(NodeA);

			if (itor != m_StickNodes.end())
			{
				NodeLieInTetra & nNode = itor->second;

				if (dist < nNode.m_MinDist)
				{
					nNode.m_MinDist = dist;
					nNode.m_Node = NodeA;
					nNode.m_Face = FaceB;
					nNode.m_ClosetPoint = closetPoint;
				}
			}
			else
			{
				NodeLieInTetra nNode(NodeA, dist);
				nNode.m_Face = FaceB;
				nNode.m_ClosetPoint = closetPoint;
				m_StickNodes.insert(std::make_pair(NodeA, nNode));
			}
		}
	}
}

//=======================================================================================================================

void UniverseEdgeFaceCallBack::ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)
{
	Real Rayweight;

	GFPhysVector3 intersectpt;

	Real triangleWeight[3];

	GFPhysSoftBodyFace * FaceB = (GFPhysSoftBodyFace*)UserData;

	bool intersect = LineIntersectTriangle(FaceB->m_Nodes[0]->m_UnDeformedPos , 
		FaceB->m_Nodes[1]->m_UnDeformedPos , 
		FaceB->m_Nodes[2]->m_UnDeformedPos , 
		m_RayStart , m_RayEnd , 
		Rayweight , 
		intersectpt ,
		triangleWeight);

	if(intersect == true && Rayweight > 0.0001f && Rayweight <= 1)
	{
		m_intersected = true;

		m_face = FaceB;
		m_FaceWeights[0] = triangleWeight[0];
		m_FaceWeights[1] = triangleWeight[1];
		m_FaceWeights[2] = triangleWeight[2];

		GPClamp(m_FaceWeights[0] , 0.0f , 1.0f);
		GPClamp(m_FaceWeights[1] , 0.0f , 1.0f);
		GPClamp(m_FaceWeights[2] , 0.0f , 1.0f);

		m_LineWeight[1] = Rayweight;
		GPClamp(m_LineWeight[1] , 0.0f , 1.0f);
		m_LineWeight[0] = 1.0f - m_LineWeight[1];
	}
}

//=======================================================================================================================

void FaceFaceCenterDistCallBack::ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
{
	GFPhysSoftBodyFace * FaceA = (GFPhysSoftBodyFace*)dynNodeA->m_UserData;

	GFPhysSoftBodyFace * FaceB = (GFPhysSoftBodyFace*)dynNodeB->m_UserData;

	GFPhysVector3 centerA = (FaceA->m_Nodes[0]->m_UnDeformedPos
		+FaceA->m_Nodes[1]->m_UnDeformedPos
		+FaceA->m_Nodes[2]->m_UnDeformedPos)*0.3333f;

	GFPhysVector3 centerB = (FaceB->m_Nodes[0]->m_UnDeformedPos
		+FaceB->m_Nodes[1]->m_UnDeformedPos
		+FaceB->m_Nodes[2]->m_UnDeformedPos)*0.3333f;

	float dist = (centerA-centerB).Length();

	if(dist < m_UnionThreasHold)
	{
		std::map<GFPhysSoftBodyFace * , FaceClosetFace>::iterator itor = m_ClostPairs.find(FaceA);

		if(itor != m_ClostPairs.end())
		{
			FaceClosetFace & nNode = itor->second;

			if(dist < nNode.m_MinDist)
			{
				nNode.m_MinDist = dist;
				nNode.m_FaceA = FaceA;
				nNode.m_FaceB = FaceB;
			}
		}
		else
		{
			FaceClosetFace nNode(FaceA , dist);
			nNode.m_FaceB = FaceB;
			m_ClostPairs.insert(std::make_pair(FaceA , nNode));
		}
	}
}
	
void FaceEdgeIntersectCallBack::ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
{
	int faceIndex =  (int)dynNodeA->m_UserData;

	GFPhysSoftBodyEdge * pEdge =  (GFPhysSoftBodyEdge *)dynNodeB->m_UserData;
	
	Real Rayweight;

	GFPhysVector3 intersectpt;

	Real triangleWeight[3];

	bool intersect = LineIntersectTriangle(m_pMeshVertex[faceIndex * 3].m_Position , 
		m_pMeshVertex[faceIndex * 3 + 1].m_Position  , 
		m_pMeshVertex[faceIndex * 3 + 2].m_Position  , 
		pEdge->m_Nodes[0]->m_UnDeformedPos , 
		pEdge->m_Nodes[1]->m_UnDeformedPos , 
		Rayweight , 
		intersectpt ,
		triangleWeight);

	if(intersect == true && Rayweight > (0.f - m_ThreasHold) && Rayweight <= (1+m_ThreasHold))
	{
		m_Nodes.insert(pEdge->m_Nodes[0]);
		m_Nodes.insert(pEdge->m_Nodes[1]);
	}
}
//=======================================================================================================================
void UniverseLinkCallBackV2::ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
{
	if(m_Type == ULCB_NODE_FACE)
		ProcessNodeWithFace(dynNodeA , dynNodeB);
	else if(m_Type == ULCB_NODE_TETRA)
		ProcessNodeWithTetra(dynNodeA , dynNodeB);
}

void UniverseLinkCallBackV2::ProcessNodeWithFace(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
{
	int nodeIndex = (int)dynNodeA->m_UserData;
	int faceIndex = (int)dynNodeB->m_UserData;

	const MisMedicObjetSerializer::MisSerialFace & face = m_pOrganConnectTo->GetSerializerFace(faceIndex);

	if(face.m_IsValid)
	{
		GFPhysVector3 closetPoint = ClosestPtPointTriangle(m_pOrganOfNode->GetSerializerNodePos(nodeIndex) , 
			m_pOrganConnectTo->GetSerializerNodePos(face.m_Index[0]) ,
			m_pOrganConnectTo->GetSerializerNodePos(face.m_Index[1]) ,
			m_pOrganConnectTo->GetSerializerNodePos(face.m_Index[2]));


		float dist = (closetPoint - m_pOrganOfNode->GetSerializerNodePos(nodeIndex)).Length();

		if(dist < m_UnionThreasHold)
		{
			std::map<int , ConnectedNode>::iterator itor = m_StickNodes.find(nodeIndex);

			if(itor != m_StickNodes.end())
			{
				ConnectedNode & nNode = itor->second;

				if(dist < nNode.m_MinDist)
				{
					nNode.m_MinDist = dist;
					nNode.m_FaceIndex = faceIndex;
					nNode.m_ClosetPoint = GPVec3ToOgre(closetPoint);
				}
			}
			else
			{
				ConnectedNode nNode(nodeIndex , dist);
				nNode.m_FaceIndex = faceIndex;
				nNode.m_ClosetPoint = GPVec3ToOgre(closetPoint);
				m_StickNodes.insert(std::make_pair(nodeIndex , nNode));
			}
		}
	}
}

void UniverseLinkCallBackV2::ProcessNodeWithTetra(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB)
{
	int nodeIndex = (int)dynNodeA->m_UserData;
	int tetraIndex = (int)dynNodeB->m_UserData;

	const MisMedicObjetSerializer::MisSerialTetra & tetra = m_pOrganConnectTo->GetSerializerTetra(tetraIndex);

	if(tetra.m_IsValid)
	{
		GFPhysVector3 closetPoint = ClosetPtPointTetrahedron(m_pOrganOfNode->GetSerializerNodePos(nodeIndex) , 
			m_pOrganConnectTo->GetSerializerNodePos(tetra.m_Index[0]) ,
			m_pOrganConnectTo->GetSerializerNodePos(tetra.m_Index[1]) ,
			m_pOrganConnectTo->GetSerializerNodePos(tetra.m_Index[2]) ,
			m_pOrganConnectTo->GetSerializerNodePos(tetra.m_Index[3]));


		float dist = (closetPoint - m_pOrganOfNode->GetSerializerNodePos(nodeIndex)).Length();

		if(dist < m_UnionThreasHold)
		{
			std::map<int , ConnectedNode>::iterator itor = m_StickNodes.find(nodeIndex);

			if(itor != m_StickNodes.end())
			{
				ConnectedNode & nNode = itor->second;

				if(dist < nNode.m_MinDist)
				{
					nNode.m_MinDist = dist;
					nNode.m_TetraIndex = tetraIndex;
					nNode.m_ClosetPoint = GPVec3ToOgre(closetPoint);
				}
			}
			else
			{
				ConnectedNode nNode(nodeIndex , dist);
				nNode.m_TetraIndex = tetraIndex;
				nNode.m_ClosetPoint = GPVec3ToOgre(closetPoint);
				m_StickNodes.insert(std::make_pair(nodeIndex , nNode));
			}
		}
	}
}