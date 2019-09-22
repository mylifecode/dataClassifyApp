#pragma once
#include "XMLSerialize.h"

class CXMLWrapperScore : public CXMLSerialize
{
public:
	CXMLWrapperScore(void);
	~CXMLWrapperScore(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Description)
	IMPL_ATTRIBUTE_FLOAT(Score)
	IMPL_ATTRIBUTE_FLOAT(TimeScore)
	IMPL_ATTRIBUTE_FLOAT(TimeLimit)
	IMPL_ATTRIBUTE_FLOAT(ExtendTimeScore)
	IMPL_ATTRIBUTE_FLOAT(ExtendTimeLimit)
	IMPL_ATTRIBUTE_STRING(Condition)
	IMPL_ATTRIBUTE_LONG(CustomPosX)
	IMPL_ATTRIBUTE_LONG(CustomPosY)
	IMPL_ATTRIBUTE_LONG(ValidTimes)
	IMPL_ATTRIBUTE_LONG(LastSeconds)
	IMPL_ATTRIBUTE_FLOAT(ValidTimeInterval)			//得分有效时间间隔

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperScore)
};
