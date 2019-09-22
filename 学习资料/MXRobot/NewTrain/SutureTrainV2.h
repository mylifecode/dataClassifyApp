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

public://�жϸ����Ƿ������Բ�̵������
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
	void DrawMarkRenderTarget();///ÿ֡�����
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
	int  m_TipState;		//��ǰ�ĸ�tip״̬

	MarkTextureState m_MarkState[6];
	bool m_bMarkState;



	SutureNeedleV2*				m_Needle;
	Real						m_HolderAngle;//dec angle Between Holder And Needle 0-90   
	bool						m_HoldPosition;		//ʵʱ��ǰ����λ�� ֻ��¼�������״̬��������ͬʱ�гֲ����㣬��������¼�Ϳ���

	Real						m_InOrganAngle[2];//dec angle Between needle And organ 0-90
	MisMedicOrgan_Ordinary*		m_pNeedleTestOrgan;
	GFPhysRigidBody*			m_pBaseTerrainRigidBody;

	GFPhysSoftBodyNode * m_ballnode0;
	GFPhysSoftBodyNode * m_ballnode1;
	GFPhysSoftBodyNode * m_ballnode2;
	GFPhysSoftBodyNode * m_ballnode3;

	unsigned int     m_index[4];
	SutureSewV2				m_ABSew;			//��ǰ��A B����ϰ
	bool					m_BaseBall[2][2];	//1��2������ǵ��Ƿ񱻴��룬���������������㻹�ǳ����? true�����

	GFPhysSoftBodyFace*		m_InoutFace[2];		//���������λ��
	Real					m_InoutWeights[6];
	int						m_AInOutPos[2];		//������롣������12�� 1��ȷ/ 2������ƫ��/ 3ƫ�ƹ���
	int						m_BInOutPos[2];

	bool					m_MinorToolHold;		//ץǯ
	bool					m_MajorToolHold;		//����ǯ�Ƿ�г�
	bool					m_PreMajorToolHold;		//����Ҳ������ɹ���

	bool					m_NeedleHoldState;		//��ǰ���Ƿ���һ���߼г֣�
	bool					m_PreNeedleHoldState;	//�ϴ��Ƿ����״̬,ʵʱ,falseû�г���

	int						m_TotalHoldCount;	//ȫ�̼������

	int						m_AInCount;			//A��������
	int						m_BInCount;			//B��������

	int						m_RetreatNeedleNum[2][2];	//AB���飬���봦������������봦�������
	// 	bool					m_InNeedleState;			//����״̬ ��������������ʱ��
	// 	bool					m_OutNeedleState;			//����״̬

	bool					m_FirstHoldSuccess;
	bool					m_FuzhuFirstHold;

	bool					m_NeedleLostStateLast;//��һ֡����䴥��״̬
	bool					m_NeedleLostState;//��ǰ����䴥��״̬
	int						m_NeedleLostCount;//��������

};

#endif