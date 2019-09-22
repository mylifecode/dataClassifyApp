#include "HookClipPliers.h"
#include "XMLWrapperTool.h"
#include "MXOgreWrapper.h"
#include "HookClipPliers.h"
#include "XMLWrapperTool.h"
#include "MXOgreWrapper.h"
#include "XMLWrapperOrgan.h"
#include "XMLWrapperTraining.h"
#include "ITraining.h"
#include "ResourceManager.h"
#include "EffectManager.h"
#include "BasicTraining.h"
#include "MXEventsDump.h"
#include "MXEvent.h"
#include "InputSystem.h"


CHookClipPliers::CHookClipPliers() : CForceps()
{
	m_fClipLastTickCount=0;
	m_bClipClamp=false;
	m_bCheckClipClamp=false;
}

CHookClipPliers::CHookClipPliers(CXMLWrapperTool * pToolConfig) : CForceps(pToolConfig)
{
	m_fClipLastTickCount=0;
	m_bClipClamp=false;
	m_bCheckClipClamp=false;
}

CHookClipPliers::~CHookClipPliers()
{

}

bool CHookClipPliers::Update(float dt)
{
	__super::Update(dt);

	return true;

}


