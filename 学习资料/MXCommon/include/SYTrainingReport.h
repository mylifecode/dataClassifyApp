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
		ͨ��������
	*/
	float GetElectricTime() {return m_electricTime;}
	void SetElectricTime(float time) {m_electricTime = time;}

	/**
		��Чͨ����ʱ��
	*/
	float GetValidElectricTime() {return m_validElectricTime;}
	void SetValidElectricTime(float time) { m_validElectricTime = time;}

	/**
		ͨ��Ч��
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
		���ĳ���ͨ�翪ʼʱ��
	*/
	float GetMaxKeeppingElectricBeginTime() {return m_maxKeeppingElectricBeginTime;}
	void SetMaxKeeppingElectricBeginTime(float time) {m_maxKeeppingElectricBeginTime = time;}

	/**
		���ĳ���ͨ��ʱ��
	*/
	float GetMaxKeeppingElectricTime() {return m_maxKeeppingElectricTime;}
	void SetMaxKeeppingElectricTime(float time) {m_maxKeeppingElectricTime = time;}

	/**
		������е�бմ���
	*/
	int GetLeftToolClosedTimes() {return m_leftToolClosedTimes;}
	void SetLeftToolClosedTimes(int times){m_leftToolClosedTimes = times;}

	/**
		������е�бմ���
	*/
	int GetRightToolClosedTimes() {return m_rightToolClosedTimes;}
	void SetRightToolClosedTimes(int times) { m_rightToolClosedTimes = times;}

	/**
		������е�ƶ�����
	*/
	float GetLeftToolMovedDistance() {return m_leftToolMovedDistance;}
	void SetLeftToolMovedDistance(float dis) {m_leftToolMovedDistance = dis;}

	/**
		������е�ƶ�����
	*/
	float GetRightToolMovedDistance() {return m_rightToolMovedDistance;}
	void SetRightToolMovedDistance(float dis) {m_rightToolMovedDistance = dis;}

	/**
		����еƽ���ƶ��ٶ�
	*/
	float GetLeftToolMovedSpeed() {return m_leftToolMovedSpeed;}
	void SetLeftToolMovedSpeed(float speed) {m_leftToolMovedSpeed = speed;}

	/**
		������еƽ���ƶ��ٶ�
	*/
	float GetRightToolMovedSpeed() {return m_rightToolMovedSpeed;}
	void SetRightToolMovedSpeed(float speed) {m_rightToolMovedSpeed = speed;}

	/**
		�ͷ��Ѽи���
	*/
	int GetNumberOfReleasedTitanicClip() {return m_nReleasedTitanicClip;}
	void SetNumberOfReleasedTitanicClip(int n) {m_nReleasedTitanicClip = n;}

	float GetElectricAffectTimeForHemoClip(){ return m_electricAffectTimeForHemoclip;}
	void SetElectricAffectTimeForHemoClip(float time){ m_electricAffectTimeForHemoclip = time;}

	float GetElectricAffectTimeForOrdinaryOrgan() {return m_electricAffectTimeForOrdinaryOrgan;}
	void SetElectricAffectTimeForOrdinaryOrgan(float time) {m_electricAffectTimeForOrdinaryOrgan = time;}

	//for organ
	/**
		������Ѫʱ��
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