#pragma once
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"

#include "TrainScoreSystem.h"
class MisMedicOrgan_Ordinary;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class MisCTool_PluginClamp;
class CTool;


class SYSealAndCut : public MisNewTraining
{
	struct ObjectOperationInfo
	{
		enum State{OkOperate,FailOperate,FailOperateNotClipBeforeCut , FaileOperateCutNotBetweenClip ,WaitOperate,NotOperate};		//操作状态
		enum Type{Unknow,Small,Ordinary,Big};				//器官类型
		enum CutPos{NotCutted , NotClippedBeforeCut , CutInOneSideOfClip , CutGood};
		ObjectOperationInfo()
		{
			state = NotOperate;
			type = Unknow;
			organ = NULL;
			operateTime[0] = operateTime[1] = operateTime[2] = 0;
			operateIndices[0] = operateIndices[1] = operateIndices[2] = 0;
			clipNum = 0;
			validClipNum = 0;
			invalidClipNum = 0;
			numClipPartFin = 0;
			upNodeIndex = 20;
			midNodeIndex = 63;
			downNodeIndex = 131;

			topNodeIndex = -1;
			centerNodeIndex = -1;
			bottomNodeIndex = -1;

			upperClipNodeIndex = -1;
			lowerClipNodeIndex = -1;

			init_length = -1.0;

			scale_factor = 1.78;  
			scale_factor_for_mark = 1.18; 

			is_show_mark = false;
			
			//is_mark_change = false;
			is_operating = false;
			m_HasMarkBeSeen = false;
			m_IsMarkAlwaysBeSeen = true;
			m_CutInfo = NotCutted;
			m_IsCutOffed = false;
			m_HasBeTeared = false;
		}
		State state;
		Type type;
		int operateTime[3];
		int operateIndices[3];
		std::vector<int> operate_indices;
		int clipNum;											//钛夹数量
		double scale_factor;								//长度超过此倍数会断开
		double scale_factor_for_mark;				//长度超此倍数显示标记
		int validClipNum;									//有效钛夹数量，不超过2
		int invalidClipNum;
		int numClipPartFin;
		int upNodeIndex;									//上表面中心Node索引
		int midNodeIndex;		
		int downNodeIndex;								//下表面中心Node索引

		int topNodeIndex;
		int centerNodeIndex;
		int bottomNodeIndex;
	
		int upperClipNodeIndex;
		int lowerClipNodeIndex;

		std::vector<int> topConnectedNodeIndices;
		std::vector<int> bottomConnectedNodeIndices;

		bool is_show_mark;						//是否显示标记
		//bool is_mark_change;					//是否改变贴图
		bool is_operating;							//是否正在操作，小血管无用
		double init_length;
		
		MisMedicOrgan_Ordinary * organ;
		static int time;

		//评分相关
		bool m_HasMarkBeSeen;
		bool m_IsMarkAlwaysBeSeen;
		bool m_IsCutOffed;
		CutPos m_CutInfo;
		bool m_HasBeTeared;
	};
public:
	SYSealAndCut(void);
	~SYSealAndCut(void);

public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual bool Update(float dt);

	virtual void InternalSimulateStart(int currStep , int TotalStep , Real dt);
	virtual GFPhysVector3 GetTrainingForceFeedBack(ITool* tool);
		
	bool IsFinished() {return m_bFinish;}

	void OnSaveTrainingReport();
private:
	void Init();
	MisCTool_PluginClamp* GetToolPlugin(CTool* tool);
	void DealWithElcCutObject(MisMedicOrgan_Ordinary * pOrgan);
	void DealWithScissorCutObject(MisMedicOrgan_Ordinary * pOrgan);
	void DealWithClipObject(MisMedicOrgan_Ordinary * pOrgan);

	void UpdateStateAndMaterial();
	void updateClampedObject(MisMedicOrgan_Ordinary* pOrgan,MisCTool_PluginClamp * pPluginClamp);
	void MakeSmoke(CTool * pTool);
	void BreakVessel(MisMedicOrgan_Ordinary * pOrgan,GFPhysSoftBodyNode * attachedNode);
	void BFSTearOffPointTetrahedron(MisMedicOrgan_Ordinary * pOrgan,GFPhysSoftBodyTetrahedron* tetra, int level, GFPhysVectorObj<GFPhysSoftBodyTetrahedron*>& result);
	void AttachNodesCenterToBody(const std::vector<GFPhysSoftBodyNode*> & nodes , GFPhysSoftBody * attacbody);

	void DrawDebugPoints();
	void DrawOnePoint(const GFPhysVector3& position , Ogre::ColourValue color , float size , int &offset);
	void TestForUV(MisMedicOrgan_Ordinary *pOrgan ,double minV , double maxV);
	void TestForUV2(MisMedicOrgan_Ordinary *pOrgan ,double minV , double maxV);

private:
	friend void NewTrainingHandleEvent11(MxEvent * pEvent, ITraining * pTraining);

	bool CheckIfConsistent(ObjectOperationInfo *pInfo);
	void SetMaterialWithoutMark(MisMedicOrgan_Ordinary *pOrgan);
	void ChangeMaterial(MisMedicOrgan_Ordinary *pOrgan , ObjectOperationInfo * pObjectInfo);
	void GetTiantumClipRange(MisMedicOrgan_Ordinary *pOrgan , float &min , float &max);
	bool CheckCutBetweenClip(MisMedicOrgan_Ordinary *pOrgan);

	SYScoreTable * GetScoreTable();

	//std::set<GFPhysSoftBodyNode*> m_NodesBeGrasped;

	typedef std::map<MisMedicOrgan_Ordinary*,ObjectOperationInfo*> OrganToObjectInfoMap;
	std::vector<int> m_eachtime_organ_id;

	OrganToObjectInfoMap m_organToObjectInfoMap;
	MisMedicOrgan_Ordinary * m_pFlesh;
	ObjectOperationInfo * m_pFleshObjectInfo;
	MisMedicOrgan_Ordinary * m_pDizuo2;

	bool m_bRunning;
	bool m_bFinish;
	bool m_bPerfectFinish;
	bool m_bAllFail;

	bool m_bBreakVessel;
	std::vector<GFPhysPositionConstraint*> m_constraints;

	//debug
	Ogre::ManualObject *m_manual;
	std::vector<GFPhysSoftBodyFace *> m_test_faces;
};
