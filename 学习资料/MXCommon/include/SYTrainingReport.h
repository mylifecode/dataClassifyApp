#pragma once
#include "MXCommon.h"
#include "float.h"
#include "MxOperateItem.h"
#include "SYScoreItemDetail.h"
#include <vector>

class SYScoreTable;

class MXCOMMON_API SYTrainingReport
{
	SYTrainingReport(void);
	~SYTrainingReport(void);
public:
	static SYTrainingReport* Instance();
	void Reset();
	
	//for training
	int GetTotalTime() {return m_totalTime;}
	void SetTotalTime(int time) {m_totalTime = time;}

	int GetRemainTime() {return m_remainTime;}
	void SetRemainTime(int time) {m_remainTime = time;}

	//for tool
	/**
		通电总世间
	*/
	float GetElectricTime() {return m_electricTime;}
	void SetElectricTime(float time) {m_electricTime = time;}

	/**
		有效通电总时间
	*/
	float GetValidElectricTime() {return m_validElectricTime;}
	void SetValidElectricTime(float time) { m_validElectricTime = time;}

	/**
		通电效率
	*/
	float GetElectricEfficiency() 
	{
		float rate = 0.f;
		if(m_electricTime > FLT_EPSILON)
		{
			rate = m_validElectricTime / m_electricTime;
		}
		return rate;
	}

	/**
		最大的持续通电开始时间
	*/
	float GetMaxKeeppingElectricBeginTime() {return m_maxKeeppingElectricBeginTime;}
	void SetMaxKeeppingElectricBeginTime(float time) {m_maxKeeppingElectricBeginTime = time;}

	/**
		最大的持续通电时间
	*/
	float GetMaxKeeppingElectricTime() {return m_maxKeeppingElectricTime;}
	void SetMaxKeeppingElectricTime(float time) {m_maxKeeppingElectricTime = time;}

	/**
		左手器械夹闭次数
	*/
	int GetLeftToolClosedTimes() {return m_leftToolClosedTimes;}
	void SetLeftToolClosedTimes(int times){m_leftToolClosedTimes = times;}

	/**
		右手器械夹闭次数
	*/
	int GetRightToolClosedTimes() {return m_rightToolClosedTimes;}
	void SetRightToolClosedTimes(int times) { m_rightToolClosedTimes = times;}

	/**
		左手器械移动距离
	*/
	float GetLeftToolMovedDistance() {return m_leftToolMovedDistance;}
	void SetLeftToolMovedDistance(float dis) {m_leftToolMovedDistance = dis;}

	/**
		右手器械移动距离
	*/
	float GetRightToolMovedDistance() {return m_rightToolMovedDistance;}
	void SetRightToolMovedDistance(float dis) {m_rightToolMovedDistance = dis;}

	/**
		左器械平均移动速度
	*/
	float GetLeftToolMovedSpeed() {return m_leftToolMovedSpeed;}
	void SetLeftToolMovedSpeed(float speed) {m_leftToolMovedSpeed = speed;}

	/**
		右手器械平均移动速度
	*/
	float GetRightToolMovedSpeed() {return m_rightToolMovedSpeed;}
	void SetRightToolMovedSpeed(float speed) {m_rightToolMovedSpeed = speed;}

	/**
		释放钛夹个数
	*/
	int GetNumberOfReleasedTitanicClip() {return m_nReleasedTitanicClip;}
	void SetNumberOfReleasedTitanicClip(int n) {m_nReleasedTitanicClip = n;}

	float GetElectricAffectTimeForHemoClip(){ return m_electricAffectTimeForHemoclip;}
	void SetElectricAffectTimeForHemoClip(float time){ m_electricAffectTimeForHemoclip = time;}

	float GetElectricAffectTimeForOrdinaryOrgan() {return m_electricAffectTimeForOrdinaryOrgan;}
	void SetElectricAffectTimeForOrdinaryOrgan(float time) {m_electricAffectTimeForOrdinaryOrgan = time;}

	//for organ
	/**
		最大的流血时间
	*/
	float GetMaxBleedTime() {return m_maxBleedTime;}
	void SetMaxBleedTime(float time) {m_maxBleedTime = time;}


	void AddOperateItems(const std::vector<MxOperateItem> & items);
	const std::vector<MxOperateItem>& GetOperateItems() {return m_operateItems;}

	void GetOperateItems(std::vector<MxOperateItem>& operateItems);

	void GetScoreItemDetails(std::vector<SYScoreItemDetail>& scoreItemDetails) const;
	
	void SetScoreItemDetail(SYScoreTable* scoreTable,const std::vector<SYScoreItemDetail>& scoreItemDetails);

	SYScoreTable* GetScoreTable() const { return m_scoreTable; }

private:
	int m_totalTime;
	int m_remainTime;
	float m_electricTime;
	float m_validElectricTime;
	float m_maxKeeppingElectricBeginTime;
	float m_maxKeeppingElectricTime;
	float m_maxBleedTime;
	int m_nReleasedTitanicClip;
	int m_leftToolClosedTimes;
	int m_rightToolClosedTimes;
	float m_leftToolMovedDistance;
	float m_rightToolMovedDistance;
	float m_leftToolMovedSpeed;
	float m_rightToolMovedSpeed;
	float m_electricAffectTimeForHemoclip;
	float m_electricAffectTimeForOrdinaryOrgan;
	std::vector<MxOperateItem> m_operateItems;

	SYScoreTable* m_scoreTable;
	std::vector<SYScoreItemDetail> m_scoreItemDetails;
};