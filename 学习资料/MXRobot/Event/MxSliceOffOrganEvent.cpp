#include "MXSliceOffOrganEvent.h"


MxSliceOffOrganEvent::MxSliceOffOrganEvent()
:MxToolEvent(MXET_SliceOffOrgan)
{

}

MxSliceOffOrganEvent::MxSliceOffOrganEvent(MisMedicOrganInterface * pOrgan,const GFPhysVector3& tearPoint)
:MxToolEvent(MXET_SliceOffOrgan),
m_tearPoint(tearPoint)
{
	SetOrgan(pOrgan);
}

MxSliceOffOrganEvent::~MxSliceOffOrganEvent(void)
{
}
