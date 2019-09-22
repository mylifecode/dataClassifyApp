/**Author:zx**/
#pragma once
#include "IMXDefine.h"
#include "math/GoPhysVector3.h"

using namespace GoPhys;

class MxSliceOffOrganEvent;
class MisMedicOrganInterface;

class MxEvent
{
public:	
	enum EventType
	{
		MXET_Cut,
		MXET_CutAreaError,
		MXET_CutThread,
		MXET_Collision,
		MXET_Fire,
        MXET_FireFailed,
		MXET_FirePositionError,
		/// ���ٱ���������е������
		MXET_ElecCoagStart,
		MXET_ElecCoagKeep,
		MXET_ElecCoagEnd,
		/// ���ٱ����е���е������
		MXET_ElecCutStart,
		MXET_ElecCutKeep,
		MXET_ElecCutEnd,
		/// �п�����
		MXET_SliceOffOrgan,	
		MXET_Clamp,
		MXET_Release,
		MXET_SwitchTool,
		MXET_AddHemoClip,
		MXET_AddHemoClipWithinValidArea,
		MXET_AddHemoClipBeyondValidArea,
		MXET_AddSilverClip,
		/// ���˱���
		MXET_PunctureSurface,	
		MXET_StartElectricHollow,
		MXET_TrainingComplete,
		MXET_TrainingFailed,		
		MXET_AddCutWithinValidArea,
		MXET_AddCoagulationComplete,
		MXET_AddHookClip,
		MXET_HookClipDirectionError,
		MXET_HookClipDirectionOK,

		MXET_TakeOutOrganWithSpecimenBag,

		MXET_BurnCutFaceBegin,
		MXET_BurnCutFaceEnd,

		/// OnNephrectomyEvent
		MXET_END_MOVIE,
		MXET_END_LEFT_TRAINING,
		MXET_END_RIGHT_TRAINING,
		MEXT_END_REAL_TRAINING,

		MXET_Coagulation_Hooked,
		MXET_Coagulation_Hooke_End,

		/// һ��������֯��pair���նϣ�����¼�����pair
		MEXT_Coaulation_DeleteLine,		
		/// һ��pair���նϣ���¼pair
		MXET_VeinConnectBurned,			
		/// һ��pair�����ϣ���¼pair
		MXET_VeinConnectBreak,			

		MEXT_END_ALL,

		/// ������Ѫ
		MXET_BleedStart,	
		/// ��Ѫֹͣ--�����ж�ֹѪ�Ƿ�ɹ�
		MXET_BleedEnd,		
		/// ����Ѫ����ɾ��
		MXET_BleedRemove,	
		/// ���ٱ���ס
		MXET_OrganBinded,

		MXET_LIVER_DAMAGE,
	};

public:

	static MxEvent* CreateEvent(EventType eventType);

public:
	MxEvent(MxEvent::EventType enmEventType)
	{
		this->m_enmEventType = enmEventType;
	}
	~MxEvent()
	{

	}
	EventType m_enmEventType;
};