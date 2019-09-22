/**Author:zx**/
#pragma once
#include "ITool.h"
#include <vector>
#include "Dynamic/GoPhysDynamicLib.h"
#include "collision/CollisionShapes/GoPhysCompoundShape.h"
#include "GoPhysContactListener.h"
#include "NewTrain/MisMedicOrganInterface.h"
#include "MXOgreGraphic.h"
using namespace::std;
using namespace GoPhys;

#define CANCLAMPNUM  3

class CXMLWrapperTool;
class CXMLWrapperTraining;
class CXMLWrapperHardwareConfig;
class MisToolCollidePart;
class MisMedicCToolPluginInterface;
class MisMedicThreadRope;
class MisMedicOrgan_Ordinary;
class MisCTool_PluginClamp;
class SutureThread;
class SutureThreadV2;
class MisCustomSimObj;
class VeinConnectPair;

enum  MisMedicRigidCategry
{
	MMRC_None = 0,

	MMRC_LeftTool  = 1,

	MMRC_RightTool = (1 << 1),
	
	MMRC_UserStart = (1 << 16),

	MMRC_UserEnd = (1 << 31),
};
//fore feed back record for some frame
class  ForceFeedBackRecord
{
public:
	ForceFeedBackRecord()
	{
		m_ContactForce = Ogre::Vector3::ZERO;//GFPhysVector3(0 , 0 , 0);
		m_DragForce    = Ogre::Vector3::ZERO;// m_PenetrateMagnitude = 0;
	}
	Ogre::Vector3 m_ContactForce;
	Ogre::Vector3 m_DragForce;
	///Ogre::Vector3 m_ForceDir;
	//float m_SurfaceMagnitude;
	//float m_PenetrateMagnitude;
	//float m_StickMagnitudeWeight;
};

//@NewTrainToolConvexData :convex rigid body represent tool's left, right and kernel node
//this is the main important class for tool's collision etc
class NewTrainToolConvexData
{
public:
	class CollideShapeData
	{
	public:
        CollideShapeData(const Ogre::Vector3 & center ,
			             const Ogre::Vector3 & extends,
						 const Ogre::Quaternion & rotate ,
						 GFPhysCollideShape * collideshape,
						 int shapeType)
		{
	            m_boxcenter  = center;
            m_boxextends = extends;
            m_boxrotate  = rotate;
			m_CollideShape = collideshape;
			m_ShapeType = shapeType;
		}
		Ogre::Vector3 m_boxcenter;

		Ogre::Vector3 m_boxextends;

		Ogre::Quaternion m_boxrotate;

		GFPhysCollideShape * m_CollideShape;
		int  m_ShapeType;
	};
	NewTrainToolConvexData();

	~NewTrainToolConvexData();

	void CreateRigidBodyByConfig(const MisToolCollidePart & DataConfig);

	//void CreateBoxConvex(bool isconvexhull , Ogre::Vector3 boxcenter , Ogre::Vector3 boxextends , Ogre::Quaternion boxorient = Ogre::Quaternion::IDENTITY);

	//void CreateZDirFrustumConvex(Ogre::Vector3 Z0Vertex[4] , Ogre::Vector3 Z1Vertex[4]);

	//void CreateConvex(int collisionAlgoType , const std::vector<Ogre::Vector3> & ConvexVertex,
	//	const std::vector<int> & FaceVertIndex,
	//	const std::vector<int> & FaceVertNum);

	//void CreateCynlinder(Ogre::Vector3 pointA , Ogre::Vector3 pointB , float radius);
	
	void CreateDebugDrawable(Ogre::SceneManager * pScenemgr);

	void SetNextWorldTransform(Ogre::Vector3 derivedpos , Ogre::Quaternion derivedorient);

	void UpdatePhysVelocity(float percent , float dt , bool clampmove);

	void UpdatePhysTransform();

	void SetAttachedNode(Ogre::Node * attachnode);

	void EnableDoubleFaceRSCollideMode();

