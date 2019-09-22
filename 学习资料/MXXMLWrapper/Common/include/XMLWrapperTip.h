#pragma once
#include "XMLSerialize.h"

class CXMLWrapperTip : public CXMLSerialize
{
public:
	CXMLWrapperTip(void);
	~CXMLWrapperTip(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Description)
	IMPL_ATTRIBUTE_STRING(IconType)
	IMPL_ATTRIBUTE_LONG(CustomPosX)
	IMPL_ATTRIBUTE_LONG(CustomPosY)
	IMPL_ATTRIBUTE_LONG(LastSeconds)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperTip)
};
