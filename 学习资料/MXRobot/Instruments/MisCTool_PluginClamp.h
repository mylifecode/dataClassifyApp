#ifndef _MISCTOOL_PLUGINCLAMP_
#define _MISCTOOL_PLUGINCLAMP_
#include "MXOgreWrapper.h"
#include "ITraining.h"
#include "ToolsMgr.h"
#include "OgreMaxScene.hpp"
#include "MXOgreWrapper.h"
#include "IObjDefine.h"
#include "Instruments/MisMedicCToolPluginInterface.h"
#include "ScoreMgr.h"
#include "TipMgr.h"
#include "Painting.h"
#include "SutureThread.h"
///#include "SutureThreadV2.h"

class MisMedicOrgan_Ordinary;
class MisMedicThreadRope;
class NewTrainToolConvexData;
class MisCTool_TubeClamp;//sub processor
class SutureThreadV2;
class MisCTool_PluginClamp : public MisMedicCToolPluginInterface , public GFPhysSoftBodyConstraint
{
public:
	enum ClampRegSide
	{
		ClampReg_Right = 0,
		ClampReg_Left  = 1,
		ClampReg_UnKnow = 8,
	};

	enum ClampStateStage
	{
		CSS_Released = 0,
		CSS_Clamped = 1,
		CSS_InRelease = 2,
	};
	//clamp region in left or right part rigid body of tool
	class ToolClampRegion
	{
	public:
		ToolClampRegion()
		{
			m_axis0Min = m_axis1Min= FLT_MAX;
			m_axis0Max = m_axis1Max = -FLT_MAX;
			m_normalSign = 1.0f;
			m_IsRect = true;
			m_RegSide = ClampReg_UnKnow;
		}

		void UpdateWorldData();

		GFPhysVector3 GetLocalClampNormal();

		GFPhysVector3 GetLocalClampCoordOrigin();
        void CalcRegionWorldAABB(float margin , GFPhysVector3 & aabbmin , GFPhysVector3 & aabbmax);

		GFPhysRigidBody * m_AttachRigid;

		GFPhysVector3 m_LocalOrigin;
		GFPhysVector3 m_Axis0Local;//Coord0Local;
		GFPhysVector3 m_Axis1Local;//Coord1Local;
		float m_HalfExt0;	//delete
		float m_HalfExt1;	//delete

		float m_axis0Min, m_axis0Max;
		float m_axis1Min, m_axis1Max;

		//world coordinate data
		GFPhysVector3 m_OriginWorldPrev;
		GFPhysVector3 m_OriginWorld;
		GFPhysVector3 m_Axis0World;
		GFPhysVector3 m_Axis1World;
		GFPhysVector3 m_ClampNormalWorld;

		//
		Real m_normalSign;

		bool m_IsRect;
		
		ClampRegSide m_RegSide;

		//GFPhysAlignedVectorObj<GFPhysVector3> m_WorldTriVerts;
		std::vector<Ogre::Vector2> m_triVertices;
	};

	//soft body faces contact "clamped region"
	class SoftBodyFaceClamped
	{
	public:
		SoftBodyFaceClamped(){}

		SoftBodyFaceClamped( MisMedicOrgan_Ordinary * organ ,
			                 GFPhysSoftBodyFace * face , 
			                 int Part , 
							 const float cweighs[3],
							 float cDepth) : m_Organ(organ) , m_PhysFace(face) , m_Part(Part)
		{
			m_ContactPtWeights[0] = cweighs[0];
			m_ContactPtWeights[1] = cweighs[1];
			m_ContactPtWeights[2] = cweighs[2];
			m_ContactDepth = cDepth;
			m_NumUnusedNode = 0;
			m_AngleToClampNorm = 0;
			m_DragForce = m_FrictForce = Ogre::Vector3(0,0,0);
			for (int n = 0; n < 3; n++)
			{
				if (m_PhysFace->m_Nodes[n]->m_StateFlag & GPSESF_CONNECTED)
					m_NumUnusedNode++;
			}
		}

		bool operator < (const SoftBodyFaceClamped & rthe) const     
		{  
			if (m_NumUnusedNode != rthe.m_NumUnusedNode)
				return m_NumUnusedNode < rthe.m_NumUnusedNode;
			else
				return m_AngleToClampNorm > rthe.m_AngleToClampNorm;
		}

