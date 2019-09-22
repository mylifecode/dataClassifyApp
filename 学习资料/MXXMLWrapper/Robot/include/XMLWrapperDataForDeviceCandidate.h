#pragma once
#include "XMLSerialize.h"

class CXMLWrapperDataForDeviceCandidate : public CXMLSerialize
{
public:
	CXMLWrapperDataForDeviceCandidate(void);
	~CXMLWrapperDataForDeviceCandidate(void);
	
	IMPL_ATTRIBUTE_STRING(DataCandidate)

    DECLARE_SERIALIZATION_CLASS(CXMLWrapperDataForDeviceCandidate)
};
