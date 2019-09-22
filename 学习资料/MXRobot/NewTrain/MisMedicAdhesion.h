#ifndef _MISMEIDCADHESION_
#define _MISMEIDCADHESION_
#include "Ogre.h"
#include "collision/GoPhysCollisionlib.h"
#include "Dynamic/GoPhysDynamicLib.h"
#include "Dynamic/Constraint/GoPhysSoftBodyDistConstraint.h"
#include "CallBackForUnion.h"
class MisMedicOrgan_Ordinary;
class MisMedicObjetSerializer;
class CXMLWrapperAdhesionCluster;
class MisNewTraining;
using namespace GoPhys;




//连接粘连(MisMedicOrgan_Ordinary)与其他MisMedicOrgan_Ordinary
class MisMedicAdhesionCluster
{
public:

	typedef std::map<GFPhysSoftBodyNode* , std::vector<int>> NodeWithIndicesMap;

	class OrganConnectionInfo
	{
	public:
		OrganConnectionInfo(MisMedicOrgan_Ordinary * pOrgan);

		NodeWithIndicesMap m_NodeOfOrganWithIndicesMap;

		MisMedicOrgan_Ordinary *m_pOrgan;

		//test version
		std::list<ConnectedNode> m_StickNodesWithTetra;

		std::list<ConnectedNode> m_StickNodesWithFace;

	};

	class ConnectionRegion
	{
	public:
		ConnectionRegion(MisMedicOrgan_Ordinary *pOrgan = NULL) :m_pConnectedOrgan(pOrgan){}
		MisMedicOrgan_Ordinary * m_pConnectedOrgan;
		std::vector<GFPhysSoftBodyFace*> m_FacesOfOrgan;//unused
		std::vector<GFPhysSoftBodyFace*> m_FacesOfAdhesion;//unused
		std::vector<int> m_AdhesionNodeIndices;
		Ogre::Vector3 m_Center;
	};

	MisMedicAdhesionCluster(MisMedicOrgan_Ordinary *pAdhesion , CXMLWrapperAdhesionCluster * pAdhersionClusterConfig);

	~MisMedicAdhesionCluster();

	void OnFaceRemoved(GFPhysSoftBodyFace * pFace , GFPhysSoftBodyShape * pHostshape);

	bool IsAdhesionScale() { return m_IsScale;}

	bool IsAdhesionAutoScale() { return m_IsAutoFindDir;}

	float GetAdhesionScaleFactor() { return m_AdhesionScaleFactor;}

	bool BuildAdhesionCluster(CXMLWrapperAdhesionCluster * pAdhesionClusterConfig, MisNewTraining * pMisNewTraining , std::set<int> & organsID);

	void AddOrgansToCluster(CXMLWrapperAdhesionCluster * pAdhesionClusterConfig, MisNewTraining * pMisNewTraining);

//test version
	bool FindAllConnection(CXMLWrapperAdhesionCluster * pAdhesionClusterConfig, MisNewTraining * pMisNewTraining , std::set<int> & organsID);

	void BuildAllConnection(float constraintStiffness = 0.99);

	int ComputeConnectionRegionV2();

	GFPhysVector3 GetScaleDir();

//test version

private:
	void BuildConnectionToOrgan(MisMedicOrgan_Ordinary * pOrgan , float threshold , float constraintStiffness = 0.99);
//test version
	void BuildNodeTetraConnectionWithOrgan(OrganConnectionInfo & organConnInfo , float constraintStiffness = 0.99);

	void BuildNodeFaceConnectionWithOrgan(OrganConnectionInfo & organConnInfo , float  constraintStiffness = 0.99);
//test version

	void ComputeConnectionRegion();

//test version
	void FindNodeTetraConnectionWithOrgan(MisMedicOrgan_Ordinary * pOrgan , float threshold);

	void FindNodeFaceConnectionWithOrgan(MisMedicOrgan_Ordinary * pOrgan , float threshold);

	void ComputeConnectionRegionOfOneOrgan(OrganConnectionInfo & organConnInfo);
//test version

	MisMedicOrgan_Ordinary *m_pAdhesion;
	
	std::set<MisMedicOrgan_Ordinary*> m_OrgansInCluster;
	
	std::vector<GFPhysPositionConstraint*> m_Constraints;
	
	std::vector<GFPhysPositionConstraint*> m_RemovedConstraints;

	std::vector<OrganConnectionInfo> m_OrganConnectionInfoVec;

	NodeWithIndicesMap m_NodeOfAdhesionWithIndicesMap;

	std::vector<ConnectionRegion> m_ConnectionRegions;

	//about scale

	bool m_IsScale;

	bool m_IsAutoFindDir;

	float m_AdhesionScaleFactor;
	
	std::vector<int> m_NodeIndicesOfAdhesionEndA;
	std::vector<int> m_NodeIndicesOfAdhesionEndB;

};

#endif