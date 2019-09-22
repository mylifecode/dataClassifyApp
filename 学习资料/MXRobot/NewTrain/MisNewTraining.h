#ifndef _MISNEWTRAINING_
#define _MISNEWTRAINING_
#include "BasicTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include "TrainScoreSystem.h"
#include "MxOperateItem.h"
#include "SYScoreItemDetail.h"
#include "MisMedicThreadRope.h"
#include "SutureNeedle.h"
#include "MisMedicObjectUnion.h"
#include "SutureNeedleV2.h"
class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;//deprecatted
class ACTubeShapeObject;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class MisNewTraining;
class MisMedicObjLink;
class MisMedicObjLink_Approach;
class MisMedicObjectEnvelop;
class MisMedicAdhesionCluster;
class MisMedicThreadRope;
class SutureThreadV2;
class STVRGCollidePair;
class SutureNeedle;
class SutureNeedleV2;
class MisMedicTubeBody;
class WaterPool;
class CXMLWrapperOperateItem;
class ViewDetection;
class SYScoreTable;
//
//	step type :		1: add,  2: waitCheck, 3: remove, 4: finish
//  action Target(Type id):  1: objectID, 2: checkPoint(node pos distance) 3: objectID, 4: finish: exit
//  stepOverType    1: distance(?)  2: left or right, 3: not using  4: not using
enum	eTS_StepType
{
	eST_Add,
	eST_waitCheck,
	eST_remove,
	eST_Finish
};

enum GradeType
{
	NO_VALUE = 0,
	CORRECT,
	BIAS,
	MISS,
};

struct MarkTextureState
{
public:
    MarkTextureState() :visible(false){}
    bool		  visible;    
    Ogre::Vector2 uvpos;
    float		 radius;
    std::string	 matname;
};

struct trainingStep
{
	int stepType;
	int actionTarget;
	int stepOverType;
	trainingStep(int a, int b, int c)
	{
		stepType = a;
		actionTarget = b;
		stepOverType = c;
	}
};

//训练中弹窗的按钮行为类型
enum TrainPopupWidgetButtonActionType
{
	TPWBT_TRAIN_GOBACK = 0xFF,						//返回
	TPWBT_TRAIN_NORMAL = 100,

	TPWBT_TRAIN_COMPLETELY = 1,						//完成训练
	TPWBT_TRAIN_TIMESUP = 2,						//超时
	TPWBT_TRAIN_FATALERROR = 3,						//严重错误
    TPWBT_TRAIN_ERRORWITHOUTQUIT = 0                //发生错误仍可继续
};


void NewTrainingHandleEvent(MxEvent * pEvent, ITraining * pTraining);