        //bool operator == (const SoftBodyFaceClamped & rthe) const
        //{
			//return (m_PhysFace == rthe.m_PhysFace) && (m_NumUnusedNode == rthe.m_NumUnusedNode);
       // }

		MisMedicOrgan_Ordinary * m_Organ;
		GFPhysSoftBodyFace * m_PhysFace;
		int m_NumUnusedNode;
		float m_AngleToClampNorm;
		float m_ContactDepth;//intersect depth in clamp region
		float m_ContactPtWeights[3];//transform contact point in region to face barycentric coordinate
		int   m_Part;
		//float m_Coord0;
		//float m_Coord1;
		float m_NodeNormDist[3][2];
		Ogre::Vector2  m_NodeTanDist[3];
		float m_TanStiff[3];

		float m_NormalStiff[3];
		Ogre::Vector3  m_CollideNorm;//only for mode1
		Ogre::Vector3  m_DragForce;
		Ogre::Vector3  m_FrictForce;
	};
	
	struct VeinConnClamped
	{
		VeinConnClamped(int veinId ,  int clustterId , int pairId) : m_VeinOrganId(veinId) , m_ClusterId(clustterId) ,  m_PairId(pairId){}
		//VeinConnClamped(VeinConnectObject * veinobj ,  int clustterId , int pairId) : m_pVeinObj(veinobj) , m_ClusterId(clustterId) ,  m_PairId(pairId){}
		//VeinConnectObject * m_pVeinObj;
		int m_VeinOrganId;
		int m_ClusterId;
		int m_PairId;
		Ogre::Vector3 m_contactPt;
	};

    class ClampedRopeSegData
    {
    public:
        ClampedRopeSegData() : m_SegmentIndex(-1){}
        Real   m_Coord0[2];
        Real   m_Coord1[2];        
        int    m_SegmentIndex;
        int    m_SegmentGlobalIndex;
        Real   m_Weight;
        GFPhysVector3 m_DragForce;
    };

	//node position coordinate in clamp region
	class ClampedNodeData
	{
	public:
		ClampedNodeData()
		{
			m_IsInClamp = true;
			m_FaceRefCount = 0;
			
		}
		float m_Tan0;
		float m_Tan1;
		
		//float m_CoordN;
		GFPhysVector3 m_LocalAnchorToCom;//
		GFPhysVector3 m_DragForces;
		GFPhysSoftBodyNode * m_NodesInClamp;
		int   m_RegionIndex;//left right
		//float m_NodeOrginMass;
		bool  m_IsInClamp;
		int   m_FaceRefCount;//how many face reference this node
	};

	class OrganBeClamped
	{
	public:
		OrganBeClamped(MisMedicOrgan_Ordinary * organ)
		{
			m_ClampMode = 0;
			m_organ = organ;
			m_IsReleased = false;
			m_MainClampFaceIndex = -1;
		}
		std::map<GFPhysSoftBodyNode* , ClampedNodeData> m_ClampedNodes;//this all the node in "m_ClampedFaces"
		std::map<GFPhysSoftBodyNode*, ClampedNodeData> m_EdgeClampedNodes;
		std::vector<SoftBodyFaceClamped> m_ClampedFaces;//soft body's face be clamped currently
		std::vector<GFPhysSoftBodyTetrahedron*> m_TetrasInClampReg;
		int m_ClampMode;
		int m_MainClampFaceIndex;
		MisMedicOrgan_Ordinary * m_organ;
		GFPhysVector3  m_ClampDirInMaterialSpace;
		bool m_IsReleased;
	};

	class NumOfFacePressed
	{
	public:
		NumOfFacePressed(){ Num[0] = Num[1] = 0;}
		int Num[2];
	};

	ATTRIBUTE_ALIGNED16(class) ClampCellData
	{
	public:
		GFPhysTransform m_transform;
		GFPhysVector3 m_localmin;
		GFPhysVector3 m_localmax;
        
		GFPhysVector3 m_worldmin;
		GFPhysVector3 m_worldmax;

		GFPhysVector3 m_CellVertsReg0[3];
        GFPhysVector3 m_CellVertsReg1[3];

		//
		GFPhysTransform m_InvTrans;//for fast use
		GFPhysVector3   m_CellVertsWorldSpace[6];

		bool m_AABBTriOverlap;//for use by selectorgan clamp
	};

