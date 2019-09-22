/**Author:zx**/
#pragma once
#include "XMLSerialize.h"

class CXMLWrapperSceneNode : public CXMLSerialize
{
public:
	CXMLWrapperSceneNode(void);
	~CXMLWrapperSceneNode(void);

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_STRING(Path)
	IMPL_ATTRIBUTE_STRING(DataDir)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperSceneNode)
};

