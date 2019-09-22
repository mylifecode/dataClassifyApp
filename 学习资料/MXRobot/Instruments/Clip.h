#pragma once
#include "tool.h"
#include "ToolsMgr.h"

class CXMLWrapperTool;
class Clip : public CTool
{
public:
	Clip();
	Clip(CXMLWrapperTool * pToolConfig);
	virtual ~Clip(void);

	virtual bool Initialize(CXMLWrapperTraining * pTraining);

protected:
	virtual void CreateCollisionWithTools();
};