#ifndef __CS_STRIGHTMINISCISSORS_
#define __CS_STRIGHTMINISCISSORS_

#include "Scissors.h"

class CXMLWrapperTool;
class CStrightMiniScissors:public CStraightScissors
{
public:
	CStrightMiniScissors();
	CStrightMiniScissors(CXMLWrapperTool * pToolConfig);
	~CStrightMiniScissors();

	bool Initialize(CXMLWrapperTraining * pTraining);
};

#endif