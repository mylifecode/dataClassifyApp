#include "StdAfx.h"
#include "XMLWrapperAdhesion.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperAdhesionCluster)
	REGISTER_CLASS(AdhesionCluster)
	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,AdhesionID,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,Range,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,Ratio,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,OrganIDs,VALUE)

	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,IsScale,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,IsAutoDir,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,ScaleFactor,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,ConstraintStiffness,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,NodeIndicesA,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperAdhesionCluster,NodeIndicesB,VALUE)
END_IMPL_SERIALIZATION_CLASS


CXMLWrapperAdhesionCluster::CXMLWrapperAdhesionCluster( void )
{
	INIT_ATTRIBUTE_LONG(AdhesionID)
	INIT_ATTRIBUTE_FLOAT_VALUE(Range,0.05)
	INIT_ATTRIBUTE_FLOAT_VALUE(Ratio, 1)
	INIT_ATTRIBUTE_STRING(OrganIDs)
	
	INIT_ATTRIBUTE_BOOL_VALUE(IsScale , false)
	INIT_ATTRIBUTE_BOOL_VALUE(IsAutoDir , false)
	INIT_ATTRIBUTE_FLOAT_VALUE(ScaleFactor , 0.5)
	INIT_ATTRIBUTE_FLOAT_VALUE(ConstraintStiffness,0.99)
	INIT_ATTRIBUTE_STRING(NodeIndicesA)
	INIT_ATTRIBUTE_STRING(NodeIndicesB)
}

CXMLWrapperAdhesionCluster::~CXMLWrapperAdhesionCluster( void )
{

}