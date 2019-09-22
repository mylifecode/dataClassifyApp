#ifndef _MISCTOOL_PLUGINSUCTION_
#define _MISCTOOL_PLUGINSUCTION_
#include "MXOgreWrapper.h"
#include "ITraining.h"
#include "ToolsMgr.h"
#include "Instruments/MisMedicCToolPluginInterface.h"
#include "IObjDefine.h"
#include "ScoreMgr.h"
#include "TipMgr.h"
//#include "Painting.h"

class MisMedicOrgan_Ordinary;

//边界无限长 
class Border2D
{
public:
	Border2D(const Ogre::Vector2 & begin , const Ogre::Vector2 & end) : m_Begin(begin) , m_End(end) , m_Dir(end - begin) {m_Dir.normalise();}
	Ogre::Vector2 m_Begin;
	Ogre::Vector2 m_End;
	Ogre::Vector2 m_Dir;
	bool IsInside(const Ogre::Vector2 & Point);
	bool FindIntersectionWithSeg(const Ogre::Vector2 & a , const Ogre::Vector2 & b , Ogre::Vector2 & result);
};

class Clipper2D
{
public:
	//按顺时针方向传入
	void SetBorders(const std::vector<Ogre::Vector2>  & ClipperPolygonVertices);
	void Sutherland_Hodgeman(const std::vector<Ogre::Vector2> & SrcVertices , std::vector<Ogre::Vector2> & DestVertices);
private:
	std::vector<Border2D> m_Borders;
};

class MisCTool_PluginSuction : public MisMedicCToolPluginInterface , public GFPhysSoftBodyConstraint
{
public:
	//todo
	/*
	加入力
	吸起的面被remove掉后
	*/
	typedef std::map<MisMedicOrgan_Ordinary* , std::list<GFPhysSoftBodyFace*>> FaceWithOrganMap;
	//clamp region in left or right part rigid body of tool
	class SuctionRegion
	{
	public:
		SuctionRegion() : m_pAttachRigid(NULL) , m_SuctionRadius(0.0f) {}

		void UpdateToWorldSpace();

		//return the area of face after project
		bool IsFaceInSuctionRange(GFPhysSoftBodyFace* pFace);

		float ProjectFace(GFPhysSoftBodyFace* pFace , std::vector<Ogre::Vector2> & results2D , float dist[3]);

		void ProjectNode(GFPhysSoftBodyNode* pNode , Ogre::Vector2 &  results2D);

		void ProjectNode(GFPhysSoftBodyNode* pNode , float component[3]);

		float GetAreaInSuctionRegion(const std::vector<Ogre::Vector2> & Triangle);
        //PaintingTool painting;

		
		GFPhysRigidBody * m_pAttachRigid;

		GFPhysVector3 m_CenterLocal;
		GFPhysVector3 m_Axis0Local;
		GFPhysVector3 m_Axis1Local;
		GFPhysVector3 m_SuctionInvDirLocal;

		GFPhysVector3 m_CenterWorld;
		GFPhysVector3 m_Axis0World;
		GFPhysVector3 m_Axis1World;
		GFPhysVector3 m_SuctionInvDirWorld;

		GFPhysVector3 m_LastCenterWorld;
		GFPhysVector3 m_LastSuctionInvDirWorld;

		float m_SuctionRadius;
		//用正方形近似吸取范围
		float m_SuctionArea;
		float m_SuctionDist;

		Clipper2D m_Clipper;
	};

	//soft body faces be sucked
	class SoftBodyFaceSucked
	{
	public:
		SoftBodyFaceSucked(GFPhysSoftBodyFace * face) : m_pFace(face) {}

		SoftBodyFaceSucked(GFPhysSoftBodyFace * face , const float cweighs[3]) : m_pFace(face)
		{
			m_pointWeights[0] = cweighs[0];
			m_pointWeights[1] = cweighs[1];
			m_pointWeights[2] = cweighs[2];
		}

		GFPhysSoftBodyFace * m_pFace;
		float m_pointWeights[3];
	};

	class SoftBodyNodeSucked
	{
	public:
		SoftBodyNodeSucked(GFPhysSoftBodyNode * pNode) : m_pNode(pNode) , 
			  m_FaceRefCount(1) {}

		GFPhysSoftBodyNode * m_pNode;
		int m_FaceRefCount;
		Ogre::Vector2 m_ProjectPos;
	};


	MisCTool_PluginSuction(CTool * tool);

	~MisCTool_PluginSuction();

	/*@ overridden */
	virtual void OnOrganBeRemovedFromWorld(MisMedicOrganInterface * organif);

	virtual void RigidSoftCollisionsSolved(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);

	//when a physics update frame start
	virtual void PhysicsSimulationStart(int currStep , int TotalStep , float dt);

	//when a physics update frame end
	virtual void PhysicsSimulationEnd(int currStep , int TotalStep , float dt);

	//when a ordinary frame update start(one ordinary frame may include zero-many physics frame)
	virtual void OneFrameUpdateStarted(float timeelapsed);

	//when a ordinary frame update end
	virtual void OneFrameUpdateEnded();
	
	//@overridden GFPhysSoftBodyConstraint
	void SolveConstraint(Real Stiffniss,Real TimeStep);

	//@overridden GFPhysSoftBodyConstraint
	void PrepareSolveConstraint(Real Stiffness,Real TimeStep);

	virtual void OnSoftBodyFaceBeDeleted(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face);

	virtual void OnSoftBodyFaceBeAdded(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face);
//=================================================================================
	void ClearSuckedFaces();

	void ClearSuckedNodes();

	void ReleaseSuckedOrgans();

	bool CheckWhetherOrganBeSucked();

	bool CheckSuckWater();

	bool isSomethingSucked();

	bool IsCanSucked() const { return m_CanSuck; }

	MisMedicOrgan_Ordinary * GetOrganBeSucked();

	void SetCanSuck(bool canSuck); 

	void SetSuctionRegion(GFPhysRigidBody * pAttachRigid,
		const GFPhysVector3 & regionCenter,
		const GFPhysVector3 & axis0,
		const GFPhysVector3 & axis1,
		const GFPhysVector3 & pointTo , 
		float suctionRadius,
		float suctionDist = 0.05);

	GFPhysVector3 GetPluginForceFeedBack();

	void ProcessSuckedNode();

	void UpdateMoveInfo(float dt);

	GFPhysVector3 GetSuctionInvDir() { return m_SuctionRegion.m_SuctionInvDirWorld; }

	GFPhysVector3 GetSuctionCenter() { return m_SuctionRegion.m_CenterWorld; }

protected:
	MisMedicOrgan_Ordinary *SelectWhichOrganToBeSucked(FaceWithOrganMap & facesWithOrgan);

	SuctionRegion m_SuctionRegion;

	typedef std::map<GFPhysSoftBodyNode* , SoftBodyNodeSucked> SuckedNodeMap;
	SuckedNodeMap m_SuckedNodes;

	std::set<GFPhysSoftBodyFace*> m_SuckedFaces;

	bool m_IsOrganSucked;

	bool m_CanSuck;

	MisMedicOrgan_Ordinary * m_pOrganBeSucked;

	//float m_SuctionAreaRatio;

	float m_MoveIncrementInSuctionInvDir;

	float m_SuctionMoveSpeed;
	
	//PaintingTool m_painting;

	GFPhysVector3 m_DragForce;
};
#endif