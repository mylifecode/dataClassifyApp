#pragma once
#include "xmlserialize.h"

class CXMLWrapperDetector : public CXMLSerialize
{
public:
	CXMLWrapperDetector(void);
	~CXMLWrapperDetector(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_LONG(Index)
	IMPL_ATTRIBUTE_STRING(MaterialName)
	IMPL_ATTRIBUTE_STRING(MeshType)
	IMPL_ATTRIBUTE_STRING(Container)
	IMPL_ATTRIBUTE_BOOL(Show)
	IMPL_ATTRIBUTE_FLOAT3(InitPos)
	IMPL_ATTRIBUTE_FLOAT3(InitRotate)
	IMPL_ATTRIBUTE_FLOAT3(InitSize)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperDetector)
};
