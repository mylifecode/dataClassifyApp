/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperShadow: public CXMLSerialize
{
public:
	CXMLWrapperShadow(void);
	~CXMLWrapperShadow(void);

	IMPL_ATTRIBUTE_STRING(Name)
		IMPL_ATTRIBUTE_LONG(ShadowType)
		IMPL_ATTRIBUTE_BOOL(CanShadow)
		IMPL_ATTRIBUTE_FLOAT3(ShadowColor)
		IMPL_ATTRIBUTE_FLOAT3(ShadowPos)//relative to light node
		IMPL_ATTRIBUTE_FLOAT3(ShadowDirection)//relative to light node
		IMPL_ATTRIBUTE_FLOAT(ShadowFarDistance)
		IMPL_ATTRIBUTE_FLOAT(ShadowDirectionalLEDistance)

		DECLARE_SERIALIZATION_CLASS(CXMLWrapperShadow)
};

