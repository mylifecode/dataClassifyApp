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
		/// 器官被电凝的器械触碰到
		MXET_ElecCoagStart,
		MXET_ElecCoagKeep,
		MXET_ElecCoagEnd,
		/// 器官被电切的器械触碰到
		MXET_ElecCutStart,
		MXET_ElecCutKeep,
		MXET_ElecCutEnd,
		/// 切开器官
		MXET_SliceOffOrgan,	
		MXET_Clamp,
		MXET_Release,
		MXET_SwitchTool,
		MXET_AddHemoClip,
		MXET_AddHemoClipWithinValidArea,
		MXET_AddHemoClipBeyondValidArea,
		MXET_AddSilverClip,
		/// 戳伤表面
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

		/// 一个或多个组织的pair被烧断，不记录具体的pair
		MEXT_Coaulation_DeleteLine,		
		/// 一个pair对烧断，记录pair
		MXET_VeinConnectBurned,			
		/// 一个pair被拉断，记录pair
		MXET_VeinConnectBreak,			

		MEXT_END_ALL,

		/// 产生流血
		MXET_BleedStart,	
		/// 流血停止--用于判断止血是否成功
		MXET_BleedEnd,		
		/// 该条血流被删除
		MXET_BleedRemove,	
		/// 器官被绑住
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