#pragma once
#include "MXCommon.h"
#include <string>
#include <vector>


class MXCOMMON_API MxOperateItem
{
public:

	struct ScoreItem
	{
		std::string name;
		std::string description;
		float score;
		float timeScore;
		int time;
		int times;
	};

	/**
		�������������������
	*/
	enum ItemCategory
	{
		/// 0:��������
		IC_Other,
		/// 1:����������
		IC_Basic,
		/// 2:��е������
		IC_Tool,
		/// 3:��ȷ������
		IC_Correct,
		/// 4:���������
		IC_Incorrect
	};

	/**
		ֵ�����ͣ���ֵ���͡��Ƿ��
		ָʾvalue��ʾ�ĺ���
	*/
	enum ValueType
	{
		VT_Unkonw = 0,
		/// ������ֵ
		VT_NumberInt = 0x1 << 1,
		/// �������͵���ֵ
		VT_NumberFloat = 0x1 << 2,
		/// ��ֵ����
		VT_Number = VT_NumberInt | VT_NumberFloat,
		/// �Ƿ�
		VT_YesNo = 0x1 << 3,
		/// ʱ������
		VT_Time = 0x1 << 4,
		/// ����ˮƽ
		VT_OperateLevel = 0x1 << 5,
		/// ����ˮƽ
		VT_ScoreLevel = 0x1 << 6
	};

	MxOperateItem();

	/**
		����һ������¼�����Ĳ�����
	*/
	MxOperateItem(int id,
				  const std::string& name,
				  const std::string& description,
				  int operateTime = -1,
				  ItemCategory categroy = IC_Other);

	/**
		����һ����¼�����Ĳ�����
	*/
	MxOperateItem(int id,
				  const std::string& operateItemName,
				  const std::string& operateItemDescription,
				  int operateTime,
				  ItemCategory category,
				  const std::string& scoreName,
				  const std::string& scoreDescription,
				  float score,
				  float timeScore);

	~MxOperateItem(void);

	inline const std::string& GetName() const {return m_name;}
	inline const std::string& GetDescription() const {return m_description;}
	inline int GetId() const {return m_id;}

	inline ItemCategory GetCategory() const {return m_itemCategory;}
	inline void SetCategory(ItemCategory category) { m_itemCategory = category;}

	inline ValueType GetValueType() const {return m_valueType;}
	inline void SetValueType(ValueType valueType) {m_valueType = valueType;}
	inline void SetValueType(const std::string& valueType) {m_valueType = ConvertStringToValueType(valueType);}

	inline float GetValue() const {return m_value;}
	inline void SetValue(float value) {m_value = value;}
	

	inline bool HasOperateTime() {return m_operateTime.size() != 0;}

	void GetOperateTime(std::vector<int> & operateTime) const;
	inline const std::vector<int>& GetOperateTime() const {return m_operateTime;}
	void SetLastOperateTime(int time);
	int GetLastOperateTime();
	inline void AddOperateTime(int time) {m_operateTime.push_back(time);}

 	void ClearOperateTime();
 	void ClearScoreItems();


	static ItemCategory ConvertValueToCategory(int value);
	static ValueType ConvertStringToValueType(const std::string& valueType);

// 	bool MergeOperateItem(const std::string& name,float value,int operateTime);
// 
// 	bool MergeOperateItem(const std::string& name,
// 						  float value,
// 						  int operateTime,
// 						  const std::string& scoreName,
// 						  const std::string& scoreDescription,
// 						  float score,
// 						  float timeScore);

	/** 
		���һ���÷���
		ע�⣺���øú��������Զ����һ������ʱ��
	*/
	void AddScoreItem(float score,float timeScore,int time);

	inline const std::vector<ScoreItem>& GetScoreItems()const { return m_scoreItems;}

	/** 
		�����еķ�����ϲ�����һ��
		useLastOperateTime�������ϲ�֮��ķ������ʱ��ֵ��ture��Ϊ���һ���ʱ�䣻false����һ���ʱ��
		����Ըò�����ԭ���Ĳ���ʱ����Ӱ��
	*/
	void MergeAllScoreItem(bool useLastOperateTime = true);

	/**
		�ϲ��ò���������в���ʱ��
		useLastOperateTime�������ϲ�֮��Ĳ������ʱ��ֵ��ture��Ϊ���һ���ʱ�䣻false����һ���ʱ��
	*/
	void MergeOperateTime(bool useLastOperateTime = true);

	/**
		��ȡ��ط���
	*/
	float GetTotalOperateScore();
	float GetTotalTimeScore();
	float GetTotalScore();

	/**
		�������һ��������ķ���ֵ��score�������֣���timeScore��ʱ��֣�
		scoreFactor:��������������
		timeScoreFactor:ʱ�����������
	*/
	void ScaleScoreValueOfLastScoreItem(float scoreFactor,float timeScoreFactor);

	void ScaleScoreValueOfLastScoreItem(float factor);

private:
	int m_id;

	std::string m_name;

	std::string m_description;

	/// value��ֵ���ڱ��ʱ�䡢�������ٶȡ������Լ�'�Ƿ�'�е�һ��
	float m_value;
	
	ItemCategory m_itemCategory;

	/// ����������е÷֣���ñ��������ݺͷ���������ݱ���һ��
	std::vector<int> m_operateTime;

	/// ����������е÷֣���ñ�����¼�÷���ϸ
	std::vector<ScoreItem> m_scoreItems;

	ValueType m_valueType;
};