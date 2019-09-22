#pragma once
#include "MxToolEvent.h"

class MisMedicOrganInterface;

class MxSliceOffOrganEvent : public MxToolEvent
{
public:
	MxSliceOffOrganEvent();
	MxSliceOffOrganEvent(MisMedicOrganInterface * pOrgan,const GFPhysVector3& tearPoint);

	~MxSliceOffOrganEvent(void);

	inline const GFPhysVector3& GetSlicePoint() {return m_tearPoint;}
	inline void SetSlicePoint(const GFPhysVector3& tearPoint) {m_tearPoint = tearPoint;}
private:
	GFPhysVector3 m_tearPoint;
};
