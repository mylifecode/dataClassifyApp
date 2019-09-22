/**Author:zx**/
#pragma once
#include "XMLSerialize.h"


class CXMLWrapperToolForTask : public CXMLSerialize
{
public:
	CXMLWrapperToolForTask(void);
	~CXMLWrapperToolForTask(void);

	IMPL_ATTRIBUTE_STRING(DeviceName)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperToolForTask)
};

