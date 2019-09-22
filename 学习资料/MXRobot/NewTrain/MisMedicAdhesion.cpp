#include "MisMedicAdhesion.h"
#include "CallBackForUnion.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicObjectSerializer.h"
#include "CustomConstraint.h"
#include "MisNewTraining.h"
#include "XMLWrapperAdhesion.h"
#include "Math/GoPhysTransformUtil.h"
#include "MXOgreGraphic.h"
#include "CustomConstraint.h"


using namespace GoPhys;

MisMedicAdhesionCluster::OrganConnectionInfo::OrganConnectionInfo(MisMedicOrgan_Ordinary * pOrgan) : m_pOrgan(pOrgan) 
{

}



MisMedicAdhesionCluster::MisMedicAdhesionCluster(MisMedicOrgan_Ordinary *pAdhesion , CXMLWrapperAdhesionCluster * pAdhersionClusterConfig)
: m_pAdhesion(pAdhesion) ,
m_IsScale(pAdhersionClusterConfig->m_IsScale),
m_IsAutoFindDir(pAdhersionClusterConfig->m_IsAutoDir),
m_AdhesionScaleFactor(pAdhersionClusterConfig->m_ScaleFactor)
{
	if(m_IsScale && !m_IsAutoFindDir)
	{
		if(pAdhersionClusterConfig->m_flag_NodeIndicesA && pAdhersionClusterConfig->m_flag_NodeIndicesB)
		{
			std::set<int> temp;

			Ogre::vector<Ogre::String>::type  nodeIndicesStrs = Ogre::StringUtil::split(pAdhersionClusterConfig->m_NodeIndicesA , ",");
			for(size_t n = 0 ; n < nodeIndicesStrs.size() ; n++)
			{
				Ogre::String & str = nodeIndicesStrs[n];
				int index = Ogre::StringConverter::parseInt(str);
				temp.insert(index);
			}

			m_NodeIndicesOfAdhesionEndA.insert(m_NodeIndicesOfAdhesionEndA.begin() , temp.begin() , temp.end());

			temp.clear();

			nodeIndicesStrs = Ogre::StringUtil::split(pAdhersionClusterConfig->m_NodeIndicesB , ",");
			for(size_t n = 0 ; n < nodeIndicesStrs.size() ; n++)
			{
				Ogre::String & str = nodeIndicesStrs[n];
				int index = Ogre::StringConverter::parseInt(str);
				temp.insert(index);
			}

			m_NodeIndicesOfAdhesionEndB.insert(m_NodeIndicesOfAdhesionEndB.begin() , temp.begin() , temp.end());
		}
		else
			m_IsScale = false;
	}
}

MisMedicAdhesionCluster::~MisMedicAdhesionCluster()
{

}

void MisMedicAdhesionCluster::OnFaceRemoved(GFPhysSoftBodyFace * pFace , GFPhysSoftBodyShape * pHostshape)
{
	if(pHostshape->m_HostBody == m_pAdhesion->m_physbody)
	{
		for(int n = 0 ; n < 3 ; n++)
		{
			GFPhysSoftBodyNode *pNode = pFace->m_Nodes[n];
			NodeWithIndicesMap::iterator nodeItor = m_NodeOfAdhesionWithIndicesMap.find(pNode);
			if(nodeItor != m_NodeOfAdhesionWithIndicesMap.end())
			{
				std::vector<int> & conIndices = nodeItor->second;
				for(size_t c = 0 ; c < conIndices.size() ; c++)
				{
					GFPhysPositionConstraint *pCons = m_Constraints[conIndices[c]];
					if(pCons != NULL)
					{
						PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(pCons);
						delete pCons;
						m_Constraints[conIndices[c]] = NULL;
					}
				}
				m_NodeOfAdhesionWithIndicesMap.erase(nodeItor);
			}
		}
	}
	else
	{
		for(size_t o = 0 ; o < m_OrganConnectionInfoVec.size() ; o++)
		{
			OrganConnectionInfo & organConnInfo = m_OrganConnectionInfoVec[o];
			if((organConnInfo.m_pOrgan)->m_physbody == pHostshape->m_HostBody)
			{
				NodeWithIndicesMap & nodeMap = organConnInfo.m_NodeOfOrganWithIndicesMap;
				for(int n = 0 ; n < 3 ; n++)
				{
					GFPhysSoftBodyNode *pNode = pFace->m_Nodes[n];
					NodeWithIndicesMap::iterator nodeItor = nodeMap.find(pNode);
					if(nodeItor != nodeMap.end())
					{
						std::vector<int> & conIndices = nodeItor->second;
						for(size_t c = 0 ; c < conIndices.size() ; c++)
						{
							GFPhysPositionConstraint *pCons = m_Constraints[conIndices[c]];
							if(pCons != NULL)
							{
								PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(pCons);
								delete pCons;
								m_Constraints[conIndices[c]] = NULL;
							}
						}
						nodeMap.erase(nodeItor);
					}
				}
			}
		}
	}
}


