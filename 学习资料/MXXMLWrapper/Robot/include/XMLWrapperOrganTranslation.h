#pragma once
#include "XMLSerialize.h"
/*
Example:
<OrganTranslations>
	<OrganTranslation OrganID="3" Offset="0,5,0"></OrganTranslation>
</OrganTranslations>
*/
class CXMLWrapperOrganTranslation : public CXMLSerialize
{
public:
	CXMLWrapperOrganTranslation(void);
	~CXMLWrapperOrganTranslation(void);

	IMPL_ATTRIBUTE_LONG(OrganID)
	IMPL_ATTRIBUTE_FLOAT3(Offset)

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperOrganTranslation)
};

