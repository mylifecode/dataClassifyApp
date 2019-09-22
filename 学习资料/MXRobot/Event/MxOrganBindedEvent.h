#pragma once
#include "MxToolEvent.h"

class MisMedicOrgan_Ordinary;
class MisMedicBindedRope;

class MxOrganBindedEvent : public MxToolEvent
{
public:
	MxOrganBindedEvent();
	~MxOrganBindedEvent(void);

	void SetRope(MisMedicBindedRope * pRope) {m_pRope = pRope;}
	MisMedicBindedRope* GetRope() {return m_pRope;}

private:
	MisMedicBindedRope* m_pRope;
};
