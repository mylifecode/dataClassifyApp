#pragma once
#include "BasicTraining.h"
#include "OgreSceneManager.h"
#include "CollisionTest.h"
#include "MisNewTraining.h"
//#include "Painting.h"
class MisMedicRigidPrimtive;
class SYCameraSkillB : public MisNewTraining
{

public:
	struct LetterMeshData
	{
		LetterMeshData()
		{
			m_WorldMeshUnFocused = "";
			m_WorldMeshFocused = "";
			m_CameraMesh = "";
		}
		LetterMeshData(std::string  worldMeshUnFocused, std::string worldMeshFocused , std::string cameraMesh)
			: m_WorldMeshUnFocused(worldMeshUnFocused), m_WorldMeshFocused(worldMeshFocused), m_CameraMesh(cameraMesh)
		{}
		std::string m_WorldMeshUnFocused;
		std::string m_WorldMeshFocused;
		std::string m_CameraMesh;
	};
	struct SceneLetterInstance
	{
		SceneLetterInstance(Ogre::SceneNode * node, Ogre::Entity * entityunfoc, Ogre::Entity * entityfoc, std::string  name, float scale) :
			                         m_SceneNode(node), 
									 m_LetterEntityUnFocuse(entityunfoc),
									 m_LetterEntityFocuse(entityfoc),
									 m_name(name), 
									 m_InitScale(scale),
									 m_Scale(1),
									// m_bOverLap(false),
									 m_OverLapTime(0),
									 m_LossOverLapTime(0),
									 m_FailedNum(0),
									 m_UsedTime(0),
									 m_OverLapSucceed(false),
									 m_BeLocated(false),
									 m_EnlargeStage(true),
									 m_IsTipShowed(false)
		{}
		Ogre::SceneNode * m_SceneNode;
		Ogre::Entity    * m_LetterEntityUnFocuse;
		Ogre::Entity    * m_LetterEntityFocuse;
		std::string m_name;
		float m_Scale;//
		bool  m_EnlargeStage;
		float m_InitScale;
		
		float m_OverLapTime;
		float m_LossOverLapTime;
		bool  m_OverLapSucceed;

		bool  m_BeLocated;// «∑Òå¶ú 
		bool  m_IsTipShowed;

		int   m_FailedNum;
		float m_UsedTime;
	};

	struct CameraLetter
	{
	public:
		CameraLetter()
		{
			m_CamLetterNode = 0;
			m_CamLetterEnt  = 0;
			m_AimSucced = false;
			m_AimUsedTime = 0;
			m_name = "";
			m_index = 0;
		}
		int m_index;
		Ogre::SceneNode * m_CamLetterNode;
		Ogre::Entity    * m_CamLetterEnt;
		std::string    m_name;
		bool m_AimSucced;
		float m_AimUsedTime;
	};
	SYCameraSkillB(MXOgreWrapper::CameraState eCS);
	
	virtual ~SYCameraSkillB(void);
	
	virtual bool Update(float dt);
	
	virtual bool BeginRendOneFrame(float timeelapsed);

	bool CheckCoincide(const std::string & checkname ,float  dt);

	void ChangeCamLetterMaterial(int mode);//0-- dashed  1-- solid
	void FinishTraining();

	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	void CreateAllLettersRandomly();

	void CreateLetterInRandomPos(const std::string & letter ,
		                         float minTheta , float maxTheta,
								 float minAlpha , float maxAlpha );
	void CreateLetterInHemiSphere(const std::string & letter , float theta , float alpha);

	void OnSaveTrainingReport();
private:

	SYScoreTable* GetScoreTable();

	void OnTrainingIlluminated();

	void UpdateCameraLetter();

	bool UpdateSceneLetters(float shrinkspeed , float dt);

	Ogre::Vector3  GetCameraPivot();

	void CreateNextCameraLetter();

	Ogre::SceneNode * CreateWorldLetter(const std::string & letter, 
									    const Ogre::Vector3 & letterPos);
	//Ogre::SceneNode * m_CamLetterNode;

	//Ogre::Entity    * m_CamLetterEnt;
	CameraLetter m_CamLetter;

	MisMedicRigidPrimtive * m_CollisonMesh;

	std::vector<SceneLetterInstance> m_WorldLettersInstance;

	std::map<std::string, LetterMeshData> m_LetterMeshMap;

	char m_CheckLetter[5];//A,B,C,D,E
};
