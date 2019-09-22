/**Author:zx**/
#include "BigHemoClip.h"
#include "XMLWrapperTool.h"
#include "InputSystem.h"
CBigHemoClip::CBigHemoClip() : Clip()
{

}

CBigHemoClip::CBigHemoClip(CXMLWrapperTool * pToolConfig) : Clip(pToolConfig)
{

}

CBigHemoClip::~CBigHemoClip()
{

}

void CBigHemoClip::Updates2m()
{
#if defined(MODELSHOW)
	bool showModel=InputSystem::GetInstance(DEVICETYPE_CAMERA_LARGE)->GetShowModel();
	if (showModel)
	{
		Draws2m();
	}
	else
	{
		if (m_pManualObj)
		{
			m_pManualObj->clear();
		}

		if (m_pToolConfig->m_CanDrawS2M)
		{
			Draws2m();
		}
	}
#endif
}