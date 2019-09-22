/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperLiquid.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperLiquid)
REGISTER_CLASS(Liquid)
REGISTER_ATTRIBUTE(CXMLWrapperLiquid,Name,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperLiquid,DtkIndexsAsPos,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperLiquid,NameInConfig,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperLiquid,TypeInConfig,VALUE)

END_IMPL_SERIALIZATION_CLASS

CXMLWrapperLiquid::CXMLWrapperLiquid(void)
{
	INIT_ATTRIBUTE_STRING(Name)
	INIT_ATTRIBUTE_STRING(DtkIndexsAsPos)
	INIT_ATTRIBUTE_STRING(NameInConfig)
	INIT_ATTRIBUTE_STRING(TypeInConfig)
}

CXMLWrapperLiquid::~CXMLWrapperLiquid(void)
{

}