class MisNewTraining :public CBasicTraining , 
	public GFPhysInternalSimulateListener , 
	public GFPhysSoftRigidContactPointListener ,
	public GFPhysRigidCollisionListener,
	public GFPhysDiscreteDynamicsWorld::GFPhysCustomWorldActionListener,
	public GFPhysDynamicWorldEventListener
{
public:
	enum	OperationCheckPointType
	{
		OCPT_Empty,
		OCPT_Burn,
		OCPT_Burn_Origin_Face,
		OCPT_Burn_Cut_Face,
		OCPT_Cut,								//电凝钩电切、剪刀等剪完成后产生的 有可能包括实际操作范围外的 
		OCPT_Clip,
		OCPT_ElecCut ,						//电凝钩电切时
		OCPT_ElecCutWithForcep ,
		OCPT_ElecCoagWithHook , 
		OCPT_Inject ,
		OCPT_Clamp,
	};

	MisNewTraining(void);

	virtual ~MisNewTraining(void);

	virtual MisMedicOrganInterface * LoadOrganism(MisMedicDynObjConstructInfo & cs, MisNewTraining *pTrain);//(CXMLWrapperTraining * pTrainingConfig, int i);        

	virtual void RemoveOrganFromWorld(MisMedicOrganInterface * organ);

	void RemoveOrganFromWorld(int organID);

	virtual void BuildOrgansVolumeTextureCoord();

	//the fake organ id will be given by the training
	virtual MisMedicOrganInterface * AddFakeOrgan(MisMedicDynObjConstructInfo & cs);
	
	virtual MisMedicOrganInterface * ManuallyCreateOrgan(MisMedicDynObjConstructInfo & cs);

	virtual void RemoveFakeOrganFromWorld(MisMedicOrganInterface * pOrgan);

	MisMedicThreadRope * CreateRopeThread(Ogre::SceneManager * scenemgr);

	MisMedicThreadRope * CreateSimpleLooper(Ogre::SceneManager * scenemgr);

	SutureNeedle * CreateNeedle(Ogre::SceneManager * scenemgr, int threadnodenum, Real restlen, const Ogre::String & needleskeleton);
	
	SutureNeedleV2 * CreateNeedleV2(Ogre::SceneManager * scenemgr, int threadnodenum, Real restlen, const Ogre::String & needleskeleton);

    MisMedicTubeBody * CreateTubeThread(Ogre::SceneManager * scenemgr);

	ACTubeShapeObject * CreateSoftTube(int ObjID,
		                               const GFPhysVector3 & center , 
		                               float ringRadius , 
									   float thickness);

	bool RemoveThreadRopeFromWorld(MisMedicThreadRope * rope);

    bool RemoveNeedleFromWorld(SutureNeedle * needle);

	bool OnRemoveSutureThreadFromWorld(SutureThread * rope);//temp method 

	bool RemoveNeedleFromWorld(SutureNeedleV2 * needle);

	bool OnRemoveSutureThreadFromWorld(SutureThreadV2 * rope);//temp method 

    bool RemoveTubeBodyFromWorld(MisMedicTubeBody * tube);

	TrainScoreSystem * GetScoreSystem();

	DynObjMap GetOrganObjectMap();

	Ogre::Vector3 GetSceneGravityDir()
	{
		return m_SceneGravityDir;
	}

	virtual void OnOrganBeElectricCutted(MisMedicOrgan_Ordinary * organ)
	{}

	virtual void SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ)
	{}
	
	//@@ give a chance for train to modify serializer
	virtual void SerializerReadFinish(MisMedicOrgan_Ordinary * organ , MisMedicObjetSerializer & serialize)
	{}

	//overridden
	virtual std::vector<VeinConnectObject*> GetVeinConnectObjects();

	//overridden
	virtual VeinConnectObject  * GetVeinConnect(int connectType);

	MisMedicOrganInterface* GetOrgan(DynamicObjType type);

	//overridden
	virtual MisMedicOrganInterface * GetOrgan(int id);

	//overridden
	virtual MisMedicOrganInterface * GetOrgan(GFPhysSoftBody * body);

	//overridden
	virtual void GetAllOrgan(std::vector<MisMedicOrganInterface*>& ogans);

	//
	virtual void GetAllTubes(std::vector<ACTubeShapeObject*> & tubes);

	inline CTool* GetLeftTool()
	{
		CTool * pTool = NULL;
		if(m_pToolsMgr)
			pTool = (CTool*)(m_pToolsMgr->GetLeftTool());
		return pTool;
	}

	inline CTool* GetRightTool()
	{
		CTool * pTool = NULL;
		if(m_pToolsMgr)
			pTool = (CTool*)(m_pToolsMgr->GetRightTool());
		return pTool;
	}

	std::vector<SutureNeedle *> & GetSutureNeedles()
	{
		return m_SutureNeedles;
	}
	std::vector<SutureNeedleV2 *> & GetSutureNeedlesV2()
	{
		return m_SutureNeedlesV2;
	}
	//////////////////////////////////////////////////////////////////////////
    inline Ogre::MeshPtr& GetStaticDomeMeshPtr()
    {
        return m_StaticDomeMeshPtr;
    }
    inline MisMedicObjectUnion& GetStaticDynDomeUnion()
    {
        return m_StaticDynDomeUnion;
    }
	//检测是否有操作项配置
	bool HasOperateItemConfig();

	virtual SYScoreTable* GetScoreTable();

	//overridden
	virtual void LoadConfig(CXMLWrapperTraining * pTrainingConfig);

	/// 导入预定义的操作项，可通过配置训练配置文件来设置
	virtual void LoadOperateItem(CXMLWrapperTraining * pTrainingConfig);

	//overridden
	virtual void LoadDynamicData(CXMLWrapperTraining * pTrainingConfig);

	//topology change  when delete add tetrahedron face edge this will be called by mismedic organ
	virtual void FacesBeAdded(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces , MisMedicOrgan_Ordinary * orgn);// GFPhysSoftBodyShape * hostshape);
	
	virtual void TetrasBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> & tetras , MisMedicOrgan_Ordinary * orgn);//GFPhysSoftBodyShape * hostshape);
    
    virtual void RefreshVeinconnectOnFacesBeModified(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & modifiedfaces, MisMedicOrgan_Ordinary * organ);

    virtual void FacesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces, GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & createdfaces, MisMedicOrgan_Ordinary * orgn);//GFPhysSoftBodyShape * hostshape);
    
	virtual void NodesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes , MisMedicOrgan_Ordinary * orgn);//GFPhysSoftBodyShape * hostshape);

    virtual void FacesBeModified(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces,  GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & createdfaces, MisMedicOrgan_Ordinary * orgn);

	virtual void UpdateNodeToTetraLinks(const std::map<GFPhysSoftBodyTetrahedron *, std::vector<GFPhysSoftBodyTetrahedron *>> & deletetetras,
		                                const GFPhysAlignedVectorObj<GFPhysSoftBodyTetrahedron*> createdtetras , 
										MisMedicOrgan_Ordinary * organ);

	//@@overridden GFPhysInternalSimulateListener
	virtual void InternalSimulateStart(int currStep , int TotalStep , Real dt);
	virtual void InternalSimulateEnd(int currStep , int TotalStep , Real dt);
	
	//@@overridden GFPhysDynamicWorldEventListener
	virtual void OnRemoveRigidBody(GFPhysRigidBody * rb);
	virtual void OnRemoveSoftBody(GFPhysSoftBody * sb);
	virtual void OnRemovePositionConstraint(GFPhysPositionConstraint * cs);
	virtual void OnRemoveJoint(GFPhysJointConstraint * cs);
	
	//@@overridden GFPhysSoftRigidContactPointListener
	virtual int onBeginCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj );//begin check collision before 2 objects
	/*virtual void onFaceConvexCollided( GFPhysCollideObject * rigidobj , 
										GFPhysCollideObject * softobj ,
										GFPhysSoftBodyFace * facecollide,
										const GFPhysVector3 &   CdpointOnFace,
										const GFPhysVector3 &   CdnormalOnFace,
										float depth,
										float weights[3],
										int   contactmode
										);*///when detect a soft body face and a rigid body
	virtual void onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj, const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints);//end check collision
	
	virtual void onRSContactsBuildBegin(ConvexSoftFaceCollidePair * collidePairs , int NumCollidePair);//give a chance for external application to modify the contacts

	virtual void onRSContactsBuildFinish( GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);//deprecated
	virtual void onRSContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftRigidContact> & RSContactConstraints);//deprecated

	//new method
	virtual void onRSFaceContactsBuildFinish(GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);//give a chance for external application to modify the contacts
	virtual void onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);//when all contact solved
	virtual void OnSSContactBuildFinish(const GFPhysAlignedVectorObj<GFPhysSSContactPoint> & ssContact, int numSSContact);

	//

	//rigid-rigid collision
	virtual void OnCollisionStart(GFPhysCollideObject * rigidA , GFPhysCollideObject * rigidB , const GFPhysManifoldPoint * contactPoints , int NumContactPoints);
	virtual void OnCollisionKeep(GFPhysCollideObject * rigidA , GFPhysCollideObject * rigidB , const GFPhysManifoldPoint * contactPoints , int NumContactPoints);
	virtual void OnCollisionEnd(GFPhysCollideObject * rigidA , GFPhysCollideObject * rigidB);

