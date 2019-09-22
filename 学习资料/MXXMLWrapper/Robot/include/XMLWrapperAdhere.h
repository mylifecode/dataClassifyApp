#pragma once
#include "XMLSerialize.h"

class CXMLWrapperAdhere : public CXMLSerialize
{
public:
	CXMLWrapperAdhere(void);
	~CXMLWrapperAdhere(void);

	IMPL_ATTRIBUTE_LONG(Object1ID)
	IMPL_ATTRIBUTE_LONG(Object2ID)
	IMPL_ATTRIBUTE_FLOAT(Range)
	IMPL_ATTRIBUTE_FLOAT(Ratio)
	IMPL_ATTRIBUTE_STRING(MergedObjectIDS)
	IMPL_ATTRIBUTE_BOOL(HideLinkFace)
	//1:"Adhere" (default)
	//2:"Insert"  Insert Obj1 to Obj2, Only set Object1ID and Object2ID
	//3:"Approach" when the distance between Obj1's node and Obj2's node less than Range,construct constraint.so need to set Object1ID¡¢Object2ID and Range
	IMPL_ATTRIBUTE_STRING(AdhereType)		
	DECLARE_SERIALIZATION_CLASS(CXMLWrapperAdhere)
};

