#ifndef _TRAINSCORESYSTEM_
#define _TRAINSCORESYSTEM_
#include <map>
class CScoreMgr;
class CTipMgr;
class CBasicTraining;
class VeinConnectObject;
class MxEvent;
class MisMedicOrganInterface;
class MxSliceOffOrganEvent;

class TrainScoreSystem
{
public:
	TrainScoreSystem(CBasicTraining * HostTrain);

	virtual ~TrainScoreSystem();

	void ProcessTrainEvent(MxEvent * pEvent);
	void SetTrainRunning(bool run);

    bool IsTrainFinished()	{ return m_bIsTrainFinished; }
    bool IsSeperateVeinFinished()	{ return m_IsSeperateVeinFinish; }

	void SetTrainSucced()
	{
		m_bIsTrainFinished = true;
	}

	void SetEnabledAddOperateItem(bool enable);

	void SaveOperateItems();


	bool FinishCutTube() {return m_TubeCutStepFinished;}

	bool m_TubeCutStepFinished;//is Tube cut step finished

	bool m_IsSeperateVeinFinish;

	bool m_IsSeperateVeinBottomFinish;

	bool m_bSevereErrorForTrain;

	bool m_bIsTrainFinished;

    bool m_bCysticDuctCutted;

    bool m_bArteriaeCysticaCutted;

private:
	void ScoreForCutTube(MisMedicOrganInterface * pOrganInterface);

	void ScoreForAddHemoClip(int typeId);
	
	void ScoreForAddSilverClip(int typeId);

	void ScoreForCongulateConnect(VeinConnectObject * connobj , int typeId);

	void ScoreForSliceOffOrgan(MxSliceOffOrganEvent * pEvent);

	void HintForDamageLiver();

	void HintForPunctureOrgan(int organid);

	void AddBleedOperateItem(int organId);

	int GetNumberOfActiveBleedPoint();

	bool m_TrainRunning;//whether train is running
	
	std::map<int , int> m_OrganCutCount;//cut count for all organ

	std::map<int , int> m_OrganHemiClipCount;//cut count for all organ
	
	CBasicTraining * m_HostTrain;



	bool m_canAddOperateItem;

	/// 胆道系统损伤次数
	int m_damageBiliaryTimes;

	/// 安全剪短的距离
	const float m_safeCutSpacing;

	/// 钛夹夹闭距离
	const float m_safeClipSpacing;
};

#endif