	void DisableDoubleFaceRSCollideMode();

	bool IsDoubleFaceRSCollideMode();

	void SetCollideCategry(uint32 cat);

	std::vector<CollideShapeData> m_CollideShapesData;

	GFPhysCompoundShape * m_CompoundShape;

	Ogre::Vector3    m_CollideShapeRelOffset;

	Ogre::Quaternion m_CollideShapeRelRot;

	Ogre::Vector3     m_centerInWorld;

	Ogre::Quaternion m_rotateInWorld;

	Ogre::Vector3     m_centerInWorldNext;

	Ogre::Quaternion  m_rotateInWorldNext;

	Ogre::Vector3     m_DstPhysCom;

	Ogre::Quaternion  m_DstPhysOrient;

	GFPhysRigidBody * m_rigidbody;

	GFPhysVector3 m_PenetrateSensorLine[2];

	Ogre::Node * m_AttachedNode;

	Ogre::SceneNode * m_debugdrawnode;

	//Ogre::Vector3  m_contactFaceNormal;
	bool m_isvalid;
	int  m_RSCollideAlgoType;

	bool m_firstTimeSetTrans;

	bool m_bSharp;//该部分是否尖锐，用于判断是否可以戳穿物体
};

//face and tool collide data in every simulation frame
//this record in tool class for further use
class ToolCollidedFace
{
public:
	int m_ContactToolPart;//left 0  right 1 center2
	
	GFPhysCollideObject * m_collideRigid;

	GFPhysCollideObject * m_collideSoft;

	GFPhysSoftBodyFace * m_collideFace;
	//GFPhysVector3 m_FaceNormal;
	GFPhysVector3 m_CollideNormal;

	float m_collideWeights[3];

	float m_FaceContactImpluse[3];

	GFPhysVector3 m_localpointInRigid;

	float m_depth;

	bool  m_IsBackFace;
};

//thread segment and tool collide in every simulation frame
//this record in tool class for further use
class ToolCollideThreadSegment
{
public:
	int m_SegmentIndex;
	
	GFPhysCollideObject * m_collideRigid;
	
	MisMedicThreadRope * m_collideThread;
	
	GFPhysVector3 m_NormalOnRigid;

	GFPhysVector3 m_localpointInRigid;

	float m_collideWeights;
	
	float m_depth;
};

class ToolCollideSutureThreadSegment
{
public:
    int m_SegmentIndex;

    GFPhysCollideObject * m_collideRigid;

    SutureThread * m_collideSutureThread;

    GFPhysVector3 m_NormalOnRigid;

    GFPhysVector3 m_localpointInRigid;

    float m_collideWeights;

    float m_depth;
};
class ToolCollideSutureThreadSegmentV2	
{
public:
	int m_SegmentIndex;

	GFPhysCollideObject * m_collideRigid;

	SutureThreadV2 * m_collideSutureThread;

	GFPhysVector3 m_NormalOnRigid;

	GFPhysVector3 m_localpointInRigid;

	float m_collideWeights;

	float m_depth;
};

class ToolCollideRigidBodyPoint
{
public:
	GFPhysRigidBody * m_collideRigidTool;
	GFPhysRigidBody * m_collideRigidOther;

	GFPhysVector3 m_localPointTool;			
	GFPhysVector3 m_localPointOther;			
	GFPhysVector3 m_positionWorldOnOther;
	GFPhysVector3 m_positionWorldOnTool;
	GFPhysVector3 m_normalWorldOnTool;
	Real	m_Dist;    
};
//two line segment in right and left part of tool which can do 
//cut operation(like scissors etc...)
//the two line segment form a quad which will do cut operation
class ToolCutBlade
{
public:
	ToolCutBlade()
	{
		m_AttachedRB = 0;
	}
	GFPhysVector3 m_LinPoints[2];
	GFPhysVector3 m_CuttDirection;

	GFPhysVector3 m_LinePointsWorld[2];
	GFPhysVector3 m_CuttDirectionWord;

