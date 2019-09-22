#include "MisMedicObjLink_Approach.h"
#include "CallBackForUnion.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include "Math/GoPhysTransformUtil.h"

using namespace GoPhys;
//=============================================================================================================
//void ShadowNodeForLinkage::MergeWith(ShadowNodeForLinkage & rths)
//{
  //  std::vector<int>::iterator itor = rths.m_PhysNodeDataIds.begin();
   // while (itor != rths.m_PhysNodeDataIds.end())
   // {
     //   m_PhysNodeDataIds.push_back(*itor);
     //   itor++;
    //}
//}
//=============================================================================================================
void ShadowNodeForLinkage::OnNodesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes, MisMedicOrgan_Ordinary * organ)
{
	std::set<NodeBeLinked>::iterator itor = m_LinkedNodes.begin();
	
	while (itor != m_LinkedNodes.end())
	{
		bool bFind = false;
		
		if (itor->m_Organ == organ)
		{
			for (size_t c = 0; c < nodes.size(); c++)
			{
				if (itor->m_PhysNodeDataId == (int)nodes[c]->m_UserPointer)
				{
					bFind = true;
				}
			}
			//if (bFind)
			//{
				//GFPhysAlignedVectorObj<PhysNode_Data> & NodeData = organ->m_PhysNodeData;
				//NodeData[itor->m_PhysNodeDataId].m_ShadowNodeIndex = -1;
			//}
		}

		if (bFind)
			itor = m_LinkedNodes.erase(itor);
		else
		    itor++;
	}
}
//=============================================================================================================
MisMedicObjLink_Approach::MisMedicObjLink_Approach(bool hidelinkFace)
{
	m_HideLinkFace = hidelinkFace;
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->AddPositionConstraint(this , 1);
}
//=============================================================================================================
MisMedicObjLink_Approach::~MisMedicObjLink_Approach()
{
	if(PhysicsWrapper::GetSingleTon().m_dynamicsWorld)
	   PhysicsWrapper::GetSingleTon().m_dynamicsWorld->RemovePositionConstraint(this);

	std::map<GFPhysSoftBodyNode* , int>::iterator itor = m_OrganALinkMap.begin();
	while(itor != m_OrganALinkMap.end())
	{
		//delete itor->second;
		itor++;
	}

	itor = m_OrganBLinkMap.begin();
	while(itor != m_OrganBLinkMap.end())
	{
		//delete itor->second;
		itor++;
	}
	m_OrganALinkMap.clear();
	m_OrganBLinkMap.clear();
}
//=============================================================================================================
void MisMedicObjLink_Approach::LinkTwoOrgans( MisMedicOrgan_Ordinary & organA , 
											  MisMedicOrgan_Ordinary & organB , 
											  float NodeThresHold , 
											  float ratio)
{
	float LinksStiffness = GFPhysGlobalConfig::GetGlobalConfig().TransferUserStiffnessToSolverStiffness(0.96f);

	m_ConnectOrganA = &organA;
	m_ConnectOrganB = &organB;

	GFPhysSoftBody * bodyA = organA.m_physbody;
	GFPhysSoftBody * bodyB = organB.m_physbody;

	GFPhysVector3 Extend(NodeThresHold , NodeThresHold , NodeThresHold);
	GFPhysDBVTree NodeTreeA;
	GFPhysDBVTree NodeTreeB;
	
	//insert undeformed aabb to treeA
	GFPhysSoftBodyNode * NodeA = bodyA->GetNodeList();
	while(NodeA)
	{
		GFPhysVector3 NodePos = NodeA->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeA.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = NodeA;
		NodeA = NodeA->m_Next;
	}

	GFPhysSoftBodyNode * NodeB = bodyB->GetNodeList();
	while(NodeB)
	{
		GFPhysVector3 NodePos = NodeB->m_UnDeformedPos;
		GFPhysDBVNode * dbvn = NodeTreeB.InsertAABBNode(NodePos-Extend , NodePos+Extend);
		dbvn->m_UserData = NodeB;
		NodeB = NodeB->m_Next;
	}


	//test coincide node with node threshold
	LinkedOrganNodeCallBack nodeCallBack(NodeThresHold);
	NodeTreeA.CollideWithDBVTree(NodeTreeB , &nodeCallBack);

	//node pair coincide in 2 organ
	const std::map<GFPhysSoftBodyNode *, NearestNode> & AdherNodeMap = nodeCallBack.m_NearestNodes;
	std::map<GFPhysSoftBodyNode *, NearestNode>::const_iterator itor = AdherNodeMap.begin();

	float customweights[2] = { 0.5f , 0.5f};

	float weight1 = ratio;
	float weight2 = 1;

	customweights[0] = weight1;
	customweights[1] = weight2;


	while(itor != AdherNodeMap.end())
	{
		GFPhysSoftBodyNode * NodeA = itor->first;
		GFPhysSoftBodyNode * NodeB = itor->second.m_Node;

		//NodeA->m_CurrPosition  = NodeB->m_CurrPosition;
		//NodeA->m_UnDeformedPos = NodeB->m_UnDeformedPos;

		NodeA->m_StateFlag |= GPSESF_CONNECTED;
		NodeB->m_StateFlag |= GPSESF_CONNECTED;


		MM_NodeToNodeLinkPair nn_linkpair(NodeA , NodeB);
		if(ratio > FLT_EPSILON)
		{
			nn_linkpair.SetCustomWeights(customweights[0] , customweights[1]);
		}
        nn_linkpair.m_LinksStiffness = LinksStiffness;

		//for quick find when node removed
		//only 2 nodes not been added yet
		if (m_OrganALinkMap.find(NodeA) == m_OrganALinkMap.end() && m_OrganBLinkMap.find(NodeB) == m_OrganBLinkMap.end())
		{
			m_NodeToNodeLinks.push_back(nn_linkpair);

			m_OrganALinkMap.insert(std::make_pair(NodeA, m_NodeToNodeLinks.size() - 1));
			m_OrganBLinkMap.insert(std::make_pair(NodeB, m_NodeToNodeLinks.size() - 1));
		}
		itor++;
	}

	for(size_t f = 0 ; f < organA.m_OriginFaces.size() ; f++)
	{
		MMO_Face & OriginFaceA = organA.m_OriginFaces[f];
		if(OriginFaceA.m_NeedRend == true)//this face not processed yet
		{
			GFPhysSoftBodyFace * physfaceA = organA.m_OriginFaces[f].m_physface;
			GFPhysSoftBodyNode * NodeA[3];
			GFPhysSoftBodyNode * NodeB[3];

			NodeA[0] = physfaceA->m_Nodes[0];
			NodeA[1] = physfaceA->m_Nodes[1];
			NodeA[2] = physfaceA->m_Nodes[2];

			NodeB[0] = NodeB[1] = NodeB[2] = 0;

			bool connect0 = (physfaceA->m_Nodes[0]->m_StateFlag & GPSESF_CONNECTED);
			bool connect1 = (physfaceA->m_Nodes[1]->m_StateFlag & GPSESF_CONNECTED);
			bool connect2 = (physfaceA->m_Nodes[2]->m_StateFlag & GPSESF_CONNECTED);
			if(connect0 && connect1 && connect2)
			{
				for(int n = 0 ; n < 3 ; n++)
				{
					std::map<GFPhysSoftBodyNode *, NearestNode>::const_iterator itor = AdherNodeMap.find(NodeA[n]);
					if(itor != AdherNodeMap.end())
					{
						NodeB[n] = itor->second.m_Node;
					}
				}
				if(NodeB[0] && NodeB[1] && NodeB[2])
				{	
					GFPhysSoftBodyFace * physfaceB = bodyB->GetSoftBodyShape().GetSurFace(NodeB[0] , NodeB[1] , NodeB[2]);
					if(physfaceB)
					{
						if(m_HideLinkFace)
						   OriginFaceA.m_NeedRend = false;
						OriginFaceA.m_physface->DisableCollideWithSoft();
                        

						int bindex = organB.GetOriginFaceIndexFromUsrData(physfaceB);

						MMO_Face & OriginFaceB = organB.GetMMOFace_OriginPart(bindex);

						if(!OriginFaceB.m_HasError)
						{
							if(m_HideLinkFace)
								OriginFaceB.m_NeedRend = false;
							OriginFaceB.m_physface->DisableCollideWithSoft();
						}

						//
						m_CoincideFaceMapA.insert(std::make_pair(physfaceA , physfaceB));
						m_CoincideFaceMapB.insert(std::make_pair(physfaceB , physfaceA));

						//add incident to data 
						/*for(int n = 0 ; n < 3 ; n++)
						{
							std::map<GFPhysSoftBodyNode* , NodeLinkData*>::iterator sitor = m_OrganALinkMap.find(NodeA[n]);
							if(sitor != m_OrganALinkMap.end())
							{
                               sitor->second->m_IncidentFaces.insert(physfaceA);
							}

							sitor = m_OrganBLinkMap.find(NodeB[n]);
							if(sitor != m_OrganBLinkMap.end())
							{
							   sitor->second->m_IncidentFaces.insert(physfaceB);
							}
						}*/
					}
				}
			}
		}
	}
	/*if (organA.m_Serializer_NodeNum)
	{
		int i = 0;
		GFPhysSoftBodyNode * NodeAend = bodyA->GetNodeList();
		while(NodeAend )
		{
			NodeAend->m_UnDeformedPos = UnDeformedA[i];
			NodeAend = NodeAend->m_Next;
			i++;
		}       
	}*/
	//delete []UnDeformedA;
}
//======================================================================================================================
void MisMedicObjLink_Approach::OnNodesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes)
{
	for(size_t c = 0 ; c < nodes.size() ; c++)
	{
		GFPhysSoftBodyNode * node = nodes[c];
		
		std::map<GFPhysSoftBodyNode* , int>::iterator itor = m_OrganALinkMap.find(node);
		
		if(itor != m_OrganALinkMap.end())
		{
		   int t = itor->second;
           
		   //int t = ldata->m_LinkIndex;
		   
		   //invalid link pair
		   m_NodeToNodeLinks[t].m_IsValid = false;
		   m_NodeToNodeLinks[t].m_NodeInA = m_NodeToNodeLinks[t].m_NodeInB = 0;
		   
		   //finally erase from map
		   m_OrganALinkMap.erase(itor);
		}
		else
		{
			itor = m_OrganBLinkMap.find(node);

			if(itor != m_OrganBLinkMap.end())
			{
			   int t = itor->second;
			   //int t = ldata->m_LinkIndex;

			   //invalid link pair
			   m_NodeToNodeLinks[t].m_IsValid = false;
			   m_NodeToNodeLinks[t].m_NodeInA = m_NodeToNodeLinks[t].m_NodeInB = 0;

			   //finally erase from map
			   //delete ldata;
			   m_OrganBLinkMap.erase(itor);
			}
		}
	}
}
//====================================================================================================================
void MisMedicObjLink_Approach::OnFaceRemoved(GFPhysSoftBodyFace * face)
{
	 /*for(int n = 0 ; n < 3 ; n++)
	 {
		 GFPhysSoftBodyNode * node = face->m_Nodes[n];

		 NodeLinkData * linkData = 0;

		 std::map<GFPhysSoftBodyNode* , NodeLinkData*>::iterator itor = m_OrganALinkMap.find(node);
		 if(itor != m_OrganALinkMap.end())
		 {
			linkData = itor->second;
		 }
		 else
		 {
			 itor = m_OrganBLinkMap.find(node);
			 if(itor != m_OrganBLinkMap.end())
			 {
				linkData = itor->second;
			 }
		 }

		 if(linkData != 0)
		 {
			 std::set<GFPhysSoftBodyFace*>::iterator sit = linkData->m_IncidentFaces.find(face);
			
			 if(sit != linkData->m_IncidentFaces.end())
			 {
                linkData->m_IncidentFaces.erase(sit);
			 }
		 }
	 }*/

	//first find in organA
	GFPhysSoftBodyFace * coincideFace = 0;
	
	MisMedicOrgan_Ordinary * coincideOrgan = 0;

	std::map<GFPhysSoftBodyFace* , GFPhysSoftBodyFace*>::iterator faceItor;
	
	faceItor = m_CoincideFaceMapA.find(face);
	if(faceItor != m_CoincideFaceMapA.end())
	{
	   coincideFace = faceItor->second;
       coincideOrgan = m_ConnectOrganB;
	   m_CoincideFaceMapA.erase(faceItor);
	   m_CoincideFaceMapB.erase(coincideFace);
	}
	else
	{
		faceItor = m_CoincideFaceMapB.find(face);
		if(faceItor != m_CoincideFaceMapB.end())
		{
		   coincideFace = faceItor->second;
		   coincideOrgan = m_ConnectOrganA;
		   m_CoincideFaceMapB.erase(faceItor);
		   m_CoincideFaceMapA.erase(coincideFace);
		}
	}
	
	if(coincideOrgan)//the coincide face is open so show it
	{
		int faceID = coincideOrgan->GetOriginFaceIndexFromUsrData(coincideFace);

		MMO_Face & OriginFace = coincideOrgan->GetMMOFace_OriginPart(faceID);

		if(!OriginFace.m_HasError)
		   OriginFace.m_NeedRend = true;//

		coincideOrgan->GetRender()->DirtyIndexBuffer();
	}
}

