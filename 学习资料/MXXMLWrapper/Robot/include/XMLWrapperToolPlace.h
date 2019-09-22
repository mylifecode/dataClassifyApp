#pragma once
#include "XMLSerialize.h"

class CXMLWrapperToolPlace : public CXMLSerialize
{
public:
	CXMLWrapperToolPlace(void);
	~CXMLWrapperToolPlace(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Type)
	
	IMPL_ATTRIBUTE_FLOAT3(LeftToolPos)
	IMPL_ATTRIBUTE_QUATERNION(LeftToolOrientation)
	
	IMPL_ATTRIBUTE_FLOAT3(RightToolPos)
	IMPL_ATTRIBUTE_QUATERNION(RightToolOrientation)
	
	IMPL_ATTRIBUTE_STRING(HardwareConfigXML)
	IMPL_ATTRIBUTE_BOOL(DebugShow)
	DECLARE_SERIALIZATION_CLASS(CXMLWrapperToolPlace)
};
