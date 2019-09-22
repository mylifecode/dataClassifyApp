#ifndef _SYCUTTINGTUBETRAIN_
#define _SYCUTTINGTUBETRAIN_
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"

#include "TrainScoreSystem.h"
#include "SmokeManager.h"
#include "VesselBleedEffect.h"
class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;
class MisMedicRigidPrimtive;

class VeinConnectObject;
class CElectricHook;
class CScissors;
class CHarmonicScalpel;
class SceneSpeciBag;

class SYCuttingTubeTrain :public MisNewTraining
{
public:
	class GradLogicData
	{
	public:
		GradLogicData(MisNewTraining * train);

		void Reset();

		void SendStepDetailScoreAndReset();

		void SetCutOff();

		void SetCutWrongPlace();

		void SetGraspInRightPlace();

		void SetThrowSucceed();

		void SetBeTeared();

		bool HasBeTeared()
		{
			return m_IsTearOff;
		}
		Ogre::Vector2 m_ValidGraspTexRange;
		int m_NumSegmentThrowSucced;
		//float m_continElecTime;
		float m_MaxContinueElecTime;
		//bool  m_IsInElec;
	protected:
		void Dirty();

		bool m_IsCutOff;//�Ƿ����
		bool m_IsCutInWrongPlace;
		bool m_IsGraspInRightPlace;
		bool m_IsThrowBageSucceed;

		

		bool m_HasSubmitItem;
		MisNewTraining * m_train;
		
		bool m_IsTearOff;

		int m_CutNum;

		
	};

	SYCuttingTubeTrain(void);
	virtual ~SYCuttingTubeTrain(void);

	GradLogicData & GetGradLogicData()
	{
		return m_gradLogicData;
	}
public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	SYScoreTable * GetScoreTable();
	virtual bool Update(float dt);
	virtual void OnSSContactBuildFinish(const GFPhysAlignedVectorObj<GFPhysSSContactPoint> & ssContact, int numSSContact);

	virtual GFPhysVector3 GetTrainingForceFeedBack(ITool* tool);
	void UpdateColor(MisMedicOrgan_Ordinary * m, float area0, float area1);
	
	int	 BFSLinkedAera();										//������ȱ���������ͨ����,����ֵΪ��ͨ���������
	bool IsCutOnRightPlace();									//�Ƿ����λ��
	void ProcessAfterCut();										//���������к�Ĵ���
	void OnSaveTrainingReport();
	//void OnHandleEvent(MxEvent* pEvent);
private:
	bool ReadTrainParam(const std::string& strFileName);		//��ȡѵ����ز���
	bool IsThrowInBag();										//�Ƿ��ӽ�����
	bool LogicProcess(float dt);								//�߼��жϺ���
	void InitStep();											//��ʼ���߼�����
	void HideCutedPartOrgan();									//���ر��������Ĳ���Ѫ��
	void ChangeBagPosition();									//�ı���ӵ�λ��
	bool PosInBagAABB(const GFPhysVector3& pos);				//��������Ѫ�ܲ��ֵĽڵ�λ���Ƿ��ڴ��ӵ�AABB��
	bool PosInGround(const GFPhysVector3& pos);					///��������Ѫ�ܲ��ֵĽڵ�λ���Ƿ����ڵ���
	bool TubeHasCutCompeletly();								//�ж�Ѫ���Ƿ���ȫ����
	bool IsTearOffTheTube();									//�ж��Ƿ񳶶�Ѫ��
	void TearOffTheTube(bool graspOnRightPlace);				//����Ѫ��
	void BFSTearOffPointTetrahedron(GFPhysSoftBodyTetrahedron* tetra, int level, GFPhysVectorObj<GFPhysSoftBodyTetrahedron*>& result);		//������ȱ�����ʼ������ֻҪ������������������������
	void LoadLevel1TearOffPoint();								//��ȡ����1�ĳ��ϵ�
	void LoadLevel2TearOffPoint();								//��ȡ����2�ĳ��ϵ�
	//bool IsGraspOnRightPlace();									//ץǯ�Ƿ�ץ������ȷ�ĵط�
	void AfterCutTubeCompletely();								//��ȫ����Ѫ�ܺ�
	void AddObjectToWorld(int id);								//������
	void RemoveObjectFromWorld(int id);							//ɾ�����
	void EnterLevel1();											//���볡��1
	void LeaveLevel1();											//�˳�����1
	void EnterLevel2();											//���볡��2
	void LeaveLevel2();											//�˳�����2
	bool PlayerHasGraspTube();									//�Ƿ�ץס��Ѫ��
	void ScaleTheBag();											//���Ŵ���
	void ChangeTexture(int flag);								//ץ��Ѫ��ʱ�Ž���ɫ���λ����ʾ����,flag=0��ʾ�ĳ��к�ɫ��ǵ�ͼ��flag=1��ʾ�ĳ��޺�ɫ��ǵ�ͼ
	bool IsClampInTheSky();										//�Ƿ�Ѫ��ץ��
	bool IsGraspOnRightPlace(Ogre::Vector2 validTexURange);
	void AddBagToWorld(int posIndex);							//��Ӵ���
	void RemoveBagFromWorld();									//ɾ������
	void RemoveAllBagFromWorld();								//ɾ�����д���
	void OnTimerTimeout(int timerId, float dt, void* userData);

