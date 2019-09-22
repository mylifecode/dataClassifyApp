/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperSphere: public CXMLSerialize
{
public:
	CXMLWrapperSphere(void);
	~CXMLWrapperSphere(void);

	IMPL_ATTRIBUTE_STRING(Name)
		IMPL_ATTRIBUTE_LONG(Type)
		IMPL_ATTRIBUTE_FLOAT3(Center)
		IMPL_ATTRIBUTE_FLOAT(InnerRadius)
		IMPL_ATTRIBUTE_FLOAT(OuterRadius)
		IMPL_ATTRIBUTE_FLOAT(YAxisOffset)

		DECLARE_SERIALIZATION_CLASS(CXMLWrapperSphere)
};

