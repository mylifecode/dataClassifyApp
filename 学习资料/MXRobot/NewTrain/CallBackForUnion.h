#ifndef _CALLBACKFORUNION_
#define _CALLBACKFORUNION_
#include "Ogre.h"
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "MisMedicObjectUnion.h"
class MisMedicOrgan_Ordinary;
class MisMedicObjetSerializer;
class CXMLWrapperAdhere;
class MisNewTraining;

using namespace GoPhys;

//=======================================================================================================================

struct NearestNode
{
	NearestNode(GFPhysSoftBodyNode * Node , float mindist) : m_Node(Node) , m_MinDist(mindist)
	{
	}
	float m_MinDist;
	GFPhysSoftBodyNode * m_Node;
};


class LinkedOrganNodeCallBack : public GFPhysNodeOverlapCallback
{
public:
	LinkedOrganNodeCallBack(float UnionThreasHold) : m_UnionThreasHold(UnionThreasHold)
	{
	}

	void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB);

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData) {}

	std::map<GFPhysSoftBodyNode * , NearestNode> m_NearestNodes;

	float m_UnionThreasHold;
};

//=======================================================================================================================

class NodeNearestCallBack : public GFPhysNodeOverlapCallback
{
public:
	NodeNearestCallBack(float UnionThreasHold) : m_UnionThreasHold(UnionThreasHold) {}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData) {}

	void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB);

	GFPhysVector3 * m_NodesPosA;

	GFPhysVector3 * m_NodesPosB;

	int * m_AToBRemap;

	int * m_BRemappedTag;

	//std::map<GFPhysSoftBodyNode * , NearestNode> m_NearestNodes;

	float m_UnionThreasHold;
};
//=======================================================================================================================

class S2DNearestCallBack : public GFPhysNodeOverlapCallback
{
public:
	S2DNearestCallBack(float UnionThreasHold) : m_UnionThreasHold(UnionThreasHold)
	{
		m_SharedVertex.reserve(2000);
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData)  {}

	void ProcessOverlappedNodes(GFPhysDBVNode * NodeForStatic , GFPhysDBVNode * NodeForDynamic);

	SubMeshVertex * m_StaticPos;

	std::vector<DynStaticSharedVertex> m_SharedVertex;

	float m_UnionThreasHold;
};

//=======================================================================================================================

struct NodeLieInTetra
{
	NodeLieInTetra(GFPhysSoftBodyNode * node, float dist) : m_Node(node), m_tetra(0) , m_Face(0) , m_MinDist(dist) {}

	GFPhysVector3 m_ClosetPoint;
	float m_MinDist;
	GFPhysSoftBodyNode * m_Node;
	GFPhysSoftBodyTetrahedron * m_tetra;
	GFPhysSoftBodyFace * m_Face;
};

//=======================================================================================================================

class UniverseLinkCallBack : public GFPhysNodeOverlapCallback
{
public:
	UniverseLinkCallBack(float UnionThreasHold, int ElementType = 0) : m_UnionThreasHold(UnionThreasHold), m_ElementType(ElementType){}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData) {}

	void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB);

	std::map<GFPhysSoftBodyNode * , NodeLieInTetra> m_StickNodes;

	float m_UnionThreasHold;

	int   m_ElementType;
};


//=======================================================================================================================
class UniverseEdgeFaceCallBack : public GFPhysNodeOverlapCallback
{
public:
	UniverseEdgeFaceCallBack(const GFPhysVector3 & RayStart , const GFPhysVector3 & RayEnd)
	{
		m_RayStart = RayStart;
		m_RayEnd = RayEnd;
		m_intersected = false;
	}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData);

	void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB) {}

	bool m_intersected;
	GFPhysVector3 m_RayStart;
	GFPhysVector3 m_RayEnd;
	GFPhysSoftBodyFace * m_face;
	float m_FaceWeights[3];
	float m_LineWeight[2];
};

//=======================================================================================================================

struct FaceClosetFace
{
	FaceClosetFace(GFPhysSoftBodyFace * face , float dist) : m_FaceA(face) , m_MinDist(dist)  {}

