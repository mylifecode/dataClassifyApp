#include "StdAfx.h"
#include "XMLWrapperViewDetection.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperViewDetection)
	REGISTER_CLASS(ViewDetection)
	REGISTER_ATTRIBUTE(CXMLWrapperViewDetection,Id,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperViewDetection,Position,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperViewDetection,IsDetectDist,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperViewDetection,IsDetectDir,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperViewDetection,DetectDist,VALUE)

	REGISTER_ATTRIBUTE(CXMLWrapperViewDetection,MinCos,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperViewDetection,IsDebug,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperViewDetection,Type,VALUE)
END_IMPL_SERIALIZATION_CLASS


CXMLWrapperViewDetection::CXMLWrapperViewDetection( void )
{
	INIT_ATTRIBUTE_LONG(Id)
	INIT_ATTRIBUTE_STRING_VALUE(Type,"Explore")
	INIT_ATTRIBUTE_FLOAT3(Position)
	INIT_ATTRIBUTE_BOOL(IsDetectDist)
	INIT_ATTRIBUTE_BOOL(IsDetectDir)
	INIT_ATTRIBUTE_FLOAT_VALUE(DetectDist, 1.0)
	INIT_ATTRIBUTE_FLOAT_VALUE(MinCos,0.0)
	INIT_ATTRIBUTE_BOOL(IsDebug)
}

CXMLWrapperViewDetection::~CXMLWrapperViewDetection( void )
{

}