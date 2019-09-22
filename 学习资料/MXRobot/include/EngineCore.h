#pragma once

#include "IMXDefine.h"
#include "OgreFrameListener.h"
#include "TrainingMgr.h"
#include "MXOgreWrapper.h"
#include "InputInterface.h"

using namespace std;

class ITool;
class EngineCore// :public Ogre::FrameListener
{
public:
	EngineCore(void);
	~EngineCore(void);

public:
	//virtual bool frameStarted(const Ogre::FrameEvent& evt);

	//virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

	//virtual bool frameEnded(const Ogre::FrameEvent& evt);

public:
	bool Initialize();

	//void PhysicUpdate();

	void ProfilerUpdate();

	void ProfilerUpdate( int fps );

	void TouchUpdate();

	float Update();

	

	bool ProcessInput(float dt);

	//void DealWithClipAppliperInput( ITool * pTool, float fShaftAside, DeviceType deviceType );

	bool Terminate();

	static bool IsUnlocked()	{	return m_bUnlock; }

	
protected:
	//bool InitConfig();

	bool InitLogic();

	double GetCurrentTimeInSeconds();
public:
	inline CTrainingMgr* GetTrainingMgr() const { return m_pTrainingMgr; }
	inline void SetTrainingMgr( CTrainingMgr* pTrainingMgr) { m_pTrainingMgr = pTrainingMgr; }

private:
	CTrainingMgr * m_pTrainingMgr;

private:
	double m_flastframe;

	std::ofstream m_writeFile;

	//std::ifstream timeSliceFile;

	//float m_fTimeSlice;

   // int m_iUpdateTimes;

	bool m_bShowFrame;
	bool m_bShowProfiler;

	//CSoundManager *m_pSoundForLeftElectric;
 //   CSoundManager *m_pSoundForRightElectric;
 //   CSoundManager *m_pSoundForHarmonicScalpel;


	double m_RightDistance;
	double m_LeftDistance;

	Ogre::Vector3 m_vecRightLastPos;
	Ogre::Vector3 m_vecLeftLastPos;

	Ogre::Quaternion m_quatLastRight;
	Ogre::Quaternion m_quatLastLeft;

	double m_dLeftRotaryX;
	double m_dLeftRotaryY;
	double m_dLeftRotaryZ;

	double m_dRightRotaryX;
	double m_dRightRotaryY;
	double m_dRightRotaryZ;


	//�Ѽ�ǯÿ���ſ��պ�ʱ����еͷ�����ƶ��ľ���
	const float m_fClAppliperDistance;
	//�Ѽ�ǯ��е��еͷ��ƫ����
	const float m_fClAppliperOffset;
	//float  m_fTempAside;//��һ֡�ĽǶȴ�С����������αȽ�
	float   m_fTempKey;//��¼һ����ʱ�Ƕ�
	//bool m_bMaxAside;//�Ƿ񵽴����Ƕ�
	//bool m_bMinAside;
	bool m_bReturn;//�Ƿ���û�������С���Ƕ�ʱ����

	bool m_bCalibrationFinished;
	bool m_bLeftTouchCalibrationFinished;
	bool m_bRightTouchCalibrationFinished;
	bool m_bCameraCalibrationFinished;
	static bool m_bUnlock;
	float m_fGradualAlpha;

	LARGE_INTEGER m_litmp;
	double m_lastframetime;

	//statistics fps
	int m_FrameCount;
	float m_AccumulateTime;
	int m_UpdateFPS;
	//

	bool m_bCameraFixed;

};
