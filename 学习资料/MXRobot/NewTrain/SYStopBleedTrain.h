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
	class BleedPoint//出血点
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
	class BleedBatch//一组出血点
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
	//时间累计
	void CheckTool(CTool * pTool, float dt, std::set<OrganSurfaceBloodTextureTrack*> & processedTrack);

	OrganSurfaceBloodTextureTrack* CreateBloodTrack(GFPhysSoftBodyFace * pFace, float weights[3], float radiusScale, float slipAcce);

	void TearOrgan(CElectricHook * pTool,const GFPhysSoftBodyFace * pFace);

private:

	MisMedicOrgan_Ordinary * m_pOrgan;

	enum ElectricState
	{
		NoneElec,					//
		ElectricNotBleedPoint,	//电到非流血点 
		ElectricBleedPoint,		//电到当前血流的流血点
	};
	ElectricState m_electricState;								//当前电器官的状态

	float m_burnRadius;											//烧的半径

	enum StepState	
	{
		NoneStep,
		NeedAdd,			//需要添加一个流血效果
		WaitRemove,			//等待移除状态
		Finish
	};
	StepState m_currStepState;									//default is None

	std::vector<GFPhysSoftBodyFace *> m_bleedFaces;				//记录流血的面片
	//std::vector<bool> m_bBleeds;								//标志当前流是否为流血状态，false：为渗血状态
	//int m_currBleedIndex;										//当前添加的血流索引
	bool m_bAllowAddBleedPoint;									//是否允许增加一个流血点
	//OrganSurfaceBloodTextureTrack * m_distBloodTrack;			//当前血流
	
	float m_LastTime;

	std::map<GFPhysSoftBodyNode*,float > m_nodeTimeMap;			//结点电凝的时间
	std::map<OrganSurfaceBloodTextureTrack*,float> m_bloodTrackTimeMap;	//血流已被电凝的时间

	std::vector<BleedBatch> m_BleedBatches;
	int m_CurrActiveBatch;

	//float m_lastTimeBegin;

	float m_trainbegintime;
	float m_trainendtime;
	float m_eleTime[10];
	int m_elePosErrCount[10];
	bool m_bSingleEleTime5s;

	
};
