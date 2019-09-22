/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperLiquid : public CXMLSerialize
{
public:
	CXMLWrapperLiquid(void);
	~CXMLWrapperLiquid(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(DtkIndexsAsPos)
	IMPL_ATTRIBUTE_STRING(NameInConfig)
	IMPL_ATTRIBUTE_STRING(TypeInConfig)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperLiquid)
};
