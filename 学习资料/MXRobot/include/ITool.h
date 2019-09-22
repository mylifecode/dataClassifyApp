
#pragma once
#include "IMXDefine.h"
#include "MXDebugInf.h"
#include "Topology/GoPhysSoftBodyCutter.h"

using namespace GoPhys;

class CXMLWrapperTool;
class CSceneManager;
class CXMLWrapperTraining;
class MisMedicOrgan_Ordinary;
class ITraining;


/**
	该类表示一个待检测的对象，当器械通电时，可能会对此对象产生影响。
*/
class ToolElectricCheckObject
{
public:

	/** 对象类型 */
	enum ObjectType
	{
		OT_OrdinaryOrgan,
		OT_HemoClip
	};

	ToolElectricCheckObject(void * object,ObjectType type,float effectDistance)
		:m_object(object),
		m_type(type),
		m_effectDistance(effectDistance)
	{
		m_effectTime = 0.f;                
	}

	~ToolElectricCheckObject()
	{

	}

	inline float GetEffecTime() { return m_effectTime;}
	inline void	 AddEffectTime(float time) { m_effectTime += time;}

	inline ObjectType GetType() {return m_type;}
	inline void * GetObject() {return m_object;}
	inline float GetEffectDistance() {return m_effectDistance;}

private:
	void * m_object;
	ObjectType m_type;
	float m_effectDistance;
	float m_effectTime;       
};


class ITool
{
public:
	enum ToolState
	{
		TS_NONE,
		TS_CLAMP,
		TS_RELEASE,
		TS_CUT,
	};

    enum ToolSide
    {
        TSD_LEFT,
        TSD_RIGHT,
    };

	/**
		器械的拥有者：左手、右手、无
	*/
	enum ToolOwner
	{
		/// 默认状态
		TO_None,
		TO_LeftHand,
		TO_RightHand
	};

public:
	virtual void SetToolSide(ToolSide tside) = 0;

	virtual ToolSide GetToolSide() = 0;

	virtual bool Initialize(CXMLWrapperTraining * pTraining) = 0;

	virtual bool Update(float dt) = 0;

	virtual void UpdateValidElectricTime(float dt) = 0;

	virtual bool Terminate() = 0;

	virtual void Reset() = 0;

	virtual void SetOwnerTraining(ITraining * val) = 0;

	virtual ITraining * GetOwnerTraining() const = 0;

	virtual void SyncToolPostureByHardware(const Ogre::Vector3 & PivotPos,//tool's pivot position in w.c.sthis should remain const all the time
		                                   const Ogre::Vector3 & TopPos, // tool's head position in w.c.s
								           const Ogre::Quaternion & Orient//tool orientation from z-axis
									       ) = 0;

	const Ogre::Vector3& GetPivotPosition() { return m_pivotPosition; }

	const Ogre::Vector3& GetLastKernelNodePosition() { return m_lastKernelNodePosition; }

	const Ogre::Quaternion& GetLastKernelNodeQuaternion() { return m_lastKernelNodeQuaternion; }

	virtual void CorrectKernelNode(const Ogre::Vector3& newPos, const Ogre::Quaternion& quaternion) = 0;

	inline Ogre::SceneNode * GetKernelNode() const { return m_pNodeKernel; }
	inline void SetKernelNode(Ogre::SceneNode * pNodeKernel) 
	{ 
		m_pNodeKernel = pNodeKernel; 
		m_lastKernelNodePosition = m_pNodeKernel->getPosition();
	}

	inline Ogre::SceneNode * GetRightNode() const { return m_pNodeRight; }
	inline void SetRightNode(Ogre::SceneNode * pNodeRight) { m_pNodeRight = pNodeRight; }

	inline Ogre::SceneNode * GetLeftNode() const { return m_pNodeLeft; }
	inline void SetLeftNode(Ogre::SceneNode * pNodeLeft) { m_pNodeLeft = pNodeLeft; }

	inline Ogre::SceneNode * GetRight_b1Node() const { return m_pNodeRight_b1; }
	inline void SetRight_b1Node(Ogre::SceneNode * pNodeRight_b1) { m_pNodeRight_b1 = pNodeRight_b1; }

