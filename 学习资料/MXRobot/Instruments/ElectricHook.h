#pragma once
#include "Tool.h"
#include "ElectricTool.h"
#include "OgreParticleSystem.h"
#include "SmokeManager.h"
#include "WaterManager.h"
class CXMLWrapperTool;
class VeinConnectObject;

class MisCTool_PluginHook;
class CElectricHook : public CElectricTool
{
public:
	

	class VeinHookShapeData
	{
	public:
		GFPhysVector3 m_HookLinePoints[2];

		GFPhysVector3 m_HookSupportOffsetVec;

		GFPhysVector3 m_HookProbDir;

		float m_HookLineRadius;
	};

	CElectricHook();
	CElectricHook(CXMLWrapperTool * pToolConfig);
	virtual ~CElectricHook(void);

	virtual Ogre::TexturePtr GetToolBrandTexture();

public:
	bool Initialize(CXMLWrapperTraining * pTraining);

	std::string GetCollisionConfigEntryName();

	//@overrridden
	virtual float GetHeadPartCollisionLen();

	//for gophysics new tool
	
	//virtual GFPhysVector3 CalculateToolCustomForceFeedBack();
	
	virtual void NewToolModeUpdate();

	virtual void onEndCheckCollide(GFPhysCollideObject * softobj , GFPhysCollideObject * rigidobj , const GFPhysAlignedVectorObj<GFPhysRSManifoldPoint> & contactPoints);

	virtual void InternalSimulationStart(int currStep , int TotalStep , float dt);

	virtual void InternalSimulationEnd(int currStep , int TotalStep , float dt);

	virtual void onFrameUpdateStarted(float timeelpased);

	virtual bool Update(float dt);
		
	//for new train
	//overridden 
	virtual bool GetForceFeedBack(Ogre::Vector3 & contactForce, Ogre::Vector3 & dragForce);

	int GetCollideVeinObjectBody(GFPhysRigidBody * bodies[3]);

	VeinHookShapeData GetLocalHookShapeData();

	VeinHookShapeData GetWorldHookShapeData();

	GFPhysRigidBody * GetHookRigidBodyPart();

	NewTrainToolConvexData & GetHookPart();

	Ogre::Vector3 CalcVeinHookForceFeedBack();

	bool EmitSpark(const Ogre::Vector3 & position , bool needRand = false ,  int probability = 40); //probability �ٷֱ� ���ؽY��--�Ƿ�l����

	bool EmitSpark(bool needRand = false ,  int probability = 40); //probability �ٷֱ� ���ؽY��--�Ƿ�l����

	virtual void OnVeinConnectBurned(const std::vector<Ogre::Vector3> & burnpos);

	virtual bool ElectricCutOrgan(MisMedicOrgan_Ordinary * organ , GFPhysSoftBodyFace * face  , float weights[3]);

	virtual void BreakAdhesion();

	virtual int TryBurnConnectStrips(VeinConnectObject * stripObj, float BurnRate, std::vector<Ogre::Vector3> & burnpos, std::vector<VeinConnectPair*> & burnpair);

	float GetCutWidth();

	virtual float GetFaceToElecBladeDist(GFPhysVector3 triVerts[3]);


	Ogre::TexturePtr m_Tex;

private:
	
	float m_ValidBurnRadiusThres;

	MisCTool_PluginHook * m_HookPlugin;
	//λ�ã��ж��Ƿ��ƶ�
	Ogre::Vector3  m_VecToolLastPos;
	//��һ֡��ʱ��
	double m_fLastTickCount;
	//�����ۻ�ʱ��
	double m_fAccumTime;
	//��һ֡������ť�Ƿ񱻰���
	bool  m_bLastBtnPress;   
	//����ʱ���Ƿ��㹻
	bool  m_bTimeEnough;

	bool  m_bHook;

	int m_iHookCoolDown;

	void * m_pDynamicObject;

	bool m_bFirstFrame;

	bool m_parkfired;
	Ogre::Vector3 m_v3ToolPos;
	bool m_bCheckBlood;
	
	Ogre::ParticleSystem *m_pSparkParticle;
	DWORD m_dwLastTime;
	std::map<int, std::pair<int, int>> m_mapAttachedTriangle;

	//for new train
	VeinHookShapeData m_hookShape_Local;
	VeinHookShapeData m_hookShape_World;

	GFPhysVector3 m_lastHookforceFeedBack;

	//float m_MinDistToHookPoint;
	//GFPhysVector3 m_MinDistPoint;
	//GFPhysSoftBodyFace * m_MinDistFace;
	//float m_MinPointWeights[3];
	//GFPhysCollideObject * m_MinHookDistBody;
	DWORD m_LastElectricTime;
	float m_ElectricPersistTime;

	bool m_canEmitSmoke;
	bool m_veinConnectElectrocuted;

	//for spark
	int m_RandNumForSpark;
	float m_TimeSliceForSpack;

	//SmokeManager m_smokeManager;
	// test node
	Ogre::SceneNode* cutNodeLeft;
	Ogre::SceneNode* cutNodeRight;
};