bool MisMedicAdhesionCluster::BuildAdhesionCluster(CXMLWrapperAdhesionCluster * pAdhesionClusterConfig, MisNewTraining * pMisNewTraining , std::set<int> & organsID)
{
	if(!m_pAdhesion)
		return false;
	
	std::set<int>::iterator itor = organsID.begin();
	for(; itor	!= organsID.end() ; ++itor)
	{
		MisMedicOrgan_Ordinary * pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(pMisNewTraining->GetOrgan(*itor));
		if(pOrgan) 
		{
			if(m_OrgansInCluster.find(pOrgan) == m_OrgansInCluster.end()) 
			{
				m_OrgansInCluster.insert(pOrgan);
				BuildConnectionToOrgan(pOrgan , pAdhesionClusterConfig->m_Range);
			}
		}
	}

	return true;
}

void MisMedicAdhesionCluster::AddOrgansToCluster(CXMLWrapperAdhesionCluster *pAdhesionClusterConfig, MisNewTraining *pMisNewTraining)
{

}

bool MisMedicAdhesionCluster::FindAllConnection(CXMLWrapperAdhesionCluster * pAdhesionClusterConfig, MisNewTraining * pMisNewTraining , std::set<int> & organsID)
{
	if(!m_pAdhesion)
		return false;
	
	bool isCreateNodeTree = false;
	std::set<int>::iterator itor = organsID.begin();
	for(; itor	!= organsID.end() ; ++itor)
	{
		MisMedicOrgan_Ordinary * pOrgan = dynamic_cast<MisMedicOrgan_Ordinary*>(pMisNewTraining->GetOrgan(*itor));
		if(pOrgan) 
		{
			if(m_OrgansInCluster.find(pOrgan) == m_OrgansInCluster.end()) 
			{
				m_OrgansInCluster.insert(pOrgan);
				/*FindConnectionToOrgan(pOrgan , pAdhesionClusterConfig->m_Range);*/
				if(!isCreateNodeTree)
				{
					m_pAdhesion->CreateSerializerNodeTree(pAdhesionClusterConfig->m_Range);
					isCreateNodeTree = true;
				}

				if(false/*pOrgan->GetCreateInfo().m_objTopologyType == DOT_VOLMESH*/)
					FindNodeTetraConnectionWithOrgan(pOrgan , pAdhesionClusterConfig->m_Range);
				else if(true/*pOrgan->GetCreateInfo().m_objTopologyType == DOT_MEMBRANE*/)
					FindNodeFaceConnectionWithOrgan(pOrgan ,  pAdhesionClusterConfig->m_Range);
			}
		}
	}

	return !m_OrganConnectionInfoVec.empty();
}

void MisMedicAdhesionCluster::BuildAllConnection(float constraintStiffness /* = 0.99 */)
{
	m_pAdhesion->m_EleCatogeryCanCut = EMMT_LayerMembrane;

	for(size_t o = 0 ; o < m_OrganConnectionInfoVec.size() ; o++)
	{
		OrganConnectionInfo & currOrganConnectionInfo = m_OrganConnectionInfoVec[o];

		MisMedicOrgan_Ordinary *pOrgan = currOrganConnectionInfo.m_pOrgan;
		
		if(false/*pOrgan->GetCreateInfo().m_objTopologyType == DOT_VOLMESH*/)
		{
			BuildNodeTetraConnectionWithOrgan(currOrganConnectionInfo , constraintStiffness);
		}
		else if(true/*pOrgan->GetCreateInfo().m_objTopologyType == DOT_MEMBRANE*/)
		{
			BuildNodeFaceConnectionWithOrgan(currOrganConnectionInfo , constraintStiffness);
		}
	}
}