	//	GFPhysVector3 m_ClosetPoint;
	float m_MinDist;
	GFPhysSoftBodyFace * m_FaceA;
	GFPhysSoftBodyFace * m_FaceB;
};

struct PointOnFaceClosetPointOnFace
{
    PointOnFaceClosetPointOnFace(GFPhysSoftBodyFace * faceA, Real weightA[], GFPhysSoftBodyFace * faceB, Real weightB[], float dist) : m_FaceA(faceA), m_FaceB(faceB), m_MinDist(dist)  
    {
        for (int i = 0; i < 3;i++)
        {
            m_weightA[i] = weightA[i];
            m_weightB[i] = weightB[i];
        }
        
    }
    
    float m_MinDist;
    GFPhysSoftBodyFace * m_FaceA;
    Real m_weightA[3];
    GFPhysSoftBodyFace * m_FaceB;
    Real m_weightB[3];
};

//=======================================================================================================================

class FaceFaceCenterDistCallBack : public GFPhysNodeOverlapCallback
{
public:
	FaceFaceCenterDistCallBack(float UnionThreasHold) : m_UnionThreasHold(UnionThreasHold) {} 

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData) {}

	void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB);

	std::map<GFPhysSoftBodyFace * , FaceClosetFace> m_ClostPairs;

	float m_UnionThreasHold;
};



//=======================================================================================================================

class FaceEdgeIntersectCallBack : public GFPhysNodeOverlapCallback
{
public:
	FaceEdgeIntersectCallBack(SubMeshVertex * pMeshVertex , unsigned long * pIndices ,  float ThreasHold , bool IsCheckInSide = false) 
		: m_pMeshVertex(pMeshVertex) ,
		 m_pIndices(pIndices) ,
		m_ThreasHold(ThreasHold) ,
		m_IsCheckInside(IsCheckInSide) { } 

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData) {}

	void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB);

	std::set<GFPhysSoftBodyNode*> m_Nodes;

	SubMeshVertex * m_pMeshVertex;

	unsigned long * m_pIndices;

	float m_ThreasHold;

	bool m_IsCheckInside;
};
//===============================================================================
//the following use the position in the serizlizer 
//===============================================================================
//used in UniverseLinkCallBackV2 and 
struct ConnectedNode
{
	ConnectedNode(int nodeIndex , float dist) : m_NodeIndex(nodeIndex) , m_MinDist(dist) , m_FaceIndex(-1) , m_TetraIndex(-1) { }

	//GFPhysVector3 m_ClosetPoint;
	Ogre::Vector3 m_ClosetPoint;

	float m_MinDist;	
	int m_NodeIndex;

	//In other organ
	int m_FaceIndex;
	int m_TetraIndex;
};

class UniverseLinkCallBackV2 : public GFPhysNodeOverlapCallback
{
public:
	enum Type
	{
		ULCB_NODE_TETRA,
		ULCB_NODE_FACE,
	};

	UniverseLinkCallBackV2(MisMedicOrgan_Ordinary *pOrganOfNode ,  MisMedicOrgan_Ordinary *pOrganConnectTo, float UnionThreasHold , UniverseLinkCallBackV2::Type type)
		: m_pOrganOfNode(pOrganOfNode) , 
		  m_pOrganConnectTo(pOrganConnectTo),
		  m_UnionThreasHold(UnionThreasHold) ,
		  m_Type(type)	{}

	void ProcessOverlappedNode(int subPart, int triangleIndex,void * UserData) {}

	void ProcessOverlappedNodes(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB);

	std::map<int , ConnectedNode> m_StickNodes;

private:
	void ProcessNodeWithFace(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB);

	void ProcessNodeWithTetra(GFPhysDBVNode * dynNodeA , GFPhysDBVNode * dynNodeB);

	MisMedicOrgan_Ordinary *m_pOrganOfNode;

	MisMedicOrgan_Ordinary *m_pOrganConnectTo;

	float m_UnionThreasHold;

	Type m_Type;
};

#endif