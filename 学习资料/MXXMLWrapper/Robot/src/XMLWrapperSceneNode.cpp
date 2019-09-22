/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperSceneNode.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperSceneNode)
	REGISTER_CLASS(SceneNode)
	REGISTER_ATTRIBUTE(CXMLWrapperSceneNode,Name,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperSceneNode,Path,VALUE)
	REGISTER_ATTRIBUTE(CXMLWrapperSceneNode,DataDir,VALUE)
	
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperSceneNode::CXMLWrapperSceneNode(void)
{
	INIT_ATTRIBUTE_STRING(Name)
	INIT_ATTRIBUTE_STRING(Path)
	INIT_ATTRIBUTE_STRING(DataDir)
}


CXMLWrapperSceneNode::~CXMLWrapperSceneNode(void)
{
}
