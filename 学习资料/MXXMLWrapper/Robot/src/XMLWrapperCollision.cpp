#include "StdAfx.h"
#include "XMLWrapperCollision.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperCollision)
	REGISTER_CLASS(CollisionEntity)
	REGISTER_ATTRIBUTE(CXMLWrapperCollision,Object1ID,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperCollision,Object2ID,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperCollision,Strength,VALUE)
END_IMPL_SERIALIZATION_CLASS


CXMLWrapperCollision::CXMLWrapperCollision( void )
{
	INIT_ATTRIBUTE_LONG(Object1ID)
	INIT_ATTRIBUTE_LONG(Object2ID)
	INIT_ATTRIBUTE_FLOAT(Strength)
}

CXMLWrapperCollision::~CXMLWrapperCollision( void )
{

}