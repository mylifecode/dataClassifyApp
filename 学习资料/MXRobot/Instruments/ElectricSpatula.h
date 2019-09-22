/**Author:zx**/
#pragma once
#include "ElectricTool.h"

class CXMLWrapperTool;
class CElectricSpatula : public CElectricTool
{
public:
	CElectricSpatula();
	CElectricSpatula(CXMLWrapperTool * pToolConfig);
	virtual ~CElectricSpatula(void);
};