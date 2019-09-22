#include "StdAfx.h"
#include "XMLWrapperToolPlace.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperToolPlace)
	REGISTER_CLASS(ToolPlace)
	REGISTER_ATTRIBUTE(CXMLWrapperToolPlace,Name,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperToolPlace,Type,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperToolPlace,LeftToolPos,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperToolPlace,LeftToolOrientation,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperToolPlace,RightToolPos,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperToolPlace,RightToolOrientation,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperToolPlace,DebugShow,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperToolPlace,HardwareConfigXML,VALUE)
		
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperToolPlace::CXMLWrapperToolPlace(void)
{
	INIT_ATTRIBUTE_STRING(Name)
	INIT_ATTRIBUTE_STRING(Type)
	INIT_ATTRIBUTE_FLOAT3(LeftToolPos)
	INIT_ATTRIBUTE_BOOL(DebugShow,false)
	INIT_ATTRIBUTE_QUATERNION(LeftToolOrientation)
		
	INIT_ATTRIBUTE_FLOAT3(RightToolPos)
	INIT_ATTRIBUTE_QUATERNION(RightToolOrientation)
	INIT_ATTRIBUTE_STRING(HardwareConfigXML)
}

CXMLWrapperToolPlace::~CXMLWrapperToolPlace(void)
{
}
