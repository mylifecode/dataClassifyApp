#include "StraightMiniScissors.h"
#include "XMLWrapperTool.h"

CStrightMiniScissors::CStrightMiniScissors()
{

}

CStrightMiniScissors::CStrightMiniScissors(CXMLWrapperTool * pToolConfig):CStraightScissors(pToolConfig)
{

}

CStrightMiniScissors::~CStrightMiniScissors()
{

}

bool CStrightMiniScissors::Initialize( CXMLWrapperTraining * pTraining )
{
	return CTool::Initialize(pTraining);
}
