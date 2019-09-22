#pragma once
#include "BasicTraining.h"
#include "OgreSceneManager.h"
//#include "CollisionTest.h"
#include "MisNewTraining.h"
#include "MisMedicOrganOrdinary.h"
#include <vector>
#include <set>

class OrganSurfaceBloodTextureTrack;


class SYStopBleedTrain : public MisNewTraining
{
public:
	class BleedPoint//��Ѫ��
	{
	public:
		BleedPoint(MisMedicOrgan_Ordinary * organ ,GFPhysSoftBodyFace * face, float weights[], float radiuscale, float gravAcc);
		
		void BeginBleed();

		void OnBleedStopped(float timeAtStop);

		OrganSurfaceBloodTextureTrack * m_BloodTrack;
		GFPhysSoftBodyFace * m_bFace;
		MisMedicOrgan_Ordinary * m_pOrgan;
		float m_Weights[3];
		float m_radiusScale;
		float m_gravAcc;

		//train logic data
		bool  m_StopSucced;
		float m_StopTime;
		float m_ContinueBurnTime;
		
	};
	class BleedBatch//һ���Ѫ��
	{
	public:
		BleedBatch();

		void AddPointToBleed(MisMedicOrgan_Ordinary * organ, GFPhysSoftBodyFace * face, float weights[], float radiuscale, float gravAcc);

		int  FindBleedPointByTrack(OrganSurfaceBloodTextureTrack * bloodtrack);

		int  GetNumBleedPointFinished()
		{
			int num = 0;
			for (int c = 0; c < m_bPoints.size(); c++)
			{
				if (m_bPoints[c].m_StopSucced)
					num++;
			}
			return num;
		}

		bool IsAllBleedPointBeStopped()
		{
			int num = 0;
			for (int c = 0; c < m_bPoints.size(); c++)
			{
				if (m_bPoints[c].m_StopSucced)
					num++;
			}
			return (num == m_bPoints.size());
		}
		void Active(float Time);
		
		void Deactive(float Time);
	
		std::vector<BleedPoint> m_bPoints;

		//
		bool  m_BurnPointDeviated;
	protected:

		float m_BatchStartTime;
		
		float m_BatchEndTime;
		
		bool  m_isActive;
		
		
	};

	SYStopBleedTrain();
	
	virtual ~SYStopBleedTrain(void);
	
	virtual bool Update(float dt);
	

public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	void BuildBleedBatches();

	void OnHandleEvent(MxEvent* pEvent);
	void OnTrainingIlluminated();
	void OnSaveTrainingReport();

	SYScoreTable * GetScoreTable();
private:
	//bool AddBleedPoint(float radiusScale, float slipAcce);
	//ʱ���ۼ�
	void CheckTool(CTool * pTool, float dt, std::set<OrganSurfaceBloodTextureTrack*> & processedTrack);

	OrganSurfaceBloodTextureTrack* CreateBloodTrack(GFPhysSoftBodyFace * pFace, float weights[3], float radiusScale, float slipAcce);

	void TearOrgan(CElectricHook * pTool,const GFPhysSoftBodyFace * pFace);

private:

	MisMedicOrgan_Ordinary * m_pOrgan;

	enum ElectricState
	{
		NoneElec,					//
		ElectricNotBleedPoint,	//�絽����Ѫ�� 
		ElectricBleedPoint,		//�絽��ǰѪ������Ѫ��
	};
	ElectricState m_electricState;								//��ǰ�����ٵ�״̬

	float m_burnRadius;											//�յİ뾶

	enum StepState	
	{
		NoneStep,
		NeedAdd,			//��Ҫ���һ����ѪЧ��
		WaitRemove,			//�ȴ��Ƴ�״̬
		Finish
	};
	StepState m_currStepState;									//default is None

	std::vector<GFPhysSoftBodyFace *> m_bleedFaces;				//��¼��Ѫ����Ƭ
	//std::vector<bool> m_bBleeds;								//��־��ǰ���Ƿ�Ϊ��Ѫ״̬��false��Ϊ��Ѫ״̬
	//int m_currBleedIndex;										//��ǰ��ӵ�Ѫ������
	bool m_bAllowAddBleedPoint;									//�Ƿ���������һ����Ѫ��
	//OrganSurfaceBloodTextureTrack * m_distBloodTrack;			//��ǰѪ��
	
	float m_LastTime;

	std::map<GFPhysSoftBodyNode*,float > m_nodeTimeMap;			//��������ʱ��
	std::map<OrganSurfaceBloodTextureTrack*,float> m_bloodTrackTimeMap;	//Ѫ���ѱ�������ʱ��

	std::vector<BleedBatch> m_BleedBatches;
	int m_CurrActiveBatch;

	//float m_lastTimeBegin;

	float m_trainbegintime;
	float m_trainendtime;
	float m_eleTime[10];
	int m_elePosErrCount[10];
	bool m_bSingleEleTime5s;

	
};
