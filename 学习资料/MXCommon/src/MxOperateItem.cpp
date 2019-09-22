#include "MxOperateItem.h"


MxOperateItem::MxOperateItem(void)
:m_id(1000000),
m_name(""),
m_description(""),
m_value(0),
m_operateTime(0),
m_itemCategory(IC_Other),
m_valueType(VT_NumberInt)
{
}

MxOperateItem::MxOperateItem(int id,
							 const std::string& name,
							 const std::string& description,
							 int operateTime,
							 ItemCategory category)
:m_id(id),
m_name(name),
m_description(description),
m_value(0),
m_valueType(VT_NumberInt),
m_itemCategory(category)
{
	if(operateTime >= 0)
		m_operateTime.push_back(operateTime);
}

MxOperateItem::MxOperateItem(int id,
							 const std::string& operateItemName,
							 const std::string& operateItemDescription,
							 int operateTime,
							 ItemCategory category,
							 const std::string& scoreName,
							 const std::string& scoreDescription,
							 float score,
							 float timeScore)
:m_id(id),
m_name(operateItemName),
m_description(operateItemDescription),
m_value(0),
m_valueType(VT_NumberInt),
m_itemCategory(category)
{
	if(operateTime >= 0)
		m_operateTime.push_back(operateTime);

	m_scoreItems.resize(1);
	ScoreItem & scoreItem = m_scoreItems[0];
	scoreItem.name = scoreName;
	scoreItem.description = scoreDescription;
	scoreItem.score = score;
	scoreItem.timeScore = timeScore;
	scoreItem.time = operateTime;
	scoreItem.times = 1;
}

MxOperateItem::~MxOperateItem(void)
{

}

MxOperateItem::ItemCategory MxOperateItem::ConvertValueToCategory(int value)
{
	ItemCategory categroy = IC_Other;
	if(value > IC_Other && value <= IC_Incorrect)
		categroy = static_cast<ItemCategory>(value);

	return categroy;
}

MxOperateItem::ValueType MxOperateItem::ConvertStringToValueType(const std::string& valueType)
{
	ValueType type = VT_Unkonw;
	if(valueType == "Number")
		type = VT_Number;
	else if(valueType == "NumberInt")
		type = VT_NumberInt;
	else if(valueType == "NumberFloat")
		type = VT_NumberFloat;
	else if(valueType == "YesNo")
		type = VT_YesNo;
	else if(valueType == "Time")
		type = VT_Time;
	else if(valueType == "OperateLevel")
		type = VT_OperateLevel;
	else if(valueType == "ScoreLevel")
		type = VT_ScoreLevel;
	
	return type;
}

// bool MxOperateItem::MergeOperateItem(const std::string& name,float value,int operateTime)
// {
// 	if(m_name == name)
// 	{
// 		m_value += value;
// 		m_operateTime.push_back(operateTime);
// 		return true;
// 	}
// 	else
// 		return false;
// }
// 
// bool MxOperateItem::MergeOperateItem(const std::string& name, 
// 									 float value,
// 									 int operateTime, 
// 									 const std::string& scoreName, 
// 									 const std::string& scoreDescription, 
// 									 float score, 
// 									 float timeScore)
// {
// 	if(m_name == name)
// 	{
// 		m_value += value;
// 		m_operateTime.push_back(operateTime);
// 
// 		int size = m_scoreItems.size();
// 		m_scoreItems.resize(size + 1);
// 
// 		ScoreItem & scoreItem = m_scoreItems[size];
// 		scoreItem.name = scoreName;
// 		scoreItem.description = scoreDescription;
// 		scoreItem.score = score;
// 		scoreItem.timeScore = timeScore;
// 		scoreItem.times = 1;
// 
// 		return true;
// 	}
// 	else
// 		return false;
// }

void MxOperateItem::GetOperateTime(std::vector<int> & operateTime) const
{
	for(std::size_t i = 0;i < m_operateTime.size();++i)
		operateTime.push_back(m_operateTime[i]);
}

void MxOperateItem::SetLastOperateTime(int time)
{
	if(m_operateTime.size())
		*(m_operateTime.rbegin()) = time;
}

int MxOperateItem::GetLastOperateTime()
{
	if(m_operateTime.size())
		return *(m_operateTime.rbegin());
	else
		return -1;
}

void MxOperateItem::ClearOperateTime()
{
	m_operateTime.clear();
}

void MxOperateItem::ClearScoreItems()
{
	m_scoreItems.clear();
}

void MxOperateItem::AddScoreItem(float score,float timeScore,int time)
{
	ScoreItem item;
	item.name = m_name;
	item.description = m_description;
	item.score = score;
	item.timeScore = timeScore;
	item.time = time;
	item.times = 1;

	m_scoreItems.push_back(item);
}

void MxOperateItem::MergeAllScoreItem(bool useLastOperateTime)
{
	if(m_scoreItems.size() > 1)
	{
		ScoreItem & scoreItem = m_scoreItems[0];
		int time = 0;
		for(std::size_t i = 1;i < m_scoreItems.size();++i)
		{
			scoreItem.score += m_scoreItems[i].score;
			scoreItem.timeScore += m_scoreItems[i].timeScore;
			//scoreItem.time = m_scoreItems[i].time;				//时间设置为最后一次操作的时间
			time = m_scoreItems[i].time;
			scoreItem.times += m_scoreItems[i].times;
		}
		
		if(useLastOperateTime)
			scoreItem.time = time;

		m_scoreItems.resize(1);
	}
}

void MxOperateItem::MergeOperateTime(bool useLastOperateTime /* = true */)
{
	if(m_operateTime.size() > 1)
	{
		if(useLastOperateTime)
			m_operateTime[0] = m_operateTime[m_operateTime.size() - 1];

		m_operateTime.resize(1);
	}
}

float MxOperateItem::GetTotalOperateScore()
{
	float score = 0.f;

	for(size_t i = 0;i < m_scoreItems.size();++i)
	{
		score += m_scoreItems[i].score;
	}

	return score;
}

float MxOperateItem::GetTotalTimeScore()
{
	float score = 0.f;

	for(size_t i = 0;i < m_scoreItems.size();++i)
	{
		score += m_scoreItems[i].timeScore;
	}

	return score;
}

float MxOperateItem::GetTotalScore()
{
	return GetTotalOperateScore() + GetTotalTimeScore();
}

void MxOperateItem::ScaleScoreValueOfLastScoreItem(float scoreFactor,float timeScoreFactor)
{
	std::vector<ScoreItem>::reverse_iterator itr = m_scoreItems.rbegin();
	if(itr != m_scoreItems.rend())
	{
		itr->score *= scoreFactor;
		itr->timeScore *= timeScoreFactor;
	}
}

void MxOperateItem::ScaleScoreValueOfLastScoreItem(float factor)
{
	ScaleScoreValueOfLastScoreItem(factor,factor);
}