#pragma once
#include "XMLSerialize.h"

class CXMLWrapperPursue : public CXMLSerialize
{
public:
	CXMLWrapperPursue(void);
	~CXMLWrapperPursue(void);

	IMPL_ATTRIBUTE_LONG(Object1ID)
		IMPL_ATTRIBUTE_LONG(Object2ID)
		IMPL_ATTRIBUTE_FLOAT(Range)
		IMPL_ATTRIBUTE_FLOAT(SlaveRatio)

		DECLARE_SERIALIZATION_CLASS(CXMLWrapperPursue)
};
