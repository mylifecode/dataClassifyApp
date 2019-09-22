#ifndef _BASIC_PickupObjectTraining
#define _BASIC_PickupObjectTraining
#include "MisNewTraining.h"
#include <OgreAxisAlignedBox.h>
#include <OgreMovablePlane.h>

class MisMedicOrgan_Ordinary;


class CPickupObjectTraining :public MisNewTraining
{
public:
	CPickupObjectTraining(float limitTime = -1.f);

	virtual ~CPickupObjectTraining(void);

public:
	virtual bool Initialize(CXMLWrapperTraining * pTrainingConfig, CXMLWrapperToolConfig * pToolConfig);

	void InitCode();

	virtual SYScoreTable* GetScoreTable();

	virtual bool Update(float dt);
	
	virtual void InternalSimulateStart(int currStep , int TotalStep , Real dt);
	virtual void InternalSimulateEnd(int currStep , int TotalStep , Real dt);
	 
	bool BeginRendOneFrame(float timeelpsed);

	void OnSaveTrainingReport();

	virtual void onRSFaceContactsSolveEnded(const GFPhysAlignedVectorObj<GFPhysSoftFaceRigidContact> & RSContactConstraints);//when all contact solved

private:
	void GenObjectConstructInfoSequence();

	void UpdateCollideData();

	void CreateOneObject();

	void RemoveCurrentObject();

	void OnTrainingIlluminated();

	void OnTimerTimeout(int id, float dt, void* userData);

	enum ObjectPosition{
		OP_AboveFloor,
		OP_ContactFloor,
		OP_UnderFloor
	};
	/** 获取当前其物体相对于地面的位置 */
	ObjectPosition GetObjectPosition();

	bool ObjectIsClamped();

	void ResetState();
		
private:
	bool checkDianzi();

	std::vector<MisMedicDynObjConstructInfo> m_objectConstructInfoSequence;

	int m_nextConstructInfoIndex;
	MisMedicOrgan_Ordinary* m_curOrganObject;

	bool m_hasUpdatedCollideData;
	Ogre::AxisAlignedBox m_aabbForTriangle;
	Ogre::AxisAlignedBox m_aabbForQuat;
	Ogre::AxisAlignedBox m_aabbForHexagon;
	//Ogre::MovablePlane* m_floorPlane;
	bool m_hasFloorPlane;
	Ogre::MovablePlane m_floorPlane;

	/// 每次投放的时间限制
	const float m_limitTime;
	const bool m_hasLimitTime;

	struct ObjectStageState{
		bool contactWithTool;
		bool isClamped;
	};

	/// 物体有三个阶段的状态:空中、与地面接触、在地面之下
	ObjectStageState m_objectStageStates[3];

	/// 物体当前的位置，即处于哪个阶段
	ObjectPosition m_curObjectPosition;
	/// 最近一次器械接触物体时的位置
	ObjectPosition m_lastToolPositionContactWithObject;
	/// 最近一次器械接触物体时是否夹中物体
	bool m_lastToolClampStateContactWithObject;
	bool m_contactWithTool;

	/// 是否有被夹起过
	bool m_hasClampedAndAboveFloor;
	int m_dropTimes;

	struct StepCode{
		/// 成功投放
		QString successPost;
		/// 位置有误
		QString positionError;
		/// 投放方式有误
		QString methodError;

		/// 夹起物体
		QString clampObject;
		/// 未夹起
		QString unclampObject;

		/// 未掉落
		QString undrop;
		/// 有掉落
		QString drop;
		/// 掉落多次
		QString multiDrop;
	};

	/// 5个步骤
	std::vector<StepCode> m_stepCodes;
};

#endif