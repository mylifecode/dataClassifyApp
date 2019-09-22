#include "StdAfx.h"
#include "XMLWrapperMucous.h"

BEGIN_IMPL_SERIALIZATION_CLASS( CXMLWrapperMucous )
REGISTER_CLASS(Mucous)
REGISTER_ATTRIBUTE(CXMLWrapperMucous,Name,VALUE)
REGISTER_ATTRIBUTE(CXMLWrapperMucous,MaxLength,VALUE)
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperMucous::CXMLWrapperMucous()
{
	INIT_ATTRIBUTE_STRING(Name)
	INIT_ATTRIBUTE_FLOAT(MaxLength)
}

CXMLWrapperMucous::~CXMLWrapperMucous()
{

}