/*GPSDK Call Back End*/
	
	virtual void onThreadConvexCollided(GFPhysCollideObject * rigidobj , 
										MisMedicThreadRope * rope ,
										int SegIndex,
										const GFPhysVector3 &   pointOnRigid,
										const GFPhysVector3 &   normalOnRigid,
										float depth,
										float weights
										);//when detect a rope segment  and a rigid body collide
    virtual void onSutureThreadConvexCollided(const GFPhysAlignedVectorObj<TRCollidePair> & TRCollidePairs,
                                              SutureThread * suturerope );//when detect a Suture rope  and a rigid body collide

	virtual void onSutureThreadConvexCollided(const GFPhysAlignedVectorObj<STVRGCollidePair> & TRCollidePairs,
		SutureThreadV2 * suturerope);//when detect a Suture rope  and a rigid body collide
    
    virtual void OnThreadClampByTool(int ClampedSegGlobalIndex, CTool* toolobject)
    {}

    virtual void OnThreadReleaseByTool(int ClampedSegGlobalIndex, CTool* toolobject)
    {}

    virtual void OnRigidClampByTool(GFPhysRigidBody * rigid)
    {}

    virtual void OnRigidReleaseByTool(GFPhysRigidBody * rigid)
    {}
	//
	virtual MisMedicOrgan_Ordinary * FilterClampedOrgan(std::vector<MisMedicOrgan_Ordinary *> & organs);
