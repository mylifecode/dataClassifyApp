#include "Cautery.h"
#include "XMLWrapperTool.h"
#include "BasicTraining.h"
//#include "DynamicObject.h"
#include "EffectManager.h"
#include "MXEventsDump.h"
#include "MXEvent.h"

CCautery::CCautery()
{
	
}

CCautery::CCautery(CXMLWrapperTool * pToolConfig) : CElectricTool(pToolConfig)
{

}

CCautery::~CCautery()
{

}

bool CCautery::Update(float dt)
{
	__super::Update(dt);

	return true;
}