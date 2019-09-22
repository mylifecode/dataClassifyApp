/**Author:zx**/
#pragma once
#include "IMXDefine.h"
#include "MXEvent.h"
#include "Singleton.h"
#include "ITool.h"
#include "MXToolEvent.h"

class ITraining;

typedef void (* HandleEventCB)(MxEvent *, ITraining * pTraining);

class CMXEventsDump : public CSingleT<CMXEventsDump> 
{
public:
	CMXEventsDump();
	~CMXEventsDump();
	
	MxToolEvent * CreateEventNew(MxEvent::EventType enmEventType, ITool * pTool, void * pUserPoint);
	
	void PushEvent(MxEvent * pEvent , bool immediately = false);
	
	void DumpEvents();
	void ClearEvent();
	//1:通过注册自定义的事件处理函数来处理事件
	//bHandleOldEvents:在注册新的处理函数时，如果还保留有旧的事件，该参数决定了是否处理处理这些函数，默认操作为清空
	void RegisterHandleEventsFunc(HandleEventCB pFuncHandleEvent, ITraining * pTraining,bool bHandleOldEvents = false);
	
	//2:用ITraining类的HandleEvent函数来处理事件
	void RegisterHandleEventsFunc(ITraining * pTraining,bool bHandleOldEvents = false);

	//删除先前注册的事件处理函数	
	void UnRegisterHandleEventsFunc();

private:
	//内部事件处理函数
	void DumpEvent(MxEvent * pEvent);

	std::queue<MxEvent *> m_queEvents;


	HandleEventCB m_pFuncHandleEvent;
	ITraining * m_pCurTraining;

	//事件处理函数的类型
	enum EventHandleType
	{
		Ignore,				//没有事件处理函数(默认处理方式)
		UseTraining,		//用ITraining类的HandelEvent函数来处理
		Customed			//使用自定义的函数处理事件
	};

	EventHandleType m_eventHandleType;
};