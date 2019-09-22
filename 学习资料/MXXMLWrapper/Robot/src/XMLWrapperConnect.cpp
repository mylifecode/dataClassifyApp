#include "StdAfx.h"
#include "XMLWrapperConnect.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperConnect)
	REGISTER_CLASS(ConnectEntity)
	REGISTER_ATTRIBUTE(CXMLWrapperConnect,Object1ID,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperConnect,Object2ID,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperConnect,Range,VALUE)
END_IMPL_SERIALIZATION_CLASS


CXMLWrapperConnect::CXMLWrapperConnect( void )
{
	INIT_ATTRIBUTE_LONG(Object1ID)
	INIT_ATTRIBUTE_LONG(Object2ID)
	INIT_ATTRIBUTE_FLOAT(Range)
}

CXMLWrapperConnect::~CXMLWrapperConnect( void )
{

}