	GFPhysRigidBody * m_AttachedRB;
};

class TubeCutStateStruct
{
public:
	TubeCutStateStruct();

	float m_maxShaftAngleContact;
	float m_leftContactDist;
	bool  m_IsLeftBladeContact;
	int   m_LeftContactSectionLocal;
	int   m_LeftContactSeg;

	float m_rightContactDist;
	bool  m_IsRightBladeContact;
	int   m_RightContactSectionLocal;
	int   m_RightContactSeg;


};

//New train struct end
class CTool :public ITool , public VeinRigidCollisionListener
{
public:
	enum ToolComponent
	{
		TC_LEFT,
		TC_RIGHT,
		TC_KERNEL
	};

	struct OrganSelectedBurnFace
	{
		OrganSelectedBurnFace(float dist,
			                  const float weights[3],
			                  GFPhysSoftBodyFace * minDistFace,
			                  GFPhysSoftBody * sb)
		{
			Reset(dist, weights, minDistFace, sb);

		}

		void Reset(float dist, const float weights[3], GFPhysSoftBodyFace * minDistFace, GFPhysSoftBody * sb)
		{
			m_ClosetDist = dist;
			m_MinDistFace = minDistFace;
			m_sb = sb;
			m_MinPointWeights[0] = weights[0];
			m_MinPointWeights[1] = weights[1];
			m_MinPointWeights[2] = weights[2];

			m_MinDistPoint = m_MinDistFace->m_Nodes[0]->m_CurrPosition*m_MinPointWeights[0]
				+ m_MinDistFace->m_Nodes[1]->m_CurrPosition*m_MinPointWeights[1]
				+ m_MinDistFace->m_Nodes[2]->m_CurrPosition*m_MinPointWeights[2];

			m_materialPos = m_MinDistFace->m_Nodes[0]->m_UnDeformedPos*m_MinPointWeights[0]
				+ m_MinDistFace->m_Nodes[1]->m_UnDeformedPos*m_MinPointWeights[1]
				+ m_MinDistFace->m_Nodes[2]->m_UnDeformedPos*m_MinPointWeights[2];

		}
		float m_ClosetDist;
		//int   m_CollideFaceIndex;
		GFPhysSoftBody * m_sb;

		GFPhysVector3 m_MinDistPoint;
		GFPhysVector3 m_materialPos;
		GFPhysSoftBodyFace * m_MinDistFace;
		float m_MinPointWeights[3];
	};

public:
	CTool();
	CTool(CXMLWrapperTool * pToolConfig);

	void RecordEmitSmoke();

	virtual ~CTool(void);

	void SetVisible(bool vis);

	virtual bool Initialize(CXMLWrapperTraining * pTraining);

	virtual std::string GetCollisionConfigEntryName();

	virtual void SetToolSide(ToolSide tside);

	virtual ToolSide GetToolSide();

	virtual float SyncShaftAsideByHardWare(float nShaftAside, float dt);

	virtual void SyncToolPostureByHardware(const Ogre::Vector3 & PivotPos,//tool's pivot position in w.c.sthis should remain const all the time
		                                   const Ogre::Vector3 & TopPos, // tool's head position in w.c.s
		                                   const Ogre::Quaternion & Orient//tool orientation from z-axis
		                                   );

	virtual void CorrectKernelNode(const Ogre::Vector3& newPos, const Ogre::Quaternion& newQuaternion);

	virtual bool Update(float dt);

	virtual void CollectOperationData(float dt);
	void ReleaseRopeWhenClampNeedleAndRope();
	float GetMaxKeeppingElectricBeginTime();
	float GetMaxKeeppingElectricTime();

	float m_currentExternalForce[3];

	/**
		更新有效通电时间，在具体的器械里面定义怎样的方式才是有效的通电,然后调用此函数进行更新
	*/
	void UpdateValidElectricTime(float dt);

	void UpdateTotalElectricTime(float dt);

