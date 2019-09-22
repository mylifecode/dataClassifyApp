/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperMeshNode.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperMeshNode)
	REGISTER_CLASS(MeshNode)
	REGISTER_ATTRIBUTE(CXMLWrapperMeshNode,Name,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperMeshNode,Mesh,VALUE)
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperMeshNode::CXMLWrapperMeshNode(void)
{
	INIT_ATTRIBUTE_STRING(Name)
	INIT_ATTRIBUTE_STRING(Mesh)
}


CXMLWrapperMeshNode::~CXMLWrapperMeshNode(void)
{
}
