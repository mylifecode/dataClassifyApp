#pragma once
#include "MisNewTraining.h"
class MisMedicOrgan_Ordinary;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class MisCTool_PluginClamp;
class CTool;





class CBasicNewTraining_Level12 : public MisNewTraining
{
	//���ٽṹ��
	struct ObjectOperationInfo
	{
		enum State{OkOperate,FailOperate,WaitOperate,NotOperate};		//����״̬
		enum Type{Unknow,Small,Ordinary,Big};							//��������,Unknow<-->δ֪��Small<-->ϸѪ�ܣ�Ordinary<-->����Big<-->��Ѫ��
		enum BreakType{Tear = 0 , BurnWrong , BurnRight};                   //Ѫ�ܶϵ����� 0 - ���� 1 - ����ڷǱ�Ǵ�  2- ����ڱ�Ǵ�
		ObjectOperationInfo()
		{
			state = NotOperate;
			type = Unknow;
			organ = NULL;

			topNodeIndex = 20;
			midNodeIndex = 44;
			downNodeIndex = 21;

			breakRange = 0.7f;
			breakThreshold = 1.0f;
			bConnected = true;
			pTopNode = NULL;
			pMidNode = NULL;
			pDownNode = NULL;

			burnNeedTime = 3.0f;
			curBurnTotalTime = 0.f;
			burnBreakRange = 0.8;
			m_pCurrentBurnFace = NULL;
			fBurnErrorTime = 0.0f;
			m_BreakType = Tear;
			m_HasBeDissectioned = false;
		}
		
		//���ô�Ѫ�ܵĸ�������
		void setBigVesselParameter(MisMedicOrgan_Ordinary * pOrgan)
		{
			if(pOrgan == NULL)
				throw "pOrgan != NULL";
			organ = pOrgan;
			type = ObjectOperationInfo::Big;
			state = ObjectOperationInfo::NotOperate;
			breakRange = 1.2f;
			burnBreakRange = 0.68f;
			breakThreshold = 1.8f;
			setPhysicsNode();
		}

		//����ϸѪ�ܵĸ�������
		void setSmallVesselParameter(MisMedicOrgan_Ordinary * pOrgan)
		{
			if(pOrgan == NULL)
				throw "pOrgan != NULL";
			organ = pOrgan;
			type = ObjectOperationInfo::Small;
			state = ObjectOperationInfo::WaitOperate;
			breakRange = 0.9f;
			breakThreshold = 1.0f;
			burnBreakRange = 0.58f;
			setPhysicsNode();
			fStopBleedTimeThreshold = 0.0f;
			fBleedTime = 0.0f;
		}

		//����ֹѪʱ��
		inline void setStopBleedTimeThreshold(float threshold){fStopBleedTimeThreshold = threshold;}

		GFPhysSoftBodyNode * getTopNode()
		{
			return pTopNode;
		}
		
		GFPhysSoftBodyNode * getMidNode()
		{
			if(organ && pMidNode)		//���pMidNode�Ѿ�ΪNULL�����������
			{
				for(int n = 0;n< (int)organ->m_OrganRendNodes.size();++n)
				{
					if(organ->m_OrganRendNodes[n].m_PhysNode == pMidNode)
						return pMidNode;
				}
			}
			return NULL;
		}

		GFPhysSoftBodyNode * getDownNode()
		{
			return pDownNode;
		}

		//�����ͬһ���������ϵ��г���3����Ѫ�ܵ�ϣ��������¼�ʱ
		bool addFaceBurnTime(const GFPhysSoftBodyFace* pBurnFacce, float burnTime)
		{
			if (curBurnTotalTime >= burnNeedTime)
			{
				return true;
			}
			curBurnTotalTime += burnTime;
			return false;
		}

