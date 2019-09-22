#include "StdAfx.h"
#include "XMLWrapperOrganTranslation.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperOrganTranslation)
	REGISTER_CLASS(OrganTranslation)
	REGISTER_ATTRIBUTE(CXMLWrapperOrganTranslation,OrganID,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperOrganTranslation,Offset,VALUE)
END_IMPL_SERIALIZATION_CLASS


CXMLWrapperOrganTranslation::CXMLWrapperOrganTranslation( void )
{
	INIT_ATTRIBUTE_LONG(OrganID)
	INIT_ATTRIBUTE_FLOAT3(Offset)
}

CXMLWrapperOrganTranslation::~CXMLWrapperOrganTranslation( void )
{

}