GFPhysVector3 MisMedicAdhesionCluster::GetScaleDir()
{
	if(m_IsAutoFindDir)
	{
		std::vector<Ogre::Vector3> centers;
		for(size_t r = 0 ; r < m_ConnectionRegions.size() ; r++)
		{
			centers.push_back(m_ConnectionRegions[r].m_Center);
		}

		std::vector<Ogre::Vector3> temp;
		while(centers.size() != 2)
		{
			float minDist = 10000;
			int pairA, pairB;
			temp.insert(temp.begin() , centers.begin(), centers.end());
			for(int i = 0 ; i < temp.size() ; i++)
			{
				Ogre::Vector3 & a = temp[i];
				for(int j = i + 1 ; j < temp.size() ; j++)
				{
					Ogre::Vector3 & b = temp[j];
					if(a.distance(b) < minDist)
					{
						pairA = i;
						pairB = j;
					}
				}
			}
			centers.clear();
			centers.push_back((temp[pairA] + temp[pairB]) * 0.5);
			for(int i = 0 ; i < temp.size() ; i++)
			{
				if(i != pairA && i != pairB)
					centers.push_back(temp[i]);
			}	
		}

		Ogre::Vector3 result = centers[1] - centers[0];
		result.normalise();
		return OgreToGPVec3(result);
	}
	else
	{
		GFPhysVector3 centerA = GFPhysVector3(0,0,0);
		GFPhysVector3 centerB = GFPhysVector3(0,0,0);

		for(int n = 0 ; n < m_NodeIndicesOfAdhesionEndA.size() ; n++)
			centerA += m_pAdhesion->GetSerializerNodePos(m_NodeIndicesOfAdhesionEndA[n]);

		for(int n = 0 ; n < m_NodeIndicesOfAdhesionEndB.size() ; n++)
			centerB += m_pAdhesion->GetSerializerNodePos(m_NodeIndicesOfAdhesionEndB[n]);

		centerA /= m_NodeIndicesOfAdhesionEndA.size();
		centerB /= m_NodeIndicesOfAdhesionEndB.size();

		return (centerA - centerB).Normalized();
	}
}

void MisMedicAdhesionCluster::FindNodeTetraConnectionWithOrgan(MisMedicOrgan_Ordinary * pOrgan , float threshold)
{
	const GFPhysDBVTree & AdhesionNodeTree = m_pAdhesion->GetSerializerNodeTree();

	const GFPhysDBVTree & OrganTetraTree = pOrgan->GetSerializerTetraTree();

	UniverseLinkCallBackV2 NodesInsideCallBack(m_pAdhesion , pOrgan , threshold , UniverseLinkCallBackV2::ULCB_NODE_TETRA);

	AdhesionNodeTree.CollideWithDBVTree(OrganTetraTree , &NodesInsideCallBack);

	if(!NodesInsideCallBack.m_StickNodes.empty())
	{
		OrganConnectionInfo temp(pOrgan);

		m_OrganConnectionInfoVec.push_back(temp);

		OrganConnectionInfo & organInfo = m_OrganConnectionInfoVec.back();

		std::map<int , ConnectedNode>::iterator nodeItor = NodesInsideCallBack.m_StickNodes.begin();

		while(nodeItor != NodesInsideCallBack.m_StickNodes.end())
		{
			ConnectedNode & connectNode = nodeItor->second;
			organInfo.m_StickNodesWithTetra.push_back(connectNode);
			++nodeItor;
		}
	}
}

void MisMedicAdhesionCluster::FindNodeFaceConnectionWithOrgan(MisMedicOrgan_Ordinary * pOrgan , float threshold)
{
	const GFPhysDBVTree & AdhesionNodeTree = m_pAdhesion->GetSerializerNodeTree();

	const GFPhysDBVTree & OrganFaceTree = pOrgan->GetSerializerFaceTree();

	UniverseLinkCallBackV2 NodesInsideCallBack(m_pAdhesion , pOrgan , threshold , UniverseLinkCallBackV2::ULCB_NODE_FACE);

	AdhesionNodeTree.CollideWithDBVTree(OrganFaceTree , &NodesInsideCallBack);

	if(!NodesInsideCallBack.m_StickNodes.empty())
	{
		OrganConnectionInfo temp(pOrgan);

		m_OrganConnectionInfoVec.push_back(temp);

		OrganConnectionInfo & organInfo = m_OrganConnectionInfoVec.back();

		std::map<int , ConnectedNode>::iterator nodeItor = NodesInsideCallBack.m_StickNodes.begin();

		while(nodeItor != NodesInsideCallBack.m_StickNodes.end())
		{
			ConnectedNode & connectNode = nodeItor->second;
			organInfo.m_StickNodesWithFace.push_back(connectNode);
			++nodeItor;
		}
	}
}

