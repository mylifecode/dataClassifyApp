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
	//器官结构体
	struct ObjectOperationInfo
	{
		enum State{OkOperate,FailOperate,WaitOperate,NotOperate};		//操作状态
		enum Type{Unknow,Small,Ordinary,Big};							//器官类型,Unknow<-->未知，Small<-->细血管，Ordinary<-->肉球，Big<-->粗血管
		enum BreakType{Tear = 0 , BurnWrong , BurnRight};                   //血管断掉类型 0 - 扯断 1 - 电断在非标记处  2- 电断在标记处
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
		
		//设置粗血管的各种属性
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

		//设置细血管的各种属性
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

		//设置止血时间
		inline void setStopBleedTimeThreshold(float threshold){fStopBleedTimeThreshold = threshold;}

		GFPhysSoftBodyNode * getTopNode()
		{
			return pTopNode;
		}
		
		GFPhysSoftBodyNode * getMidNode()
		{
			if(organ && pMidNode)		//如果pMidNode已经为NULL，则无需更新
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

		//如果在同一个三角面上电切超过3秒则将血管电断，否则重新计时
		bool addFaceBurnTime(const GFPhysSoftBodyFace* pBurnFacce, float burnTime)
		{
			if (curBurnTotalTime >= burnNeedTime)
			{
				return true;
			}
			curBurnTotalTime += burnTime;
			return false;
		}

		//重置电凝钩灼烧面和时间
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
		State state;					//操作状态
		Type type;						//器官类型
		int topNodeIndex;				//上表面中心Node索引
		int midNodeIndex;		
		int downNodeIndex;				//下表面中心Node索引
		float breakThreshold;			//扯断阈值
		float breakRange;				//扯断范围
		float burnBreakRange;			//烧断的范围
		bool  bConnected;				//血管是否为完整的而没有被扯断
		BreakType   m_BreakType;
		float fStopBleedTimeThreshold;	//止血时间阈值
		float fBleedTime;				//已流血的时间
		float fBurnErrorTime;			//没电到绿色标记位置的时间
		MisMedicOrgan_Ordinary * organ;	
		bool m_HasBeDissectioned;
	private:
		GFPhysSoftBodyNode * pTopNode;	//上固定点
		GFPhysSoftBodyNode * pMidNode;	//中间点
		GFPhysSoftBodyNode * pDownNode;	//下固定点

		//burn info

		float burnNeedTime;				//烧断所需时间
		float curBurnTotalTime;			//当前对象所接受烧操作的时间
		std::map<GFPhysSoftBodyNode*,float> nodeToBurnTimeMap;		//血管节点与灼烧时间的映射
		GFPhysSoftBodyFace* m_pCurrentBurnFace;						//当前正在被灼烧的面
		
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
	bool IsElectricBreakOnGreenArea(MisMedicOrgan_Ordinary* pOgran, const GFPhysSoftBodyFace* pFace , float weights[3]);								//判断是否电断了标记区域
	void SetCurrentOperateTubeIndex(int index);																											//随机设置绿色标记区域
	int GetOrganID(MisMedicOrgan_Ordinary* pOrgan);																					//根据器官指针返回id
	MisCTool_PluginClamp* GetToolPlugin(CTool* tool);
	void UpdateBurn(CTool * pTool,float dt);																					
	void updateClampedObject(MisMedicOrgan_Ordinary* pOrgan,MisCTool_PluginClamp * pPluginClamp);
	bool BreakVessel(MisMedicOrgan_Ordinary * pOrgan,ObjectOperationInfo * pObjectInfo,GFPhysSoftBodyNode * attachedNode,float range = 0.7f,bool bFirstBreak = false);	//扯断血管
    virtual void FacesBeRemoved(const GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & faces, GFPhysAlignedVectorObj<GFPhysSoftBodyFace*> & createdfaces, MisMedicOrgan_Ordinary * orgn);

	bool ReadTrainParam(const std::string& strFileName);		//读取训练相关参数
	void DealWithElecCutObject(MisMedicOrgan_Ordinary * pOrgan);																										//电血管
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
	MisMedicOrgan_Ordinary * m_pLeftBigOrgan;					//左侧大血管
	MisMedicOrgan_Ordinary * m_pRightBigOrgan;					//右侧大血管
	MisMedicOrgan_Ordinary * m_pSkinOrgan;

	MisMedicOrgan_Ordinary * m_pSmallVesselLeftFront;			//左侧前小血管
	MisMedicOrgan_Ordinary * m_pSmallVesselLeftBack;			//左侧后小血管
	MisMedicOrgan_Ordinary * m_pSmallVesselRightFront;			//右侧前小血管
	MisMedicOrgan_Ordinary * m_pSmallVesselRightBack;			//右侧后小血管

	std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > m_vecFacePairLeftFront;				//左大血管与左前小血管的面对
	std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > m_vecFacePairLeftBack;					//左大血管与左后小血管的面对
	std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > m_vecFacePairRightFront;				//右大血管与左前小血管的面对
	std::vector<std::pair<GFPhysSoftBodyFace*,GFPhysSoftBodyFace*> > m_vecFacePairRightBack;				//右大血管与左后小血管的面对


	std::vector<std::pair<GFPhysSoftBodyNode*,GFPhysSoftBodyNode*> > m_vecPointPairLeftFront;				//左大血管与左前小血管的面对
	std::vector<std::pair<GFPhysSoftBodyNode*,GFPhysSoftBodyNode*> > m_vecPointPairLeftBack;				//左大血管与左后小血管的面对
	std::vector<std::pair<GFPhysSoftBodyNode*,GFPhysSoftBodyNode*> > m_vecPointPairRightFront;				//右大血管与左前小血管的面对
	std::vector<std::pair<GFPhysSoftBodyNode*,GFPhysSoftBodyNode*> > m_vecPointPairRightBack;				//右大血管与左后小血管的面对




	float m_fBleedEclpseTime;									//累计流血的时间

	bool m_bFinish;


	std::map<int, MisMedicOrgan_Ordinary*> m_Objects;
	std::vector<int> m_vecLastTubeIndex;						//剩下没被电断的血管ID
	int m_iCurrentNeedToBurnTubeId;								//当前需要被电段的血管ID
	int m_nCutSmallTubeCount;									//弄断小血管的数量
	int m_nCutBigTubeCount;										//弄断大血管的数量
	int m_nTeafOffSmallTubeCount;								//扯断的四根小血管的数量
	int m_count;												//完成训练时用于延迟弹窗
	float m_fTipsAliveTimeThreshold;							//操作错误和操作成功提示语的存活时间阈值
	float m_fCurrentTipsAliveTime;								//当前提示语存活时间
	bool m_bHasTipAlive;										//是否有提示语存活
	float m_fGreenUVAreaBegin;									//绿色标记区域uv范围的起始值
	float m_fGreenUVAreaEnd;									//绿色标记区域uv范围的终点值
	Ogre::TexturePtr m_CurrMarkTex;                             //当前的标记贴图
	const int m_nSmallTubeStep;									//小血管的分段数，初始值为10
	bool m_bLeftBackTubeBreak;									//左后小血管断了
    bool m_bShpereBeenClamped;

	int m_SphereBeClampedNum;
};