	MisCTool_PluginClamp(CTool * tool , float checkClampStartShaft = 6.0f);

	~MisCTool_PluginClamp();

	void SetMaxReleasingTime(float time);

	void UpdateClampRegions();

	void GetClampSpaceWorldAABB(GFPhysVector3 & regMin , GFPhysVector3 & regMax);
	
	int GetNumClampSpaceCell()
	{
		return m_ClampSpaceCells.size();
	}

	ClampCellData & GetClampSpaceCell(int index)
	{
		return m_ClampSpaceCells[index];
	}

	/*@ overridden */
	virtual void OnOrganBeRemovedFromWorld(MisMedicOrganInterface * organif);

	/*@ overridden */
    virtual void OnCustomSimObjBeRemovedFromWorld(MisCustomSimObj * rope);
 
	void ReleaseClampedOrgans();

	void ClearClampedVeinConn();

	void ReleaseClampedVeinConn();

    //void ClearClampedThread();

    //void ReleaseClampedThread();


	//软体夹持
	void SelectOrgansToClamp(std::set<MisMedicOrgan_Ordinary*> & organSet,
		                     bool allowUnTouchClamp);
	bool CheckOrganBeClampedV2();
	void ProcessFacesAfterClamped(OrganBeClamped & clampOrgan);
	

	//胆囊连接夹持
	bool CheckVeinObjBeClamped();

	//剪刀或者超声刀用
	int DisconnectClampedVeinConnectPairs();
    //缝合线夹持
	SutureThread* GetRopeBeClamped() { return m_pClampedRope; }
	SutureThreadV2* GetRopeBeClampedV2() { return m_pClampedRopeV2; }
    void ReleaseClampedRope();
    bool CheckRopeBeClamped();

	void ReleaseClampedRopeV2();
	bool CheckRopeBeClampedV2();
	//end
	
	bool isInClampState();
   
	bool GetThreadClampState(){ return m_IsThreadClamp; }
	
	int  GetNumOrgansBeClamped();
	void GetOrgansBeClamped(std::vector<MisMedicOrgan_Ordinary *> & organsclamped);
	
	int  GetNumVeinConnectPairsBeClamped();
	void GetVeinConnectBeClamped(std::vector<VeinConnClamped> & connectclamped);

    const std::vector<ClampedRopeSegData> & GetRopeSegmentsBeClamped(){ return m_ClampedRopeSegments; }

	void SetIgnored(MisMedicOrgan_Ordinary *organ);
	void ClearIgnored();


	void SetClampRegion(NewTrainToolConvexData & convexRigid , //GFPhysRigidBody * attachRigid,
						const GFPhysVector3 & center,
						const GFPhysVector3 & axis0,
						const GFPhysVector3 & axis1,
						float size0,
						float size1,
						ClampRegSide regSide,
						Real  normalSign);

	void SetClampRegion(NewTrainToolConvexData & convexRigid ,
						const GFPhysVector3 & center,
						const GFPhysVector3 & axis0,
						const GFPhysVector3 & axis1,
						Ogre::Vector2 triVertices[] , 
						int		numVertices,
						ClampRegSide regSide,
						Real    normalSign);
	
	GFPhysVector3 TransFormWorldClipDirToLocalDir(const GFPhysVector3 & LeftworldDir,
												  std::vector<GFPhysSoftBodyFace*> & faceInRegL ,
												  std::vector<GFPhysSoftBodyFace*> & faceInRegR);
	//check whether a face in intersect with the clamp region
    int  TestFaceContactClampReg(GFPhysSoftBodyFace * face, const float threshold, GFPhysVector3 & ResultPtReg, GFPhysVector3 & ResultPtFace, float & contactDepth);
  
    bool TestNodeContactClampReg(GFPhysSoftBodyNode * node);

    bool IsVeinConnContactClampRegion(GFPhysVector3 faceVertices[] , GFPhysVector3 & ResultPtInClampReg , GFPhysVector3 & ResultPtInFace , float seperateDist = 0);

