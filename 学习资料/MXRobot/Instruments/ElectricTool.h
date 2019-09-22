/**Author:zx**/
#pragma once
#include "tool.h"

class CXMLWrapperTool;
class CElectricTool : public CTool
{
public:
	CElectricTool();
	CElectricTool(CXMLWrapperTool * pToolConfig);
	virtual ~CElectricTool(void);
	virtual bool CanExpandHole()
	{
		return false;
	}
};