public:

	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	virtual void ReadCustomDataFile(const Ogre::String & customDataFile);

	virtual bool BeginRendOneFrame(float timeelapsed);

	virtual bool FinishRendOneFrame(float timeelapsed){return true;}

	void SetEditorMode(bool set);

	void UpdateForceFeedBack();

	virtual bool Update(float dt);

	virtual bool Terminate();

	virtual void PerformCustomCollision(GFPhysDiscreteDynamicsWorld * dynworld);

	virtual void GetTempPosConstraints(GFPhysVectorObj<GFPhysPositionConstraint*>);
	
	void TrainTimeOver();

	void TrainingFinish();

	void AddBleedingRecords();

    void TrainingFatalError(Ogre::String errorTip = "fatalError");
    void TrainingErrorWithoutQuit(Ogre::String errorTip = "fatalError");
	//overridden
	void ScisscorCutOrgan(CScissors * scissor , MisMedicOrgan_Ordinary * organToCut);

	//void triggerTrainPopupWidget(TrainPopupWidgetButtonActionType popupType,const std::string& title, const std::string& description, const std::string& leftButtonText, const std::string& rightButtongText);

	virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif);

	virtual void GetAllMaterialReceiveShadow(std::vector<Ogre::String> & matRecShadow);

	int GetNumOfWaterPools() { return m_WaterPools.size();}

	WaterPool* GetWaterPools(int index) { return m_WaterPools[index];}

	void GetObjectLink_Approach(int objectId1,int objectId2 , std::vector<MisMedicObjLink_Approach*> & adhersions);

	bool DetectLineSegmentIntersectOrgan(MisMedicOrganInterface * insecOrgan = NULL, GFPhysSoftBodyFace * insecface = NULL);

	void OnSaveTrainingReport();

	virtual void OnTimerTimeout(int timerId,float dt , void* userData) { }

	/** 开始一个计时器，并返回唯一的id.超时后自动调用OnTimerTimeout函数，所以如果在子类中需要使用到计时器，应该重写这个回调函数 */
	int StartTimer(float time,void* userData = nullptr);

	void RemoveAllTimer();

	void RemoveTimer(int timerId);

    virtual void CalcCameraSpeed(Real dt, bool isFixed);

	//@overrideen
	virtual float GetCameraSpecialAngle();

	virtual void SetCameraSpecialAngle(float angle);

	void BurnHookedAndContactedConnect(CTool* tool,Real dt);
	/**
		操作项的增加方式
	*/
	enum AddMode
	{
		/// 直接添加
		AM_Add = 0x1,

		/// 只合并值
		AM_MergeValue = 0x1 << 2,
		/// 只合并操作时间,
		AM_MergeTime = 0x1 << 3,
		/// 只合并分数项，仅对带有分数项的操作项有效，普通操作项无效.
		AM_MergeScoreItem = 0x1 << 4,
		/// 合并所有：value、time、scoreItem
		AM_MergeAll = AM_MergeValue | AM_MergeTime | AM_MergeScoreItem,

		/// 用当前值替换已经存在操作项的值
		AM_ReplaceOnlyValue = 0x1 << 5,
		/// 用当前操作项替换已经存在的操作项
		AM_ReplaceAll = AM_ReplaceOnlyValue
	};

	bool AddOperateItem(const std::string& operateItemName,float value = 0.0f,bool setOperateTime = true,AddMode addMode = AM_Add,MxOperateItem ** ppOperateItem = NULL);
	
	bool AddOperateItem(const std::string& operateItemName,bool setOperateTime = true,AddMode addMode = AM_Add,MxOperateItem ** ppOperateItem = NULL);

	MxOperateItem* GetLastOperateItem(const std::string& operateItemName);

	MxOperateItem* GetOperateItem(const std::string& operateItemName);

	bool HasOperateItem(const std::string& operateItem);

	void RemoveLastOperateItem(const std::string& operateItemName);

	/** 该函数在训练初始化时被调用，用于保存一些默认的评分项细节，如一些未操作项 */
	virtual void AddDefaultScoreItemDetail();

	void AddScoreItemDetail(const QString& scoreCode, int time);

	void RemoveScoreItemDetail(const QString& scoreCode);

	void ShowTip(const std::string& tip);

	void SetNextTip(const std::string& tip,float waitTime = 2.5f);

	TrainPopupWidgetButtonActionType GetTrainQuitType()
	{
		return m_QuitType;
	}
	const std::vector<ShadowNodeForLinkage> & GetNodesLinkData()
	{
		return m_ShadowNodesLinkage;
	}
    const Real GetCameraSpeed() { return m_CameraSpeed; }

    //需要调用筋膜连接的原子操作来更新所有相关面的
    //当只有筋膜被删除的时候也需要用到这个原子操作
    //其他不在被删除面中但通过连接相连的面也需要更新 m_VeinconnectOnFace
   // void RemoveClustbyID(VeinConnectObject * veinobj, int clusterID);
    void OnVeinconnectChanged(VeinConnectObject * veinobj, int clusterID);
