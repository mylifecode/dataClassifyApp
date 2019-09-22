#pragma once
#include "XMLSerialize.h"

class CXMLWrapperConnect : public CXMLSerialize
{
public:
	CXMLWrapperConnect(void);
	~CXMLWrapperConnect(void);

	IMPL_ATTRIBUTE_LONG(Object1ID)
	IMPL_ATTRIBUTE_LONG(Object2ID)
	IMPL_ATTRIBUTE_FLOAT(Range)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperConnect)
};

