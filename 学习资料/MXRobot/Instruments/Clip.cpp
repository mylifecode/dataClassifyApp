#include "Clip.h"
#include "XMLWrapperTool.h"

Clip::Clip() : CTool()
{

}

Clip::Clip(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig)
{

}

Clip::~Clip()
{

}

bool Clip::Initialize(CXMLWrapperTraining * pTraining)
{
	m_pTrainingConfig = pTraining;
	
	SetOriginalInfo(CTool::TC_KERNEL);
	SetOriginalInfo(CTool::TC_LEFT);
	SetOriginalInfo(CTool::TC_RIGHT);

	return true;
}

void Clip::CreateCollisionWithTools()
{
	if ( GetOwnerTraining() == NULL )
	{
		return;
	}
}