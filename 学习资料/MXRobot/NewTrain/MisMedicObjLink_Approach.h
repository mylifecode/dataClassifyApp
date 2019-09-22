#ifndef _MISMEDICOBJLINKAPPROACH_
#define _MISMEDICOBJLINKAPPROACH_

#include "MisMedicObjLink.h"

//shadow node for multi nodes' link to one
class ShadowNodeForLinkage
{
public:
	class NodeBeLinked
	{
	public:
		NodeBeLinked(MisMedicOrgan_Ordinary * organ , int NodeDataID)
		{
			m_Organ = organ;
			m_PhysNodeDataId = NodeDataID;
		}
		bool operator < (const NodeBeLinked & rths) const
		{
			if (rths.m_Organ != m_Organ)
				return m_Organ < rths.m_Organ;
			else
				return m_PhysNodeDataId < rths.m_PhysNodeDataId;
		}
		bool operator == (const NodeBeLinked & rths)
		{
			return (rths.m_Organ == m_Organ) && (rths.m_PhysNodeDataId == m_PhysNodeDataId);
		}

		int m_PhysNodeDataId;
		MisMedicOrgan_Ordinary * m_Organ;
	};
    Ogre::Vector3 m_AvgPosition;
    Ogre::Vector3 m_AvgNormal;
	std::set<NodeBeLinked> m_LinkedNodes;
    int m_NumNodes;
   // std::vector<int> m_PhysNodeDataIds;
   // std::vector<MisMedicOrgan_Ordinary*> m_PhysNodeBelongOrgans;

    //void MergeWith(ShadowNodeForLinkage & rths);
    void OnNodesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes, MisMedicOrgan_Ordinary * organ);

};

class MisMedicObjLink_Approach  : public MisMedicObjLink  , GFPhysSoftBodyConstraint
{
public:
	//class NodeLinkData
	//{
	//public:
		//NodeLinkData(int linkindex) : m_LinkIndex(linkindex)
		//{}
		//std::set<GFPhysSoftBodyFace*> m_IncidentFaces;
		//int m_LinkIndex;
	//};
	MisMedicObjLink_Approach(bool hidelinkFace);

	virtual ~MisMedicObjLink_Approach();

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);
	
	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	//@ link 2 organs by coincide vertex "LinkNodeThresHold" is the range of coincide
	void LinkTwoOrgans( MisMedicOrgan_Ordinary & organA , MisMedicOrgan_Ordinary & organB , float LinkNodeThresHold , float ratio);

	//@ overridden
	void OnFaceRemoved(GFPhysSoftBodyFace * face);

	void OnNodesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes);
	
	//@ Node - Node pair link constraint
	std::vector<MM_NodeToNodeLinkPair>  m_NodeToNodeLinks;

	//@ for fast delete 
	std::map<GFPhysSoftBodyNode* , int> m_OrganALinkMap;

	std::map<GFPhysSoftBodyNode* , int> m_OrganBLinkMap;

	std::map<GFPhysSoftBodyFace* , GFPhysSoftBodyFace*> m_CoincideFaceMapA;
	std::map<GFPhysSoftBodyFace* , GFPhysSoftBodyFace*> m_CoincideFaceMapB;

	bool m_HideLinkFace;

};
#endif