int MisMedicAdhesionCluster::ComputeConnectionRegionV2()
{
	for(size_t o = 0 ; o < m_OrganConnectionInfoVec.size() ; o++)
	{
		ComputeConnectionRegionOfOneOrgan(m_OrganConnectionInfoVec[o]);
	}

	for(size_t r = 0 ; r < m_ConnectionRegions.size() ; r++)
	{
		GFPhysVector3 center = GFPhysVector3(0,0,0);
		std::vector<int> &indices = m_ConnectionRegions[r].m_AdhesionNodeIndices;
		for(size_t n = 0 ; n < indices.size() ; n++)
		{
			center += m_pAdhesion->GetSerializerNodePos(indices[n]);
		}
		center /= indices.size();
		m_ConnectionRegions[r].m_Center = GPVec3ToOgre(center);
	}

	return m_ConnectionRegions.size();
}

void MisMedicAdhesionCluster::ComputeConnectionRegionOfOneOrgan(OrganConnectionInfo & organConnInfo)
{
	std::vector<int> region;
	
	std::map<int,int> tetraWithRegion;

	std::map<int,int> faceWithRegion;

	std::map<int,int> tetraNodeWithRegion;

	std::map<int,int> faceNodeWithRegion;


	MisMedicOrgan_Ordinary * pOrgan = organConnInfo.m_pOrgan;
		
	if(false/*pOrgan->GetCreateInfo().m_objTopologyType == DOT_VOLMESH*/)
	{
		std::list<ConnectedNode> & StickNodesWithTetra =  organConnInfo.m_StickNodesWithTetra;
		std::list<ConnectedNode>::iterator AdhesionNodeItor = StickNodesWithTetra.begin();
		for(; AdhesionNodeItor != StickNodesWithTetra.end() ; ++AdhesionNodeItor )
		{
			ConnectedNode & nodeWithTetra = *AdhesionNodeItor;
			const MisMedicObjetSerializer::MisSerialTetra & tetra = pOrgan->GetSerializerTetra(nodeWithTetra.m_TetraIndex);
			int lastRegion = -1;
			for(int n = 0 ; n < 4 ; n++)
			{
				int tetraNodeIndex = tetra.m_Index[n];
				std::map<int,int>::iterator tetraNodeItor = tetraNodeWithRegion.find(tetraNodeIndex);
				if(lastRegion == -1)	//first node's index
				{
					if(tetraNodeItor != tetraNodeWithRegion.end())		//已标记区域
					{
						lastRegion = tetraNodeItor->second;
					}
					else
					{
						region.push_back(region.size());
						lastRegion = region.back();
						tetraNodeWithRegion[tetraNodeIndex] = lastRegion;
					}
				}
				else
				{
					if(tetraNodeItor != tetraNodeWithRegion.end())		//已标记区域
					{
						int currRegion = tetraNodeItor->second;
						int originRegion = region[currRegion];
						if(originRegion == currRegion)
						{
							region[lastRegion] = originRegion;
							lastRegion = originRegion;
						}
						else
						{
							region[currRegion] = lastRegion;
							region[originRegion] = lastRegion;
						}
					}
					else
					{
						tetraNodeWithRegion[tetraNodeIndex] = lastRegion;
					}
				}
			}
			tetraWithRegion[nodeWithTetra.m_TetraIndex] = lastRegion;
		}

		//
		std::map<int , ConnectionRegion> regionMap;
		AdhesionNodeItor = StickNodesWithTetra.begin();
		for(; AdhesionNodeItor != StickNodesWithTetra.end() ; ++AdhesionNodeItor )
		{
			ConnectedNode & nodeWithTetra = *AdhesionNodeItor;
			int regionIndex = tetraWithRegion[nodeWithTetra.m_TetraIndex];
			while(region[regionIndex] != regionIndex)
			{
				regionIndex = region[regionIndex];
			}
			std::map<int , ConnectionRegion>::iterator regionItor = regionMap.find(regionIndex);
			if(regionItor != regionMap.end())
			{
				ConnectionRegion &r = regionItor->second;
				r.m_AdhesionNodeIndices.push_back(nodeWithTetra.m_NodeIndex);
			}
			else
			{
				ConnectionRegion r(pOrgan);
				r.m_AdhesionNodeIndices.push_back(nodeWithTetra.m_NodeIndex);
				regionMap[regionIndex] = r;
			}
		}

		std::map<int , ConnectionRegion>::iterator regionItor = regionMap.begin();
		while(regionItor != regionMap.end())
		{
			m_ConnectionRegions.push_back(regionItor->second);
			++regionItor;
		}
	}
	else if(true/*pOrgan->GetCreateInfo().m_objTopologyType == DOT_MEMBRANE*/)
	{
		std::list<ConnectedNode> & StickNodesWithFace =  organConnInfo.m_StickNodesWithFace;
		std::list<ConnectedNode>::iterator AdhesionNodeItor = StickNodesWithFace.begin();
		for(; AdhesionNodeItor != StickNodesWithFace.end() ; ++AdhesionNodeItor )
		{
			ConnectedNode & nodeWithFace = *AdhesionNodeItor;
			const MisMedicObjetSerializer::MisSerialFace & face = pOrgan->GetSerializerFace(nodeWithFace.m_FaceIndex);
			int lastRegion = -1;
			for(int n = 0 ; n < 3 ; n++)
			{
				int faceNodeIndex = face.m_Index[n];
				std::map<int,int>::iterator faceNodeItor = faceNodeWithRegion.find(faceNodeIndex);
				if(lastRegion == -1)	//first node's index
				{
					if(faceNodeItor != faceNodeWithRegion.end())		//已标记区域
					{
						lastRegion = faceNodeItor->second;
					}
					else
					{
						region.push_back(region.size());
						lastRegion = region.back();
						faceNodeWithRegion[faceNodeIndex] = lastRegion;
					}
				}
				else
				{
					if(faceNodeItor != faceNodeWithRegion.end())		//已标记区域
					{
						int currRegion = faceNodeItor->second;
						int originRegion = region[currRegion];
						if(originRegion == currRegion)
						{
							region[lastRegion] = originRegion;
							lastRegion = originRegion;
						}
						else
						{
							region[currRegion] = lastRegion;
							region[originRegion] = lastRegion;
						}
					}
					else
					{
						faceNodeWithRegion[faceNodeIndex] = lastRegion;
					}
				}
			}
			faceWithRegion[nodeWithFace.m_FaceIndex] = lastRegion;
		}

		//
		std::map<int , ConnectionRegion> regionMap;
		AdhesionNodeItor = StickNodesWithFace.begin();
		for(; AdhesionNodeItor != StickNodesWithFace.end() ; ++AdhesionNodeItor )
		{
			ConnectedNode & nodeWithFace = *AdhesionNodeItor;
			int regionIndex = faceWithRegion[nodeWithFace.m_FaceIndex];
			while(region[regionIndex] != regionIndex)
			{
				regionIndex = region[regionIndex];
			}
			std::map<int , ConnectionRegion>::iterator regionItor = regionMap.find(regionIndex);
			if(regionItor != regionMap.end())
			{
				ConnectionRegion &r = regionItor->second;
				r.m_AdhesionNodeIndices.push_back(nodeWithFace.m_NodeIndex);
			}
			else
			{
				ConnectionRegion r(pOrgan);
				r.m_AdhesionNodeIndices.push_back(nodeWithFace.m_NodeIndex);
				regionMap[regionIndex] = r;
			}
		}

		std::map<int , ConnectionRegion>::iterator regionItor = regionMap.begin();
		while(regionItor != regionMap.end())
		{
			m_ConnectionRegions.push_back(regionItor->second);
			++regionItor;
		}
	}
}

