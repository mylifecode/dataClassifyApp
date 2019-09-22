/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperMeshNode: public CXMLSerialize
{
public:
	CXMLWrapperMeshNode(void);
	~CXMLWrapperMeshNode(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Mesh)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperMeshNode)
};

