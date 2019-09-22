#pragma once
#include "XMLSerialize.h"

class CXMLWrapperCollision : public CXMLSerialize
{
public:
	CXMLWrapperCollision(void);
	~CXMLWrapperCollision(void);

	IMPL_ATTRIBUTE_LONG(Object1ID)
	IMPL_ATTRIBUTE_LONG(Object2ID)
	IMPL_ATTRIBUTE_FLOAT(Strength)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperCollision)
};

