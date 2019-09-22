#pragma once
#include "XMLSerialize.h"

class CXMLWrapperOperateItem : public CXMLSerialize
{
public:
	CXMLWrapperOperateItem(void);
	~CXMLWrapperOperateItem(void);

	
	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Description)
	/// 主要用于同种类的操作项之间的排序,值越大就越靠后
	IMPL_ATTRIBUTE_LONG(ID)
	/// 操作项的种类，0：其他项；1：基础数据项；2：器械数据项；3、正确操作项；4：错误操作项
	IMPL_ATTRIBUTE_LONG(Category)
	/// 动作分
	IMPL_ATTRIBUTE_FLOAT(Score)
	/// 时间分
	IMPL_ATTRIBUTE_FLOAT(TimeScore)
	/// 时间限制1
	IMPL_ATTRIBUTE_FLOAT(TimeLimit1)
	/// 时间限制2
	IMPL_ATTRIBUTE_FLOAT(TimeLimit2)
	/// 时间分衰减因子，默认0.1
	IMPL_ATTRIBUTE_FLOAT(FalloffFactor)
	/// 有效得分次数
	IMPL_ATTRIBUTE_LONG(ValidTimes)
	/// 最小得分间隔，防止1秒内多次得分
	IMPL_ATTRIBUTE_FLOAT(MinScoreInterval)
	/// 该操作项的值
	IMPL_ATTRIBUTE_FLOAT(Value)
	/// 值的类型：数值类型（Number）、是否（YesNo）
	IMPL_ATTRIBUTE_STRING(ValueType)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperOperateItem)
};
