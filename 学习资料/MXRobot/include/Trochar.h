/**Author:zx**/
#pragma once
#include "Needle.h"

//pneumoperitoneum
class CXMLWrapperTool;
class CTrochar : public CNeedle
{
public:
	CTrochar();
	CTrochar(CXMLWrapperTool * pToolConfig);
	virtual ~CTrochar(void);
};