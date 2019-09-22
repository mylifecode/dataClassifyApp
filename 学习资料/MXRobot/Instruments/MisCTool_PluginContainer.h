#ifndef _MISCTOOL_PLUGINCONTAINER_
#define _MISCTOOL_PLUGINCONTAINER_
#include "MXOgreWrapper.h"
#include "ITraining.h"
#include "ToolsMgr.h"
#include "Instruments/MisMedicCToolPluginInterface.h"
#include "IObjDefine.h"
#include "ScoreMgr.h"
#include "TipMgr.h"
#include "Painting.h"

class MisMedicOrgan_Ordinary;

//temp 对剪下部分操作的功能不全

class MisCTool_PluginContainer : public MisMedicCToolPluginInterface , public GFPhysSoftBodyConstraint
{
public:
// 	class InsideNodeInfo
// 	{
// 	public:
// 		InsideNodeInfo(GFPhysSoftBodyNode * pNode) : m_pNode(pNode) {}
// 		GFPhysSoftBodyNode *m_pNode;
// 		Ogre::Vector3 m_LocalCoord;
// 	};
	enum InsideState
	{
		CONTAINER_NOT_INSIDE,
		CONTAINER_INSIDE_FREE,
		CONTAINER_INSIDE_FIXED
	};

	class InsideFaceInfo
	{
	public:
		InsideFaceInfo(GFPhysSoftBodyFace * pFace , int organId) : m_pFace(pFace) , m_OrganId(organId) {}
		GFPhysSoftBodyFace *m_pFace;
		Ogre::Vector3 m_Coords[3];
		int m_OrganId;
	};

	class OrganInfo
	{
	public:		
		OrganInfo(MisMedicOrgan_Ordinary *pOrgan = NULL) : m_pOrgan(pOrgan) , 
																							m_CanFix(false),
																							m_InsideState(CONTAINER_INSIDE_FREE) , 
																							m_TopFace(NULL) , 
																							m_BottomFace(NULL) {}
		MisMedicOrgan_Ordinary *m_pOrgan;
		std::list<InsideFaceInfo> m_FacesInside;
		std::set<int> m_SubPart;
		bool m_CanFix;
		InsideState m_InsideState;

		GFPhysSoftBodyFace * m_TopFace;
		GFPhysSoftBodyFace * m_BottomFace;
		Ogre::Vector3 m_TopFaceCoords[3];
		Ogre::Vector3 m_BottomFaceCoords[3];
	};

	class ContainerRegion
	{
	public:
		void UpdateToWorldSpace();

		Ogre::Vector3 GetLocalCoord(GFPhysSoftBodyNode * pNode);

		GFPhysVector3 GetLocalCoord(const GFPhysVector3 & worldCoord);

		GFPhysVector3 GetWorldPos(const Ogre::Vector3 & localCoord);

		bool IsInside(const Ogre::Vector3 & node , float allowRange = 0.0f);

		GFPhysRigidBody * m_pAttachRigid;

		GFPhysVector3 m_TestMin;

		GFPhysVector3 m_TestMax;


		GFPhysVector3 m_LocalMin;

		GFPhysVector3 m_LocalMax;

		GFPhysVector3 m_OriginLocal;

		GFPhysVector3 m_XAxisLocal;

		GFPhysVector3 m_YAxisLocal;

		GFPhysVector3 m_ZAxisLocal;

		GFPhysVector3 m_OriginWorld;

		GFPhysVector3 m_XAxisWorld;

		GFPhysVector3 m_YAxisWorld;

		GFPhysVector3 m_ZAxisWorld;
	};
	MisCTool_PluginContainer(CTool * tool);

	~MisCTool_PluginContainer();

	void SetContainerRegion(GFPhysRigidBody * pAttachRigid,
		const GFPhysVector3 & localOrigin,
		const GFPhysVector3 & localX,
		const GFPhysVector3 & localY,
		const GFPhysVector3 & localZ,
		const GFPhysVector3 & containMin ,
		const GFPhysVector3 & containMax);

	/*@ overridden */
	virtual void OnOrganBeRemovedFromWorld(MisMedicOrganInterface * organif);

	virtual void RigidSoftCollisionsSolved(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);

	//when a physics update frame start
	virtual void PhysicsSimulationStart(int currStep , int TotalStep , float dt);

	//when a physics update frame end
	virtual void PhysicsSimulationEnd(int currStep , int TotalStep , float dt);

	//when a ordinary frame update start(one ordinary frame may include zero-many physics frame)
	virtual void OneFrameUpdateStarted(float timeelapsed);


	//@overridden GFPhysSoftBodyConstraint
	void SolveConstraint(Real Stiffniss,Real TimeStep);

	//@overridden GFPhysSoftBodyConstraint
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void OnSoftBodyFaceBeDeleted(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face);

	virtual void OnSoftBodyFaceBeAdded(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face);

	GFPhysVector3 GetPluginForceFeedBack();

	//=================================================================================
protected:
	ContainerRegion m_ContainerRegion;

	std::list<InsideFaceInfo> m_InsideFacesInfo;

//	std::map<MisMedicOrgan_Ordinary*,std::list<InsideFaceInfo>> m_OrganMap;

	std::map<MisMedicOrgan_Ordinary*,OrganInfo> m_OrganMap;
	
	/// 器官比例，范围0-100%
	int m_OrganRatio;

	
	PaintingTool m_painting;

};
#endif