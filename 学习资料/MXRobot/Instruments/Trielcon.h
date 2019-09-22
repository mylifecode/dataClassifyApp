/**Author:zx**/
#pragma once
#include "tool.h"

class CXMLWrapperTool;
class CTrielcon : public CTool
{
public:
	CTrielcon();
	CTrielcon(CXMLWrapperTool * pToolConfig);
	virtual ~CTrielcon(void);
};