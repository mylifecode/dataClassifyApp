/**Author:zx**/
#pragma once
#include "XMLWrapperTool.h"
#include "XMLSerialize.h"

class CXMLWrapperToolConfig : public CXMLSerialize
{
public:
	CXMLWrapperToolConfig(void);
	~CXMLWrapperToolConfig(void);

	vector<CXMLWrapperTool *> m_Configs;
	bool m_flag_Configs;

	void __stdcall Set_Configs(Variant value);
	void __stdcall Get_Configs(Variant * pValue);

	DECLARE_SERIALIZATION_CLASS(CXMLWrapperToolConfig)
};

