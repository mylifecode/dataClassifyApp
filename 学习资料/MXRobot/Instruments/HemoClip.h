/**Author:zx**/
#pragma once
#include "Clip.h"

class CXMLWrapperTool;
class CHemoClip : public Clip
{
public:
	CHemoClip();
	CHemoClip(CXMLWrapperTool * pToolConfig);
	virtual ~CHemoClip(void);

	void Updates2m();
};