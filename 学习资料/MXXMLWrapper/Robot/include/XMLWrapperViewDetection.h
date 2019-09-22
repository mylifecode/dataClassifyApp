#pragma once
#include "XMLSerialize.h"

class CXMLWrapperViewDetection: public CXMLSerialize
{
public:
	CXMLWrapperViewDetection(void);
	~CXMLWrapperViewDetection(void);

	IMPL_ATTRIBUTE_LONG(Id)
	IMPL_ATTRIBUTE_STRING(Type)
	IMPL_ATTRIBUTE_FLOAT3(Position)
	IMPL_ATTRIBUTE_BOOL(IsDetectDist)
	IMPL_ATTRIBUTE_BOOL(IsDetectDir)
	IMPL_ATTRIBUTE_FLOAT(DetectDist)
	IMPL_ATTRIBUTE_FLOAT(MinCos)
	IMPL_ATTRIBUTE_BOOL(IsDebug)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperViewDetection)
};

