/**Author:zx**/
#pragma once
#include "Instruments/tool.h"

class CXMLWrapperTool;
class CNeedle : public CTool
{
public:
	CNeedle();
	CNeedle(CXMLWrapperTool * pToolConfig);
	virtual ~CNeedle(void);
};