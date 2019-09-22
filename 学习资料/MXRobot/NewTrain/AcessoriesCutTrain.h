#ifndef _ACESSORIESCUTRAIN_
#define _ACESSORIESCUTRAIN_

#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include "TrainScoreSystem.h"
#include "MisMedicObjectUnion.h"
class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;
class VeinConnectObject;
class CElectricHook;
class CScissors;



class CAcessoriesCutTraining :public MisNewTraining
{
public:
	CAcessoriesCutTraining( const Ogre::String & strName);

	virtual ~CAcessoriesCutTraining(void);

	bool BeginRendOneFrame(float timeelapsed);

	virtual bool OrganShouldBleed(MisMedicOrgan_Ordinary* organ, int faceID, float* pFaceWeight);

public:

// 	enum	AcessoriesCutCheckPointType
// 	{
// 		ACCPT_Empty,
// 		ACCPT_Burn,
// 		ACCPT_Cut,								//电凝钩电切、剪刀等剪完成后 
// 		ACCPT_Clip,
// 		ACCPT_ElecCut ,						//电凝钩电切时
// 		ACCPT_ElecCutWithForcep ,
// 		ACCPT_ElecCoagWithHook , 
// 		ACCPT_Inject ,
// 	};
	enum	AreaType
	{
		AT_Uterus,//	子宫体
		AT_Fallopian_Tube_Left,//	左输卵管
		AT_Fallopian_Tube_Right,//	右输卵管
		AT_Ovary_Left,//	左卵巢
		AT_Ovary_Right,//	右卵巢
		AT_Ligament_Left,//	左悬韧带固有韧带
		AT_Ligament_Right,//	右悬韧带固有韧带
		AT_Determine_Area_Burn_1,
		AT_Determine_Area_Burn_2,
		AT_Determine_Area_Cut_1,
		AT_Determine_Area_Cut_2,
		AT_Focal_Zone_1,
		AT_Focal_Zone_2,
		AT_Focal_Zone_3,
		AT_TOTALNUM,
		AT_NONE
	};

	virtual void ReadCustomDataFile(const Ogre::String & customDataFile);
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	virtual bool Update(float dt);

	virtual MisMedicOrganInterface * LoadOrganism(MisMedicDynObjConstructInfo & cs, MisNewTraining* ptrain);//(CXMLWrapperTraining * pTrainingConfig, int i);
	virtual void SetCustomParamBeforeCreatePhysicsPart(MisMedicOrgan_Ordinary * organ);
	//Ogre::MeshPtr m_StaticPartMeshPtr;

	//MisMedicObjectUnion m_StaticDynamicUnion;

	virtual void receiveCheckPointList(MisNewTraining::OperationCheckPointType type,const std::vector<Ogre::Vector2> & texCord, const std::vector<Ogre::Vector2> & TFUV , ITool * tool , MisMedicOrganInterface * oif);
	
	virtual void CheckClampAreaRegion(const std::vector<Ogre::Vector2> & texCord , ITool * tool , MisMedicOrganInterface * oif);
	virtual AreaType getAreaType(MisMedicOrganInterface * oif , const Ogre::Vector2 & texCoord) { return AT_NONE; }
	
	virtual bool checkDianzi();
	virtual void	 doFinishCount(){}

	virtual void	 ElecCutStart(){}
	virtual void	 ElecCutEnd(){}
	virtual void OnTakeOutSomething(int OrganID) {}

	void	AreaCutDetermationSave();

	void KeyPress(QKeyEvent * event);

protected:

	Ogre::String m_trainName;

	virtual void RegisterMistake(int	mistakeType);
	void testMistake(int markValue);

	uint32 GetPixelFromAreaTesture(float u, float v);
	
	//virtual void OnOrganCutByTool(MisMedicOrganInterface * organ, bool iselectriccut);

	virtual void onOrganTopologyRefreshed(MisMedicOrgan_Ordinary* organ);

	void   OnWombBeCutted(MisMedicOrgan_Ordinary * womb);

	Ogre::TexturePtr m_Area;
	int		m_AreaWidth;
	int		m_AreaHeight;
	bool	m_bNeedCheck;

	bool	m_bToDebugDisplayMode;
	std::string	m_materialName;
	std::string	m_AreaMarkTextureName;

	int		areaMatchNum[AT_TOTALNUM];
	int		m_mistakeCounterMax;
	bool	m_foundMistake;
	int		m_ScoreTimes[AT_TOTALNUM];
	bool	m_bFinished;
 	uint32	m_BurnMarkArea[4194304];
	int		m_BurnMark_W;
	int		m_BurnMark_H;

	Ogre::ColourValue GetColorFromImage(float u, float v);

	Ogre::TexturePtr  m_TexturePtr;//R G B A R-white value G- value B-heat value  A-BloodValue
	Ogre::Image m_TexImage;
	std::vector<int> m_HiddedFaceIndex;

    Ogre::uint * m_pDest;
	
	MisMedicObjectEnvelop * m_pEnvelopEmbryoAndUterus;

	uint32 m_NumOfUterusBeClamped;
	uint32 m_NumOfFallopianTubeBeClamped;
	uint32 m_NumOfOvaryBeClamped;
    
    uint32 m_NumOfUterusBeDamaged;
    uint32 m_NumOfFallopianTubeBeDamaged;
    uint32 m_NumOfOvaryBeDamaged;

};

#endif