	/**
		更新检测对象的通电影响时间。只有在器械真正具有带电的属性时才有效.
		该函数由具体的器械类调用
		如果器械通电后会对周围的物体造成影响， 则应该在器械的Update函数中调用此函数进行数据的统计
	*/
	void UpdateAffectTimeOfCheckObject(float dt);

	virtual bool IsInElecCutting(MisMedicOrgan_Ordinary * organ) { return false; }

	virtual bool IsInElecCogulation(MisMedicOrgan_Ordinary * organ){ return false; }

	virtual bool Terminate();

	virtual void Reset();

	virtual void SetOwnerTraining(ITraining * val) { m_pOwnerTraining = val; }

	void SetOriginalInfo(ToolComponent enmToolComponent);

	virtual int GetTrianglePointsID() const { return m_nTrianglePointsID; };

	virtual ITraining * GetOwnerTraining() const { return m_pOwnerTraining; };	

	virtual void SetBackupMaterial();

	virtual void SetToSmoke(Ogre::Vector3 smokepos);

	virtual void OnVeinConnectBurned(const std::vector<Ogre::Vector3> & burnpos) {};

	virtual float GetHeadPartCollisionLen(){ return 1.0f;}//used in tool vs tool collision to omit some lenth from kernal position

	//for new train start
	void ReadCollisionData(const Ogre::String & toolname);

	int  GetRigidBodyPart(GFPhysCollideObject * rigidbody);//0-left 1-right 2-center -1none

	const NewTrainToolConvexData * GetRigidBodyBelongPart(GFPhysCollideObject * rigidbody);

	virtual void GetSoftNodeBeingGrasped(std::set<GFPhysSoftBodyNode*> & NodesBeGrasped);

	virtual void onFrameUpdateStarted(float timeelpased);

	virtual void onFrameUpdateEnded();

	virtual bool GetForceFeedBack(Ogre::Vector3 & contactForce, Ogre::Vector3 & dragForce);

	virtual Ogre::Vector3 GetForceFeedBackPoint();

	virtual void ClearForceFeedBack();

	virtual float ModifySurfaceFeedback(float CurrSurfaceFeed , float LastSurfaceFeed);

	virtual int GetCollideVeinObjectBody(GFPhysRigidBody * bodies[3]);

	virtual void OnOrganBeRemoved(MisMedicOrganInterface * organif);
	
	virtual void OnCustomSimObjBeRemovedFromWorld(MisCustomSimObj * simobj);

	virtual void OnRigidBodyBeRemoved(GFPhysRigidBody * rb);

