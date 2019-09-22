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

class CElectroCoagulationTrain : public MisNewTraining
{
	struct OnLineGradeInfo
	{
		OnLineGradeInfo()
		{
			m_iBurnStage = -1;
			m_iCutStage = -1;

			m_iCutMiss = 0;
			m_iCutOutMark = 0;
			m_NumCutInBurnReg = 0;
			m_NumCutNotInBurnReg = 0;
			m_iBurnOutRegCount = 0;
		}
		int m_NumCutInBurnReg;
		int m_NumCutNotInBurnReg;

		int m_iBurnStage;
		int m_iCutStage;
		int m_iCutMiss;
		int m_iCutOutMark;
		int m_iBurnOutRegCount;
	};
public:
	class BurnAndCutInfo
	{
	public:
//just use this
		double up_start_v , up_end_v ; 
		double bottom_start_v , bottom_end_v;

		//[0]---up [1]---bottom
		bool is_burn_start[2];
		bool is_burn_end[2];

		bool is_cut_start[2];
		bool is_cut_end[2];

		bool is_burn_enough[2];
		bool is_cut_enough[2];
		
		bool is_all_burn;
		bool is_all_cut;
		bool is_finish;

		double cut_allowable_error;
		double burn_allowable_error;

		std::vector<bool> is_burn[2];
		std::vector<bool> is_cut[2];
		std::vector<double> segment_height[2];
	
		double left;
		double right;
		double top;
		double bottom;
		
		
		BurnAndCutInfo();
		void SetSegment(double upStart , double upEnd, double bottomStrat , double bottomEnd , int segment = 10);
		void SetBurnArea(const Ogre::Vector2 & topLeft , const Ogre::Vector2 & bottomLeft , const Ogre::Vector2 & topRight , const Ogre::Vector2 & bottomRight);
		bool TestForBurn(const std::vector<Ogre::Vector2> & texCoords);
		bool TestForCut(const std::vector<Ogre::Vector2> & texCoords);
		bool TestForBurnV2(const std::vector<Ogre::Vector2> & texCoords);
		bool TestForBurnV3(const std::vector<Ogre::Vector2> & texCoords , double partitionHeight);
		bool TestForCutV2(const std::vector<Ogre::Vector2> & texCoords);
		bool TestForCutV3(const std::vector<Ogre::Vector2> & texCoords , double partitionHeight , bool isRelaxed = false);

		float m_burnOnceBeginTime;
		float m_burnOnceEndTime;
	};
	
public:
// 	enum OperationCheckPointType
// 	{
// 		ELECTRAIN_BURN,
// 		ELECTRAIN_CUT,
// 	};

	enum OrganType
	{
		ELECTRAIN_FLESH = 21,
	};

	enum OperationType
	{
		BURN_OUTSIDE_MARKAREA,						//电到标记外区域
		BURN_INSIDE_MARKAREA,						//电到标记区域
		CUT_OUSIDE_MARKAREA,						//剪到标记外区域
		CUT_INSIDE_WITHOUT_BURN,					//剪到标记内无电凝区域
		CUT_INSIDE_WITH_BURN,						//剪到标记内电凝区域
		CUT_OVER_BURN_AREA,							//剪到超出电凝区域
	};

public:
	CElectroCoagulationTrain(void);
	~CElectroCoagulationTrain(void);
public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual bool Update(float dt);
	virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif);
	virtual void OnHandleEvent(MxEvent* pEvent);
	bool CheckOrganInBurn(MisMedicOrganInterface * organ);
	bool CheckOrganCut(MisMedicOrganInterface * organ);
	//virtual void InternalSimulateStart(int currStep , int TotalStep , Real dt);

	//for debug
	void CollectFacesClamped(GFPhysVectorObj<GFPhysSoftBodyFace*> & faces_clamped);

	bool IsFinished() {return false;}
	void SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ);

	void OnSaveTrainingReport();
	SYScoreTable * GetScoreTable();
private:
	void Init();
	void InitCutAndBurnStrs();
	uint32 GetPixelFromAreaTexture(float cx, float cy);
	bool CheckIfBurnPointInRightArea(const Ogre::Vector2 & texcoord);
	bool CheckIfCutInRightArea(const Ogre::Vector2 & texcoord);
	void DealWithOperation(OperationType type , const std::vector<Ogre::Vector2> & texCoord , bool tipFlag = true); 
	void ShowStageTip(OperationType type , int stage);
	void DealStageScore(OperationType type , int stage);
	//for debug
	void DrawFacesClamped();
	void TestForUV(MisMedicOrgan_Ordinary * organ);

	Ogre::TexturePtr m_area_mark_ptr;
	int m_mark_tex_width;
	int m_mark_tex_height;
	Ogre::uint * m_pMarkDest;

	Ogre::Real m_up_end_v;
	Ogre::Real m_center_of_markarea_v;
	Ogre::Real m_bottom_end_v; 

	int m_segment_num;

	
	double m_burn_success_percent;
	double m_cut_success_percent;
	double m_cut_in_burn_success_percent;

	bool m_is_burn_change;

	bool m_isfailed;

	bool m_is_finish;

	MisMedicOrgan_Ordinary *m_pflesh;

	std::vector<Ogre::String> m_burn_strs;
	std::vector<Ogre::String> m_cut_strs;

	std::vector<BurnAndCutInfo> m_burnAndCutInfos;

	std::vector<Ogre::Vector2> m_burn_texcoord_buffer;
	std::vector<Ogre::Vector2> m_cut_texcoord_buffer;

	bool m_is_set_material[5];

	Ogre::TexturePtr m_textuer_ptr[5];

	//for debug
	Ogre::ManualObject *m_manual;
	std::vector<GFPhysSoftBodyFace *> m_faces_clamped;
	std::vector<GFPhysSoftBodyFace *> m_faces_tested;

	VeinConnectObject * m_StripConnect;

	OnLineGradeInfo m_OnLineGradeInfo;
};
