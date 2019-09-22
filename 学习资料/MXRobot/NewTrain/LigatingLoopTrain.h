#ifndef _LIGATINGLOOPTRAIN_
#define _LIGATINGLOOPTRAIN_
#include "MisNewTraining.h"
class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class CHarmonicScalpel;

class CLigatingLoopTrain :public MisNewTraining
{
public:
	
	struct TUBEINFO
	{
		TUBEINFO()
		{
			m_pTube = NULL; 
			m_bHasBeenTearOff = false;
			m_bArea1BindComplete = false;
			m_bArea2BindComplete = false;
			m_bArea1CutComplete = false;
			m_bArea2CutComplete = false;
			m_nCurrentBindedRopeCount = 0;
			//m_nPreBindedRopeCount = 0;
			m_pFixPoint = NULL;
			m_pEndPoint = NULL;
			m_fCurrentBleedTime = 0.0f;
			m_fCurrentDisapearTime = 0.0f;
			m_bHasCalc = false;
            m_bindError = false;
            m_bindErrorNum = 0;
		}
		TUBEINFO(MisMedicOrgan_Ordinary *pTube)
		{
			m_pTube = pTube; 
			m_bHasBeenTearOff = false;
			m_bArea1BindComplete = false;
			m_bArea2BindComplete = false;
			m_bArea1CutComplete = false;
			m_bArea2CutComplete = false;
			m_nCurrentBindedRopeCount = 0;
			//m_nPreBindedRopeCount = 0;
			m_pFixPoint = NULL;
			m_pEndPoint = NULL;
			m_fCurrentBleedTime = 0.0f;
			m_fCurrentDisapearTime = 0.0f;
			m_bHasCalc = false;
            m_bindError = false;
            m_bindErrorNum = 0;
		}
		//���������ϵ����в����Ƿ����
		inline bool MissonComplete()
		{
			return m_bArea1BindComplete && m_bArea2BindComplete && m_bArea1CutComplete && m_bArea2CutComplete;
		} 
		//��ӳ��ϵ�
		void AddTearOffPoint(const int& index)
		{
			m_vecTearOffPoint.push_back(m_pTube->m_physbody->GetNode(index));
		}
		//���ù̶�������ɶ˶˵�
		void SetFixAndEndPoint(const int& fixIndex, const int& endIndex)
		{
			m_pFixPoint = m_pTube->m_physbody->GetNode(fixIndex);
			m_pEndPoint = m_pTube->m_physbody->GetNode(endIndex);
		}

		//��ӹ̶���
		void AddFixPoint(const int& fixPtIndex)
		{
			m_vecFixPoint.push_back(m_pTube->m_physbody->GetNode(fixPtIndex));
		}

		//�����Ҫ��ֹ������ײ�ĵ�
		void AddDisableRRCollisionPoint(const int& RRCollisonPoint)
		{
			m_vecDisableRRCollisonPoint.push_back(m_pTube->m_physbody->GetNode(RRCollisonPoint));
		}


		MisMedicOrgan_Ordinary *m_pTube;
		bool m_bArea1BindComplete;											//����1��Ȧ�ɹ�
		bool m_bArea2BindComplete;											//����2��Ȧ�ɹ�
		bool m_bArea1CutComplete;											//����1���ϳɹ�
		bool m_bArea2CutComplete;											//����2���ϳɹ�
		bool m_bHasBeenTearOff;												//�Ƿ񱻳���
		bool m_bHasCalc;
		int m_nCurrentBindedRopeCount;										//Ѫ���ϵĵ�ǰ���Ӹ���
		//int m_nPreBindedRopeCount;											//Ѫ���ϵ�֮ǰ���Ӹ���
		float m_fCurrentBleedTime;											//��ǰ��Ѫʱ��
		float m_fCurrentDisapearTime;										//��ǰ��ʧʱ��

		GFPhysSoftBodyNode* m_pFixPoint;									//�̶���
		GFPhysSoftBodyNode* m_pEndPoint;									//���ɶ˶˵�
		std::vector<GFPhysSoftBodyNode*> m_vecTearOffPoint;					//���ϵ�
		std::vector<GFPhysSoftBodyNode*> m_vecFixPoint;						//�̶���
		std::vector<GFPhysSoftBodyNode*> m_vecDisableRRCollisonPoint;		//��Ҫ��ֹ������ײ�ĵ�

        bool m_bindError;
        int m_bindErrorNum;
		
	};

	CLigatingLoopTrain(void);
	virtual ~CLigatingLoopTrain(void);
    int m_nsumBindCount;
    void JudgeRopeCutted();															//�жϼ����Ƿ�����˵ط�

private:
	int	 BFSLinkedAera(int index);													//������ȱ���������ͨ����,����ֵΪ��ͨ���������
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	void SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ);

	virtual bool Update(float dt);
	bool ReadTrainParam(const std::string& strFileName);							//��ȡѵ����ز���
	void LogicProcess();															//�߼��ж�
	void JudgeRopeBindOnRightArea(MisMedicOrgan_Ordinary* pTube);					//�жϴ���Ƿ����˵ط�
	void ChangeTexture(const int& flag);											//�ı���ͼ����,flag=0:������ͼ���ó��ޱ������,flag=1:��ǰѪ�ܱ�ǳ�����
	int UserHasClamTube();															//�Ƿ�ץסѪ��,�������ĸ�Ѫ�ܱ�ץס
	bool IsTearOffTheTube(const int& index);										//�ж��Ƿ񳶶�Ѫ��
	bool ShouldShowArea(const int& index);											//�ж��Ƿ���ʾ�������
	void TearOffTheTube(const int& index);
	virtual void OnToolCreated(ITool * tool, int side);
    void OnSaveTrainingReport();

private:
	Ogre::TexturePtr m_TubeTexturPtr[3];											//Ѫ����ͼָ��

	int	m_currentStep;
    int m_nSumBindedRopeCount;
	std::vector<std::vector<GFPhysSoftBodyNode*> > m_vecTearOffPoint;			    //���ϵ�
	bool m_bFinished;

    
	float m_area0,m_area1,m_area2, m_area3;
	int	m_counter;																	//֡��������

	int m_LastTexChangeFlag;
	int m_LastChangeTubeIndex;
	std::map<int, MisMedicOrgan_Ordinary*> m_Objects;

	std::map<int, MisMedicDynObjConstructInfo> m_ConstructInfoMap;

	std::vector<TUBEINFO> m_vecTube;
	std::vector<std::set<GFPhysSoftBodyNode*> > m_vecClusterResult;
	std::vector<GFPhysVectorObj<GFPhysSoftBodyTetrahedron*>> m_vecClusterTetrahedron;
	float m_fBleedTimeThreshold;													//��Ѫʱ����ֵ
	float m_fDisapearTimeThreshold;													//��ʧʱ����ֵ

	//�߼���ر���
	int m_nCurrentTubeIndex;														//��ǰ���ڲ�����Ѫ��
	float	m_fTearOffThreshold;													//���ϳ�����ֵ
	float	m_fShowAreaThreshold;													//��ʾ�������ĳ�����ֵ
	std::vector<int> m_bTubeBeTearOff;												//Ѫ���Ƿ񱻳���
	bool m_bShowLigatingErrorTip;													//�Ƿ���ʾ����������� 
	bool m_bPerfect;
};

#endif