	std::vector<trainingStep> m_steps;
	int	m_currentStep;

	int m_step;													//�����������ڵķֶ�������ʼֵΪ21
	int m_iClusterNum;											//Ѫ�ܱ��ֳɵĶ��� ���������ɵĶ���

	std::map<int, MisMedicOrgan_Ordinary*> m_Objects;

	std::map<int, MisMedicDynObjConstructInfo> m_ConstructInfoMap;


	float m_fEnterNextLevelTimeThreshold;						//������һ������ʱ����ֵ
	float m_fCurrentTime;										//��ǰ��ʱ
	bool m_bScaleBag;											//�Ƿ����ս�����



	float	m_area0, m_area1;
	float m_fScale;												//���ڸı�uv

	int   m_LastTexChangeFlag;

	float m_LastTexChangeOffset;

	MisMedicOrgan_Ordinary *m_pTerrain;							//��Ƥ
	MisMedicOrgan_Ordinary *m_pTube;							//Ѫ��
	SceneSpeciBag *m_pBag;								//����
	std::set<GFPhysSoftBodyNode*> m_TubeNodesContactTerrain;
	//MisMedicRigidPrimtive * m_CupCol;

	//Ogre::SceneNode* m_pBagForScale;							//��̬����
	//Ogre::SceneNode* m_pBagCircle;

	std::vector<std::set<GFPhysSoftBodyNode*> > m_vecClusterResult;
	std::vector<GFPhysVectorObj<GFPhysSoftBodyTetrahedron*>> m_vecClusterTetrahedron;
	Ogre::TexturePtr m_TubeTexturPtr;							//Ѫ����ͼָ��
	GFPhysSoftBodyNode* m_pFixPoint;							//�̶���
	GFPhysSoftBodyNode* m_pEndPoint;							//���ɶ˶˵�
	std::vector<GFPhysSoftBodyNode*> m_vecTearOffPoint;			//���ϵ�
	std::vector<int> m_vecTube1TearOffPtID;						//��һ��������Ѫ�ܵĳ��ϵ�ID
	std::vector<int> m_vecTube2TearOffPtID;						//�ڶ���������Ѫ�ܵĳ��ϵ�ID
	int m_iCurrentBagPosIndex;									//��ǰ����λ������
	std::vector<Ogre::Vector3> m_vecBagPosition;				//���ӵ�λ��
	float m_fTearOffThreshold;									//�ж����ϵ���ֵ
	bool m_bCompeletlyCut;										//Ѫ����ȫ������
	int m_CutCount;												//��ȷ������
	bool m_bPerfectInLevel1;									//����1���Ƿ������������
	bool m_bPerfectInLevel2;									//����2���Ƿ������������
	bool m_bCutTestPass;										//�����Ƿ���ȷ
	float m_fBagScaleX;											//��ǰ�걾��X������
	float m_fBagScaleY;											//��ǰ�걾��Y������
	float m_fBagScaleZ;											//��ǰ�걾��Z������
	int m_throwErrorCount;										//Ͷ��ʧ�ܵĴ���
	bool m_bHasGraspTube;										//�Ƿ�ץסѪ��
	bool m_bNeedEnterNextLevel;									//�Ƿ������һ����
	bool m_bScaling;											//�����Ƿ���������
	bool m_bNeedThrowInBag;										//�Ƿ���Ҫ��Ѫ���ӽ�������
	bool m_bPreInGround;								//���ڱ����ظ�����Ͷ��ʧ�ܵı��
	bool m_bInGround;									//���ڱ����ظ�����Ͷ��ʧ�ܵı��
	bool m_bShouldShowCutArea;									//�Ƿ�Ӧ����ʾ��ɫ��������
	bool m_bHasBeenTearOffTube;
	int m_iCurrentBagID;										//��ǰ����ID
	float m_fCutRightRatio;										//����������ɫ�����ڵı�ֵ	
	float m_fXScalePerFrame;									//��������ʱ��X�᷽��ÿ֡���ŵ���
	float m_fZScalePerFrame;									//��������ʱ��Z�᷽��ÿ֡���ŵ���

	bool  m_usedDone[6];										///true�ôμ��в������ˣ�false �ôβ���û��
	float m_showGreenTime[6];
	int   m_cuttedPosCorrect[6];								///0 û���У�1 ����λ����ȷ��2 ���д���
	float m_cuttedPosTime[6];
	bool  m_isTearoff[6];										///true����
	int   m_fallCount[6];										///����������
	int   m_currentCut;										///��ǰ����ִ���ĸ����в���

	float m_trainbegintime;

	float m_TagOffsetPercent;

	
	GradLogicData m_gradLogicData;
};

#endif