#include "ThinMenbraneForceps.h"
#include "XMLWrapperTool.h"
#include "XMLWrapperOrgan.h"
#include "BasicTraining.h"
#include "EffectManager.h"
#include "MisRobotInput.h"
#include "MXDebugInf.h"
#include "MXEvent.h"
#include "MXEventsDump.h"

#include "XMLWrapperPart.h"

CThinMenbraneForceps::CThinMenbraneForceps()
{

}

CThinMenbraneForceps::CThinMenbraneForceps(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig)
{

}

CThinMenbraneForceps::~CThinMenbraneForceps()
{

}


bool CThinMenbraneForceps::Update(float dt)
{
	__super::Update(dt);

	return true;
}