	inline Ogre::SceneNode * GetLeft_a1Node() const { return m_pNodeLeft_a1; }
	inline void SetLeft_a1Node(Ogre::SceneNode * pNodeLeft_a1) { m_pNodeLeft_a1 = pNodeLeft_a1; }

	inline Ogre::SceneNode * GetRight_b2Node() const { return m_pNodeRight_b2; }
	inline void SetRight_b2Node(Ogre::SceneNode * pNodeRight_b2) { m_pNodeRight_b2 = pNodeRight_b2; }

	inline Ogre::SceneNode * GetLeft_a2Node() const { return m_pNodeLeft_a2; }
	inline void SetLeft_a2Node(Ogre::SceneNode * pNodeLeft_a2) { m_pNodeLeft_a2 = pNodeLeft_a2; }

	inline Ogre::SceneNode * GetCenterNode() const { return m_pNodeCenter; }
	inline void SetCenterNode(Ogre::SceneNode * pNodeCenter) { m_pNodeCenter = pNodeCenter; }

	inline CXMLWrapperTool * GetConfig() const { return m_pToolConfig; }

	inline const Ogre::String & GetName() const { return m_strName; }
	inline void SetName(Ogre::String strName) { m_strName = strName; }

	inline const Ogre::String & GetType() const { return m_strType; }
	inline void SetType(Ogre::String strType) { m_strType = strType; }

	inline const Ogre::String& GetSubType() const { return m_strSubType;}
	inline void SetSubType(const Ogre::String& subType) { m_strSubType = subType;}

	virtual int GetTrianglePointsID(){ return -1; };

	// original left info
	inline Ogre::Vector3 GetOriginalLeftPos() const { return m_v3OriginalLeftPos; }
	inline void SetOriginalLeftPos(Ogre::Vector3 v3Pos) { m_v3OriginalLeftPos = v3Pos; }

	inline Ogre::Quaternion GetOriginalLeftOrientation() const { return m_quatOriginalLeft; }
	inline void SetOriginalLeftOrientation(Ogre::Quaternion quatOriginal) { m_quatOriginalLeft = quatOriginal; }

	inline Ogre::Matrix4 GetOriginalLeftMatrix() const { return m_mxOriginalLeft; }
	inline void SetOriginalLeftMatrix(Ogre::Matrix4 mxOriginal) { m_mxOriginalLeft = mxOriginal; }

	// original right info
	inline Ogre::Vector3 GetOriginalRightPos() const { return m_v3OriginalRightPos; }
	inline void SetOriginalRightPos(Ogre::Vector3 v3Pos) { m_v3OriginalRightPos = v3Pos; }

	inline Ogre::Quaternion GetOriginalRightOrientation() const { return m_quatOriginalRight; }
	inline void SetOriginalRightOrientation(Ogre::Quaternion quatOriginal) { m_quatOriginalRight = quatOriginal; }

	inline Ogre::Matrix4 GetOriginalRightMatrix() const { return m_mxOriginalRight; }
	inline void SetOriginalRightMatrix(Ogre::Matrix4 mxOriginal) { m_mxOriginalRight = mxOriginal; }

	// original kernel info
	inline Ogre::Vector3 GetOriginalKernelPos() const { return m_v3OriginalKernelPos; }
	inline void SetOriginalKernelPos(Ogre::Vector3 v3Pos) { m_curKernelPos = m_v3OriginalKernelPos = v3Pos; }

	inline Ogre::Quaternion GetOriginalKernelOrientation() const { return m_quatOriginalKernel; }
	inline void SetOriginalKernelOrientation(Ogre::Quaternion quatOriginal) { m_quatOriginalKernel = quatOriginal; }

	inline Ogre::Matrix4 GetOriginalKernelMatrix() const { return m_mxOriginalKernel; }
	inline void SetOriginalKernelMatrix(Ogre::Matrix4 mxOriginal) { m_mxOriginalKernel = mxOriginal; }

	// original points
	//inline float GetRawShaftAside() const { return m_RawShaftAside; }
	//inline void  SetRawShaftAside(float nShaftAside) { m_RawShaftAside = nShaftAside; }