		//���õ������������ʱ��
		void resetBurnTime()
		{
			m_pCurrentBurnFace = NULL;
			curBurnTotalTime = 0.0f;
		}
	private:
		void setPhysicsNode()
		{
			if(organ == NULL)
				throw "pOrgan == NULL";
			pTopNode = organ->m_physbody->GetNode(topNodeIndex);
			pMidNode = organ->m_physbody->GetNode(midNodeIndex);
			pDownNode = organ->m_physbody->GetNode(downNodeIndex);
		}
	public:
		State state;					//����״̬
		Type type;						//��������
		int topNodeIndex;				//�ϱ�������Node����
		int midNodeIndex;		
		int downNodeIndex;				//�±�������Node����
		float breakThreshold;			//������ֵ
		float breakRange;				//���Ϸ�Χ
		float burnBreakRange;			//�նϵķ�Χ
		bool  bConnected;				//Ѫ���Ƿ�Ϊ�����Ķ�û�б�����
		BreakType   m_BreakType;
		float fStopBleedTimeThreshold;	//ֹѪʱ����ֵ
		float fBleedTime;				//����Ѫ��ʱ��
		float fBurnErrorTime;			//û�絽��ɫ���λ�õ�ʱ��
		MisMedicOrgan_Ordinary * organ;	
		bool m_HasBeDissectioned;
	private:
		GFPhysSoftBodyNode * pTopNode;	//�Ϲ̶���
		GFPhysSoftBodyNode * pMidNode;	//�м��
		GFPhysSoftBodyNode * pDownNode;	//�¹̶���

		//burn info

		float burnNeedTime;				//�ն�����ʱ��
		float curBurnTotalTime;			//��ǰ�����������ղ�����ʱ��
		std::map<GFPhysSoftBodyNode*,float> nodeToBurnTimeMap;		//Ѫ�ܽڵ�������ʱ���ӳ��
		GFPhysSoftBodyFace* m_pCurrentBurnFace;						//��ǰ���ڱ����յ���
		
	};
public:
	CBasicNewTraining_Level12(void);
	~CBasicNewTraining_Level12(void);
public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual bool Update(float dt);

private:
	std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > GetFacePairBetweenBigTubeAndSmallTube(MisMedicOrgan_Ordinary* bigTube, MisMedicOrgan_Ordinary* smallTube);
	void Init();
	bool IsElectricBreakOnGreenArea(MisMedicOrgan_Ordinary* pOgran, const GFPhysSoftBodyFace* pFace , float weights[3]);								//�ж��Ƿ����˱������
	void SetCurrentOperateTubeIndex(int index);																											//���������ɫ�������
	int GetOrganID(MisMedicOrgan_Ordinary* pOrgan);																					//��������ָ�뷵��id
	MisCTool_PluginClamp* GetToolPlugin(CTool* tool);
	void UpdateBurn(CTool * pTool,float dt);																					
	void updateClampedObject(MisMedicOrgan_Ordinary* pOrgan,MisCTool_PluginClamp * pPluginClamp);
	bool BreakVessel(MisMedicOrgan_Ordinary * pOrgan,ObjectOperationInfo * pObjectInfo,GFPhysSoftBodyNode * attachedNode,float range = 0.7f,bool bFirstBreak = false);	//����Ѫ��
    virtual void FacesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces, GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & createdfaces, MisMedicOrgan_Ordinary * orgn);

	bool ReadTrainParam(const std::string& strFileName);		//��ȡѵ����ز���
	void DealWithElecCutObject(MisMedicOrgan_Ordinary * pOrgan);																										//��Ѫ��
	friend void NewTrainingHandleEvent12(MxEvent*,ITraining*);
    void OnSaveTrainingReport();
	
	SYScoreTable * GetScoreTable();

