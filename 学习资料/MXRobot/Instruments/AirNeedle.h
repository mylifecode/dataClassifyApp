/**Author:zx**/
#pragma once
#include "Needle.h"

//pneumoperitoneum
class CXMLWrapperTool;
class CAirNeedle : public CNeedle
{
public:
	CAirNeedle();
	CAirNeedle(CXMLWrapperTool * pToolConfig);
	virtual ~CAirNeedle(void);
};