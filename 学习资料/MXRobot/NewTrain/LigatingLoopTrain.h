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
		//整根管子上的所有操作是否完成
		inline bool MissonComplete()
		{
			return m_bArea1BindComplete && m_bArea2BindComplete && m_bArea1CutComplete && m_bArea2CutComplete;
		} 
		//添加扯断点
		void AddTearOffPoint(const int& index)
		{
			m_vecTearOffPoint.push_back(m_pTube->m_physbody->GetNode(index));
		}
		//设置固定点和自由端端点
		void SetFixAndEndPoint(const int& fixIndex, const int& endIndex)
		{
			m_pFixPoint = m_pTube->m_physbody->GetNode(fixIndex);
			m_pEndPoint = m_pTube->m_physbody->GetNode(endIndex);
		}

		//添加固定点
		void AddFixPoint(const int& fixPtIndex)
		{
			m_vecFixPoint.push_back(m_pTube->m_physbody->GetNode(fixPtIndex));
		}

		//添加需要禁止软体碰撞的点
		void AddDisableRRCollisionPoint(const int& RRCollisonPoint)
		{
			m_vecDisableRRCollisonPoint.push_back(m_pTube->m_physbody->GetNode(RRCollisonPoint));
		}


		MisMedicOrgan_Ordinary *m_pTube;
		bool m_bArea1BindComplete;											//区域1套圈成功
		bool m_bArea2BindComplete;											//区域2套圈成功
		bool m_bArea1CutComplete;											//区域1剪断成功
		bool m_bArea2CutComplete;											//区域2剪断成功
		bool m_bHasBeenTearOff;												//是否被扯断
		bool m_bHasCalc;
		int m_nCurrentBindedRopeCount;										//血管上的当前绳子个数
		//int m_nPreBindedRopeCount;											//血管上的之前绳子个数
		float m_fCurrentBleedTime;											//当前流血时间
		float m_fCurrentDisapearTime;										//当前消失时间

		GFPhysSoftBodyNode* m_pFixPoint;									//固定点
		GFPhysSoftBodyNode* m_pEndPoint;									//自由端端点
		std::vector<GFPhysSoftBodyNode*> m_vecTearOffPoint;					//扯断点
		std::vector<GFPhysSoftBodyNode*> m_vecFixPoint;						//固定点
		std::vector<GFPhysSoftBodyNode*> m_vecDisableRRCollisonPoint;		//需要禁止软体碰撞的点

        bool m_bindError;
        int m_bindErrorNum;
		
	};

	CLigatingLoopTrain(void);
	virtual ~CLigatingLoopTrain(void);
    int m_nsumBindCount;
    void JudgeRopeCutted();															//判断剪刀是否剪对了地方

private:
	int	 BFSLinkedAera(int index);													//广度优先遍历所有联通区域,返回值为联通区域的数量
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	void SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ);

	virtual bool Update(float dt);
	bool ReadTrainParam(const std::string& strFileName);							//读取训练相关参数
	void LogicProcess();															//逻辑判断
	void JudgeRopeBindOnRightArea(MisMedicOrgan_Ordinary* pTube);					//判断打结是否打对了地方
	void ChangeTexture(const int& flag);											//改变贴图数据,flag=0:所有贴图设置成无标记区域,flag=1:当前血管标记出区域
	int UserHasClamTube();															//是否抓住血管,并返回哪个血管被抓住
	bool IsTearOffTheTube(const int& index);										//判断是否扯断血管
	bool ShouldShowArea(const int& index);											//判断是否显示标记区域
	void TearOffTheTube(const int& index);
	virtual void OnToolCreated(ITool * tool, int side);
    void OnSaveTrainingReport();

private:
	Ogre::TexturePtr m_TubeTexturPtr[3];											//血管贴图指针

	int	m_currentStep;
    int m_nSumBindedRopeCount;
	std::vector<std::vector<GFPhysSoftBodyNode*> > m_vecTearOffPoint;			    //扯断点
	bool m_bFinished;

    
	float m_area0,m_area1,m_area2, m_area3;
	int	m_counter;																	//帧数计数器

	int m_LastTexChangeFlag;
	int m_LastChangeTubeIndex;
	std::map<int, MisMedicOrgan_Ordinary*> m_Objects;

	std::map<int, MisMedicDynObjConstructInfo> m_ConstructInfoMap;

	std::vector<TUBEINFO> m_vecTube;
	std::vector<std::set<GFPhysSoftBodyNode*> > m_vecClusterResult;
	std::vector<GFPhysVectorObj<GFPhysSoftBodyTetrahedron*>> m_vecClusterTetrahedron;
	float m_fBleedTimeThreshold;													//流血时间阈值
	float m_fDisapearTimeThreshold;													//消失时间阈值

	//逻辑相关变量
	int m_nCurrentTubeIndex;														//当前正在操作的血管
	float	m_fTearOffThreshold;													//扯断长度阈值
	float	m_fShowAreaThreshold;													//显示标记区域的长度阈值
	std::vector<int> m_bTubeBeTearOff;												//血管是否被扯断
	bool m_bShowLigatingErrorTip;													//是否显示套扎区域错误 
	bool m_bPerfect;
};

#endif