	inline  float GetShaftAside() const { return m_nShaftAside; }
	virtual float SyncShaftAsideByHardWare(float nShaftAside, float dt) = 0;// { m_nShaftAside = nShaftAside; }
	inline  void  AssignShaftValueDirectly(float shaft){ m_nShaftAside = shaft; }

	inline float GetLeftShaftAsideScale() const { return m_leftShaftAsideScale;}
	inline void SetLeftShaftAsideScale(float scale) {m_leftShaftAsideScale = scale;}

	inline float GetRightShaftAsideScale() const {return m_rightShaftAsdieScale;}
	inline void SetRightShaftAsideScale(float scale) {m_rightShaftAsdieScale = scale;}

    inline float GetMinShaftAside() const { return m_nMinShaftAside; }
    inline void  SetMinShaftAside(float nMinShaftAside) { m_nMinShaftAside = nMinShaftAside; }

	inline float GetMaxShaftAside() const { return m_nMaxShaftAside; }
	inline void  SetMaxShaftAside(float nMaxShaftAside) { m_nMaxShaftAside = nMaxShaftAside; }

	inline bool GetForceRelease() const { return m_bForceRelease; }
	inline void SetForceRelease(bool bForceRelease) { m_bForceRelease = bForceRelease; }

	inline bool HasElectricAttribute() {return m_hasRealElectricAttribute;}
	inline void SetHasElectricAttribute(bool value) {m_hasRealElectricAttribute = value;}

	inline bool GetElectricButton() const { return m_bElectricButton; }
	inline void SetElectricButton(bool bElectricButton) { m_bElectricButton = bElectricButton; }

	inline bool GetElectricLeftPad() { return m_bElectricLeftPad;}		//设置左踏板是否有电状态
	inline void SetElectricLeftPad(bool bElectric) {  m_bElectricLeftPad  = bElectric;}

	inline bool GetElectricRightPad() {return m_bElectricRightPad;}
	inline void SetElectricRightPad(bool bElectric) {m_bElectricRightPad = bElectric;}

	inline float GetTotalElectricTime() {return m_totalElectricTime;}
	inline float GetValidElectricTime() {return m_validElectricTime;}

	virtual float GetMaxKeeppingElectricBeginTime() {return m_maxKeeppingElectricBeginTime;}

	virtual float GetMaxKeeppingElectricTime() {return m_maxKeeppingElectricTime;}

	inline float GetLeftPadElectricTime() const {return m_leftPadElectricTime;}
	inline float GetRightPadElectricTime() const {return m_rightPadElectricTime;}

	inline ToolOwner GetToolOwner() const { return m_toolOwner;}
	inline void SetToolOwner(ToolOwner owner) {m_toolOwner = owner;}

	/**
		获取释放的钛夹数量，如果当前器械不能释放钛夹或者未释放过钛夹，则返回0
	*/
	inline int GetNumberOfReleasedTitanicClip() {return m_nReleasedTitanicClip;}
	inline void ReleaseOneTitanicClip() {m_nReleasedTitanicClip++;}

	/**
		获取工具夹闭次数
	*/
	inline int GetToolClosedTimes() {return m_toolClosedTimes;}
	inline void SetCanClosed(bool canClosed) {m_canClosed = canClosed;}

	inline float GetMovedTime() {return m_movedTime;}
	inline float GetMovedDistance() {return m_movedDistance;}
	inline unsigned int GetMovedFastestTimes() { return m_moveFastestTimes; }

	inline bool GetElectricAttribute() const { return m_bElectricAttribute; }
	inline void SetElectricAttribute(bool bElectricAttribute) { m_bElectricAttribute = bElectricAttribute; }

	//通电优先级
	inline int GetElecPriority() const { return m_ElecPriority; }
	inline bool IsIgnoreElecPriority() const { return m_IsIgnoreElecPriority; }

	/** 用于判断器械在非操作情况下是否处于闭合状态。当张开的器械分离器官一定时间时，设置该值为true */ 
	inline bool IsClosedInSeparateTime() {return m_IsClosedInSeparateTime;}

	inline void AddCheckObjectForToolElectric(const ToolElectricCheckObject& object) { m_curFrameToolElectricCheckObject.push_back(object);}

