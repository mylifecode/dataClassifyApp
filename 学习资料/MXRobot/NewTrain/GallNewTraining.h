#ifndef _GALLNEWTRAINING_
#define _GALLNEWTRAINING_
#include "MisNewTraining.h"
class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;
class VeinConnectObject;
class CElectricHook;
class CScissors;
class VeinConnectPair;

class CNewGallTraining :public MisNewTraining
{
public:
	CNewGallTraining(void);

	virtual ~CNewGallTraining(void);

public:
	virtual void SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ);

	virtual bool Update(float dt);

	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	virtual void OnOrganCutByTool(MisMedicOrganInterface * organ , bool iselectriccut);

	virtual void OnVeinConnectCuttingByElecTool(std::vector<VeinConnectPair*> & cuttingPair);
	
	//train score logic data
	bool  m_isDetectDomeFinish;
	float m_CholeyStripPercent;
	float m_BedStripPercent;
	bool  m_HasCysticDuctBeIngure;
	bool  m_HasCysticArteryBeIngure;

	bool  m_HasCysticDuctBeFatalError;
	bool  m_HasCysticArteryBeFatalError;

	bool  m_HasClipCysticDuctGood;//胆囊管是否正确被夹闭
	int   m_NumClipInCysticDuct;//胆囊管钛架数量
	int   m_HasCysticDuctBeCutInRightPlace;//-1 not cut 0 cut in right place 1 cut in wrong place

	bool  m_HasClipCysticArteryGood;//胆囊动脉是否正确被夹闭
	int   m_NumClipInCysticArtery;//胆囊动脉钛架数量
	int   m_HasCysticArteryBeCutInRightPlace;
	//
protected:
	virtual void CreateTrainingScene(CXMLWrapperTraining * pTrainingConfig);
	
	void OnCameraStateChanged(bool isFixed);

	void OnSaveTrainingReport();

	/**
		for debug !!!
	*/
	void onDebugMessage(const std::string&);

private:
	void UpdateSafeClampDistance();
	bool CalClampDistance(std::vector<GFPhysSoftBodyFace*>& clampedFaces, GFPhysVector3& clampPoint);
	void UpdateMixTexture(void);

	void LoadGallbladderNeckInfo();

	void UpdateTargetPointSeeState();
private:
	GFPhysSoftBodyConstraint * m_GallPressConstraint;

	int m_min,m_max;

	MisMedicOrgan_Ordinary* m_cysticDuct;
	MisMedicOrgan_Ordinary* m_gallbladder;
	VeinConnectObject* m_veinConnect;

	/// 器械抓取面到夹子的最短距离，默认-1
	float m_minDisOfClampedFaceAndClip;
	/// 抓取面和胆囊颈的最短距离，默认-1
	float m_minDisOfClampedFaceAndGallbladderNeck;
	float m_minDisOfClampedFace;
	GFPhysVector3 m_gallbladderNeckPos;
	std::set<GFPhysSoftBodyFace*> m_neckFaces;
	GFPhysVector3 m_neckPos;

	std::string m_veinMaterialName;
	const std::string m_dynamicTextureName;
	Ogre::TexturePtr m_sceneTexture;
	Ogre::RenderTarget * m_renderTarget;

	bool m_cameraIsFixed;
    bool m_bFinish;
	struct LocateInfo
	{
		float startTime;
		float stopTime;
		float rate;
	};
	std::vector<LocateInfo> m_locateInfos;
	Ogre::String m_TrainName;

    bool m_ElectricCutVeinBottom;
    bool m_ElectricCutVeinConnect;
    bool m_HurtDuringSeperateTriangleCount;

    bool m_Cut_BRAVERY_ARTERY;
    bool m_Cut_COMMON_BILE_DUCT;
    bool m_Cut_BRAVERY_CYSTIC_DUCT;


};

#endif