void MisMedicAdhesionCluster::BuildConnectionToOrgan(MisMedicOrgan_Ordinary * pOrgan , float threshold , float constraintStiffness /* = 0.99 */)
{
	m_OrganConnectionInfoVec.push_back(OrganConnectionInfo(pOrgan));

	OrganConnectionInfo & currOrganConnectionInfo = m_OrganConnectionInfoVec.back();

	const GFPhysDBVTree & OrganTetraTree = pOrgan->m_physbody->GetSoftBodyShape().GetTetrahedronBVTree(false);
	
	GFPhysVector3 Extend(threshold , threshold , threshold);

	GFPhysDBVTree AdhesionNodeTree;

	GFPhysSoftBodyNode * pNode = m_pAdhesion->m_physbody->GetNodeList();

	while(pNode)
	{
		GFPhysDBVNode * pDbvn = AdhesionNodeTree.InsertAABBNode(pNode->m_UnDeformedPos - Extend , pNode->m_UnDeformedPos + Extend);
		pDbvn->m_UserData = pNode;
		pNode = pNode->m_Next;
	}

	//check those node lie in the threshold of A' s tetrahedron
	UniverseLinkCallBack NodesInsideCallBack(threshold);

	AdhesionNodeTree.CollideWithDBVTree(OrganTetraTree , &NodesInsideCallBack);

	std::map<GFPhysSoftBodyNode * , NodeLieInTetra>::iterator nodeItor = NodesInsideCallBack.m_StickNodes.begin();

	while(nodeItor != NodesInsideCallBack.m_StickNodes.end())
	{
		NodeLieInTetra temp = nodeItor->second;

		GFPhysSoftBodyTetrahedron * tetra = temp.m_tetra;

		float TetraWeights[4];

		bool  gettedf = GetPointBarycentricCoordinate(  tetra->m_TetraNodes[0]->m_UnDeformedPos,
			tetra->m_TetraNodes[1]->m_UnDeformedPos,
			tetra->m_TetraNodes[2]->m_UnDeformedPos,
			tetra->m_TetraNodes[3]->m_UnDeformedPos,
			temp.m_ClosetPoint,
			TetraWeights);

		if(gettedf)
		{
			//temp.m_Node->m_InvM *= 0.2f;//increase mass
			TetrahedronAttachConstraint * cs = new TetrahedronAttachConstraint( temp.m_Node , 
				tetra,
				TetraWeights);

			cs->SetStiffness(constraintStiffness);

			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(cs);

			m_Constraints.push_back(cs);

			temp.m_Node->m_StateFlag |= GPSESF_CONNECTED;

			int currConsIndex = m_Constraints.size() - 1;
			//四面体四个node
			for(int n = 0 ; n < 4; n++)
			{
				GFPhysSoftBodyNode * pTetraNode = tetra->m_TetraNodes[n];

				NodeWithIndicesMap::iterator nodeItor = currOrganConnectionInfo.m_NodeOfOrganWithIndicesMap.find(pTetraNode);

				if(nodeItor != currOrganConnectionInfo.m_NodeOfOrganWithIndicesMap.end())
				{
					std::vector<int> & indices =nodeItor->second;
					indices.push_back(currConsIndex);
				}
				else
				{
					std::vector<int> indices;
					indices.push_back(currConsIndex);
					currOrganConnectionInfo.m_NodeOfOrganWithIndicesMap.insert(std::make_pair(pTetraNode , indices));
				}
			}	
			//adhesion一个node
			NodeWithIndicesMap::iterator nodeItor = m_NodeOfAdhesionWithIndicesMap.find(temp.m_Node);
			if(nodeItor != m_NodeOfAdhesionWithIndicesMap.end())
			{
				std::vector<int> & indices = nodeItor->second;
				indices.push_back(currConsIndex);
			}
			else
			{
				std::vector<int> indices;
				indices.push_back(currConsIndex);
				m_NodeOfAdhesionWithIndicesMap.insert(std::make_pair(temp.m_Node , indices));
			}
		}
		nodeItor++;
	}

	return;

	//now for every edge check whether intersect organ
	GFPhysSoftBodyEdge * pEdge = m_pAdhesion->m_physbody->GetEdgeList();
	while(pEdge)
	{
		GFPhysSoftBodyNode * edgeNodes[2];

		edgeNodes[0] = pEdge->m_Nodes[0];
		edgeNodes[1] = pEdge->m_Nodes[1];

		GFPhysVector3  startRay = edgeNodes[0]->m_UnDeformedPos;

		GFPhysVector3  EndRay   = edgeNodes[1]->m_UnDeformedPos;

		UniverseEdgeFaceCallBack EdgeFaceCallBack(startRay , EndRay);

		pOrgan->m_physbody->GetSoftBodyShape().TraverseFaceTreeAgainstRay(&EdgeFaceCallBack , startRay , EndRay , false);

		if(EdgeFaceCallBack.m_intersected == true)
		{
			GFPhysSoftBodyNode * faceNodes[3];

			faceNodes[0] = EdgeFaceCallBack.m_face->m_Nodes[0];
			faceNodes[1] = EdgeFaceCallBack.m_face->m_Nodes[1];
			faceNodes[2] = EdgeFaceCallBack.m_face->m_Nodes[2];

			FaceEdgePointAttachConstraint * cs = new  FaceEdgePointAttachConstraint(EdgeFaceCallBack.m_face , pEdge ,  EdgeFaceCallBack.m_FaceWeights , EdgeFaceCallBack.m_LineWeight , 1.0f , 1.0f);

			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(cs);

			cs->SetStiffness(constraintStiffness);

			m_Constraints.push_back(cs);

			int currConsIndex = m_Constraints.size() - 1;
			//face三个node
			for(int n = 0 ; n < 3 ; n++)
			{
				NodeWithIndicesMap::iterator nodeItor = currOrganConnectionInfo.m_NodeOfOrganWithIndicesMap.find(faceNodes[n]);

				if(nodeItor != currOrganConnectionInfo.m_NodeOfOrganWithIndicesMap.end())
				{
					std::vector<int> & indices =nodeItor->second;
					indices.push_back(currConsIndex);
				}
				else
				{
					std::vector<int> indices;
					indices.push_back(currConsIndex);
					currOrganConnectionInfo.m_NodeOfOrganWithIndicesMap.insert(std::make_pair(faceNodes[n] , indices));
				}
			}
			//edge 两个node
			for(int n = 0 ; n < 2 ; n++)
			{
				NodeWithIndicesMap::iterator nodeItor = m_NodeOfAdhesionWithIndicesMap.find(edgeNodes[n]);

				if(nodeItor != m_NodeOfAdhesionWithIndicesMap.end())
				{
					std::vector<int> & indices =nodeItor->second;
					indices.push_back(currConsIndex);
				}
				else
				{
					std::vector<int> indices;
					indices.push_back(currConsIndex);
					m_NodeOfAdhesionWithIndicesMap.insert(std::make_pair(edgeNodes[n] , indices));
				}
			}
		}
		pEdge = pEdge->m_Next;
	}
}

