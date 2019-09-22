/**Author:zx**/
#pragma once
#include "MXEvent.h"
#include "collision/GoPhysCollisionlib.h"

class ITool;
class MisMedicOrganInterface;

class MxToolEvent : public MxEvent
{
public:
	MxToolEvent(EventType type);

	MxToolEvent(EventType enmEventType, ITool * pTool);

	MxToolEvent(EventType enmEventType, ITool * pTool, void * pUserPoint , int UserData);

	~MxToolEvent();

	void SetWeights(const float weights[3]);

	inline void SetOrgan(MisMedicOrganInterface * pOrgan) {m_pOrgan = pOrgan;}
	inline MisMedicOrganInterface* GetOrgan() { return m_pOrgan;}

	ITool * m_pTool;

	void * m_pUserPoint;
	int m_UserData;
	float m_weights[3];

	std::vector<GoPhys::GFPhysSoftBodyTetrahedron*> m_OperatorTetras;
	std::vector<GoPhys::GFPhysSoftBodyFace*> m_OperatorFaces;
	float m_DurationTime;

protected:
	MisMedicOrganInterface * m_pOrgan;
};