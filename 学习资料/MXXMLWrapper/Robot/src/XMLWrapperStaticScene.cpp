/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperStaticScene.h"

BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperStaticScene)
	REGISTER_CLASS(StaticScene)
	REGISTER_ATTRIBUTE(CXMLWrapperStaticScene,Mesh,ARRAY)
	REGISTER_ATTRIBUTE(CXMLWrapperStaticScene,Scene,ARRAY)
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperStaticScene::CXMLWrapperStaticScene(void)
{
	m_flag_Scene = false;
	m_flag_Mesh = false;
}


CXMLWrapperStaticScene::~CXMLWrapperStaticScene(void)
{
}

void __stdcall CXMLWrapperStaticScene::Set_Mesh(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}
	
	m_Mesh.push_back((CXMLWrapperMeshNode *)value.lVal);
	m_flag_Mesh = true;
}

void __stdcall CXMLWrapperStaticScene::Get_Mesh(Variant * pValue)
{

}

void __stdcall CXMLWrapperStaticScene::Set_Scene(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}
	
	m_Scene.push_back((CXMLWrapperSceneNode *)value.lVal);
	m_flag_Scene = true;
}

void __stdcall CXMLWrapperStaticScene::Get_Scene(Variant * pValue)
{

}