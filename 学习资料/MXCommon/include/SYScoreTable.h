#pragma once
#include "MXCommon.h"
#include <QVector>
#include <QSet>
#include <QString>
#include <memory>

class SYUserData;
class SYTrainData;
class SYStepData;
class SYScoreItemData;
class SYScorePointDetailData;

enum SYTrainAbility
{
	TA_ActionStablity = 0,		//动作稳定性
	TA_ActionAccuracy,			//动作精确性
	TA_CameraDirectionSense,	//镜下方向感
	TA_ToolUsed,				//器械使用
	TA_DoubleHandCoordination,	//双手协调配合
	TA_HandEyeCoordination,		//手眼协调
	TA_ClinicalThinking,		//临床思维
	TA_CuttingSkill,			//切割技巧
	TA_SeparationSkill,			//分离技巧
	TA_FineOperation,			//精细操作
	TA_ClampStablity,			//夹持稳定性
	TA_StopBleedingSkill,		//止血技术
	TA_LigatureSkill,			//结扎技术
	TA_SutureSkill,				//缝合技巧

	TA_NumOfAbility				//计数器（程序使用）
};

class MXCOMMON_API SYScoreTable
{
public:
	SYScoreTable();

	~SYScoreTable();

	/** 
		获取评分表的编码
		8位数字:2位产品编码 + 1位训练类型 + 3位训练编码 + 2位版本序列
	*/
	QString GetCode() const;

	const std::shared_ptr<SYUserData>& GetTopLevelUserData() const { return m_topLevelUserData; }

	void SetTopLevelUserData(const std::shared_ptr<SYUserData>& userData);

	//void SetTopLevelUserData(const QString& code);

	const std::shared_ptr<SYTrainData>& GetTrainData() const { return m_trainData; }

	void SetTrainData(const std::shared_ptr<SYTrainData>& trainData);

	void SetTrainData(const QString& trainName, const QString& trainCode);

	const QSet<SYStepData*>& GetAllStepData() const { return m_stepDatas; }

	void AddStepData(SYStepData* stepData);

	SYStepData* AddStepData(const QString& stepName, const QString& stepCode, const QString& stepDetailedDescription, int isKeyStep);

	const QSet<SYScoreItemData*>& GetAllScoreItemData() const { return m_scoreItemDatas; }

	void AddScoreItemData(SYScoreItemData* scoreItemData);

	/** 在指定的步骤数据下添加一个评分项数据,成功返回对应的评分项数据，失败返回nullptr*/
	SYScoreItemData* AddScoreItemData(SYStepData* stepData, const QString& scoreItemContent, const QString& scoreItemCode, int scoreValue);

	const QSet<SYScorePointDetailData*>& GetAllScorePointDetailData() const { return m_scorePointDetailDatas; }

	void AddScorePointDetailData(SYScorePointDetailData* scorePointDetailData);

	/** 在指定的评分项下添加一条评分项详细数据，成功返回对应的评分项详细数据，失败返回nullptr */
	SYScorePointDetailData* AddScorePointDetailData(SYScoreItemData* scoreItemData,const QString& scorePointDetailName,const QString& scorePointDetailCode,int scoreValue,int abilitys);

	/** 获取当前评分表的编码 */
	QString GetScoreTableCode();

	/** 获取当前评分项编码 */
	QString GetScoreCode(SYScorePointDetailData* spdd);

	QString GetScoreCode(SYStepData* stepData, SYScoreItemData* scoreItemData, SYScorePointDetailData* spdd);
	

	/** 更新数据到数据库中 */
	void Update();

	void Delete(SYUserData* userData);

	/** 根据具体某一项评分编码获取 stepData、scoreItemData、spdd */
	void GetData(const QString& scoreCode, SYStepData** ppStepData, SYScoreItemData** ppScoreItemData, SYScorePointDetailData** ppSpdd);

	/** 根据具体某一项评分编码获取对应的分数值，如镜头训练I中的编码:0010100111，将返回6 */
	int GetScoreValue(const QString& scoreCode) const;

	static const QString& ConvertTrainTypeCodeToTrainTypeName(const QString& typeCode);

	/** 获取训练类型 */
	const QString& GetTrainTypeName() const;

	/** 获取训练类型编码 */
	const QString& GetTrainTypeCode() const;

	/** 获取训练名 */
	const QString& GetTrainName() const;

	/** 获取训练编码 */
	const QString& GetTrainCode() const;

	int GetNumOfScorePointDetailData() const { return m_scorePointDetailDatas.size(); }

	void UpdateAbilityCounters();

	/** 获取指定能力的计数器 */
	int GetCounterOfAbility(SYTrainAbility ability);

	const int* GetAbilityCounters() const { return m_abilityCounters; }

	/** 获取前n个能力计数器 */
	void GetAbilityCounters(int* counters, int n) const ;

	/** static functions */
	static const QString& GetAbilityName(SYTrainAbility ability);

	/** 判断abilitys中是否包含能力a */
	static bool HasAbility(int abilitys, SYTrainAbility a);

	/** 根据训练类型编码及训练编码获取对应的评分表编码 */
	static QString GetScoreTableCode(const QString& trainTypeCode, const QString& trainCode);

public:
	/// 产品编码
	const static QString ProductCode;

	/// 版本序列
	const static QString VersionCode;

	const static int ScoreTableCodeLength;

	const static int ScoreCodeLength;

	const static int TrainCodeLength;

	const static int StepCodeLength;

	const static int ScoreItemCodeLength;

	const static int ScorePointDetailCodeLength;

	const static QString TrainAbilityNames[TA_NumOfAbility];

private:
	const QString m_emptyString;
	QString m_tempScoreTableCode;
	QString m_tempScoreCode;
	//SYUserData* m_userData;
	//SYTrainData* m_trainData;
	std::shared_ptr<SYUserData> m_topLevelUserData;
	std::shared_ptr<SYTrainData> m_trainData;
	QSet<SYStepData*> m_stepDatas;
	QSet<SYScoreItemData*> m_scoreItemDatas;
	QSet<SYScorePointDetailData*> m_scorePointDetailDatas;
	/// 能力计数器，用于统计每种能力的个数
	int m_abilityCounters[TA_NumOfAbility];
};

