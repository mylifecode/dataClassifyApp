#include "LosslessForceps.h"
#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "BasicTraining.h"
#include "EffectManager.h"

CLosslessForceps::CLosslessForceps()
{
	
}

CLosslessForceps::CLosslessForceps(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig)
{
	
}

CLosslessForceps::~CLosslessForceps()
{

}

bool CLosslessForceps::Update(float dt)
{
	__super::Update(dt);

	return true;
}