#ifndef _MISMEIDCOBJECTUNION_
#define _MISMEIDCOBJECTUNION_
#include "MisMedicObjLink.h"
class MisMedicOrgan_Ordinary;
class MisMedicObjetSerializer;
class CXMLWrapperAdhere;
class MisNewTraining;



using namespace GoPhys;


class SubMeshVertex
{
public:
	SubMeshVertex(int submehs , int VertIndex , const Ogre::Vector3 & pos , bool sharedMesh)
	{
		m_SubMesh = submehs;
		m_VertexOffset = VertIndex;
		m_Position= GFPhysVector3(pos.x,pos.y,pos.z);
		m_InSharedMesh = sharedMesh;
	}
	SubMeshVertex()
	{

	}
	int m_SubMesh;
	int m_VertexOffset;
	GFPhysVector3 m_Position;
	bool m_InSharedMesh;
};
//vertex where one node from dynamic mesh shared with some static mesh point
class DynStaticSharedVertex
{
public:
	GFPhysSoftBodyNode * m_DynamicNode;
	GFPhysAlignedVectorObj<SubMeshVertex> m_NodesInStatic;
};

void GetOgreMeshVertexIndex(  const Ogre::MeshPtr mesh , 
							size_t & vertex_count,
							SubMeshVertex * & vertices,
							size_t & index_count,
							unsigned long* &indices,
							const Ogre::Vector3 & position,
							const Ogre::Quaternion & orient,
							const Ogre::Vector3 & scale ,
							bool IsGetIndices = false);

/*class SubMeshToDynMap_Struct
{
public:
	int m_outter;
	int m_inner;
};*/
class MisMedicObjectUnion
{
public:
	void CreateMergedObject(MisMedicObjetSerializer & DstSerializer , 
							 int ObjAID,
							 int ObjBID,
							 Ogre::String fileA , 
							 Ogre::String fileB ,
							 Ogre::Vector3 offset ,
							 float NodeThresHold ,
							 const std::vector<int> & fixindexA,
							 const std::vector<int> & fixindexB,
							 std::vector<int> & dstfixindex);
	
	void MergeObjectToExist(int LayerIndex, 
		                    MisMedicObjetSerializer & DstSerializer,
		                    int ObjAID, Ogre::String fileA,
		                    Ogre::Vector3 offset, float NodeThresHold,
		                    const std::vector<int> & fixindexA,
		                    std::vector<int> & dstfixindex);

	void AttachStaticMeshToDynamicOrgan( Ogre::MeshPtr meshptr, 
		                                 Ogre::Node * meshNode,
									     Ogre::Vector3 position,
									     Ogre::Quaternion orient,
									     Ogre::Vector3 scale,
									     MisMedicOrgan_Ordinary * organObj,
									     float NodeThresHold);

	void UpdateStaticVertexByDynamic(Ogre::MeshPtr meshptr);

	std::vector<DynStaticSharedVertex> m_StaticDynamicSharedVertex;
	std::vector<std::vector<unsigned int>> m_VertexToDynMap;
};



class MisMedicObjectAdhersion  : public MisMedicObjLink  , GFPhysSoftBodyConstraint
{
public:
	enum Adh_Type
	{
		ADH_APPROCH ,
		ADH_INSERT,
		ADH_SPACEKEEP,
	};
	MisMedicObjectAdhersion();

	virtual ~MisMedicObjectAdhersion();

	virtual void PrepareSolveConstraint(Real Stiffness,Real TimeStep);
	
	virtual void SolveConstraint(Real Stiffness,Real TimeStep);

	bool BuildObjectAdhesion(CXMLWrapperAdhere * adhereconfig , MisNewTraining * mistrain);

	//void BuildAdhersionBetWeenOrgan( MisMedicOrgan_Ordinary & organA , 
									 //MisMedicOrgan_Ordinary & organB ,
									 //float NodeThresHold , 
									 //float ratio);

	void AttachNodesCenterToBody(const std::vector<GFPhysSoftBodyNode*> & nodes , GFPhysSoftBody * attacbody);

	void BuildUniversalLinkFromAToB(const MisMedicOrgan_Ordinary & organA , const MisMedicOrgan_Ordinary & organB , float stiffness ,float threshold = 0.01f);

	void BuildNodeSurfaceLinkFromAToB(const MisMedicOrgan_Ordinary & organA, const MisMedicOrgan_Ordinary & organB, float stiffness, float threshold = 0.01f);

	void BuildSpaceKeepLink(const MisMedicOrgan_Ordinary & organA , const MisMedicOrgan_Ordinary & organB , float stiffness , float threshold = 0.1f);

	void OnFaceRemoved(GFPhysSoftBodyFace * face);

	void OnNodesRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes);

	void OnTetrasRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> & tetras);
	
	std::vector<GFPhysPositionConstraint*> m_constraints;

	//@ Node - Node pair link constraint
	//std::vector<MM_NodeToNodeLinkPair>  m_NodeToNodeLinks;
	//std::map<GFPhysSoftBodyNode* , int> m_NodesLinkMap;
	
	//@ Node - Tetra pair link
	std::vector<TetrahedronAttachConstraint> m_NodeToTetraLinks;
	//

	std::vector<FaceEdgePointAttachConstraint> m_FaceToEdgeLinks;

	Adh_Type m_AdhType;

	//MisMedicOrgan_Ordinary * m_ConnectOrganA;
	//MisMedicOrgan_Ordinary * m_ConnectOrganB;

	bool m_HideLinkFace;

};

