/**Author:zx**/
#include "MXToolEvent.h"
#include <ITool.h>

MxToolEvent::MxToolEvent(EventType type)
:MxEvent(type)
{
	m_pTool = NULL;
	m_UserData = 0;
	m_pUserPoint = NULL;
	m_weights[0] = m_weights[1] = m_weights[2] = 0.f;
	m_pOrgan = NULL;
	m_DurationTime = 0.f;
}

MxToolEvent::MxToolEvent(EventType enmEventType, ITool * pTool) : MxEvent(enmEventType)
{
	m_pTool = pTool;

	m_pUserPoint = 0;
	m_UserData = 0;
	for(int w = 0;w < 3 ;++w)
		m_weights[w] = 0.0f;

	m_pOrgan = NULL;
	m_DurationTime = 0.f;
}

MxToolEvent::MxToolEvent(EventType enmEventType, ITool * pTool, void * pUserPoint , int UserData) : MxEvent(enmEventType)
{
	m_pTool = pTool;

	m_pUserPoint = pUserPoint;
	m_UserData = UserData;

	m_pOrgan = NULL;
	m_DurationTime = 0.f;
}

void MxToolEvent::SetWeights(const float weights[3])
{
	assert(weights);
	for(int w = 0;w < 3 ;++w)
		m_weights[w] = weights[w];
}

MxToolEvent::~MxToolEvent()
{

}
