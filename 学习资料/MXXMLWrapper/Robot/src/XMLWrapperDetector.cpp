#include "XMLWrapperDetector.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperDetector)
	REGISTER_CLASS(Detector)
	REGISTER_ATTRIBUTE(CXMLWrapperDetector,Name,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperDetector,Index,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperDetector,MaterialName,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperDetector,MeshType,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperDetector,Show,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperDetector,Container,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperDetector,InitPos,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperDetector,InitRotate,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperDetector,InitSize,VALUE)
END_IMPL_SERIALIZATION_CLASS


CXMLWrapperDetector::CXMLWrapperDetector(void)
{

	INIT_ATTRIBUTE_STRING(Name)
	INIT_ATTRIBUTE_LONG(Index)
	INIT_ATTRIBUTE_STRING(MaterialName)
	INIT_ATTRIBUTE_STRING(Container)
	INIT_ATTRIBUTE_STRING(MeshType)
	INIT_ATTRIBUTE_BOOL(Show)
	INIT_ATTRIBUTE_FLOAT3(InitPos)
	INIT_ATTRIBUTE_FLOAT3(InitRotate)
	INIT_ATTRIBUTE_FLOAT3(InitSize)

}

CXMLWrapperDetector::~CXMLWrapperDetector(void)
{
}
