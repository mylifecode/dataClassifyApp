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

		bool m_IsCutOff;//是否剪断
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
	
	int	 BFSLinkedAera();										//广度优先遍历所有联通区域,返回值为联通区域的数量
	bool IsCutOnRightPlace();									//是否剪对位置
	void ProcessAfterCut();										//超声刀电切后的处理
	void OnSaveTrainingReport();
	//void OnHandleEvent(MxEvent* pEvent);
private:
	bool ReadTrainParam(const std::string& strFileName);		//读取训练相关参数
	bool IsThrowInBag();										//是否扔进袋子
	bool LogicProcess(float dt);								//逻辑判断函数
	void InitStep();											//初始化逻辑步骤
	void HideCutedPartOrgan();									//隐藏被剪下来的部分血管
	void ChangeBagPosition();									//改变袋子的位置
	bool PosInBagAABB(const GFPhysVector3& pos);				//剪下来的血管部分的节点位置是否在袋子的AABB里
	bool PosInGround(const GFPhysVector3& pos);					///剪下来的血管部分的节点位置是否落在地面
	bool TubeHasCutCompeletly();								//判断血管是否被完全剪断
	bool IsTearOffTheTube();									//判断是否扯断血管
	void TearOffTheTube(bool graspOnRightPlace);				//扯断血管
	void BFSTearOffPointTetrahedron(GFPhysSoftBodyTetrahedron* tetra, int level, GFPhysVectorObj<GFPhysSoftBodyTetrahedron*>& result);		//广度优先遍历起始四面体只要求层数的所有相关联的四面体
	void LoadLevel1TearOffPoint();								//读取场景1的扯断点
	void LoadLevel2TearOffPoint();								//读取场景2的扯断点
	//bool IsGraspOnRightPlace();									//抓钳是否抓在了正确的地方
	void AfterCutTubeCompletely();								//完全剪断血管后
	void AddObjectToWorld(int id);								//添加物件
	void RemoveObjectFromWorld(int id);							//删除物件
	void EnterLevel1();											//进入场景1
	void LeaveLevel1();											//退出场景1
	void EnterLevel2();											//进入场景2
	void LeaveLevel2();											//退出场景2
	bool PlayerHasGraspTube();									//是否抓住了血管
	void ScaleTheBag();											//缩放袋子
	void ChangeTexture(int flag);								//抓起血管时才将黑色标记位置显示出来,flag=0表示改成有黑色标记的图，flag=1表示改成无黑色标记的图
	bool IsClampInTheSky();										//是否将血管抓起
	bool IsGraspOnRightPlace(Ogre::Vector2 validTexURange);
	void AddBagToWorld(int posIndex);							//添加袋子
	void RemoveBagFromWorld();									//删除袋子
	void RemoveAllBagFromWorld();								//删除所有袋子
	void OnTimerTimeout(int timerId, float dt, void* userData);

	std::vector<trainingStep> m_steps;
	int	m_currentStep;

	int m_step;													//剪切区域所在的分段数，初始值为21
	int m_iClusterNum;											//血管被分成的段数 被剪刀剪成的段数

	std::map<int, MisMedicOrgan_Ordinary*> m_Objects;

	std::map<int, MisMedicDynObjConstructInfo> m_ConstructInfoMap;


	float m_fEnterNextLevelTimeThreshold;						//进入下一场景的时间阈值
	float m_fCurrentTime;										//当前计时
	bool m_bScaleBag;											//是否在收紧袋子



	float	m_area0, m_area1;
	float m_fScale;												//用于改变uv

	int   m_LastTexChangeFlag;

	float m_LastTexChangeOffset;

	MisMedicOrgan_Ordinary *m_pTerrain;							//表皮
	MisMedicOrgan_Ordinary *m_pTube;							//血管
	SceneSpeciBag *m_pBag;								//袋子
	std::set<GFPhysSoftBodyNode*> m_TubeNodesContactTerrain;
	//MisMedicRigidPrimtive * m_CupCol;

	//Ogre::SceneNode* m_pBagForScale;							//静态袋子
	//Ogre::SceneNode* m_pBagCircle;

	std::vector<std::set<GFPhysSoftBodyNode*> > m_vecClusterResult;
	std::vector<GFPhysVectorObj<GFPhysSoftBodyTetrahedron*>> m_vecClusterTetrahedron;
	Ogre::TexturePtr m_TubeTexturPtr;							//血管贴图指针
	GFPhysSoftBodyNode* m_pFixPoint;							//固定点
	GFPhysSoftBodyNode* m_pEndPoint;							//自由端端点
	std::vector<GFPhysSoftBodyNode*> m_vecTearOffPoint;			//扯断点
	std::vector<int> m_vecTube1TearOffPtID;						//第一个场景的血管的扯断点ID
	std::vector<int> m_vecTube2TearOffPtID;						//第二个场景的血管的扯断点ID
	int m_iCurrentBagPosIndex;									//当前袋子位置索引
	std::vector<Ogre::Vector3> m_vecBagPosition;				//袋子的位置
	float m_fTearOffThreshold;									//判定扯断的阈值
	bool m_bCompeletlyCut;										//血管完全被剪断
	int m_CutCount;												//正确剪断数
	bool m_bPerfectInLevel1;									//场景1中是否完美完成任务
	bool m_bPerfectInLevel2;									//场景2中是否完美完成任务
	bool m_bCutTestPass;										//剪断是否正确
	float m_fBagScaleX;											//当前标本袋X缩放量
	float m_fBagScaleY;											//当前标本袋Y缩放量
	float m_fBagScaleZ;											//当前标本袋Z所放量
	int m_throwErrorCount;										//投放失败的次数
	bool m_bHasGraspTube;										//是否抓住血管
	bool m_bNeedEnterNextLevel;									//是否进入下一场景
	bool m_bScaling;											//袋子是否正在缩放
	bool m_bNeedThrowInBag;										//是否需要将血管扔进袋子中
	bool m_bPreInGround;								//用于避免重复计算投放失败的标记
	bool m_bInGround;									//用于避免重复计算投放失败的标记
	bool m_bShouldShowCutArea;									//是否应该显示绿色剪切区域
	bool m_bHasBeenTearOffTube;
	int m_iCurrentBagID;										//当前袋子ID
	float m_fCutRightRatio;										//电切面在绿色区域内的比值	
	float m_fXScalePerFrame;									//袋子缩放时沿X轴方向每帧缩放的量
	float m_fZScalePerFrame;									//袋子缩放时沿Z轴方向每帧缩放的量

	bool  m_usedDone[6];										///true该次剪切操作做了，false 该次操作没做
	float m_showGreenTime[6];
	int   m_cuttedPosCorrect[6];								///0 没剪切，1 剪切位置正确，2 剪切错误
	float m_cuttedPosTime[6];
	bool  m_isTearoff[6];										///true扯断
	int   m_fallCount[6];										///意外掉落次数
	int   m_currentCut;										///当前正在执行哪个剪切操作

	float m_trainbegintime;

	float m_TagOffsetPercent;

	
	GradLogicData m_gradLogicData;
};

#endif