	/** 
		获取对象被通电器械所影响的时间 
	*/
	inline float GetElectricAffectTimeForHemoClip()
	{
		float time = 0.f;
		for(std::map<void*,ToolElectricCheckObject>::iterator itr = m_toolElectricCheckObjectMap.begin();itr != m_toolElectricCheckObjectMap.end();++itr)
		{
			ToolElectricCheckObject& checkedObject = itr->second;
			if(checkedObject.GetType() == ToolElectricCheckObject::OT_HemoClip)
				time += checkedObject.GetEffecTime();
		}
		
		return time;
	}

	inline float GetElectricAffectTimeForOrdinaryOrgan()
	{
		float time = 0.f;
		for(std::map<void*,ToolElectricCheckObject>::iterator itr = m_toolElectricCheckObjectMap.begin();itr != m_toolElectricCheckObjectMap.end();++itr)
		{
			ToolElectricCheckObject& checkedObject = itr->second;
			if(checkedObject.GetType() == ToolElectricCheckObject::OT_OrdinaryOrgan)
				time += checkedObject.GetEffecTime();
		}

		return time;
	}

	inline CXMLWrapperTraining * GetTrainingConfig() const { return m_pTrainingConfig; }
	inline void SetTrainingConfig(CXMLWrapperTraining * val) { m_pTrainingConfig = val; }

	inline bool IsClosedInsertion() {return m_isClosedInsertion;}
	inline bool IsInUse() const { return m_bIsInUse; }
	inline void Use() { m_bIsInUse = true; }
	inline void NoUse() { m_bIsInUse = false; }

	inline bool IsDisable() { return m_bDisable; }

	//
	inline bool GetOrganForce() const { return m_bOrganForce; }
	inline void SetOrganForce(bool val) { m_bOrganForce=val; }

	inline float GetForce_X_K() const { return m_fForce_X_K; }
	inline void SetForce_X_K(float val) { m_fForce_X_K=val; }

	inline float GetForce_Y_K() const { return m_fForce_Y_K; }
	inline void SetForce_Y_K(float val) { m_fForce_Y_K=val; }

	inline float GetForce_Z_K() const { return m_fForce_Z_K; }
	inline void SetForce_Z_K(float val) { m_fForce_Z_K=val; }

	virtual void Updates4m(){}
	
    inline bool GetCollision2m() const { return m_bCollision2m; }
    inline void SetCollision2m(bool bCollision2m) { m_bCollision2m = bCollision2m; }


	virtual void SetBackupMaterial(){};
	virtual void warn(){};
	
	//for new train start
	virtual float GetMaxShaftSpeed(){return 1000.0f;}
	virtual void onFrameUpdateStarted(float timeElapsed) = 0;

	virtual void onFrameUpdateEnded() = 0;

	virtual void InternalSimulationStart(int currStep , int TotalStep , float dt) = 0;

	virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt) = 0;

	//virtual void UpdateIntepolatedConvex(float percent)=0;

	virtual bool GetForceFeedBack(Ogre::Vector3 & contactForce , Ogre::Vector3 & dragForce) = 0;

	virtual Ogre::Vector3 GetForceFeedBackPoint() = 0;

	virtual void ClearForceFeedBack() = 0;

	/*when a soft body face element be removed due to cut or other operation*/
	virtual void OnSoftBodyFaceBeDeleted(GFPhysSoftBody * sb , GFPhysSoftBodyFace * face)=0;

	/*when a soft body face created by small one due to cut or other operation*/
	virtual void OnSoftBodyFaceBeAdded(GFPhysSoftBody *sb , GFPhysSoftBodyFace *face) = 0;
	
	virtual void OnSoftBodyNodesBeDeleted(GFPhysSoftBody *sb , const GFPhysAlignedVectorObj<GFPhysSoftBodyNode*> & nodes) = 0;

	virtual Ogre::TexturePtr GetToolBrandTexture()
	{
		return Ogre::TexturePtr();
	}

	virtual void BreakAdhesion() {};
	//for new train end