	bool IsThreadContactClampRegion(SutureThread* thread, int segIndex, GFPhysVector3 & ResultPtInClampReg, GFPhysVector3 & ResultPtInTri, float seperateDist = 0);
	bool IsThreadContactClampRegion(SutureThreadV2* thread, int segIndex, GFPhysVector3 & ResultPtInClampReg, GFPhysVector3 & ResultPtInTri, float seperateDist = 0);

	GFPhysVector3 GetPluginForceFeedBack();

    GFPhysVector3 CalcRopeClampedForceFeedBack();

	GFPhysVector3 CalcVeinClampedForceFeedBack();
	
	virtual void onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair);

	virtual void RigidSoftCollisionsSolved(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);

	virtual void onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);

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

	const ToolClampRegion & GetClampRegion(int index);

	const ToolClampRegion * GetClampRegions()const;

	GFPhysVector3 GetClampRegNormal(int RegIndex);

	void GetClampRegTangent(int RegIndex, GFPhysVector3 & tan0 , GFPhysVector3 & tan1);
	
	//void UpdateClampRegionWorldAABB();

	//get tetrahedrons in clamped space
	void CollectTetrasInClampRegion(OrganBeClamped & organclamped);

	// a decal like algorithm
	void CollectSubFaceInClampRegionUV(GFPhysSoftBodyFace * face, int nodeIndex, float& U, float& V,float regionScale);

	void CollectPos();

	void UpdateMoveInfo(float dt);
    void CollectClampCandidateInfo();
    bool IsCloseToClampFace(GFPhysSoftBodyNode * node, Real threshold);
	//test whether the face is in burned region
	void GetFaceVertices2DPos(const GFPhysSoftBodyFace& face , Ogre::Vector2 faceVertices[3]);
	void GetClampCoordSys(GFPhysVector3 &origin,GFPhysVector3 &right,GFPhysVector3 &up);
	inline float GetHalfExt0() {return halfExt0;}
	inline float GetHalfExt1() {return halfExt1;}

	float GetMoveDistAfterClamped() {return m_MoveDistAfterClamped;}

	//void GetClampRange(const GFPhysVectorObj<GFPhysSoftBodyFace*> &FacesInClamp , float &left , float &right , float &top , float &bottom);


	//Get Faces In m_FaceInClamp variable
	void GetFacesBeClamped(GFPhysVectorObj<GFPhysSoftBodyFace*> & resultFaces , MisMedicOrgan_Ordinary * organ);
	
	virtual void OnSoftBodyNodesBeDeleted(GFPhysSoftBody * sb, const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes);

	virtual void OnSoftBodyFaceBeDeleted(GFPhysSoftBody * sb, GFPhysSoftBodyFace *face);

	virtual void OnSoftBodyFaceBeAdded(GFPhysSoftBody * sb, GFPhysSoftBodyFace *face);

// 	virtual void CollideVeinConnectPair(VeinConnectObject * veinobject ,
// 		GFPhysCollideObject * convexobj,
// 		int cluster , 
// 		int pair,
// 		const GFPhysVector3 & collidepoint);

	//ClampedCluster m_ClampCluster;
	float m_NormalSolveSoftDist;

	std::vector<OrganBeClamped*> m_ClampedOrgans;

	//GFPhysVector3  m_ClampDirInMaterialSpace;

	//clamp region axis aligned box
	GFPhysVector3  m_WorldClampRegMin;
	GFPhysVector3  m_WorldClampRegMax;
	float          m_ClampRegThickNess;

	//std::map<GFPhysSoftBodyNode* , ClampedNodeData> m_ClampedNodeCluster;//this all the node in "m_ClampedFaces"

	//std::vector<SoftBodyFaceClamped> m_ClampedFaces;//soft body's face be clamped currently

    std::vector<VeinConnClamped> m_ClampVeinConn;
    //std::vector<ThreadSegClamped> m_ClampThreadSegs;

	
	//std::set<GFPhysSoftBodyFace*> m_FacesToIgnored;

	
   // MisMedicOrgan_Ordinary * m_TetraOrgan;

	GFPhysAlignedVectorObj<ClampCellData> m_ClampSpaceCells;

	bool  m_ShowClampRegion;//for debug

	float m_minShaftRangeUpperValue;
	float m_minShaftRangeLowerValue;

	bool  m_CanClampConnect;
	//int   m_FaceCollidedWithPart[2];
	std::set<MisMedicOrganInterface*> m_OrgansCollidedLeftPart;
	std::set<MisMedicOrganInterface*> m_OrgansCollidedRightPart;

    bool  m_CanClampThread;
    bool  m_CanClampLargeFace;
	bool  m_CheckClampForAllOrgans;