void MisMedicAdhesionCluster::BuildNodeTetraConnectionWithOrgan(OrganConnectionInfo & organConnInfo , float constraintStiffness /* = 0.99 */)
{
	MisMedicOrgan_Ordinary * pOrgan = organConnInfo.m_pOrgan;

	std::list<ConnectedNode> & connections = organConnInfo.m_StickNodesWithTetra;

	std::list<ConnectedNode>::iterator connItor = connections.begin();

	for( ; connItor != connections.end() ; ++connItor)
	{
		ConnectedNode & conn = *connItor;

		GFPhysSoftBodyTetrahedron * pTetra = pOrgan->m_physbody->GetTetrahedronAtIndex(conn.m_TetraIndex);

		float TetraWeights[4];

		bool  gettedf = GetPointBarycentricCoordinate(pTetra->m_TetraNodes[0]->m_UnDeformedPos,
			pTetra->m_TetraNodes[1]->m_UnDeformedPos,
			pTetra->m_TetraNodes[2]->m_UnDeformedPos,
			pTetra->m_TetraNodes[3]->m_UnDeformedPos,
			OgreToGPVec3(conn.m_ClosetPoint),
			TetraWeights);

		if(gettedf)
		{
			GFPhysSoftBodyNode * pNodeInAdhesion = m_pAdhesion->m_physbody->GetNode(conn.m_NodeIndex);

			TetrahedronAttachConstraint * cs = new TetrahedronAttachConstraint( pNodeInAdhesion, 
																				pTetra,//->m_TetraNodes,
																				TetraWeights);

			cs->SetStiffness(constraintStiffness);

			PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(cs);

			m_Constraints.push_back(cs);

			int currConsIndex = m_Constraints.size() - 1;
			//四面体四个node
			for(int n = 0 ; n < 4; n++)
			{
				GFPhysSoftBodyNode * pTetraNode = pTetra->m_TetraNodes[n];

				NodeWithIndicesMap::iterator nodeItor = organConnInfo.m_NodeOfOrganWithIndicesMap.find(pTetraNode);

				if(nodeItor != organConnInfo.m_NodeOfOrganWithIndicesMap.end())
				{
					std::vector<int> & indices =nodeItor->second;
					indices.push_back(currConsIndex);
				}
				else
				{
					std::vector<int> indices;
					indices.push_back(currConsIndex);
					organConnInfo.m_NodeOfOrganWithIndicesMap.insert(std::make_pair(pTetraNode , indices));

					//只set一次
					pTetraNode->SetMass(pTetraNode->m_Mass * 3);
				}
			}	
			//adhesion一个node
			NodeWithIndicesMap::iterator nodeItor = m_NodeOfAdhesionWithIndicesMap.find(pNodeInAdhesion);
			if(nodeItor != m_NodeOfAdhesionWithIndicesMap.end())
			{
				std::vector<int> & indices = nodeItor->second;
				indices.push_back(currConsIndex);
			}
			else
			{
				std::vector<int> indices;
				indices.push_back(currConsIndex);
				m_NodeOfAdhesionWithIndicesMap.insert(std::make_pair(pNodeInAdhesion, indices));
			}
		}
	}
}