protected:
	Ogre::SceneNode * m_pNodeKernel;
	Ogre::SceneNode * m_pNodeLeft;
	Ogre::SceneNode * m_pNodeRight;
	Ogre::SceneNode * m_pNodeLeft_a1;
	Ogre::SceneNode * m_pNodeRight_b1;
	Ogre::SceneNode * m_pNodeLeft_a2;
	Ogre::SceneNode * m_pNodeRight_b2;
	Ogre::SceneNode * m_pNodeCenter;
	Ogre::String m_strName;
	Ogre::String m_strType;
	Ogre::String m_strSubType;

	
	bool m_bIsInUse;
	bool m_bDisable;

	Ogre::Vector3 m_v3OriginalLeftPos;
	Ogre::Quaternion m_quatOriginalLeft;
	Ogre::Matrix4 m_mxOriginalLeft;

	Ogre::Vector3 m_v3OriginalRightPos;
	Ogre::Quaternion m_quatOriginalRight;
	Ogre::Matrix4 m_mxOriginalRight;
	
	/// 初始位置，即第一帧的位置
	Ogre::Vector3 m_kernelInitPos;
	Ogre::Vector3 m_curKernelPos;
	Ogre::Vector3 m_v3OriginalKernelPos;
	Ogre::Quaternion m_quatOriginalKernel;
	Ogre::Matrix4 m_mxOriginalKernel;

	float m_nShaftAside;
	//float m_RawShaftAside;//shaft aside with out clamp be min and max
    float m_nMinShaftAside;
	//最大张角
	float m_nMaxShaftAside;
	bool m_bForceRelease;
	//左右张角缩放因子
	float m_leftShaftAsideScale;
	float m_rightShaftAsdieScale;

	bool m_IsTouchingOrgan;
	/// 本次与器官没有接触的时间总和,只针对能闭合的器械
	float m_SeparateTimeWithOrgan;
	/// 用于判断器械在非操作情况下是否处于闭合状态。当张开的器械分离器官一定时间时，设置该值为true
	bool m_IsClosedInSeparateTime;

    bool m_bCollision2m;

	bool m_bCollideFlag;

	bool m_bOrganForce;
	float m_fForce_X_K;
	float m_fForce_Y_K;
	float m_fForce_Z_K;
	ToolSide m_enmSide;

	ToolOwner m_toolOwner;
	float m_totalElectricTime;
	float m_validElectricTime;
	float m_leftPadElectricTime;
	float m_rightPadElectricTime;
	float m_tempElectricBeginTime;
	float m_maxKeeppingElectricBeginTime;
	float m_maxKeeppingElectricTime;
	/// 记录前一帧是否有电
	bool m_hasElectricAtPreFrame;
	/// 被释放的夹子数
	int m_nReleasedTitanicClip;
	/// 夹闭次数
	int m_toolClosedTimes;
	bool m_canClosed;
	/// 是否闭合
	bool m_isClosed;
	/// 是否闭合插入
	bool m_isClosedInsertion;
	bool m_canCheckClosedInsertion;

	//通电优先级
	int m_ElecPriority;
	bool m_IsIgnoreElecPriority;

	float m_TimesAfterLastStatistic;
	float m_movedTime;
	float m_movedDistance;

	bool m_moveFastestState;
	unsigned int	 m_moveFastestTimes;
	//clock_t m_beignMoveFastTime;

	std::vector<ToolElectricCheckObject> m_curFrameToolElectricCheckObject;
	std::map<void*,ToolElectricCheckObject> m_toolElectricCheckObjectMap;

public:
	CXMLWrapperTool * m_pToolConfig;
	CXMLWrapperTraining * m_pTrainingConfig;
	ToolState m_enmState;
	bool m_bElectricButton;
	bool m_bElectricAttribute;
    
	bool m_bElectricLeftPad;		//左踏板是否有电
	bool m_bElectricRightPad;		//右踏板是否有电

	/// 是否真的带电，对于某些器械，踩下踏板并不表示带电，对于此种器械，该属性为false.默认值为false
	bool m_hasRealElectricAttribute;

	bool m_bCheckToolIn;

	Ogre::Vector3 m_pivotPosition;
	Ogre::Vector3 m_lastKernelNodePosition;
	Ogre::Quaternion m_lastKernelNodeQuaternion;
};
