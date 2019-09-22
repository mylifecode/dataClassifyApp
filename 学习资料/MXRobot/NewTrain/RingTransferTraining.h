#ifndef _BASIC_RINGTRANFERTRAINING
#define _BASIC_RINGTRANFERTRAINING
#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
//#include "MisMedicOrganTube.h"
#include "TrainScoreSystem.h"
class MisMedicOrgan_Ordinary;
class MisMedicOrgan_Tube;
class VeinConnectObject;
class CElectricHook;
class CScissors;

class ACTubeShapeObject;

class CRingTransferTrain :public MisNewTraining
{
public:
	enum RingState
	{
		RS_WAITBECLAMP,
		RS_WAITFORPASS,
		RS_INPASS,
		RS_FINISHPASS,
		RS_FINISHPUT,
	};
	class TubeInScene
	{
	public:
		TubeInScene(ACTubeShapeObject * tube, 
			        Ogre::Entity * pillar ,
					int side,
					int index);
		
		void ResetToOriginColor();

		ACTubeShapeObject * m_TubeObj;
		
		Ogre::Entity * m_CouplePillar;
		bool m_isPassedByTwoTool;
		int  m_Side;//0 -- left 1 -- right
		int  m_Index;
		Ogre::ColourValue m_TubeOriginColor[2];//ambient + diffuse
		Ogre::ColourValue m_PillarOriginColor[2];

		Ogre::ColourValue m_TubeCurrColor[2];
		Ogre::ColourValue m_PillarCurrColor[2];
		//bool m_HasBeenPassed;
		bool m_IsPutSucceed;
		RingState m_State;

		int m_FirstClampToolSide;//首先抓起的是哪个手

		//logic data
		bool   m_TouchGroundWhenPass;
		bool   m_HasPassSucced;
		int    m_PutFaileNum;

	};
	CRingTransferTrain(void);

	virtual ~CRingTransferTrain(void);

public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	
	virtual bool Update(float dt);

	bool BeginRendOneFrame(float timeelpsed);

	void OnSaveTrainingReport();
		
private:
	bool CheckTubeSurroundPillar(ACTubeShapeObject * tubeObj , Ogre::Entity * pillar);

	void UpdateTubeColor(ACTubeShapeObject * tubeObj , 
		                 const Ogre::ColourValue & ambient,
						 const Ogre::ColourValue & diffuse);

	void UpdatePillarColor(Ogre::Entity * pillar, 
		                   const Ogre::ColourValue & ambient,
		                   const Ogre::ColourValue & diffuse);

	void onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);

	float TubeTouchGroundPercent(ACTubeShapeObject * tubeObj);
	
	SYScoreTable * GetScoreTable();

	std::vector<TubeInScene> m_Tubes;
};

#endif