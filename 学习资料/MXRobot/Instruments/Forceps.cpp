/**Author:zx**/
#include "Forceps.h"
#include "XMLWrapperTool.h"
#include "InputSystem.h"
#include "IObjDefine.h"
#include "ResourceManager.h"
#include "MXEventsDump.h"
#include "MXOgreWrapper.h"
#include "XMLWrapperOrgan.h"
#include "../Include/stdafx.h"

CForceps::CForceps()
{
	m_bClipClamp = false;
	m_bCheckClipClamp = false;
	m_fClipAccumTime = 0;
	m_fClipLastTickCount = 0;
	m_bUseClampThread = false;
	m_bCanCreateClip = true;
	m_bHasCandiPoint = false;
}

CForceps::CForceps(CXMLWrapperTool * pToolConfig) : CTool(pToolConfig)
{
	m_bClipClamp = false;
	m_bCheckClipClamp = false;
	m_fClipAccumTime = 0;
	m_fClipLastTickCount = 0;
	m_bUseClampThread = false;
	m_bCanCreateClip = true;
	m_bHasCandiPoint = false;
}

CForceps::~CForceps()
{

}	

