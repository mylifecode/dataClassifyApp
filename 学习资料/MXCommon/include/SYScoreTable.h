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
	TA_ActionStablity = 0,		//�����ȶ���
	TA_ActionAccuracy,			//������ȷ��
	TA_CameraDirectionSense,	//���·����
	TA_ToolUsed,				//��еʹ��
	TA_DoubleHandCoordination,	//˫��Э�����
	TA_HandEyeCoordination,		//����Э��
	TA_ClinicalThinking,		//�ٴ�˼ά
	TA_CuttingSkill,			//�и��
	TA_SeparationSkill,			//���뼼��
	TA_FineOperation,			//��ϸ����
	TA_ClampStablity,			//�г��ȶ���
	TA_StopBleedingSkill,		//ֹѪ����
	TA_LigatureSkill,			//��������
	TA_SutureSkill,				//��ϼ���

	TA_NumOfAbility				//������������ʹ�ã�
};

class MXCOMMON_API SYScoreTable
{
public:
	SYScoreTable();

	~SYScoreTable();

	/** 
		��ȡ���ֱ�ı���
		8λ����:2λ��Ʒ���� + 1λѵ������ + 3λѵ������ + 2λ�汾����
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

	/** ��ָ���Ĳ������������һ������������,�ɹ����ض�Ӧ�����������ݣ�ʧ�ܷ���nullptr*/
	SYScoreItemData* AddScoreItemData(SYStepData* stepData, const QString& scoreItemContent, const QString& scoreItemCode, int scoreValue);

	const QSet<SYScorePointDetailData*>& GetAllScorePointDetailData() const { return m_scorePointDetailDatas; }

	void AddScorePointDetailData(SYScorePointDetailData* scorePointDetailData);

	/** ��ָ���������������һ����������ϸ���ݣ��ɹ����ض�Ӧ����������ϸ���ݣ�ʧ�ܷ���nullptr */
	SYScorePointDetailData* AddScorePointDetailData(SYScoreItemData* scoreItemData,const QString& scorePointDetailName,const QString& scorePointDetailCode,int scoreValue,int abilitys);

	/** ��ȡ��ǰ���ֱ�ı��� */
	QString GetScoreTableCode();

	/** ��ȡ��ǰ��������� */
	QString GetScoreCode(SYScorePointDetailData* spdd);

	QString GetScoreCode(SYStepData* stepData, SYScoreItemData* scoreItemData, SYScorePointDetailData* spdd);
	

	/** �������ݵ����ݿ��� */
	void Update();

	void Delete(SYUserData* userData);

	/** ���ݾ���ĳһ�����ֱ����ȡ stepData��scoreItemData��spdd */
	void GetData(const QString& scoreCode, SYStepData** ppStepData, SYScoreItemData** ppScoreItemData, SYScorePointDetailData** ppSpdd);

	/** ���ݾ���ĳһ�����ֱ����ȡ��Ӧ�ķ���ֵ���羵ͷѵ��I�еı���:0010100111��������6 */
	int GetScoreValue(const QString& scoreCode) const;

	static const QString& ConvertTrainTypeCodeToTrainTypeName(const QString& typeCode);

	/** ��ȡѵ������ */
	const QString& GetTrainTypeName() const;

	/** ��ȡѵ�����ͱ��� */
	const QString& GetTrainTypeCode() const;

	/** ��ȡѵ���� */
	const QString& GetTrainName() const;

	/** ��ȡѵ������ */
	const QString& GetTrainCode() const;

	int GetNumOfScorePointDetailData() const { return m_scorePointDetailDatas.size(); }

	void UpdateAbilityCounters();

	/** ��ȡָ�������ļ����� */
	int GetCounterOfAbility(SYTrainAbility ability);

	const int* GetAbilityCounters() const { return m_abilityCounters; }

	/** ��ȡǰn������������ */
	void GetAbilityCounters(int* counters, int n) const ;

	/** static functions */
	static const QString& GetAbilityName(SYTrainAbility ability);

	/** �ж�abilitys���Ƿ��������a */
	static bool HasAbility(int abilitys, SYTrainAbility a);

	/** ����ѵ�����ͱ��뼰ѵ�������ȡ��Ӧ�����ֱ���� */
	static QString GetScoreTableCode(const QString& trainTypeCode, const QString& trainCode);

public:
	/// ��Ʒ����
	const static QString ProductCode;

	/// �汾����
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
	/// ����������������ͳ��ÿ�������ĸ���
	int m_abilityCounters[TA_NumOfAbility];
};

