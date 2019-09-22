/**Author:zx**/
#include "StdAfx.h"
#include "XMLWrapperToolConfig.h"


BEGIN_IMPL_SERIALIZATION_CLASS(CXMLWrapperToolConfig)
	REGISTER_CLASS(ToolConfig)
	REGISTER_ATTRIBUTE(CXMLWrapperToolConfig,Configs,ARRAY)
END_IMPL_SERIALIZATION_CLASS

CXMLWrapperToolConfig::CXMLWrapperToolConfig(void)
{
	m_flag_Configs = false;
}


CXMLWrapperToolConfig::~CXMLWrapperToolConfig(void)
{
}

void __stdcall CXMLWrapperToolConfig::Set_Configs(Variant value)
{
	if (value.vt != SYVT_I4 && value.lVal != 0)
	{
		return;	
	}
	
	m_Configs.push_back((CXMLWrapperTool *)value.lVal);

	m_flag_Configs = true;
}

void __stdcall CXMLWrapperToolConfig::Get_Configs(Variant * pValue)
{

}