	virtual int onBeginCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj );

	virtual void  onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj , const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints);

	virtual void  onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair);

	virtual void  onRSContactsBuildFinish( GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);//deprecated

	virtual void  onRSContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);//deprecated

	//new method
	virtual void  onRSFaceContactsBuildFinish( GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);
	virtual void  onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);
	//

	virtual void  OnThreadSegmentCollided(GFPhysCollideObject * rigidobj ,
										  MisMedicThreadRope * rope,
										  int segIndex,
										  const GFPhysVector3 & pointOnRigid,
										  const GFPhysVector3 & normalOnRigid,
										  float weights,
										  float depth
										  );
    void OnSutureThreadSegmentCollided( GFPhysCollideObject * rigidobj , SutureThread * suturerope, int segIndex, const GFPhysVector3 & pointOnRigid, const GFPhysVector3 & normalOnRigid, float weights, float depth );
	void OnSutureThreadSegmentCollided(GFPhysCollideObject * rigidobj, SutureThreadV2 * suturerope, int segIndex, const GFPhysVector3 & pointOnRigid, const GFPhysVector3 & normalOnRigid, float weights, float depth);

	virtual void  OnRigidCollided(GFPhysRigidBody * ra , GFPhysRigidBody * rb , const GFPhysManifoldPoint * contactPoints , int NumContactPoints);

	virtual void  CalculateToolForceFeedBack(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);//deprecated
	
	//new method
	virtual void  CalculateToolForceFeedBack(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);
	//

	virtual GFPhysVector3 CalculateToolCustomForceFeedBack();

	virtual void  UpdateConvex();

	virtual void  UpdateConvexVelocity(float percent , float dt);

	virtual void  UpdateConvexTransform();

	virtual void  InternalSimulationStart(int currStep , int TotalStep , float dt);

	virtual void  InternalSimulationEnd(int currStep , int TotalStep , float dt);

	virtual void  OnCutBladeClampedTube(MisMedicOrgan_Tube & tubeclamp , int segment , int localsection , Real sectionWeight);

	virtual float GetCutWidth(){return 0;}

	virtual void  OnVeinConnectPairCollide(MisMedicOrganInterface * veinobject ,
		GFPhysCollideObject * convexobj,
		int cluster , 
		int pair,
		const GFPhysVector3 & collidepoint);

	virtual void OnSoftBodyNodesBeDeleted(GFPhysSoftBody *sb , const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes);

	virtual void OnSoftBodyFaceBeDeleted(GFPhysSoftBody * sb , GFPhysSoftBodyFace * face);

	virtual void OnSoftBodyFaceBeAdded(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face);

	virtual void OnFirstFrame();

	virtual void GetToolCutPlaneVerts(GFPhysVector3 cutQuads[4]);

	virtual bool ElectricCutOrgan(MisMedicOrgan_Ordinary * organ , GFPhysSoftBodyFace * face  , float weights[3]);

	virtual int TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnpair);
	
	virtual void  TryElecBurnTouchedOrgans(std::vector<OrganSelectedBurnFace> & touchedOrgan , float dt);

	virtual float GetFaceToElecBladeDist(GFPhysVector3 triVerts[3]){ return -1; }
	
	virtual bool  IsLeftPartConductElectric() { return true; }

	virtual bool  IsRightPartConductElectric(){ return true; }

	const GFPhysCollideObject * GetDistCollideObject();

	const GFPhysSoftBodyFace * GetDistCollideFace();

	void GetDistPointWeight(float weights[3]);

	GFPhysVector3 GetDistPoint();

	//for new train end

	void GetKernelLine(Ogre::Vector3& vecmin, Ogre::Vector3& vecmax);

	virtual void warn();

    virtual void ReleaseClampedOrgans();
	virtual void ReleaseHoldRigid();
	virtual	void ReleaseClampedRope();

	virtual bool HasGraspSomeThing();
	
	virtual void GetGraspedOrgans(std::vector<MisMedicOrgan_Ordinary*> & organs);

	virtual int  DisconnectClampedVeinConnectPairs();

	virtual MisCTool_PluginClamp * GetClampPlugin();
protected:
	virtual void checkWarn();

	virtual void InitializeConvexTransform();

private:
	void SetNodeMaterial(Ogre::Node * node);
	Ogre::SceneNode* GetTipNode();
