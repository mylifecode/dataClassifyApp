#pragma once
#include "XMLSerialize.h"

class CXMLWrapperMucous : public CXMLSerialize
{
public:
	CXMLWrapperMucous( void );
	~CXMLWrapperMucous( void );

	IMPL_ATTRIBUTE_STRING(Name)
	IMPL_ATTRIBUTE_FLOAT(MaxLength)
	//IMPL_ATTRIBUTE_STRING(NameInConfig)
	//IMPL_ATTRIBUTE_STRING(TypeInConfig)

	DECLARE_SERIALIZATION_CLASS( CXMLWrapperMucous )
};