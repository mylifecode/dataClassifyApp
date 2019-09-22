#include "MXEvent.h"
#include "MxEventsDump.h"
#include "MxSliceOffOrganEvent.h"
#include "MxOrganBindedEvent.h"

static inline void AddEvent(MxEvent* pEvent)
{
	if(pEvent)
		CMXEventsDump::Instance()->PushEvent(pEvent);
}

MxEvent* MxEvent::CreateEvent(EventType eventType)
{
	MxEvent* pEvent = NULL;
	
	switch(eventType)
	{
	case MXET_SliceOffOrgan:
		pEvent = new MxSliceOffOrganEvent();
		break;
	case MXET_OrganBinded:
		pEvent = new MxOrganBindedEvent();
		break;
	}

	AddEvent(pEvent);
	return pEvent;
}