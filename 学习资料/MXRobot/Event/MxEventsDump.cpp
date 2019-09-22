/**Author:zx**/
#include "MXEventsDump.h"
#include "MXToolEvent.h"
#include "ITraining.h"
#include <cassert>


CMXEventsDump::CMXEventsDump()
{
	m_queEvents.empty();
	m_pFuncHandleEvent = NULL;
	m_pCurTraining = NULL;
	m_eventHandleType = Ignore;
}

CMXEventsDump::~CMXEventsDump()
{

}

void CMXEventsDump::PushEvent(MxEvent * pEvent , bool immediately)
{
	if (pEvent)
	{
		if(immediately)
		{
			DumpEvent(pEvent);
			delete pEvent;
		}
		else
		  m_queEvents.push(pEvent);
	}
}

void CMXEventsDump::DumpEvent(MxEvent * pEvent)
{
	switch(m_eventHandleType)
	{
	case UseTraining:
		m_pCurTraining->OnHandleEvent(pEvent);
		break;
	case Customed:
		m_pFuncHandleEvent(pEvent, m_pCurTraining);
		break;
	}
}

void CMXEventsDump::DumpEvents()
{
	int nEventsCount = m_queEvents.size();
	while (nEventsCount > 0)
	{
		MxEvent * pEvent = m_queEvents.front();
		m_queEvents.pop();
		DumpEvent(pEvent);
		delete pEvent; pEvent = NULL;
		nEventsCount--;
	}
}

void CMXEventsDump::ClearEvent()
{
	int nEventsCount = m_queEvents.size();
	while (nEventsCount > 0)
	{
		MxEvent * pEvent = m_queEvents.front();
		m_queEvents.pop();
		delete pEvent;
		pEvent = NULL;
		nEventsCount--;
	}
}

void CMXEventsDump::RegisterHandleEventsFunc(HandleEventCB pFuncHandleEvent, ITraining * pTraining,bool bHandleOldEvents)
{
	assert(pFuncHandleEvent != NULL);
	if(bHandleOldEvents)
		DumpEvents();
	else
		ClearEvent();
	m_pFuncHandleEvent = pFuncHandleEvent;
	m_pCurTraining = pTraining;
	m_eventHandleType = Customed;
}

void CMXEventsDump::RegisterHandleEventsFunc(ITraining * pTraining,bool bHandleOldEvents)
{
	assert(pTraining != NULL);
	if(bHandleOldEvents)
		DumpEvents();
	else
		ClearEvent();
	m_pCurTraining = pTraining;
	m_eventHandleType = UseTraining;
}

void CMXEventsDump::UnRegisterHandleEventsFunc()
{
	ClearEvent();
	m_pFuncHandleEvent = NULL;
	m_pCurTraining = NULL;
	m_eventHandleType = Ignore;
}

MxToolEvent * CMXEventsDump::CreateEventNew(MxEvent::EventType enmEventType, ITool * pTool, void * pUserPoint)
{
	MxToolEvent * pEvent = new MxToolEvent(enmEventType, pTool, pUserPoint,0);
	return pEvent;
}