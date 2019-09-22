#pragma once
#include "Forceps.h"

class CXMLWrapperTool;
class CLosslessForceps : public CForceps
{
public:
	CLosslessForceps();
	CLosslessForceps(CXMLWrapperTool * pToolConfig);
	virtual ~CLosslessForceps(void);

	virtual bool Update(float dt);
	
private:
	
};