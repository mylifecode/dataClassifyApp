#pragma once
#include "XMLSerialize.h"

class CXMLWrapperOperateItem : public CXMLSerialize
{
public:
	CXMLWrapperOperateItem(void);
	~CXMLWrapperOperateItem(void);

	
	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Description)
	/// ��Ҫ����ͬ����Ĳ�����֮�������,ֵԽ���Խ����
	IMPL_ATTRIBUTE_LONG(ID)
	/// ����������࣬0�������1�����������2����е�����3����ȷ�����4�����������
	IMPL_ATTRIBUTE_LONG(Category)
	/// ������
	IMPL_ATTRIBUTE_FLOAT(Score)
	/// ʱ���
	IMPL_ATTRIBUTE_FLOAT(TimeScore)
	/// ʱ������1
	IMPL_ATTRIBUTE_FLOAT(TimeLimit1)
	/// ʱ������2
	IMPL_ATTRIBUTE_FLOAT(TimeLimit2)
	/// ʱ���˥�����ӣ�Ĭ��0.1
	IMPL_ATTRIBUTE_FLOAT(FalloffFactor)
	/// ��Ч�÷ִ���
	IMPL_ATTRIBUTE_LONG(ValidTimes)
	/// ��С�÷ּ������ֹ1���ڶ�ε÷�
	IMPL_ATTRIBUTE_FLOAT(MinScoreInterval)
	/// �ò������ֵ
	IMPL_ATTRIBUTE_FLOAT(Value)
	/// ֵ�����ͣ���ֵ���ͣ�Number�����Ƿ�YesNo��
	IMPL_ATTRIBUTE_STRING(ValueType)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperOperateItem)
};