private:

	typedef std::map<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*>, GFPhysPositionConstraint*> FFCsMap;

	std::map<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*>, GFPhysPositionConstraint*> m_mapLeftFrontConstraint;
	std::map<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*>, GFPhysPositionConstraint*> m_mapLeftBackConstraint;
	std::map<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*>, GFPhysPositionConstraint*> m_mapRightFrontConstraint;
	std::map<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*>, GFPhysPositionConstraint*> m_mapRightBackConstraint;

	typedef std::map<MisMedicOrgan_Ordinary*,ObjectOperationInfo*> OrganToObjectInfoMap;

	OrganToObjectInfoMap m_organToObjectInfoMap;
	MisMedicOrgan_Ordinary * m_pSphereOrgan;
	MisMedicOrgan_Ordinary * m_pLeftBigOrgan;					//����Ѫ��
	MisMedicOrgan_Ordinary * m_pRightBigOrgan;					//�Ҳ��Ѫ��
	MisMedicOrgan_Ordinary * m_pSkinOrgan;

	MisMedicOrgan_Ordinary * m_pSmallVesselLeftFront;			//���ǰСѪ��
	MisMedicOrgan_Ordinary * m_pSmallVesselLeftBack;			//����СѪ��
	MisMedicOrgan_Ordinary * m_pSmallVesselRightFront;			//�Ҳ�ǰСѪ��
	MisMedicOrgan_Ordinary * m_pSmallVesselRightBack;			//�Ҳ��СѪ��

	std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > m_vecFacePairLeftFront;				//���Ѫ������ǰСѪ�ܵ����
	std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > m_vecFacePairLeftBack;					//���Ѫ�������СѪ�ܵ����
	std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > m_vecFacePairRightFront;				//�Ҵ�Ѫ������ǰСѪ�ܵ����
	std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > m_vecFacePairRightBack;				//�Ҵ�Ѫ�������СѪ�ܵ����


	std::vector<std::pair<GFPhysSoftBodyNode*,GFPhysSoftBodyNode*> > m_vecPointPairLeftFront;				//���Ѫ������ǰСѪ�ܵ����
	std::vector<std::pair<GFPhysSoftBodyNode*,GFPhysSoftBodyNode*> > m_vecPointPairLeftBack;				//���Ѫ�������СѪ�ܵ����
	std::vector<std::pair<GFPhysSoftBodyNode*,GFPhysSoftBodyNode*> > m_vecPointPairRightFront;				//�Ҵ�Ѫ������ǰСѪ�ܵ����
	std::vector<std::pair<GFPhysSoftBodyNode*,GFPhysSoftBodyNode*> > m_vecPointPairRightBack;				//�Ҵ�Ѫ�������СѪ�ܵ����




	float m_fBleedEclpseTime;									//�ۼ���Ѫ��ʱ��

	bool m_bFinish;


	std::map<int, MisMedicOrgan_Ordinary*> m_Objects;
	std::vector<int> m_vecLastTubeIndex;						//ʣ��û����ϵ�Ѫ��ID
	int m_iCurrentNeedToBurnTubeId;								//��ǰ��Ҫ����ε�Ѫ��ID
	int m_nCutSmallTubeCount;									//Ū��СѪ�ܵ�����
	int m_nCutBigTubeCount;										//Ū�ϴ�Ѫ�ܵ�����
	int m_nTeafOffSmallTubeCount;								//���ϵ��ĸ�СѪ�ܵ�����
	int m_count;												//���ѵ��ʱ�����ӳٵ���
	float m_fTipsAliveTimeThreshold;							//��������Ͳ����ɹ���ʾ��Ĵ��ʱ����ֵ
	float m_fCurrentTipsAliveTime;								//��ǰ��ʾ����ʱ��
	bool m_bHasTipAlive;										//�Ƿ�����ʾ����
	float m_fGreenUVAreaBegin;									//��ɫ�������uv��Χ����ʼֵ
	float m_fGreenUVAreaEnd;									//��ɫ�������uv��Χ���յ�ֵ
	Ogre::TexturePtr m_CurrMarkTex;                             //��ǰ�ı����ͼ
	const int m_nSmallTubeStep;									//СѪ�ܵķֶ�������ʼֵΪ10
	bool m_bLeftBackTubeBreak;									//���СѪ�ܶ���
    bool m_bShpereBeenClamped;

	int m_SphereBeClampedNum;
};
