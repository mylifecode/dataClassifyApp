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
	/** ��ȡ��ǰ����������ڵ����λ�� */
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

	/// ÿ��Ͷ�ŵ�ʱ������
	const float m_limitTime;
	const bool m_hasLimitTime;

	struct ObjectStageState{
		bool contactWithTool;
		bool isClamped;
	};

	/// �����������׶ε�״̬:���С������Ӵ����ڵ���֮��
	ObjectStageState m_objectStageStates[3];

	/// ���嵱ǰ��λ�ã��������ĸ��׶�
	ObjectPosition m_curObjectPosition;
	/// ���һ����е�Ӵ�����ʱ��λ��
	ObjectPosition m_lastToolPositionContactWithObject;
	/// ���һ����е�Ӵ�����ʱ�Ƿ��������
	bool m_lastToolClampStateContactWithObject;
	bool m_contactWithTool;

	/// �Ƿ��б������
	bool m_hasClampedAndAboveFloor;
	int m_dropTimes;

	struct StepCode{
		/// �ɹ�Ͷ��
		QString successPost;
		/// λ������
		QString positionError;
		/// Ͷ�ŷ�ʽ����
		QString methodError;

		/// ��������
		QString clampObject;
		/// δ����
		QString unclampObject;

		/// δ����
		QString undrop;
		/// �е���
		QString drop;
		/// ������
		QString multiDrop;
	};

	/// 5������
	std::vector<StepCode> m_stepCodes;
};

#endif