void MisMedicAdhesionCluster::BuildNodeFaceConnectionWithOrgan(OrganConnectionInfo & organConnInfo , float constraintStiffness /* = 0.99 */)
{
	MisMedicOrgan_Ordinary *pOrgan = organConnInfo.m_pOrgan;

	std::list<ConnectedNode> & connections = organConnInfo.m_StickNodesWithFace;

	std::list<ConnectedNode>::iterator connItor = connections.begin();

	for( ; connItor != connections.end() ; ++connItor)
	{
		ConnectedNode & conn = *connItor;

		GFPhysSoftBodyFace * pFace = pOrgan->m_physbody->GetFaceAtIndex(conn.m_FaceIndex);

		Real FaceWeights[3];

		CalcBaryCentric(pFace->m_Nodes[0]->m_UnDeformedPos,
			pFace->m_Nodes[1]->m_UnDeformedPos,
			pFace->m_Nodes[2]->m_UnDeformedPos,
			OgreToGPVec3(conn.m_ClosetPoint),
			FaceWeights[0],
			FaceWeights[1],
			FaceWeights[2]);

		GFPhysSoftBodyNode * pNodeInAdhesion = m_pAdhesion->m_physbody->GetNode(conn.m_NodeIndex);

		SoftBodyFaceNodeDistConstraint * pCs = new SoftBodyFaceNodeDistConstraint/*CustomedSoftBodyFaceNodeDistConstraint*/();

		pCs->Initalize(pFace , pNodeInAdhesion , FaceWeights);

		pCs->SetStiffness(constraintStiffness);

		pCs->SetRestLength(0);

		PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(pCs);

		m_Constraints.push_back(pCs);

		int currConsIndex = m_Constraints.size() - 1;
		//face三个node
		for(int n = 0 ; n < 3; n++)
		{
			GFPhysSoftBodyNode * pFaceNode = pFace->m_Nodes[n];

			NodeWithIndicesMap::iterator nodeItor = organConnInfo.m_NodeOfOrganWithIndicesMap.find(pFaceNode);

			if(nodeItor != organConnInfo.m_NodeOfOrganWithIndicesMap.end())
			{
				std::vector<int> & indices =nodeItor->second;
				indices.push_back(currConsIndex);
			}
			else
			{
				std::vector<int> indices;
				indices.push_back(currConsIndex);
				organConnInfo.m_NodeOfOrganWithIndicesMap.insert(std::make_pair(pFaceNode , indices));
				pFaceNode->SetMass(pFaceNode->m_Mass * 3);
			}
		}	
		//adhesion一个node
		NodeWithIndicesMap::iterator nodeItor = m_NodeOfAdhesionWithIndicesMap.find(pNodeInAdhesion);
		if(nodeItor != m_NodeOfAdhesionWithIndicesMap.end())
		{
			std::vector<int> & indices = nodeItor->second;
			indices.push_back(currConsIndex);
		}
		else
		{
			std::vector<int> indices;
			indices.push_back(currConsIndex);
			m_NodeOfAdhesionWithIndicesMap.insert(std::make_pair(pNodeInAdhesion, indices));
			//pNodeInAdhesion->SetMass(pNodeInAdhesion->m_OriginMass * 3);
		}
	}
}


void MisMedicAdhesionCluster::ComputeConnectionRegion()
{
}

