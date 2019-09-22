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
		用于描述操作项的种类
	*/
	enum ItemCategory
	{
		/// 0:其他类型
		IC_Other,
		/// 1:基础数据项
		IC_Basic,
		/// 2:器械数据项
		IC_Tool,
		/// 3:正确操作项
		IC_Correct,
		/// 4:错误操作项
		IC_Incorrect
	};

	/**
		值的类型：数值类型、是否等
		指示value标示的含义
	*/
	enum ValueType
	{
		VT_Unkonw = 0,
		/// 整形数值
		VT_NumberInt = 0x1 << 1,
		/// 浮点类型的数值
		VT_NumberFloat = 0x1 << 2,
		/// 数值类型
		VT_Number = VT_NumberInt | VT_NumberFloat,
		/// 是否
		VT_YesNo = 0x1 << 3,
		/// 时间类型
		VT_Time = 0x1 << 4,
		/// 操作水平
		VT_OperateLevel = 0x1 << 5,
		/// 分数水平
		VT_ScoreLevel = 0x1 << 6
	};

	MxOperateItem();

	/**
		构造一个不记录分数的操作项
	*/
	MxOperateItem(int id,
				  const std::string& name,
				  const std::string& description,
				  int operateTime = -1,
				  ItemCategory categroy = IC_Other);

	/**
		构造一个记录分数的操作项
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
		添加一个得分项
		注意：调用该函数不会自动添加一个操作时间
	*/
	void AddScoreItem(float score,float timeScore,int time);

	inline const std::vector<ScoreItem>& GetScoreItems()const { return m_scoreItems;}

	/** 
		将所有的分数项合并到第一项
		useLastOperateTime：决定合并之后的分数项的时间值，ture：为最后一项的时间；false：第一项的时间
		不会对该操作项原来的操作时间有影响
	*/
	void MergeAllScoreItem(bool useLastOperateTime = true);

	/**
		合并该操作项的所有操作时间
		useLastOperateTime：决定合并之后的操作项的时间值，ture：为最后一项的时间；false：第一项的时间
	*/
	void MergeOperateTime(bool useLastOperateTime = true);

	/**
		获取相关分数
	*/
	float GetTotalOperateScore();
	float GetTotalTimeScore();
	float GetTotalScore();

	/**
		缩放最后一个分数项的分数值：score（动作分）、timeScore（时间分）
		scoreFactor:动作分缩放因子
		timeScoreFactor:时间分缩放因子
	*/
	void ScaleScoreValueOfLastScoreItem(float scoreFactor,float timeScoreFactor);

	void ScaleScoreValueOfLastScoreItem(float factor);

private:
	int m_id;

	std::string m_name;

	std::string m_description;

	/// value的值用于表达时间、次数、速度、长度以及'是否'中的一个
	float m_value;
	
	ItemCategory m_itemCategory;

	/// 如果操作项有得分，则该变量的内容和分数项的内容保持一致
	std::vector<int> m_operateTime;

	/// 如果操作项有得分，则该变量记录得分明细
	std::vector<ScoreItem> m_scoreItems;

	ValueType m_valueType;
};