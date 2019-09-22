/**Author:zx**/
#include "HemoClip.h"
#include "XMLWrapperTool.h"
#include "InputSystem.h"

CHemoClip::CHemoClip() : Clip()
{

}

CHemoClip::CHemoClip(CXMLWrapperTool * pToolConfig) : Clip(pToolConfig)
{

}

CHemoClip::~CHemoClip()
{

}

void CHemoClip::Updates2m()
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