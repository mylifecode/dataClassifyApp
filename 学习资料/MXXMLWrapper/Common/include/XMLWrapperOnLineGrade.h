#pragma once
#include "XMLSerialize.h"

class CXMLWrapperOnLineGrade : public CXMLSerialize
{
public:
	CXMLWrapperOnLineGrade(void);
	~CXMLWrapperOnLineGrade(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Description)

	IMPL_ATTRIBUTE_STRING(ItemCode)
	
	DECLARE_SERIALIZATION_CLASS(CXMLWrapperOnLineGrade)
};
