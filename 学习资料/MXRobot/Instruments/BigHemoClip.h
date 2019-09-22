#pragma once
#include "Clip.h"

class CXMLWrapperTool;
class CBigHemoClip : public Clip
{
public:
	CBigHemoClip();
	CBigHemoClip(CXMLWrapperTool * pToolConfig);
	virtual ~CBigHemoClip(void);

	void Updates2m();
};