protected:
	bool IsEdgeBeClamped(GFPhysSoftBodyEdge * edge);

    void CalculateThreadSegmentsInClampRegions(float NormalSpan,SutureThread * threadRope);
	void CalculateThreadSegmentsInClampRegions(float NormalSpan,SutureThreadV2 * threadRope);
	void InternalFreeClampedFaces(OrganBeClamped & organclamp);

	void DrawClampAxisAndRegion();

	void CalculateVeinConnInClampRegions(VeinConnectObject * veinobj);


	
	//void AddTriClampRegion(Ogre::Vector2 triVertices[] , int numVertices, int regionIndex);
	MisCTool_TubeClamp * m_TubeClampProcessor;

	bool m_CanClampMultiBody;
	ToolClampRegion m_ClampReg[2];
	
	bool  m_IsOpenAngleEnough;

	ClampStateStage  m_ClampStateStage;
    float m_TimeSinceRelease;
	float m_MaxReleasingTime;

	float m_InReleaseTime;

	bool  m_InVeinClamp;
    bool  m_IsThreadClamp;


	
	//float m_clampKeeprTime;
	//int   m_InnerClampState;//0 not clamp 1 clamp release but keeper 2 clamp release
	//MisMedicOrgan_Ordinary * m_OrganInClamp;
	//std::vector<MisMedicOrgan_Ordinary *> m_ClampedOrgans;
	std::set<MisMedicOrgan_Ordinary*> m_OrgansIgnored;
    float m_IgnorCoolDownTime;
	//MisMedicOrgan_Ordinary * m_OrganIgnored;

	//new mode
	GFPhysVector3 m_ClampedRestCom;
	GFPhysVector3 m_ClampedComLocal;
	GFPhysQuaternion m_ClampedRotLocal;
	//


	//for rope
	std::vector<ClampedRopeSegData> m_ClampedRopeSegments;
	//GFPhysRigidBody*				m_pCollideWithRopeRigidbody;
	GFPhysVector3					m_localpointInRigid;
	SutureThread*					m_pClampedRope;  //夹持的rope
	SutureThreadV2*					m_pClampedRopeV2;  //夹持的ropev2
	std::set<int>					m_vRopeNodeClampedIndexList;//夹持后碰撞取消的ropenode index列表
	
	
    Real m_MaxPerSistentShaftAside;

	//std::set<GFPhysSoftBodyFace*> m_FaceCollision;
	//for vein
	GFPhysVector3 m_lastVeinforceFeedBack;

	//int  m_RopeSegmentIndex;

	float m_ReleaseClampShaft;

	float m_clampCheckAngleStart;

	//int   m_PressedForceCount[2];

	//std::map<GFPhysSoftBody* , NumOfFacePressed> m_PressedForceCountMap;
	
	bool m_IsRect;

	//bool m_First;
	
	//[0]--left [1]--right [2]--top [3]--bottom 
	float m_burnScaleFactor[4];

	Ogre::ManualObject *m_manual;

	GFPhysVector3 centerWorld;// = (clampReg_0.m_CenterWorld + clampReg_1.m_CenterWorld) * 0.5f;
	GFPhysVector3 axis0World;// = centerWorld + (clampReg_0.m_CenterWorld - clampReg_0.m_Axis0World);
	GFPhysVector3 axis1World;// = (clampReg_0.m_Axis1World + clampReg_1.m_Axis1World) * 0.5f;
	float	halfExt0;
	float	halfExt1;

	GFPhysVector3 m_LastClampPointTo;
	GFPhysVector3 m_ClampCenterAtClamped;
	GFPhysVector3 m_LastClampCenter;
	float m_MoveIncrementInPointTo;
	float m_ClampMoveSpeed;
	float m_MoveDistAfterClamped;
};
#endif