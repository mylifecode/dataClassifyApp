#ifndef _ACPRECISENAVIGATIONTRAIN_
#define _ACPRECISENAVIGATIONTRAIN_

#include "MisNewTraining.h"
#include "PhysicsWrapper.h"
#include "MisMedicOrganOrdinary.h"
#include "MisMedicRigidPrimtive.h"
class MisMedicOrgan_Ordinary;

class ACPreciseNavigationTrain :public MisNewTraining
{
public:
	enum BallsState
	{
		BS_NONE,
		BS_LOCATED,
		BS_SUCCEED,
	};
	class PBall
	{
	public:
		PBall(MisMedicRigidPrimtive * prim , int Color);

		void ExtractMesh();

		//return value 0 not touched 1 touched wrong place 2 touche write
		int CheckSegmentIntersect(const GFPhysVector3 & setStart, const GFPhysVector3 & setEnd);//brute forces check need optimize
	
		GFPhysVector3 m_InitPos;
		
		MisMedicRigidPrimtive * m_Primitive;
		float m_Radius;
		std::vector<Ogre::Vector3>  m_Vertex;
		std::vector<Ogre::Vector2>  m_TexCoords;
		std::vector<unsigned int>   m_Indices;
		int m_Color;//0- red  1- blue 2 - green

		//logic data
		bool m_HasBeenTouched;
		bool m_HasTouchedByWrongTool;
		bool m_IsTouchOrderWrong;
		bool m_HasLocatedBeFailed;
		bool m_HasBeenLocated;

		bool m_HasDeviateInProcess;
		//bool m_HasTouchedInWrongPlace;
		//bool m_IsDeviateFromeRightPlace;

		//bool m_HasTouchedRightPlace;

		
		BallsState m_State;
		float m_LocatedKeepTime;


	};
	ACPreciseNavigationTrain(void);
	
	virtual ~ACPreciseNavigationTrain(void);

public:
	
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);
	
	virtual bool Update(float dt);

	int GetCurrentBallColor()
	{
		return m_pballs[m_CurrDstBallIndex].m_Color;
	}

	int GetLeftToolColor()
	{
		return 0;
	}

	int GetRightToolColor()
	{
		return 1;
	}
private:

	virtual SYScoreTable * GetScoreTable();

	void OnSaveTrainingReport();

	virtual void AddDefaultScoreItemDetail()
	{

	}

	void OnToolCreated(ITool * tool, int side);

	GFPhysAlignedVectorObj<PBall> m_pballs;

	Ogre::MaterialPtr m_RedCrePtr;
	Ogre::MaterialPtr m_BlueCrePtr;

	Ogre::MaterialPtr m_GreenFinishPtr;
	float m_TouchTime;
	int   m_CurrDstBallIndex;
	//TrainState m_state;
};

#endif