/**Supplement:zx**/
#pragma once
#include <set>
#include <vector>
using namespace::std;
#include "Singleton.h"
#include "Inception.h"
#include "OgreMaxScene.hpp"
#include "IObjDefine.h"
#include "NewTrain/MisMedicOrganInterface.h"
class CVesselBleedEffect;
class CScreenEffect;
class SmokeManager;
class WaterManager;
class BubbleManager;

class EffectManager : public CSingleT<EffectManager>
{
public:
	enum TextureType
	{
		TT_Fire,
		TT_Blood,
	};	 	

	enum CameraType
	{
		CT_NORMAL,
		CT_FLASH,
		CT_SMOOTH,
	};

public:
	EffectManager();
	~EffectManager();

	static Ogre::Pass * GetMaterialPass(Ogre::MaterialPtr  mat , int techid , int passid);

	static Ogre::TextureUnitState * GetMaterialTexUnit(Ogre::MaterialPtr  mat , int techid , int passid,int texid);
	
	// create screen arrow, zx
	void ShowArrow(Ogre::Vector2 v2ScreenPos, int nShowSeconds, Ogre::String strTransitionName = "", bool bForceCreateNew = false);
	void ShowArrow(Ogre::Vector3 v3TargetPos, Ogre::Vector2 v2ScreenOffset, Ogre::Camera * pCamera, int nShowSeconds, Ogre::String strTransitionName = "", bool bForceCreateNew = false);
	void ShowFixedArrow(Ogre::Vector3 v3TargetPos, Ogre::Vector2 v2ScreenPos, Ogre::Camera * pCamera, int nShowSeconds, Ogre::String strTransitionName = "", bool bForceCreateNew = false);
	void HideArrow();
	void ClearArrow();

	// show float window text, zx
	void ShowFloatWindowText(const std::wstring & strText, TipInfo::TipPosition enmTipPositioin, int nLastSeconds = -1, int nDelaySeconds = 0, TipInfo::TipIconType eIconType = TipInfo::TIT_TIP_INFO, int nCustomPosX = 0, int nCustomPosY = 0); // ms

	void BeginRendOneFrame(float dt);

	void CreateSmokeManager();
	void DestorySmokeManager();

	void CreateBubbleManager();
	void DestoryBubbleManager();

	void CreateWaterManager();
	void DestoryWaterManager();

	SmokeManager  * GetSmokeManager();
	BubbleManager * GetBubbleManager();
	WaterManager  * GetWaterManager();

	// 粒子烟效果 [3/22/2012 yl]
	//void SmokeEffectOn(const Ogre::Vector3 & vec);
	//void SmokeEffectOff();

	//火花效果
	void SparkEffectOn(const Ogre::Vector3 & vec , bool needRand = true);
	void SparkEffectOff();

	//设置物体透明效果
	void SetNodeTransparent(const std::string &meterialName,int nAlpha);

	//设置渗血效果
	//void BloodEffectOn(const Ogre::Vector3 & vec);
	//void BloodEffectOff();

	//移动镜头位置到指定名字
	bool MoveCamera( Ogre::String cameraName, Ogre::int32 nCushion = 100 );
	void UpdateCamera( OgreMax::OgreMaxScene *pOms );

	EffectManager::CameraType GetCameraType() const { return m_enmCameraType; }
	void SetCameraType(EffectManager::CameraType val) { m_enmCameraType = val; }

	void DrawOgreAxis(float mlength,float mTipOffset);

	void DrawOgreAxisByMesh(Ogre::Vector3 v3Pos);

	void DrawLightPosByMesh(Ogre::Vector3 v3Pos,Ogre::Quaternion quatDir);

	void SetMaterialColour(const Ogre::String & MaterialName,
		Ogre::Real alpha = 0.8,
		Ogre::LayerBlendSource source1 = Ogre::LBS_TEXTURE,
		Ogre::LayerBlendSource source2 = Ogre::LBS_CURRENT,            
		const Ogre::ColourValue& arg1 = Ogre::ColourValue::White,
		const Ogre::ColourValue& arg2 = Ogre::ColourValue::White);

	void SetMaterialAmbient(const Ogre::String & MaterialName,const Ogre::ColourValue& newColor);
	void SetMaterialDiffuse(const Ogre::String & MaterialName,const Ogre::ColourValue& newColor);

	OgreMax::OgreMaxScene* GetOgreMaxScene();
	void SetOgreMaxScene(OgreMax::OgreMaxScene* pOgreMaxScene);

	
	CVesselBleedEffect*  createVesselBleedEffect(MisMedicOrganInterface * organif);
	void createVesselBleedEffect(Ogre::Vector3 bleedpos);
	void	removeVesselBleedEffect(CVesselBleedEffect* eff);

	Ogre::Vector3 m_vPosition;
	Ogre::Vector3 m_vecCurrentPosition;
	//bool m_bSmokeOn;
	//DWORD m_dwSmokeStartTime;

	bool m_bArrowOn;
	DWORD m_dwArrowStartTime;
	DWORD m_nArrowOnTime;
	
	Ogre::String m_strArrowTransitionName;

	// 粒子烟效果
	//Ogre::SceneNode *m_smokeNode;
	//Ogre::ParticleSystem *m_smokeParicle;
	//Ogre::ParticleEmitter *particleEmit;//
	//bool m_bSmokeEffectOn;
	//DWORD m_dwSmokeEffectStartTime;

	// 渗血效果 
	//Ogre::ParticleSystem *m_pBloodParticle;
	//Ogre::SceneNode *m_pBloodNode;
	//bool m_bBloodEffectOn;
	//DWORD m_dwBloodEffectStartTime;

	//火花
	Ogre::ParticleSystem *m_pSparkParticle;
	Ogre::SceneNode *m_pSparkNode;
	bool m_bSparkEffectOn;
	DWORD m_dwSparkEffectStartTime;

	CameraType m_enmCameraType;
	
	std::list<CVesselBleedEffect*>		m_EffectList;

private:
	SmokeManager  * m_SmokeMgr;
	WaterManager  * m_WaterMgr;
	BubbleManager * m_BubbleManager;
	bool m_arybCanvas[EDOT_ORGAN_MAX];
	Ogre::int32 m_nCushion;
	bool m_bUpdateFlashCamera;
	OgreMax::OgreMaxScene *m_pOgreMaxScene;

public:
	bool m_bOperationPositionError;
};