//=============================================================================================================
void MisMedicObjLink_Approach::PrepareSolveConstraint(Real Stiffness,Real TimeStep)
{

}
//=============================================================================================================
void MisMedicObjLink_Approach::SolveConstraint(Real Stiffness,Real TimeStep)
{
	//solve linked node pairs
	for(size_t c = 0 ; c < m_NodeToNodeLinks.size() ; c++)
	{
		MM_NodeToNodeLinkPair & nnlink = m_NodeToNodeLinks[c];

		if(nnlink.m_IsValid == false)
			continue;

		Real w1 = (nnlink.m_UseCustomWeight ? nnlink.m_CustomedWeight[0] : nnlink.m_NodeInA->m_InvM);

		Real w2 = (nnlink.m_UseCustomWeight ? nnlink.m_CustomedWeight[1] : nnlink.m_NodeInB->m_InvM);

		

		bool collideA = nnlink.m_NodeInA->m_StateFlag & (GPSESF_ATTACHED);//GPSESF_COLLIDRIGID | 
		bool collideB = nnlink.m_NodeInB->m_StateFlag & (GPSESF_ATTACHED);

		//if (!(collideA && collideB))
		//{
			//if (collideA)
			//	w1 = 0;
			if (collideB)
				w2 = 0;
	//	}

		Real w = w1 + w2;

		if(w <= GP_EPSILON)
		   continue;

		GFPhysVector3 Diff = nnlink.m_NodeInA->m_CurrPosition - nnlink.m_NodeInB->m_CurrPosition;

		GFPhysVector3 Deta1 = -(w1 / (w1+w2))  * Diff;
		GFPhysVector3 Deta2 =  (w2 / (w1+w2))  * Diff;
		
		nnlink.m_NodeInA->m_CurrPosition += Deta1;//(Deta1 * nnlink.m_LinksStiffness);
		nnlink.m_NodeInB->m_CurrPosition += Deta2;//(Deta2 * nnlink.m_LinksStiffness);
	}
}