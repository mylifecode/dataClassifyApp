#include "StdAfx.h"
#include "XMLWrapperOnLineGrade.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperOnLineGrade)
REGISTER_CLASS(OnLineGrade)
REGISTER_ATTRIBUTE(CXMLWrapperOnLineGrade, Name, VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperOnLineGrade, Description, VALUE)

REGISTER_ATTRIBUTE(CXMLWrapperOnLineGrade, ItemCode, VALUE)

END_IMPL_SERIALIZATION_CLASS

CXMLWrapperOnLineGrade::CXMLWrapperOnLineGrade(void)
{
	INIT_ATTRIBUTE_STRING(Name)
	INIT_ATTRIBUTE_STRING(Description)
	
	INIT_ATTRIBUTE_STRING(ItemCode)	
	
}

CXMLWrapperOnLineGrade::~CXMLWrapperOnLineGrade(void)
{

}
