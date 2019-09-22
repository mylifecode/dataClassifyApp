#include "StdAfx.h"
#include "XMLWrapperDataForDeviceCandidate.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperDataForDeviceCandidate)
	REGISTER_CLASS(DataForDeviceCandidate)
	REGISTER_ATTRIBUTE(CXMLWrapperDataForDeviceCandidate,DataCandidate,VALUE)	
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperDataForDeviceCandidate::CXMLWrapperDataForDeviceCandidate(void)
{	
	INIT_ATTRIBUTE_STRING(DataCandidate)
}

CXMLWrapperDataForDeviceCandidate::~CXMLWrapperDataForDeviceCandidate(void)
{
}
