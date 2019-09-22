/**Author:zx**/
#pragma once
#include "tool.h"

class CXMLWrapperTool;
class CStraw : public CTool
{
public:
	CStraw();
	CStraw(CXMLWrapperTool * pToolConfig);
	virtual ~CStraw(void);
};