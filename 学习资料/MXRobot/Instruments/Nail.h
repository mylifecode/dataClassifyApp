#pragma once
#include "tool.h"

class CXMLWrapperTool;
class CNail : public CTool
{
public:
	CNail();
	CNail(CXMLWrapperTool * pToolConfig);
	virtual ~CNail(void);
};