class NodeDistConstraintWrapper
{
public:
	NodeDistConstraintWrapper(GFPhysSoftBodyNode * nodeA , GFPhysSoftBodyNode * nodeB , float stiffness) : m_IsValid(true)
	{
		m_Nodes[0] = nodeA;
		m_Nodes[1] = nodeB;
		m_RestLen = (nodeA->m_CurrPosition - nodeB->m_CurrPosition).Length();
		m_Stiffness = stiffness;
		m_Lambda = 0.0f;
	}
	
	void SetValid(bool valid) 
	{ 
		m_IsValid = valid; 
	}
	bool IsValid() 
	{ 
		return m_IsValid;
	}
	
	GFPhysSoftBodyNode * m_Nodes[2];

	float m_Lambda;
	float m_Stiffness;
	float m_RestLen;

	bool  m_IsValid;
};


//包裹
class MisMedicObjectEnvelop : public GFPhysSoftBodyConstraint
{
	/*todo
		1) 去除面后有subpart去除其连接
		2)去除约束时去掉node的GPSESF_CONNECTED状态
	*/
public:
	class FaceInfo;
	class NodeInfo
	{
	public:
		NodeInfo() : m_pOrgan(NULL),m_IsProcessed(false) , m_IsFacesAllRemoved(false) {}
		int FindFaceIndex(std::vector<FaceInfo>& faces , GFPhysSoftBodyFace * pFace);
		void OnFaceRemoved(int faceIndex);
		void RestoreStateOfFaces(std::vector<FaceInfo>& faces , bool isUnhide = true);
		
		MisMedicOrgan_Ordinary * m_pOrgan;
		std::vector<int> m_ConstraintIndices;
		std::vector<int> m_OwnerFacesIndices; 
		bool m_IsProcessed;
		bool m_IsFacesAllRemoved;
	};

	class FaceInfo
	{
	public:
		FaceInfo(GFPhysSoftBodyFace * pFace) : m_pFace(pFace) , m_IsValid(true) {}
		void SetValid(bool isValid) { m_IsValid = isValid;}
		bool m_IsValid;
		GFPhysSoftBodyFace *m_pFace;
	};

	MisMedicObjectEnvelop();

	~MisMedicObjectEnvelop();

	virtual void PrepareSolveConstraint(Real Stiffness, Real TimeStep);

	virtual void SolveConstraint(Real Stiffness, Real TimeStep);

	void BuildEnvelop( MisMedicOrgan_Ordinary & organA , 
		MisMedicOrgan_Ordinary & organB , 
		float nodeThresHold , 
		float ratio , 
		bool isHideAFace = true, bool isHideBFace = true ,
		int organABleedCount = 0 , int organBBleedCount = 0);

	float CheckTearConnect(float forceTear);

	void RemoveConstraints();

	void OnFaceRemoved(GFPhysSoftBodyFace * pFace , GFPhysSoftBodyShape * pHostshape);

	void OnNodeRemoved(GFPhysSoftBodyNode * pNode, GFPhysSoftBodyShape * pHostshape);

	int GetNumOfConstraint() { return m_NodeDistConstraints.size(); }

	int GetNumOfConstraintRemoved() { return m_NumOfConsRemoved; }

	int GetNumOfConsRemovedWithFace() { return m_NumOfConsRemovedWithFace; }

	std::vector<NodeDistConstraintWrapper> m_NodeDistConstraints;

private:
	void BuildNodeDistConstraint(float nodeThreshold , float ratio);

	void BuildNodesInfo();

	void ProcessConnectedFace(bool isHideAFace = true, bool isHideBFace = true);

	//unused
	void ConnectNodeWithFace();

	int RemoveConstraintWithNode(GFPhysSoftBodyNode *pNode);

	int RemoveConstraintWithNode(NodeInfo & nodeInfo);

	int FindFaceIndex(GFPhysSoftBodyFace * pFace);

	//unused
	std::vector<std::pair<GFPhysSoftBodyNode* , GFPhysSoftBodyNode*>> m_NodesBeConnect;

	typedef std::map<GFPhysSoftBodyNode*,GFPhysSoftBodyNode*> NearestNodeMap;
	NearestNodeMap m_NearestNodeMap;

	typedef std::map<GFPhysSoftBodyNode*, NodeInfo> NodeInfoMap;
	NodeInfoMap m_NodesInfoMap;

	typedef std::map<GFPhysSoftBodyFace*, GFPhysSoftBodyFace*> FaceMap;
	FaceMap m_ConnectFaceMap;

	MisMedicOrgan_Ordinary * m_OrganA;
	MisMedicOrgan_Ordinary * m_OrganB;
	
	std::vector<FaceInfo> m_FacesInfoVec;

	bool m_IsValid;
	
	int m_NumOfConsRemovedWithFace;

	int m_NumOfConsRemoved;

	int m_OrganABleedCount;

	int m_OrganBBleedCount;

};
#endif