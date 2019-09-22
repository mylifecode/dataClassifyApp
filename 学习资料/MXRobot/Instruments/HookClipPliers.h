#pragma once
#include "Forceps.h"

class CXMLWrapperTool;
class CHookClipPliers :public CForceps
{
public:
	CHookClipPliers();
	CHookClipPliers(CXMLWrapperTool * pToolConfig);
	virtual ~CHookClipPliers(void);
	
	bool Update(float dt);

protected:


};