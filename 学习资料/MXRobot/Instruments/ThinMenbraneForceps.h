
#pragma once
#include "Forceps.h"

class CXMLWrapperTool;
class CThinMenbraneForceps : public CForceps
{
public:
	CThinMenbraneForceps();
	CThinMenbraneForceps(CXMLWrapperTool * pToolConfig);
	virtual ~CThinMenbraneForceps(void);

	virtual bool Update(float dt);
};