/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperShadow.h"


BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperShadow)
REGISTER_CLASS(Shadow)
REGISTER_ATTRIBUTE(CXMLWrapperShadow,Name,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperShadow,ShadowType,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperShadow,CanShadow,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperShadow,ShadowColor,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperShadow,ShadowPos,VALUE)//relative to camera
REGISTER_ATTRIBUTE(CXMLWrapperShadow,ShadowDirection,VALUE)//relative to camera
REGISTER_ATTRIBUTE(CXMLWrapperShadow,ShadowFarDistance,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperShadow,ShadowDirectionalLEDistance,VALUE)
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperShadow::CXMLWrapperShadow(void)
{
	INIT_ATTRIBUTE_STRING(Name)
		INIT_ATTRIBUTE_LONG(ShadowType)
		INIT_ATTRIBUTE_BOOL_VALUE(CanShadow,true)
		INIT_ATTRIBUTE_FLOAT3(ShadowColor)
		INIT_ATTRIBUTE_FLOAT3(ShadowPos)
		INIT_ATTRIBUTE_FLOAT3(ShadowDirection)
		INIT_ATTRIBUTE_FLOAT_VALUE(ShadowFarDistance,10000)
		INIT_ATTRIBUTE_FLOAT_VALUE(ShadowDirectionalLEDistance,10000)
}


CXMLWrapperShadow::~CXMLWrapperShadow(void)
{
}
