#ifndef _SUTURETRAINV2_
#define _SUTURETRAINV2_
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
//#include "MisMedicOrganTube.h"
#include "TrainScoreSystem.h"
#include "CustomConstraint.h"
class MisMedicOrgan_Ordinary;
class MisCTool_PluginClamp;
class CTool;
class SutureNeedleV2;
class MisCTool_PluginRigidHold;

//=============================================================================================
class CSutureTrainV2 : public MisNewTraining, public NeedleActionListenerV2
{

public:
	CSutureTrainV2(void);

	virtual ~CSutureTrainV2(void);

public://判断钢针是否掉落在圆盘的面积上
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual void SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ);
	bool Update(float dt);
	void DetectNeedleLost();
	void DetectNeedleLostCreate();
	virtual void KeyPress(QKeyEvent * event);
	void InternalSimulateStart(int currStep, int TotalStep, Real dt);
	void InternalSimulateEnd(int currStep, int TotalStep, Real dt);
	void InitBallPos();
	void UpdateInOutBallPos();
	bool GetBaseBallPos(const int i, GFPhysVector3& pos);

	void OnCreateInAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
	void OnCreateOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
	void OnRemoveInAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
	void OnRemoveOutAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
	void OnSwitchAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);
	void OnMovingRopeAnchor(const SutureThreadV2 * thread, const int index, const Real weights[]);
	void OnRemoveRopeAnchor(const GFPhysSoftBodyFace* face, const Real weights[]);

	virtual void OnCollisionKeep(GFPhysCollideObject * rigidA, GFPhysCollideObject * rigidB, const GFPhysManifoldPoint * contactPoints, int NumContactPoints);
	void OnSaveTrainingReport();
	void DrawMarkRenderTarget();///每帧都清空
	void OnSimpleUIEvent(const SimpleUIEvent & event);
private:
	void GrabNeedleState();
	MisCTool_PluginRigidHold* GrabNeedleStateInternal(CTool* tool);
	void CalcHolderAngle(MisCTool_PluginRigidHold * rigidholdPlugin);

private:
	enum SutureSewV2
	{
		ASew,
		BSew,
		SewDone
	};

	bool m_GraspNeedleInit;		//TipState  1
	bool m_NeedleInStart;		//			2
	bool m_NeedleInDegree;		//			3
	bool m_NeedleInPositionTip; //			4
	bool m_NeedleOutPositionTip;
	bool m_PullNeedleOut;
	bool m_NeedleInRepeatErrorTip;
	bool m_NeedleOutRepeatErrorTip;
	bool m_GraspNeedleCarefully;
	bool m_GraspNeedleAgain;
	bool m_FirstPassFinish;		//			11		    
	int  m_TipState;		//当前哪个tip状态

	MarkTextureState m_MarkState[6];
	bool m_bMarkState;



	SutureNeedleV2*				m_Needle;
	Real						m_HolderAngle;//dec angle Between Holder And Needle 0-90   
	bool						m_HoldPosition;		//实时当前夹针位置 只记录最后合理的状态，两个针同时夹持不计算，保持最后记录就可以

	Real						m_InOrganAngle[2];//dec angle Between needle And organ 0-90
	MisMedicOrgan_Ordinary*		m_pNeedleTestOrgan;
	GFPhysRigidBody*			m_pBaseTerrainRigidBody;

	GFPhysSoftBodyNode * m_ballnode0;
	GFPhysSoftBodyNode * m_ballnode1;
	GFPhysSoftBodyNode * m_ballnode2;
	GFPhysSoftBodyNode * m_ballnode3;

	unsigned int     m_index[4];
	SutureSewV2				m_ABSew;			//当前是A B组练习
	bool					m_BaseBall[2][2];	//1、2两个标记点是否被串入，如果被串入是入针点还是出针点? true入针点

	GFPhysSoftBodyFace*		m_InoutFace[2];		//入针出针球位置
	Real					m_InoutWeights[6];
	int						m_AInOutPos[2];		//入针出针。并不是12球。 1正确/ 2有少量偏移/ 3偏移过大
	int						m_BInOutPos[2];

	bool					m_MinorToolHold;		//抓钳
	bool					m_MajorToolHold;		//持针钳是否夹持
	bool					m_PreMajorToolHold;		//不用也可以完成功能

	bool					m_NeedleHoldState;		//当前针是否被任一工具夹持，
	bool					m_PreNeedleHoldState;	//上次是否持针状态,实时,false没有持针

	int						m_TotalHoldCount;	//全程夹针次数

	int						m_AInCount;			//A组进针次数
	int						m_BInCount;			//B组进针次数

	int						m_RetreatNeedleNum[2][2];	//AB两组，入针处退针次数，出针处退针次数
	// 	bool					m_InNeedleState;			//入针状态 入针持续在肉里的时候
	// 	bool					m_OutNeedleState;			//出针状态

	bool					m_FirstHoldSuccess;
	bool					m_FuzhuFirstHold;

	bool					m_NeedleLostStateLast;//上一帧针掉落触地状态
	bool					m_NeedleLostState;//当前针掉落触地状态
	int						m_NeedleLostCount;//针掉落次数

};

#endif