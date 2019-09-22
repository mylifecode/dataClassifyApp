#pragma once
#include "Clip.h"

class CXMLWrapperTool;
class CHookClip : public Clip
{
public:
	CHookClip();
	CHookClip(CXMLWrapperTool * pToolConfig);
	virtual ~CHookClip(void);
};