protected:
	bool m_tosmoke;
	Ogre::Vector3 m_tosmokepos;

	int m_nTrianglePointsID;
	Ogre::ManualObject * m_pManualObj;
	Ogre::SceneNode * m_ps4mNode;

	Ogre::ManualObject * m_pTriangleObj;
	Ogre::SceneNode * m_pTriangleNode;

	ITraining * m_pOwnerTraining;

	MisCTool_PluginClamp * m_pluginclamp;

	std::vector<OrganSelectedBurnFace> m_OrganFaceSelToBurn;
	//std::set<GFPhysSoftBodyNode*> m_DisableCollisionNodes;
	// original attached points, zx
	//vector<AttachedPoint> m_vecAttachedPoints;

	//void Updates4m(Ogre::Quaternion & quat, Ogre::Vector3 & v3Pos, bool bLeft = true);
	//virtual void Updates4m();
	//virtual void Updates2m();
	//virtual void UpdateTriangle();
	
	//virtual void UpdateAttachedPoints();


	//DWORD m_dwLastCuttingTickCount;

	//int m_nCollisionResponseIDLeft;
	//int m_nCollisionResponseIDRight;
	//int m_nCollisionResponseIDKernel;
	//int m_nLeftCountForFire;
	//int m_nRightCountForFire;
	//int m_nLeftPointIndexForFire[3];
	//int m_nRightPointIndexForFire[3];

	//DWORD m_dwLeftFirstPointCurTime;
	//DWORD m_dwRightFirstPointCurTime;
	bool m_bIsPointHaveBreak;

	//float m_nLastShaftAside;
	//夹到器官后，最小夹合角度 
	//float   m_nCurMinShaftAside;
	//判断是否满足夹住的最大角度 
	//float   m_cClampShaftAside;
	//夹合动作的累积次数
	//size_t  m_nNumClamp;

	Ogre::Vector3  m_vecToolFirstPos;
	//int m_nLeftFireCount;
	//int m_nRightFireCount;

	bool m_bClampThenCut;//超声刀使用

	DWORD m_dwStartCutTubeTime;

	double m_fBloodTimeBefore;

	bool m_BubbleWhenBurn;
private:
	bool m_bWarnState;
	bool m_bFirstEnterWarn;
	double m_WarnTick;
	Ogre::ColourValue m_PassColor[2];
	vector< Ogre::String > m_vecToolMaterial;
	vector< Ogre::String > m_vecToolMaterial_backup;
	vector< Ogre::ColourValue > m_vecAmbient;
	vector< Ogre::ColourValue > m_vecDiffuse;

public:
	//New Train Data start
	std::vector<MisMedicCToolPluginInterface*> m_ToolPlugins;

	NewTrainToolConvexData m_lefttoolpartconvex;

	NewTrainToolConvexData m_righttoolpartconvex;

	NewTrainToolConvexData m_centertoolpartconvex;

	ToolCutBlade m_CutBladeLeft;

	ToolCutBlade m_CutBladeRight;

	GFPhysAlignedVectorObj<ToolCollidedFace> m_ToolColliedFaces;
	
	GFPhysAlignedVectorObj<ToolCollideThreadSegment> m_ToolCollidedThreads;

	GFPhysAlignedVectorObj<ToolCollideSutureThreadSegment> m_ToolCollidedSutureThreads;

	GFPhysAlignedVectorObj<ToolCollideSutureThreadSegmentV2> m_ToolCollidedSutureThreadsV2;

	GFPhysAlignedVectorObj<ToolCollideRigidBodyPoint> m_ToolCollidedRigids;

	std::map<int , TubeCutStateStruct> m_TubeCutStateData;

	/*force feed back*/
	std::vector<ForceFeedBackRecord> m_ForceFeedRecords;

	std::vector<Ogre::Vector3> m_RawForceFeed;

	std::vector<Ogre::Vector3> m_SurfaceForceFeed;

	Ogre::Vector3 m_FeedBackForcePoint;

	//GFPhysVector3 m_LastFeedBackForceDir;

	float m_PentrateForce;

	bool  m_UseNewToolMode;

	std::set<int> m_ContactedOrganTubes;

	bool  m_NewCanClampTube;

	bool  m_CanPunctureOgran;

	float m_PuncThresholdMultiply;

	float m_ToolForceBackRate;

	bool  m_InClampState;
	// line tip node
	Ogre::SceneNode* m_pTipNode;
	GFPhysVector3 m_ToolFightingForce;
	//std::set<GFPhysSoftBodyFace*> m_FaceCollided;
	//newtrain dataend

	bool m_isFirstFrame;

    Ogre::Vector2 m_TextureCoord;//the last Electric position
    float m_CumTime; //The cumulative time of the electrical appliances

	GFPhysVector3 RawSurfaceForceFeedLeft;

	GFPhysVector3 RawSurfaceForceFeedRight;

	GFPhysVector3 RawSurfaceForceFeedCenter;
};