protected:
	virtual void CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig);

	virtual void OnAllOrdinaryOrganReadObjectFile(CXMLWrapperTraining * pTrainingConfig);

	virtual void OnAllOrdinaryOrganCreated(CXMLWrapperTraining * pTrainingConfig);

	//探查 定位
	virtual bool DetectExplore(float dt);

	virtual bool DetectLocation(float dt);

	void BuildObjectAdhesion(CXMLWrapperTraining * pTrainingConfig);

	//粘连
	void BuildAdhesionClusters(CXMLWrapperTraining * pTrainingConfig);

	void FindAdhesionClustersConnections(CXMLWrapperTraining * pTrainingConfig);

	void BuildAdhesionClustersConnections();

	void BuildMergedObjectConstructInFo(CXMLWrapperTraining * pTrainingConfig , std::vector<MisMedicDynObjConstructInfo> & DstUnioned , std::set<int> & objectBeUnioned);

	void TranslateOrgans(CXMLWrapperTraining * pTrainingConfig);

	void AddWaterPools(CXMLWrapperTraining * pTrainingConfig);

	void AddViewDetections(CXMLWrapperTraining *pTrainingConfig);

	void BuildVeinConnectionConstraint();

	void UpdateTrocarTransform();

	//std::vector<VeinConnectObjectV2*> m_veinObjV2s;

	std::vector<MisMedicObjLink*> m_ObjAdhersions;

	std::vector<MisMedicObjectEnvelop*> m_ObjEnvelops;

	std::vector<ShadowNodeForLinkage> m_ShadowNodesLinkage;//
	//粘连
	std::map<MisMedicOrgan_Ordinary* , MisMedicAdhesionCluster*> m_Adhesions;

	std::vector<WaterPool*> m_WaterPools;

	//探查 
	std::vector<ViewDetection*> m_ExploreDetections;
	//定位
	ViewDetection* m_pLocationDetection;
	//探查结果
	bool m_ExploreResult;
	//是否需要探查
	bool m_NeedExplore;

    std::vector<MisMedicThreadRope *> m_ThreadRopes;
	std::vector<SutureNeedle *> m_SutureNeedles;
	std::vector<SutureNeedleV2 *> m_SutureNeedlesV2;

    std::vector<MisMedicTubeBody *> m_TubeBodies;

	/// 保存没有没被创建的器官构造信息
	std::vector<MisMedicDynObjConstructInfo> m_reservedConstructInfos;

	DynObjMap m_DynObjMap;

	DynObjMap m_DynObjMapForNonOrgan;

	TrainScoreSystem * m_ScoreSys;

	bool m_IsFinalMovieShowed;

	Ogre::Vector3 m_SceneGravityDir;

	/// 当前训练预定义的操作项
	std::map<std::string,CXMLWrapperOperateItem*> m_operateItemMap;
	std::map<std::string,int> m_operateItemTimesMap;
	/// 当前训练所触发的操作项
	std::vector<MxOperateItem> m_operateItems;
	
	/// 评分细节
	std::vector<SYScoreItemDetail> m_scoreItemDetails;

    int m_SutureNodeNum;
    Real m_SutureRsLength;
    Ogre::String m_NeedleSkeletonFile;

	Ogre::Vector3 m_camPivotPos;
	bool m_DetectCameraIntersect;
	bool m_HasCameraTouchSomething;

    Ogre::Vector3 m_preCameraPos;
    Real m_disCameraMove;
    Real m_timeCameraMove;
    Real m_CameraSpeed;

	float m_CameraAngle;//

    MisMedicObjectUnion m_StaticDynDomeUnion;
    Ogre::MeshPtr m_StaticDomeMeshPtr;

	struct Timer{
		int id;
		float curTime;
		float totalTime;
		void* userData;
	};

	std::list<Timer*> m_timers;

	int m_AutoGenOrganID;

	TrainPopupWidgetButtonActionType m_QuitType;
};

#endif